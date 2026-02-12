// Swift sample

import Foundation
import UIKit

// compile directive
#if DEBUG
#warning("Debug build")
#endif

#if canImport(UIKit)
#error("error message")
#endif

// compile directive as value
let file = #file
let line = #line
let fn = #function
let col = #column
let fid = #fileID
let fp = #filePath

// Attribute
@available(iOS 13.0, *)
protocol Drawable {
    associatedtype Output
    func draw() -> String
}

// class inheritance: multiple protocols
@objc class Shape: NSObject, Drawable, Codable {
    typealias Output = String
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

    deinit {
        print("deinit")
    }
}

// generic class inheritance
class Container<K, V>: Sequence where K: Hashable {
    var items: Dictionary<K, V> = [:]

    subscript(key: K) -> V? {
        return items[key]
    }
}

// struct definition
struct Point {
    var x: Double
    var y: Double

    mutating func translate(dx: Double, dy: Double) {
        x += dx
        y += dy
    }
}

// enum generic
enum Result<T> {
    case success(T)
    case failure(Error)
}

// extension + where constraint
extension Array where Element: Comparable {
    func isSorted() -> Bool {
        return zip(self, self.dropFirst()).allSatisfy { $0 <= $1 }
    }
}

// actor definition
actor DataStore {
    var data: [String] = []
}

// generic function + inout
func swap<T>(_ a: inout T, _ b: inout T) {
    let temp = a
    a = b
    b = temp
}

// function return type + throws
func loadData(from path: String) throws -> Data {
    return Data()
}

// function return generic type
func createMap(size: Int) -> Dictionary<String, Int> {
    return [:]
}

// async throws return
func fetchUser(id: Int) async throws -> String {
    return ""
}

// where constraint after function return type
func findAll<T: Equatable>(in items: [T]) -> [T] where T: Hashable {
    return items
}

// some return type
func makeView() -> some Equatable {
    return 42
}

// multiple parameter label
func move(from start: Point, to end: Point) -> Double {
    return 0.0
}

func main() {
    let shape = Shape(name: "Triangle", sides: 3)
    print(shape.draw())

    // numeric literal
    let hex = 0xFF_AB
    let bin = 0b1100_0011
    let oct = 0o77
    let pi: Float = 3.14
    let big: Double = 1.5e10

    let flag: Bool = true
    let empty: String? = nil
    let selfRef = self

    // array/optional type
    let names: [String] = []
    var scores: [Int]? = nil

    // string
    let greeting = "Hello, \(shape.name)!"
    let multiline = """
        line 1
        line \(2 + 1)
        """

    // control flow
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

    // closure
    let sorted = [3, 1, 4].sorted { $0 < $1 }

    // async/await
    Task {
        try await Task.sleep(nanoseconds: 1_000_000)
    }

    /* multi-line comment
       can span multiple lines */
}
