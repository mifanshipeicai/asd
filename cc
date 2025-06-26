要避免在单元测试中测试数据源路由、健康检查和负载均衡逻辑，可以通过以下策略实现隔离：

### 1. 分层测试策略（推荐）
```java
// 只测试纯业务逻辑，不涉及数据源路由
@ExtendWith(MockitoExtension.class)
class DremioRepositoryTest {
    
    @Mock
    private JdbcTemplate jdbcTemplate;
    
    @InjectMocks
    private DremioRepository repository;
    
    @Test
    void queryData_shouldReturnResults() {
        // 模拟数据库响应
        when(jdbcTemplate.queryForList(anyString()))
            .thenReturn(List.of(Map.of("id", 1)));
        
        List<Map<String, Object>> result = repository.queryData();
        
        assertEquals(1, result.size());
        verify(jdbcTemplate).queryForList("SELECT * FROM my_table");
    }
}
```

### 2. 使用测试配置覆盖
```java
// src/test/java/com/example/TestConfig.java
@TestConfiguration
public class TestConfig {
    @Bean
    public DataSource dataSource() {
        // 使用单一测试数据源
        return new EmbeddedDatabaseBuilder()
            .setType(EmbeddedDatabaseType.H2)
            .addScript("classpath:schema.sql")
            .build();
    }
    
    @Bean
    @Primary  // 覆盖原数据源配置
    public DynamicDataSource dynamicDataSource() {
        // 返回无路由逻辑的简单数据源
        return new NoOpDynamicDataSource(dataSource());
    }
    
    static class NoOpDynamicDataSource extends DynamicDataSource {
        public NoOpDynamicDataSource(DataSource ds) {
            setDefaultTargetDataSource(ds);
        }
        
        @Override
        protected Object determineCurrentLookupKey() {
            return null; // 禁用路由逻辑
        }
    }
}
```

### 3. 禁用健康检查定时任务
```java
// 在测试类上禁用定时任务
@SpringBootTest
@EnableAutoConfiguration(exclude = SchedulingAutoConfiguration.class)
class ServiceLayerTest {
    
    @Autowired
    private BusinessService service;
    
    @Test
    void businessLogicTest() {
        // 测试服务层逻辑
    }
}
```

### 4. 使用Profile隔离配置
```yaml
# application-test.yml
dremio:
  datasource:
    only-for-test:
      url: jdbc:h2:mem:testdb
      driver-class-name: org.h2.Driver
```

```java
// 测试类指定test profile
@ActiveProfiles("test")
@SpringBootTest
class IntegrationTest {
    // 测试将使用H2内存数据库
}
```

### 5. 单元测试路由逻辑（独立测试）
```java
// 单独测试路由逻辑
class DynamicDataSourceTest {
    
    @Test
    void shouldRouteToHealthyDataSource() {
        DynamicDataSource ds = new DynamicDataSource();
        Map<Object, Boolean> healthMap = Map.of(
            "ds1", true,
            "ds2", false
        );
        ds.setDataSourceHealth(healthMap);
        
        Object selected = ds.determineCurrentLookupKey();
        
        assertEquals("ds1", selected);
    }
}
```

### 6. 使用MockBean禁用健康检查
```java
@SpringBootTest
class ControllerTest {
    
    @MockBean
    private DataSourceHealthChecker healthChecker; // 禁用健康检查器
    
    @Autowired
    private WebTestClient webClient;
    
    @Test
    void apiEndpointTest() {
        webClient.get("/api/data")
            .exchange()
            .expectStatus().isOk();
    }
}
```

### 最佳实践总结

1. **分层测试策略**：
   - 业务逻辑：使用`@Mock`隔离数据库访问
   - 数据访问层：使用H2内存数据库
   - 路由逻辑：单独单元测试

2. **测试配置覆盖**：
   ```java
   @TestConfiguration
   static class TestDataSourceConfig {
       @Bean
       @Primary
       public DataSource testDataSource() {
           // 返回简化数据源
       }
   }
   ```

3. **禁用非必要组件**：
   ```java
   @SpringBootTest(properties = {
       "spring.autoconfigure.exclude=" +
       "org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration"
   })
   ```

4. **使用测试Profile**：
   ```yaml
   # application-test.yml
   spring:
     datasource:
       url: jdbc:h2:mem:testdb
   ```

5. **关键组件独立测试**：
   - 单独测试`DynamicDataSource`的路由算法
   - 单独测试`DataSourceHealthChecker`的连接验证逻辑

6. **集成测试控制**：
   ```java
   @SpringBootTest(webEnvironment = WebEnvironment.RANDOM_PORT)
   @AutoConfigureMockMvc
   @DirtiesContext(classMode = AFTER_EACH_TEST_METHOD)
   class FullIntegrationTest {
       // 完整环境测试（慎用）
   }
   ```

通过以上方法，您可以：
1. 完全隔离数据源路由和健康检查逻辑
2. 避免单元测试中启动真实数据库连接
3. 保持测试快速执行（毫秒级）
4. 独立验证核心路由算法
5. 按需进行不同层次的测试

> **重要提示**：生产环境的高可用和负载均衡逻辑应通过专门的集成测试和混沌工程验证，而非放在常规单元测试中。