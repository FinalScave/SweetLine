import SweetLine
import SwiftUI
import UIKit

struct HighlightTheme: Identifiable, Equatable {
    let id = UUID()
    let name: String
    let backgroundColor: UInt32
    let textColor: UInt32
    let colorMap: [Int32: UInt32]

    static let styleKeyword: Int32 = 1
    static let styleString: Int32 = 2
    static let styleNumber: Int32 = 3
    static let styleComment: Int32 = 4
    static let styleClass: Int32 = 5
    static let styleMethod: Int32 = 6
    static let styleVariable: Int32 = 7
    static let stylePunctuation: Int32 = 8
    static let styleAnnotation: Int32 = 9
    static let stylePreprocessor: Int32 = 10
    static let styleMacro: Int32 = 11
    static let styleLifetime: Int32 = 12
    static let styleSelector: Int32 = 13
    static let styleBuiltin: Int32 = 14
    static let styleURL: Int32 = 15
    static let styleProperty: Int32 = 16

    var background: Color { Color(uiColor: backgroundUIColor) }
    var text: Color { Color(uiColor: textUIColor) }
    var backgroundUIColor: UIColor { uiColor(backgroundColor) }
    var textUIColor: UIColor { uiColor(textColor) }
    var isDark: Bool {
        var red: CGFloat = 0
        var green: CGFloat = 0
        var blue: CGFloat = 0
        var alpha: CGFloat = 0
        guard backgroundUIColor.getRed(&red, green: &green, blue: &blue, alpha: &alpha) else {
            return true
        }
        return 0.2126 * red + 0.7152 * green + 0.0722 * blue < 0.3
    }

    func color(for styleId: Int32) -> UIColor {
        uiColor(colorMap[styleId] ?? textColor)
    }

    func foreground(for style: TokenStyle) -> UIColor {
        switch style {
        case let .styleId(styleId):
            return color(for: styleId)
        case let .inline(inlineStyle):
            return uiColor(inlineStyle.foreground == 0 ? textColor : UInt32(bitPattern: inlineStyle.foreground))
        }
    }

    static func builtinThemes() -> [HighlightTheme] {
        [sweetLineDark(), monokai(), dracula(), oneDark(), solarizedDark(), nord(), githubDark()]
    }

    static func sweetLineDark() -> HighlightTheme {
        HighlightTheme(name: "SweetLine Dark", backgroundColor: 0xFF1E1E1E, textColor: 0xFFD4D4D4, colorMap: [
            styleKeyword: 0xFF569CD6,
            styleString: 0xFFBD63C5,
            styleNumber: 0xFFE4FAD5,
            styleComment: 0xFF60AE6F,
            styleClass: 0xFF4EC9B0,
            styleMethod: 0xFF9CDCFE,
            styleVariable: 0xFF9B9BC8,
            stylePunctuation: 0xFFD69D85,
            styleAnnotation: 0xFFFFFD9B,
            stylePreprocessor: 0xFF569CD6,
            styleMacro: 0xFF9B9BC8,
            styleLifetime: 0xFF4EC9B0,
            styleSelector: 0xFF4EC9B0,
            styleBuiltin: 0xFF569CD6,
            styleURL: 0xFF4FC1FF,
            styleProperty: 0xFF9CDCFE,
        ])
    }

    static func monokai() -> HighlightTheme {
        HighlightTheme(name: "Monokai", backgroundColor: 0xFF272822, textColor: 0xFFF8F8F2, colorMap: [
            styleKeyword: 0xFFF92672,
            styleString: 0xFFE6DB74,
            styleNumber: 0xFFAE81FF,
            styleComment: 0xFF75715E,
            styleClass: 0xFFA6E22E,
            styleMethod: 0xFFA6E22E,
            styleVariable: 0xFFF8F8F2,
            stylePunctuation: 0xFFF8F8F2,
            styleAnnotation: 0xFFE6DB74,
            stylePreprocessor: 0xFFF92672,
            styleMacro: 0xFFAE81FF,
            styleLifetime: 0xFFFD971F,
            styleSelector: 0xFFA6E22E,
            styleBuiltin: 0xFF66D9EF,
            styleURL: 0xFF66D9EF,
            styleProperty: 0xFFA6E22E,
        ])
    }

    static func dracula() -> HighlightTheme {
        HighlightTheme(name: "Dracula", backgroundColor: 0xFF282A36, textColor: 0xFFF8F8F2, colorMap: [
            styleKeyword: 0xFFFF79C6,
            styleString: 0xFFF1FA8C,
            styleNumber: 0xFFBD93F9,
            styleComment: 0xFF6272A4,
            styleClass: 0xFF8BE9FD,
            styleMethod: 0xFF50FA7B,
            styleVariable: 0xFFF8F8F2,
            stylePunctuation: 0xFFF8F8F2,
            styleAnnotation: 0xFFFFB86C,
            stylePreprocessor: 0xFFFF79C6,
            styleMacro: 0xFFBD93F9,
            styleLifetime: 0xFFFFB86C,
            styleSelector: 0xFF50FA7B,
            styleBuiltin: 0xFF8BE9FD,
            styleURL: 0xFF8BE9FD,
            styleProperty: 0xFF50FA7B,
        ])
    }

