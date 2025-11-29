// Rust sample

use std::collections::HashMap;
use std::fmt;
use std::sync::Arc;

/// document comment
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

// lifetime annotation
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() > y.len() { x } else { y }
}

// struct member field highlight test
struct Config {
    pub name: String,
    timeout: u64,
    data: &'static str,
}

struct Wrapper<'a, T: 'a> {
    data: &'a T,
}

// impl namespace + for type + generic test
impl<'a, T: fmt::Display + 'static> Wrapper<'a, T> {
    fn show(&self) {
        println!("{}", self.data);
    }
}

// function return type highlight test
fn create_map(size: usize) -> HashMap<String, Vec<i32>> {
    HashMap::new()
}

fn get_name<'a>(input: &'a str) -> &'a str {
    input
}

// macro definition
macro_rules! create_map {
    ($($key:expr => $val:expr),* $(,)?) => {
        {
            let mut map = HashMap::new();
            $(map.insert($key, $val);)*
            map
        }
    };
}

// attribute macro
#![allow(unused)]

fn main() {
    // macro call
    let map = create_map! {
        "one" => 1,
        "two" => 2,
    };
    println!("{:?}", map);
    eprintln!("error message");
    assert_eq!(map.len(), 2);

    // Vec macro
    let nums = vec![1, 2, 3, 4, 5];
    let formatted = format!("nums: {:?}", nums);

    // built-in enum variant
    let opt: Option<i32> = Some(42);
    let none: Option<i32> = None;
    let ok: Result<i32, String> = Ok(100);
    let err: Result<i32, String> = Err("failed".to_string());

    // boolean value
    let flag = true;
    let off = false;

    // numeric literal
    let hex = 0xFF_u32;
    let bin = 0b1010_i8;
    let oct = 0o77_u64;
    let float = 3.14_f64;
    let sci = 1.5e10;
    let big = 1_000_000_usize;

    // string
    let s = "Hello, World!";
    let ch = 'A';
    let escape = "tab:\t newline:\n";
    let raw = r#"raw string with "quotes""#;
    let byte = b"bytes";

    // lifetime usage
    let result = longest("hello", "world!");
    let wrapper = Wrapper { data: &42 };
    wrapper.show();

    // closure and iterator
    let doubled: Vec<i32> = nums.iter().map(|&x| x * 2).collect();
    let sum: i32 = doubled.iter().sum();

    // pattern matching
    match opt {
        Some(val) if val > 0 => println!("Positive: {}", val),
        Some(_) => println!("Non-positive"),
        None => println!("Nothing"),
    }

    // error handling
    if let Ok(val) = ok {
        println!("Value: {}", val);
    }

    // smart pointer
    let shared: Arc<Vec<i32>> = Arc::new(vec![1, 2, 3]);
    let boxed: Box<dyn Drawable> = Box::new(Shape::Circle(5.0));

    /* multi-line comment
       can span multiple lines */
}
