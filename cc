在Java中，如果你有一个`List<Map<String, String>>`，需要将每个`Map`中值为空字符串(`""`)的条目替换为指定的值，可以通过遍历列表和每个Map来实现。以下是详细步骤和示例代码：

### 实现步骤
1. **遍历List中的每个Map**
2. **遍历当前Map中的每个键值对**
3. **检查值是否为空字符串**（使用`isEmpty()`或直接比较）
4. **替换为空字符串的值**为指定值（注意：直接修改Map会改变原始数据，若需保留原数据需先复制）

### 示例代码
```java
import java.util.*;

public class ReplaceEmptyMapValues {
    public static void main(String[] args) {
        // 1. 创建示例数据
        List<Map<String, String>> listOfMaps = new ArrayList<>();
        
        Map<String, String> map1 = new HashMap<>();
        map1.put("name", "Alice");
        map1.put("email", "");      // 空字符串
        map1.put("address", null);   // null值（不替换）
        
        Map<String, String> map2 = new HashMap<>();
        map2.put("name", "Bob");
        map2.put("email", "bob@example.com");
        map2.put("phone", "");       // 空字符串
        
        listOfMaps.add(map1);
        listOfMaps.add(map2);
        
        // 2. 替换空字符串为指定值（如："N/A"）
        replaceEmptyValues(listOfMaps, "N/A");
        
        // 3. 验证结果
        System.out.println(listOfMaps);
        // 输出：[{name=Alice, email=N/A, address=null}, {name=Bob, phone=N/A, email=bob@example.com}]
    }
    
    /**
     * 替换List中所有Map的空字符串值为指定值
     * @param listOfMaps 目标List
     * @param replacement 用于替换的值
     */
    public static void replaceEmptyValues(
        List<Map<String, String>> listOfMaps, 
        String replacement
    ) {
        for (Map<String, String> map : listOfMaps) {
            // 遍历当前Map的键集合（避免并发修改异常）
            for (String key : new ArrayList<>(map.keySet())) {
                String value = map.get(key);
                // 检查是否为空字符串（注意：不处理null）
                if (value != null && value.isEmpty()) {
                    map.put(key, replacement); // 直接修改原Map
                }
            }
        }
    }
}
```

### 关键点说明
1. **区分空字符串和null**：
   - 仅替换`""`（使用`value.isEmpty()`判断）
   - 保留`null`值不变（通过`value != null`过滤）
   
2. **直接修改原始Map**：
   - 代码直接操作传入的`List`和`Map`，会改变原始数据
   - 若需保留原数据，可先深拷贝（示例未展示，需用`new HashMap<>(map)`复制）

3. **避免并发修改**：
   - 遍历时使用`new ArrayList<>(map.keySet())`创建键的副本
   - 防止在遍历原始`keySet`时修改导致的`ConcurrentModificationException`

### 其他方案：Java 8 Stream API（不修改原数据）
如果需要生成新对象而非修改原数据：
```java
public static List<Map<String, String>> replaceEmptyValuesStream(
    List<Map<String, String>> originalList,
    String replacement
) {
    return originalList.stream()
        .map(map -> map.entrySet().stream()
            .collect(Collectors.toMap(
                Map.Entry::getKey,
                e -> (e.getValue() != null && e.getValue().isEmpty()) 
                    ? replacement 
                    : e.getValue()
            ))
        )
        .collect(Collectors.toList());
}
```

### 使用建议
- **原地替换**：使用第一个示例，效率高且节省内存
- **不可变数据**：若需保留原数据，选择Stream方案或先深拷贝
- **复杂逻辑**：可在遍历时扩展其他校验规则（如空白字符串`isBlank()`）

通过以上方法，你可以高效地将`List<Map>`中的空字符串值替换为指定内容。