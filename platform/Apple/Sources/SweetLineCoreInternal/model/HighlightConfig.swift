import Foundation

public struct HighlightConfig: Sendable, Equatable {
    public let showIndex: Bool
    public let inlineStyle: Bool

    public init(showIndex: Bool = true, inlineStyle: Bool = false) {
        self.showIndex = showIndex
        self.inlineStyle = inlineStyle
    }

    public static let `default` = HighlightConfig()
}
