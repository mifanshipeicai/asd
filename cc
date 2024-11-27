ClickHouse 的 map<string, float> 数据类型在使用 Java 和 Spark 通过 JDBC 写入时可能不直接支持，因为 JDBC 驱动程序通常不支持复杂的嵌套数据类型，如 Map。因此，我们需要通过一定的变通方法将数据写入表中，而无需更改表结构。

以下是实现方法的步骤：

方法：将 map<string, float> 转换为 JSON 字符串

在写入数据之前，将 Map 转换为 JSON 字符串，并通过 Spark 写入 ClickHouse。ClickHouse 支持将 JSON 格式的字符串自动解析为 map 类型的数据。

1. 配置 Spark 和 ClickHouse

确保你已经配置了 Spark 和 ClickHouse 的 JDBC 驱动程序。

添加 Maven 依赖：

<dependency>
    <groupId>ru.yandex.clickhouse</groupId>
    <artifactId>clickhouse-jdbc</artifactId>
    <version>0.3.2-patch14</version>
</dependency>

2. 数据转换逻辑

在 Java 中，可以通过以下方式将 Map<String, Float> 转换为 JSON 字符串：

import com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;

import java.util.HashMap;
import java.util.Map;

public class ClickHouseWriter {
    public static void main(String[] args) throws Exception {
        SparkSession spark = SparkSession.builder()
                .appName("ClickHouseWriter")
                .master("local[*]")
                .getOrCreate();

        // 模拟数据
        Map<String, Float> sampleMap = new HashMap<>();
        sampleMap.put("key1", 1.1f);
        sampleMap.put("key2", 2.2f);

        // 将 Map 转换为 JSON 字符串
        ObjectMapper objectMapper = new ObjectMapper();
        String jsonString = objectMapper.writeValueAsString(sampleMap);

        // 创建一个 DataFrame
        Dataset<Row> df = spark.createDataFrame(
                List.of(new ClickHouseData("example_id", jsonString)),
                ClickHouseData.class
        );

        // 写入 ClickHouse
        df.write()
                .format("jdbc")
                .option("url", "jdbc:clickhouse://<your_host>:<port>/<database>")
                .option("dbtable", "your_table")
                .option("user", "default")
                .option("password", "")
                .save();
    }

    // 数据类
    public static class ClickHouseData {
        private String id;
        private String mapColumn;

        public ClickHouseData(String id, String mapColumn) {
            this.id = id;
            this.mapColumn = mapColumn;
        }

        // Getters and Setters
        public String getId() {
            return id;
        }

        public void setId(String id) {
            this.id = id;
        }

        public String getMapColumn() {
            return mapColumn;
        }

        public void setMapColumn(String mapColumn) {
            this.mapColumn = mapColumn;
        }
    }
}

3. ClickHouse 自动解析 JSON 为 map

当 JSON 字符串写入 map<string, float> 列时，ClickHouse 会自动解析并存储为对应的 map 类型。例如，{"key1":1.1,"key2":2.2} 会被存储为 map<string, float>。

注意事项

	1.	数据验证：确保 JSON 字符串的格式与 map<string, float> 的要求一致，否则会报解析错误。
	2.	性能优化：如果写入大量数据，建议批量写入以提升性能。
	3.	JDBC 驱动：使用最新的 ClickHouse JDBC 驱动以支持更多特性。

这种方法可以在不更改表结构的情况下实现将 map<string, float> 数据插入 ClickHouse。