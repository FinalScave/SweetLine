// Swift 高亮示例

import Foundation
import UIKit

// 编译指令 (#selector 等)
#if DEBUG
#warning("Debug build")
#endif

@available(iOS 13.0, *)
protocol Drawable {
    func draw() -> String
}

@objc class Shape: NSObject, Drawable {
    let name: String
    var sides: Int

    init(name: String, sides: Int) {
        self.name = name
        self.sides = sides
        super.init()
    }

    @discardableResult
    func draw() -> String {
        return "\(name) with \(sides) sides"
    }
}

struct Point {
    var x: Double
    var y: Double

    mutating func translate(dx: Double, dy: Double) {
        x += dx
        y += dy
    }
}

enum Result<T> {
    case success(T)
    case failure(Error)
}

// 编译指令
let file = #file
let line = #line
let fn = #function
let col = #column

// 泛型函数
func swap<T>(_ a: inout T, _ b: inout T) {
    let temp = a
    a = b
    b = temp
}

// 扩展
extension Array where Element: Comparable {
    func isSorted() -> Bool {
        return zip(self, self.dropFirst()).allSatisfy { $0 <= $1 }
    }
}

func main() {
    let shape = Shape(name: "Triangle", sides: 3)
    print(shape.draw())

    // 数字字面量
    let hex = 0xFF_AB
    let bin = 0b1100_0011
    let oct = 0o77
    let pi: Float = 3.14
    let big: Double = 1.5e10

    let flag: Bool = true
    let empty: String? = nil
    let selfRef = self

    // 字符串
    let greeting = "Hello, \(shape.name)!"
    let multiline = """
        line 1
        line \(2 + 1)
        """

    // 控制流
    guard let value = empty else {
        return
    }

    switch shape.sides {
    case 0...2:
        break
    case 3:
        fallthrough
    default:
        print("polygon")
    }

    // 闭包
    let sorted = [3, 1, 4].sorted { $0 < $1 }

    // async/await
    Task {
        try await Task.sleep(nanoseconds: 1_000_000)
    }

    /* 多行注释
       跨越多行 */
}
