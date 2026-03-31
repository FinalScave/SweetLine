import Foundation

enum SyntaxRegistry {
    private static let extensionMapping: [String: String] = [
        ".c": "c.json",
        ".cpp": "cpp.json",
        ".cs": "csharp.json",
        ".go": "go.json",
        ".java": "java.json",
        ".js": "javascript.json",
        ".json": "json-sweetline.json",
        ".kt": "kotlin.json",
        ".m": "objc.json",
        ".md": "markdown.json",
        ".py": "python.json",
        ".rs": "rust.json",
        ".swift": "swift.json",
        ".ts": "typescript.json",
    ]

    static func syntaxFileName(forExtension pathExtension: String) -> String? {
        extensionMapping[pathExtension.lowercased()]
    }
}
