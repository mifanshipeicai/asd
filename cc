import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.ArgumentCaptor;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import software.amazon.awssdk.services.s3.S3Client;
import software.amazon.awssdk.services.s3.model.PutObjectRequest;

import java.nio.file.Path;
import java.nio.file.Paths;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.mockito.Mockito.*;

@ExtendWith(MockitoExtension.class)
public class S3UtilsTest {

    @Mock
    private S3Client mockS3Client;

    @InjectMocks
    private S3Utils s3Utils;

    @BeforeEach
    void setUp() {
        // 如果需要在测试前做一些初始化，可以放在这里
    }

    @Test
    void testUploadFileToS3() {
        // 准备参数
        String bucketName = "test-bucket";
        String key = "test-key";
        Path filePath = Paths.get("test-file.txt");

        // 调用被测试的方法
        s3Utils.uploadFileToS3(mockS3Client, bucketName, key, filePath);

        // 使用 ArgumentCaptor 捕获调用 putObject 时传入的参数
        ArgumentCaptor<PutObjectRequest> requestCaptor = ArgumentCaptor.forClass(PutObjectRequest.class);
        ArgumentCaptor<Path> pathCaptor = ArgumentCaptor.forClass(Path.class);

        // 验证 mockS3Client.putObject(...) 是否被正确调用了一次
        verify(mockS3Client, times(1)).putObject(requestCaptor.capture(), pathCaptor.capture());

        // 断言传入的请求参数是否正确
        PutObjectRequest capturedRequest = requestCaptor.getValue();
        Path capturedPath = pathCaptor.getValue();

        assertEquals(bucketName, capturedRequest.bucket());
        assertEquals(key, capturedRequest.key());
        assertEquals(filePath, capturedPath);
    }

    @Test
    void testGetClient_FirstTimeShouldCreate() {
        // 先断言一下 s3Client 静态变量是否为 null（如果你愿意，可以用反射或者单独启动一个新进程来断言；不过这里仅示例）
        // 调用 getClient
        S3Client client = s3Utils.getClient("http://localhost:9000", "accessKey", "secretKey", "us-east-1");

        // 验证返回的 client 不为 null
        // （这里我们没有对 builder 进行 mock，因为 getClient 内部直接 new 出了一个真实的 S3Client。
        //  如果想“真正”测试 builder 逻辑，需要在代码中对 S3Client.builder() 做进一步抽象或使用 PowerMockito 等手段。）
        assertEquals(client, s3Utils.getClient("http://localhost:9000", "accessKey", "secretKey", "us-east-1"),
                "第二次调用应该返回同一个静态实例");
    }

    @Test
    void testGetClient_SubsequentCallsShouldReturnSameInstance() {
        // 第一次调用
        S3Client client1 = s3Utils.getClient("http://endpoint1", "ak1", "sk1", "us-east-1");
        // 第二次调用，参数不同，但因为静态变量已经被赋值，所以应返回同一个实例
        S3Client client2 = s3Utils.getClient("http://endpoint2", "ak2", "sk2", "us-west-2");

        // 断言是同一个实例
        assertEquals(client1, client2);
    }
}