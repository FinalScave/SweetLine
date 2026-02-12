# 一级标题
## 二级标题
### 三级标题
#### 四级标题
##### 五级标题
###### 六级标题

这是普通文本，包含**加粗文本**和*斜体文本*以及~~删除线文本~~。

还可以使用__下划线加粗__和_下划线斜体_。

行内代码：`println("Hello")` 和 `int x = 42`。

---

> 这是一段引用文本
> 引用可以有多行

***

## 链接与图片

[SweetLine项目](https://github.com/example/sweetline)

![Logo图片](https://example.com/logo.png)

[引用链接文本][ref-id]

___

## 列表

- 无序列表项一
- 无序列表项二
* 星号列表项
+ 加号列表项

1. 有序列表项一
2. 有序列表项二
3. 有序列表项三

- [x] 已完成任务
- [ ] 未完成任务
- [X] 大写X也表示完成

## 表格

| 语言 | 类型 | 特点 |
|:-----|:----:|-----:|
| Java | 编译型 | **跨平台** |
| Python | 解释型 | `简洁易读` |
| Rust | 编译型 | **内存安全** |

## 代码块

```java
public class Hello {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
```

```python
def greet(name: str) -> str:
    """问候函数"""
    return f"Hello, {name}!"

print(greet("SweetLine"))
```

```go
package main

import "fmt"

func main() {
    ch := make(chan int)
    go func() { ch <- 42 }()
    fmt.Println(<-ch)
}
```

```kotlin
fun main() {
    val list = listOf(1, 2, 3)
    list.forEach { println("Item: $it") }
}
```
## 混合内联样式

这段文字包含**加粗**、*斜体*、~~删除线~~、`行内代码`和[链接](https://example.com)。
