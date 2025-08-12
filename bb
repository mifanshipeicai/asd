在Spring Boot中，同一个Bean名称不能注册多个实例（会覆盖），但可以通过策略模式+工厂Bean实现需求。核心步骤如下：

1. 定义接口与多个实现类

public interface MyService {
    void execute();
}

@Component("serviceImpl1") // 不同实现用不同名称
public class ServiceImpl1 implements MyService {
    @Override
    public void execute() {
        System.out.println("Service 1 called");
    }
}

@Component("serviceImpl2")
public class ServiceImpl2 implements MyService {
    @Override
    public void execute() {
        System.out.println("Service 2 called");
    }
}


2. 创建随机工厂Bean

@Component
public class ServiceFactory {
    private final List<MyService> services;
    private final Random random = new Random();

    // 注入所有MyService实现
    @Autowired
    public ServiceFactory(List<MyService> services) {
        this.services = services;
    }

    // 随机获取实例
    public MyService getRandomService() {
        return services.get(random.nextInt(services.size()));
    }
}


3. 使用示例

@RestController
public class MyController {
    private final ServiceFactory serviceFactory;

    @Autowired
    public MyController(ServiceFactory serviceFactory) {
        this.serviceFactory = serviceFactory;
    }

    @GetMapping("/execute")
    public void execute() {
        // 每次调用随机获取Bean
        MyService service = serviceFactory.getRandomService();
        service.execute();
    }
}


关键说明：

1. Bean命名规则

  ◦ 各实现类使用不同名称（如@Component("serviceImpl1")）

  ◦ 不直接注册同名Bean（违反Spring规范）

2. 随机获取原理

  ◦ ServiceFactory 自动收集所有 MyService 实现

  ◦ 调用 getRandomService() 时通过随机索引获取实例

3. 性能优化

// 如需优化频繁调用，可缓存实例列表
private final MyService[] serviceArray; // 初始化时转换数组

public MyService getRandomService() {
    return serviceArray[random.nextInt(serviceArray.length)];
}


4. 动态添加实现

  ◦ 新增MyService实现类并加@Component即可自动加入随机池

替代方案（谨慎使用）

若坚持使用同名Bean（需特殊场景）：

@Configuration
public class ServiceConfig {
    @Bean(name = "myService")
    @Scope(value = WebApplicationContext.SCOPE_REQUEST, proxyMode = ScopedProxyMode.INTERFACES)
    public MyService randomService(List<MyService> services) {
        return services.get(new Random().nextInt(services.size()));
    }
}


注意：此方案在每次请求时生成新代理，可能有性能损耗和线程安全问题。

推荐使用工厂模式，符合Spring设计理念，避免潜在冲突。