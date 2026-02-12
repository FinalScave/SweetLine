# H1 Heading
## H2 Heading
### H3 Heading
#### H4 Heading
##### H5 Heading
###### H6 Heading

This is plain text with **bold text** and *italic text* and ~~strikethrough text~~.

You can also use __underline bold__ and _underline italic_.

Inline code: `println("Hello")` and `int x = 42`.

---

> This is a blockquote
> Blockquotes can span multiple lines

***

## Links

[SweetLine Project](https://github.com/FinalScave/SweetLine)

[Reference link text][ref-id]

[ref-id]: https://example.com "Reference Link Definition"

___

## Footnotes

This text has a footnote[^1] and another one[^note].

[^1]: This is the first footnote definition.
[^note]: This is a named footnote definition.

## Lists

- Unordered list item one
- Unordered list item two
* Asterisk list item
+ Plus list item

1. Ordered list item one
2. Ordered list item two
3. Ordered list item three

- [x] Completed task
- [ ] Incomplete task
- [X] Uppercase X also means completed

## Escape Characters

Use \* to show a literal asterisk without triggering italic.

Use \# to show a literal hash symbol.

Escaped brackets: \[ and \] and backslash: \\

## HTML Support

<!-- This is an HTML comment -->

<!--
This is a multi-line
HTML comment
-->

<div class="container">
  <p>HTML paragraph tag</p>
  <br/>
  <a href="https://example.com">HTML link</a>
</div>

## Bare URLs

Visit https://github.com/FinalScave/SweetLine for the project.

## Images

![Alt text](https://example.com/image.png)

## Table

| Language | Type | Feature |
|:---------|:----:|--------:|
| Java | Compiled | **Cross-platform** |
| Python | Interpreted | `Concise & readable` |
| Rust | Compiled | **Memory safe** |
| Swift | Compiled | *Modern syntax* |
| Go | Compiled | ~~Complex generics~~ |

## Code Blocks

```java
public class Hello {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
```

```python
def greet(name: str) -> str:
    """Greeting function"""
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

## Mixed Inline Styles

This text contains **bold**, *italic*, ~~strikethrough~~, `inline code` and [link](https://example.com).
