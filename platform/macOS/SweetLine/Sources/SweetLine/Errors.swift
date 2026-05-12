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
    public static let ok: Int32 = 0
    public static let jsonPropertyMissed: Int32 = -1
    public static let jsonPropertyInvalid: Int32 = -2
    public static let patternInvalid: Int32 = -3
    public static let stateInvalid: Int32 = -4
    public static let jsonInvalid: Int32 = -5
    public static let fileNotExists: Int32 = -6
    public static let fileInvalid: Int32 = -7
    public static let importSyntaxNotFound: Int32 = -8
    public static let stateReferenceNotFound: Int32 = -9
    public static let inlineStyleReferenceNotFound: Int32 = -10

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
