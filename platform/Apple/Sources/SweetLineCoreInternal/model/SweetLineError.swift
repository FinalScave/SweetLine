import Foundation

public enum SweetLineError: Error, Equatable {
    case nativeError(code: Int, message: String)
    case invalidResult(String)
    case unsupportedSyntax(String)
}

extension SweetLineError {
    static func from(_ error: NSError) -> SweetLineError {
        .nativeError(code: error.code, message: error.localizedDescription)
    }
}
