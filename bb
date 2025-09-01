# 在Spring Boot中实现基于用户权限组的缓存

在Spring Boot中，你可以通过自定义缓存配置来实现基于用户权限组的缓存机制，而无需借助外部中间件。下面是一个完整的实现方案：

## 实现思路

1. 自定义缓存键生成器，将查询条件和用户权限组信息组合成缓存键
2. 使用Spring的ConcurrentMapCacheManager作为缓存管理器
3. 实现带有过期时间的缓存机制
4. 在需要缓存的方法上使用@Cacheable注解

## 代码实现

### 1. 创建自定义缓存键生成器

```java
import org.springframework.cache.interceptor.KeyGenerator;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;

import java.lang.reflect.Method;
import java.util.Collection;
import java.util.StringJoiner;

@Component("permissionBasedKeyGenerator")
public class PermissionBasedKeyGenerator implements KeyGenerator {

    @Override
    public Object generate(Object target, Method method, Object... params) {
        // 获取当前用户的权限信息
        String permissionKey = getCurrentUserPermissionKey();
        
        // 组合查询条件和权限信息生成缓存键
        StringJoiner keyJoiner = new StringJoiner(":");
        keyJoiner.add(permissionKey);
        
        // 添加方法名
        keyJoiner.add(target.getClass().getSimpleName() + "." + method.getName());
        
        // 添加参数信息
        if (params != null) {
            for (Object param : params) {
                if (param != null) {
                    keyJoiner.add(param.toString());
                }
            }
        }
        
        return keyJoiner.toString();
    }
    
    private String getCurrentUserPermissionKey() {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        if (authentication == null || !authentication.isAuthenticated()) {
            return "anonymous";
        }
        
        // 获取用户权限并生成权限键
        Collection<? extends GrantedAuthority> authorities = authentication.getAuthorities();
        StringJoiner permissionJoiner = new StringJoiner(",");
        for (GrantedAuthority authority : authorities) {
            permissionJoiner.add(authority.getAuthority());
        }
        
        // 使用权限的哈希值作为权限键的一部分
        return String.valueOf(permissionJoiner.toString().hashCode());
    }
}
```

### 2. 创建带过期时间的缓存管理器

```java
import org.springframework.cache.Cache;
import org.springframework.cache.CacheManager;
import org.springframework.cache.annotation.EnableCaching;
import org.springframework.cache.concurrent.ConcurrentMapCache;
import org.springframework.cache.concurrent.ConcurrentMapCacheManager;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.annotation.Scheduled;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

@Configuration
@EnableCaching
public class CacheConfig {

    // 缓存过期时间（毫秒）
    private static final long CACHE_EXPIRATION_TIME = TimeUnit.MINUTES.toMillis(30);
    
    // 存储缓存创建时间
    private final Map<String, Long> cacheCreationTimes = new ConcurrentHashMap<>();
    
    @Bean
    public CacheManager cacheManager() {
        return new ConcurrentMapCacheManager() {
            @Override
            protected Cache createConcurrentMapCache(String name) {
                // 记录缓存创建时间
                cacheCreationTimes.put(name, System.currentTimeMillis());
                return new ConcurrentMapCache(name);
            }
        };
    }
    
    // 定期清理过期缓存
    @Scheduled(fixedRate = TimeUnit.MINUTES.toMillis(5))
    public void evictExpiredCaches() {
        long currentTime = System.currentTimeMillis();
        CacheManager manager = cacheManager();
        
        for (String cacheName : manager.getCacheNames()) {
            Long creationTime = cacheCreationTimes.get(cacheName);
            if (creationTime != null && 
                currentTime - creationTime > CACHE_EXPIRATION_TIME) {
                // 清除过期缓存
                Cache cache = manager.getCache(cacheName);
                if (cache != null) {
                    cache.clear();
                }
                // 更新缓存创建时间
                cacheCreationTimes.put(cacheName, currentTime);
            }
        }
    }
}
```

### 3. 在Service中使用缓存

```java
import org.springframework.cache.annotation.Cacheable;
import org.springframework.stereotype.Service;

@Service
public class DataService {

    // 使用自定义的keyGenerator和缓存名称
    @Cacheable(value = "userDataCache", keyGenerator = "permissionBasedKeyGenerator")
    public Object getDataWithPermissions(String queryParam1, String queryParam2) {
        // 这里是模拟的数据查询逻辑
        // 实际应用中，这里会是从数据库或其他数据源获取数据的代码
        simulateSlowService();
        
        // 返回查询结果
        return "Data for query: " + queryParam1 + ", " + queryParam2 + 
               " with user permissions";
    }
    
    // 模拟耗时操作
    private void simulateSlowService() {
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
```

### 4. 控制器示例

```java
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class DataController {

    @Autowired
    private DataService dataService;
    
    @GetMapping("/data")
    public Object getData(@RequestParam String param1, @RequestParam String param2) {
        return dataService.getDataWithPermissions(param1, param2);
    }
}
```

## 配置说明

1. **缓存键生成**：`PermissionBasedKeyGenerator` 会基于当前用户的权限组和查询参数生成唯一的缓存键
2. **缓存过期**：通过定时任务每5分钟检查一次缓存，清理超过30分钟的缓存
3. **缓存使用**：在Service方法上使用`@Cacheable`注解，指定缓存名称和键生成器

## 使用注意事项

1. 确保已配置Spring Security来获取用户权限信息
2. 如果用户权限发生变化，可能需要手动清除相关缓存
3. 这种方法适用于单机应用，对于分布式环境仍需考虑分布式缓存解决方案

## 扩展功能

如果需要更精细的缓存控制，可以考虑：

1. 添加手动清除特定用户组缓存的方法
2. 实现基于LRU(最近最少使用)算法的缓存淘汰策略
3. 添加缓存统计信息监控

这个实现方案完全基于Spring Boot的内置功能，不需要任何外部缓存中间件，同时满足了基于用户权限组的缓存需求。