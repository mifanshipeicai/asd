在使用 Spring 的 `HttpClient` 和 `@HttpExchange` 时，处理服务端返回的错误状态并打印错误信息可以通过以下步骤实现：

### 方法 1：捕获 `WebClientResponseException` 异常
当服务端返回非 2xx 状态码时，`HttpClient` 会抛出 `WebClientResponseException` 异常，你可以捕获该异常并提取错误信息。

```java
import org.springframework.web.client.RestClient.*;
import org.springframework.web.client.WebClientResponseException;

public class MyServiceClient {
    private final MyServiceApi myServiceApi;

    public MyServiceClient(WebClient webClient) {
        this.myServiceApi = HttpServiceProxyFactory.builder(WebClientAdapter.forClient(webClient))
                .build()
                .createClient(MyServiceApi.class);
    }

    public void callService() {
        try {
            myServiceApi.someMethod();
        } catch (WebClientResponseException e) {
            // 打印状态码、响应头和响应体
            System.err.println("错误状态码: " + e.getStatusCode());
            System.err.println("响应头: " + e.getResponseHeaders());
            System.err.println("响应体: " + e.getResponseBodyAsString());
            // 可以抛出自定义异常或进行其他处理
            throw new RuntimeException("服务调用失败", e);
        }
    }
}

@HttpExchange(url = "/api")
interface MyServiceApi {
    @PostExchange
    void someMethod();
}
```

### 方法 2：配置全局错误处理（使用 `ExchangeFilterFunction`）
通过自定义 `ExchangeFilterFunction` 拦截所有请求/响应，统一处理错误。

```java
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.client.RestClient.*;
import org.springframework.web.reactive.function.client.ExchangeFilterFunction;
import reactor.core.publisher.Mono;

@Configuration
public class WebClientConfig {

    @Bean
    public WebClient webClient() {
        return WebClient.builder()
                .filter(logErrorFilter())
                .build();
    }

    private ExchangeFilterFunction logErrorFilter() {
        return (request, next) -> next.exchange(request)
                .flatMap(response -> {
                    if (response.statusCode().isError()) {
                        return response.bodyToMono(String.class)
                                .flatMap(body -> {
                                    // 记录错误信息
                                    System.err.println("错误状态码: " + response.statusCode());
                                    System.err.println("响应体: " + body);
                                    // 抛出包含详细信息的异常
                                    return Mono.error(new WebClientResponseException(
                                            response.statusCode().value(),
                                            "服务端错误",
                                            response.headers().asHttpHeaders(),
                                            body.getBytes(),
                                            null
                                    ));
                                });
                    }
                    return Mono.just(response);
                });
    }
}
```

### 方法 3：使用 `@ExceptionHandler`（Spring MVC 环境）
如果处于 Spring MVC 环境中，可以通过全局异常处理器统一处理。

```java
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.client.WebClientResponseException;

@RestControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(WebClientResponseException.class)
    public ResponseEntity<String> handleWebClientError(WebClientResponseException ex) {
        // 记录错误日志
        System.err.println("服务端错误: " + ex.getResponseBodyAsString());
        // 返回自定义错误信息
        return ResponseEntity.status(ex.getStatusCode())
                .body("请求失败: " + ex.getResponseBodyAsString());
    }
}
```

### 关键点总结
1. **捕获异常**：使用 `try-catch` 捕获 `WebClientResponseException`，从中提取状态码、响应头和响应体。
2. **全局过滤器**：通过 `ExchangeFilterFunction` 拦截所有错误响应，统一记录日志或转换异常。
3. **响应体处理**：使用 `getResponseBodyAsString()` 或 `bodyToMono(String.class)` 获取错误详情。
4. **自定义异常**：根据需求抛出包含详细信息的业务异常。

选择适合你的场景的方法，确保在服务端返回错误时能够清晰记录问题原因。