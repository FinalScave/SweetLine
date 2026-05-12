public enum SweetLineError: Error, Sendable, Equatable, CustomStringConvertible {
    case nativeError(code: Int32, action: String)
    case nullHandle(action: String)
    case closedObject(String)
    case invalidNativeBuffer(String)

    public var description: String {
        switch self {
        case let .nativeError(code, action):
            return "\(action) failed with native error code \(code)."
        case let .nullHandle(action):
            return "\(action) returned a null native handle."
        case let .closedObject(name):
            return "\(name) is already closed."
        case let .invalidNativeBuffer(message):
            return message
        }
    }
}

public struct SyntaxCompileError: Error, Sendable, Equatable, CustomStringConvertible {
    public let code: Int32
    public let message: String

    public init(code: Int32, message: String) {
        self.code = code
        self.message = message
    }

    public var description: String {
        if message.isEmpty {
            return "Syntax compile failed with native error code \(code)."
        }
        return message
    }
}
