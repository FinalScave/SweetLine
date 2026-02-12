// Rust 高亮示例

use std::collections::HashMap;
use std::fmt;
use std::sync::Arc;

/// 文档注释
#[derive(Debug, Clone)]
struct Point {
    x: f64,
    y: f64,
}

#[allow(dead_code)]
enum Shape {
    Circle(f64),
    Rectangle { width: f64, height: f64 },
    Triangle(Point, Point, Point),
}

trait Drawable {
    fn draw(&self) -> String;
    fn area(&self) -> f64;
}

impl Drawable for Shape {
    fn draw(&self) -> String {
        match self {
            Shape::Circle(r) => format!("Circle(r={})", r),
            Shape::Rectangle { width, height } => {
                format!("Rect({}x{})", width, height)
            }
            _ => String::from("Unknown"),
        }
    }

    fn area(&self) -> f64 {
        match self {
            Shape::Circle(r) => std::f64::consts::PI * r * r,
            Shape::Rectangle { width, height } => width * height,
            _ => 0.0,
        }
    }
}

impl fmt::Display for Point {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

// 生命周期标注
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() > y.len() { x } else { y }
}

struct Wrapper<'a, T: 'a> {
    data: &'a T,
}

impl<'a, T: fmt::Display + 'static> Wrapper<'a, T> {
    fn show(&self) {
        println!("{}", self.data);
    }
}

// 宏定义
macro_rules! create_map {
    ($($key:expr => $val:expr),* $(,)?) => {
        {
            let mut map = HashMap::new();
            $(map.insert($key, $val);)*
            map
        }
    };
}

// 属性宏
#![allow(unused)]

fn main() {
    // 宏调用
    let map = create_map! {
        "one" => 1,
        "two" => 2,
    };
    println!("{:?}", map);
    eprintln!("error message");
    assert_eq!(map.len(), 2);

    // Vec 宏
    let nums = vec![1, 2, 3, 4, 5];
    let formatted = format!("nums: {:?}", nums);

    // 内置枚举变体
    let opt: Option<i32> = Some(42);
    let none: Option<i32> = None;
    let ok: Result<i32, String> = Ok(100);
    let err: Result<i32, String> = Err("failed".to_string());

    // 布尔值
    let flag = true;
    let off = false;

    // 数字字面量
    let hex = 0xFF_u32;
    let bin = 0b1010_i8;
    let oct = 0o77_u64;
    let float = 3.14_f64;
    let sci = 1.5e10;
    let big = 1_000_000_usize;

    // 字符串
    let s = "Hello, World!";
    let ch = 'A';
    let escape = "tab:\t newline:\n";
    let raw = r#"raw string with "quotes""#;
    let byte = b"bytes";

    // 生命周期使用
    let result = longest("hello", "world!");
    let wrapper = Wrapper { data: &42 };
    wrapper.show();

    // 闭包和迭代器
    let doubled: Vec<i32> = nums.iter().map(|&x| x * 2).collect();
    let sum: i32 = doubled.iter().sum();

    // 模式匹配
    match opt {
        Some(val) if val > 0 => println!("Positive: {}", val),
        Some(_) => println!("Non-positive"),
        None => println!("Nothing"),
    }

    // 错误处理
    if let Ok(val) = ok {
        println!("Value: {}", val);
    }

    // 智能指针
    let shared: Arc<Vec<i32>> = Arc::new(vec![1, 2, 3]);
    let boxed: Box<dyn Drawable> = Box::new(Shape::Circle(5.0));

    /* 多行注释
       跨越多行 */
}
