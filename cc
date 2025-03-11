@ExtendWith(MockitoExtension.class)
class SftpConfigTest {

    @Mock
    private Resource privateKeyResource;

    @InjectMocks
    private SftpConfig sftpConfig;

    @Test
    @DisplayName("应成功创建SFTP会话")
    void shouldCreateValidSession() throws Exception {
        // 模拟配置参数
        ReflectionTestUtils.setField(sftpConfig, "host", "sftp.example.com");
        ReflectionTestUtils.setField(sftpConfig, "port", 22);
        ReflectionTestUtils.setField(sftpConfig, "username", "testuser");
        ReflectionTestUtils.setField(sftpConfig, "passphrase", "secret");

        // 模拟私钥文件输入流
        ByteArrayInputStream keyStream = new ByteArrayInputStream(
            ("-----BEGIN RSA PRIVATE KEY-----\nMOCK_KEY_DATA\n-----END RSA PRIVATE KEY-----").getBytes()
        );
        when(privateKeyResource.getInputStream()).thenReturn(keyStream);

        // 执行配置方法
        Session session = sftpConfig.sftpSession();

        // 验证基础配置
        assertThat(session).isNotNull();
        assertThat(session.getHost()).isEqualTo("sftp.example.com");
        assertThat(session.getPort()).isEqualTo(22);
        assertThat(session.getUserName()).isEqualTo("testuser");
        assertThat(session.isConnected()).isTrue();

        session.disconnect();
    }

    @Test
    @DisplayName("应抛出异常当私钥无效时")
    void shouldThrowExceptionWhenInvalidKey() throws IOException {
        // 模拟损坏的私钥
        ByteArrayInputStream badKeyStream = new ByteArrayInputStream("invalid_key".getBytes());
        when(privateKeyResource.getInputStream()).thenReturn(badKeyStream);

        // 验证异常抛出
        assertThrows(JSchException.class, () -> sftpConfig.sftpSession());
    }
}
