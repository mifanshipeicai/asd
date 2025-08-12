根据您的需求，我将提供一个完整的解决方案，使用Spring Boot创建多个WebClient实例，并实现随机选择机制。以下是逐步实现：

### 1. 配置文件（application.yml）
```yaml
custom:
  endpoints:
    - https://api.service1.com
    - https://api.service2.com
    - https://api.backup-service.com
```

### 2. 配置属性类
```java
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Configuration;
import java.util.ArrayList;
import java.util.List;

@Configuration
@ConfigurationProperties(prefix = "custom")
public class EndpointConfig {
    private List<String> endpoints = new ArrayList<>();

    // Getter 和 Setter
    public List<String> getEndpoints() {
        return endpoints;
    }

    public void setEndpoints(List<String> endpoints) {
        this.endpoints = endpoints;
    }
}
```

### 3. WebClient配置类（核心实现）
```java
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.reactive.function.client.WebClient;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ThreadLocalRandom;
import java.util.function.Function;
import java.util.stream.Collectors;

@Configuration
public class WebClientConfig {

    // 创建WebClient容器Bean
    @Bean
    public WebClientContainer webClientContainer(EndpointConfig endpointConfig) {
        return new WebClientContainer(endpointConfig.getEndpoints());
    }

    // WebClient容器类
    public static class WebClientContainer {
        private final Map<String, WebClient> clients;
        private final List<String> endpointKeys;

        public WebClientContainer(List<String> endpoints) {
            // 创建WebClient实例的并发安全Map
            this.clients = Collections.unmodifiableMap(
                endpoints.stream().collect(Collectors.toMap(
                    Function.identity(),
                    endpoint -> WebClient.builder()
                                        .baseUrl(endpoint)
                                        .build()
                ))
            );
            
            // 保存可随机访问的键列表
            this.endpointKeys = new ArrayList<>(clients.keySet());
        }

        // 随机获取一个WebClient实例
        public WebClient getRandomClient() {
            if (endpointKeys.isEmpty()) {
                throw new IllegalStateException("No endpoints configured");
            }
            int randomIndex = ThreadLocalRandom.current().nextInt(endpointKeys.size());
            return clients.get(endpointKeys.get(randomIndex));
        }

        // 获取所有WebClient实例（按需使用）
        public Map<String, WebClient> getAllClients() {
            return clients;
        }
    }
}
```

### 4. 使用示例Service
```java
import org.springframework.stereotype.Service;
import reactor.core.publisher.Mono;

@Service
public class ApiService {
    
    private final WebClientConfig.WebClientContainer webClientContainer;

    public ApiService(WebClientConfig.WebClientContainer webClientContainer) {
        this.webClientContainer = webClientContainer;
    }

    public Mono<String> fetchData(String path) {
        WebClient client = webClientContainer.getRandomClient();
        return client.get()
                   .uri(path)
                   .retrieve()
                   .bodyToMono(String.class);
    }
}
```

### 5. 工作原理说明

1. **动态创建WebClient**：
   - 根据配置文件中`custom.endpoints`的数量动态创建对应数量的WebClient实例
   - 每个WebClient使用其endpoint作为baseUrl

2. **随机选择机制**：
   - 使用`ThreadLocalRandom`实现高效线程安全的随机选择
   - 每次调用`getRandomClient()`时从所有实例中随机选取一个

3. **容器设计**：
   - `WebClientContainer`封装所有WebClient实例
   - 提供两种访问方式：
     - `getRandomClient()` 随机获取实例
     - `getAllClients()` 获取全部实例（可用于监控/管理）

### 6. 高级特性扩展

如果需要添加通用配置（如超时设置、拦截器等）：

```java
// 修改WebClient创建逻辑
WebClient.builder()
    .baseUrl(endpoint)
    .clientConnector(new ReactorClientHttpConnector(
        HttpClient.create().responseTimeout(Duration.ofSeconds(5))
    ))
    .filter((request, next) -> {
        // 添加统一请求头
        ClientRequest filtered = ClientRequest.from(request)
            .header("X-Request-ID", UUID.randomUUID().toString())
            .build();
        return next.exchange(filtered);
    })
    .build();
```

### 7. 测试用例

```java
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import static org.junit.jupiter.api.Assertions.assertNotNull;

@SpringBootTest
class WebClientConfigTest {

    @Autowired
    private WebClientConfig.WebClientContainer container;

    @Test
    void shouldReturnRandomClient() {
        WebClient client = container.getRandomClient();
        assertNotNull(client);
    }

    @Test
    void shouldContainAllEndpoints() {
        assert container.getAllClients().size() == 3;
    }
}
```

### 优势总结：
1. **动态扩展**：配置文件增减endpoint时自动调整实例数量
2. **负载均衡**：通过随机选择实现简易负载均衡
3. **线程安全**：使用`ConcurrentHashMap`和`ThreadLocalRandom`保证并发安全
4. **资源隔离**：每个WebClient实例独立配置，互不影响
5. **可扩展性**：易于添加连接池、监控等高级功能

此方案完全满足您的需求，实现了基于配置的WebClient动态创建和随机选择机制。实际部署时，您可以根据需要调整超时时间、重试策略等参数。