    static func oneDark() -> HighlightTheme {
        HighlightTheme(name: "One Dark", backgroundColor: 0xFF282C34, textColor: 0xFFABB2BF, colorMap: [
            styleKeyword: 0xFFC678DD,
            styleString: 0xFF98C379,
            styleNumber: 0xFFD19A66,
            styleComment: 0xFF5C6370,
            styleClass: 0xFFE5C07B,
            styleMethod: 0xFF61AFEF,
            styleVariable: 0xFFE06C75,
            stylePunctuation: 0xFFABB2BF,
            styleAnnotation: 0xFFE5C07B,
            stylePreprocessor: 0xFFC678DD,
            styleMacro: 0xFFD19A66,
            styleLifetime: 0xFF56B6C2,
            styleSelector: 0xFFE5C07B,
            styleBuiltin: 0xFF56B6C2,
            styleURL: 0xFF61AFEF,
            styleProperty: 0xFF61AFEF,
        ])
    }

    static func solarizedDark() -> HighlightTheme {
        HighlightTheme(name: "Solarized Dark", backgroundColor: 0xFF002B36, textColor: 0xFF839496, colorMap: [
            styleKeyword: 0xFF859900,
            styleString: 0xFF2AA198,
            styleNumber: 0xFFD33682,
            styleComment: 0xFF586E75,
            styleClass: 0xFFB58900,
            styleMethod: 0xFF268BD2,
            styleVariable: 0xFFCB4B16,
            stylePunctuation: 0xFF839496,
            styleAnnotation: 0xFFB58900,
            stylePreprocessor: 0xFF859900,
            styleMacro: 0xFFCB4B16,
            styleLifetime: 0xFFD33682,
            styleSelector: 0xFF268BD2,
            styleBuiltin: 0xFF268BD2,
            styleURL: 0xFF268BD2,
            styleProperty: 0xFF268BD2,
        ])
    }

    static func nord() -> HighlightTheme {
        HighlightTheme(name: "Nord", backgroundColor: 0xFF2E3440, textColor: 0xFFD8DEE9, colorMap: [
            styleKeyword: 0xFF81A1C1,
            styleString: 0xFFA3BE8C,
            styleNumber: 0xFFB48EAD,
            styleComment: 0xFF616E88,
            styleClass: 0xFF8FBCBB,
            styleMethod: 0xFF88C0D0,
            styleVariable: 0xFFD8DEE9,
            stylePunctuation: 0xFFECEFF4,
            styleAnnotation: 0xFFEBCB8B,
            stylePreprocessor: 0xFF81A1C1,
            styleMacro: 0xFFB48EAD,
            styleLifetime: 0xFFEBCB8B,
            styleSelector: 0xFF8FBCBB,
            styleBuiltin: 0xFF5E81AC,
            styleURL: 0xFF88C0D0,
            styleProperty: 0xFF88C0D0,
        ])
    }

    static func githubDark() -> HighlightTheme {
        HighlightTheme(name: "GitHub Dark", backgroundColor: 0xFF0D1117, textColor: 0xFFC9D1D9, colorMap: [
            styleKeyword: 0xFFFF7B72,
            styleString: 0xFFA5D6FF,
            styleNumber: 0xFF79C0FF,
            styleComment: 0xFF8B949E,
            styleClass: 0xFFFFA657,
            styleMethod: 0xFFD2A8FF,
            styleVariable: 0xFFFFA657,
            stylePunctuation: 0xFFC9D1D9,
            styleAnnotation: 0xFFFFA657,
            stylePreprocessor: 0xFFFF7B72,
            styleMacro: 0xFF79C0FF,
            styleLifetime: 0xFFFFA657,
            styleSelector: 0xFF7EE787,
            styleBuiltin: 0xFF79C0FF,
            styleURL: 0xFF79C0FF,
            styleProperty: 0xFF79C0FF,
        ])
    }

    private func uiColor(_ argb: UInt32) -> UIColor {
        let alpha = CGFloat((argb >> 24) & 0xFF) / 255.0
        let red = CGFloat((argb >> 16) & 0xFF) / 255.0
        let green = CGFloat((argb >> 8) & 0xFF) / 255.0
        let blue = CGFloat(argb & 0xFF) / 255.0
        return UIColor(red: red, green: green, blue: blue, alpha: alpha)
    }
}
