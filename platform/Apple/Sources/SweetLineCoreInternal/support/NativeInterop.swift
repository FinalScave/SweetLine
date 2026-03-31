import Foundation
import SweetLineBridge

enum NativeInterop {
    static func stringResult(_ value: UnsafePointer<CChar>?) -> String? {
        guard let value else {
            return nil
        }
        return String(cString: value)
    }

    static func withCString<T>(_ string: String, _ body: (UnsafePointer<CChar>) throws -> T) rethrows -> T {
        try string.withCString(body)
    }

    static func withInt32Array<T>(_ values: [Int], _ body: (UnsafeMutablePointer<Int32>) throws -> T) rethrows -> T {
        var copied = values.map(Int32.init)
        return try copied.withUnsafeMutableBufferPointer { buffer in
            try body(buffer.baseAddress!)
        }
    }

    static func createDataAndFree(from ownedBuffer: UnsafeMutablePointer<Int32>?, count: Int) -> Data {
        guard let ownedBuffer, count > 0 else {
            return Data()
        }
        let data = Data(bytes: ownedBuffer, count: count * MemoryLayout<Int32>.size)
        sl_free_buffer(ownedBuffer)
        return data
    }

    static func syntaxError(_ error: sl_syntax_error_t) throws {
        guard error.err_code == SL_OK else {
            throw SweetLineError.nativeError(
                code: Int(error.err_code.rawValue),
                message: stringResult(error.err_msg) ?? "Syntax compilation failed."
            )
        }
    }

    static func nativeError(_ error: sl_error_t, action: String) throws {
        guard error == SL_OK else {
            throw SweetLineError.nativeError(code: Int(error.rawValue), message: "Failed to \(action).")
        }
    }

    static func countDocumentHighlightInts(from values: [Int32]) throws -> Int {
        guard values.count >= 3 else {
            throw SweetLineError.invalidResult("Document highlight payload is too short.")
        }
        let stride = max(Int(values[1]), 0)
        let lineCount = max(Int(values[2]), 0)
        var cursor = 3
        for _ in 0..<lineCount {
            guard cursor < values.count else {
                throw SweetLineError.invalidResult("Document highlight payload ended early.")
            }
            let spanCount = max(Int(values[cursor]), 0)
            cursor += 1 + spanCount * stride
        }
        return cursor
    }

    static func countLineAnalyzeInts(from values: [Int32]) throws -> Int {
        guard values.count >= 5 else {
            throw SweetLineError.invalidResult("Line analyze payload is too short.")
        }
        let stride = max(Int(values[1]), 0)
        let spanCount = max(Int(values[2]), 0)
        return 5 + spanCount * stride
    }

    static func countDocumentHighlightSliceInts(from values: [Int32]) throws -> Int {
        guard values.count >= 5 else {
            throw SweetLineError.invalidResult("Document highlight slice payload is too short.")
        }
        let stride = max(Int(values[1]), 0)
        let lineCount = max(Int(values[4]), 0)
        var cursor = 5
        for _ in 0..<lineCount {
            guard cursor < values.count else {
                throw SweetLineError.invalidResult("Document highlight slice payload ended early.")
            }
            let spanCount = max(Int(values[cursor]), 0)
            cursor += 1 + spanCount * stride
        }
        return cursor
    }

    static func countIndentGuideInts(from values: [Int32]) throws -> Int {
        guard values.count >= 4 else {
            throw SweetLineError.invalidResult("Indent guide payload is too short.")
        }
        let guideCount = max(Int(values[0]), 0)
        let lineStateCount = max(Int(values[2]), 0)
        var cursor = 4
        for _ in 0..<guideCount {
            guard cursor + 5 < values.count else {
                throw SweetLineError.invalidResult("Indent guide payload ended early.")
            }
            let branchCount = max(Int(values[cursor + 5]), 0)
            cursor += 6 + branchCount * 2
        }
        return cursor + lineStateCount * 4
    }

    static func copyInt32Prefix(from ownedBuffer: UnsafeMutablePointer<Int32>?, initialCount: Int, totalCount: (Array<Int32>) throws -> Int) throws -> Data {
        guard let ownedBuffer else {
            return Data()
        }
        let prefix = Array(UnsafeBufferPointer(start: ownedBuffer, count: initialCount))
        let count = try totalCount(prefix)
        return createDataAndFree(from: ownedBuffer, count: count)
    }
}
