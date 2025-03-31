在 Amazon S3 这样的对象存储中，"目录"本质上是基于前缀的虚拟结构。以下是使用 **AWS SDK for Java 2.x** 检查指定前缀下是否存在超过 150MB 对象的完整方案：

---

### 步骤 1：添加依赖
```xml
<dependency>
    <groupId>software.amazon.awssdk</groupId>
    <artifactId>s3</artifactId>
    <version>2.20.26</version> <!-- 使用最新版本 -->
</dependency>
```

---

### 步骤 2：Java 代码实现
```java
import software.amazon.awssdk.core.exception.SdkException;
import software.amazon.awssdk.regions.Region;
import software.amazon.awssdk.services.s3.S3Client;
import software.amazon.awssdk.services.s3.model.*;

public class S3LargeObjectChecker {
    private static final long MAX_SIZE_BYTES = 150L * 1024 * 1024; // 150MB
    private static final String BUCKET_NAME = "your-bucket-name";
    private static final String TARGET_PREFIX = "your/directory/prefix/"; // 以"/"结尾

    public static void main(String[] args) {
        S3Client s3 = S3Client.builder()
                .region(Region.US_EAST_1) // 根据实际情况修改
                .build();

        boolean hasLargeObject = checkLargeObjects(s3);
        System.out.println("存在超过150MB的对象: " + hasLargeObject);
    }

    private static boolean checkLargeObjects(S3Client s3) {
        try {
            ListObjectsV2Request request = ListObjectsV2Request.builder()
                    .bucket(BUCKET_NAME)
                    .prefix(TARGET_PREFIX)
                    .build();

            // 使用分页遍历所有对象（避免截断）
            for (ListObjectsV2Response response : s3.listObjectsV2Paginator(request)) {
                boolean found = response.contents().stream()
                        .anyMatch(s3Object -> s3Object.size() > MAX_SIZE_BYTES);
                if (found) {
                    return true;
                }
            }
            return false;
        } catch (S3Exception e) {
            System.err.println("S3错误: " + e.awsErrorDetails().errorMessage());
            return false;
        } catch (SdkException e) {
            System.err.println("SDK错误: " + e.getMessage());
            return false;
        }
    }
}
```

---

### 关键点说明

1. **前缀而非目录**：
   - S3 没有真正的目录结构，通过 `prefix` 参数模拟目录查询（例如 `"reports/2023/"`）。
   - 确保 `prefix` 以 `/` 结尾，避免匹配类似前缀的其他对象。

2. **分页处理**：
   - 使用 `listObjectsV2Paginator` 自动处理分页，确保获取所有对象。

3. **性能优化**：
   - 使用 `anyMatch` 短路操作，发现首个大对象后立即终止检查。

4. **异常处理**：
   - 捕获 `S3Exception` 处理权限、Bucket不存在等问题。
   - 通用 `SdkException` 捕获网络等底层错误。

---

### 补充：权限要求
确保 IAM 用户/角色拥有以下权限：
```json
{
    "Version": "2012-10-17",
    "Statement": [{
        "Effect": "Allow",
        "Action": ["s3:ListBucket"],
        "Resource": ["arn:aws:s3:::your-bucket-name"]
    }]
}
```

---

### 验证方式
1. 替换代码中的 `BUCKET_NAME` 和 `TARGET_PREFIX`。
2. 配置 AWS 凭证（通过环境变量、`~/.aws/credentials` 文件或 IAM 角色）。
3. 执行代码检查输出结果。