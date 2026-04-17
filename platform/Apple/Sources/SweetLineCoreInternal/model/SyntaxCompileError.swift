import Foundation

public struct SyntaxCompileError: Error, Equatable {
    public static let errJSONPropertyMissed = -1
    public static let errJSONPropertyInvalid = -2
    public static let errPatternInvalid = -3
    public static let errStateInvalid = -4
    public static let errJSONInvalid = -5
    public static let errFileNotExists = -6
    public static let errFileInvalid = -7
    public static let errImportSyntaxNotFound = -8
    public static let errStateReferenceNotFound = -9

    public let errorCode: Int
    public let message: String

    public init(errorCode: Int, message: String) {
        self.errorCode = errorCode
        self.message = message
    }
}
