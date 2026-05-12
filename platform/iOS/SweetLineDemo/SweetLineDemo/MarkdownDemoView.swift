import Combine
import SweetLine
import SwiftUI
import UIKit

struct MarkdownDemoView: View {
    let theme: HighlightTheme
    @StateObject private var viewModel = MarkdownDemoViewModel()

    var body: some View {
        VStack(spacing: 0) {
            MarkdownAttributedTextView(attributedText: viewModel.attributedText, theme: theme)
                .background(theme.background)

            Divider()

            HStack {
                if viewModel.isLoading {
                    ProgressView()
                        .controlSize(.small)
                }
                Text(viewModel.status)
                    .font(.caption.monospaced())
                    .lineLimit(2)
                Spacer(minLength: 0)
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 7)
            .background(.bar)
        }
        .navigationTitle("Markdown")
        .navigationBarTitleDisplayMode(.inline)
        .task {
            await viewModel.loadIfNeeded(theme: theme)
        }
    }
}

private struct MarkdownAttributedTextView: UIViewRepresentable {
    let attributedText: NSAttributedString
    let theme: HighlightTheme

    func makeUIView(context: Context) -> UITextView {
        let textView = UITextView()
        textView.isEditable = false
        textView.isSelectable = true
        textView.isScrollEnabled = true
        textView.alwaysBounceVertical = true
        textView.backgroundColor = theme.backgroundUIColor
        textView.textContainerInset = UIEdgeInsets(top: 14, left: 14, bottom: 18, right: 14)
        textView.textContainer.lineFragmentPadding = 0
        return textView
    }

    func updateUIView(_ textView: UITextView, context: Context) {
        textView.backgroundColor = theme.backgroundUIColor
        textView.indicatorStyle = theme.isDark ? .white : .black
        textView.attributedText = attributedText
    }
}

@MainActor
private final class MarkdownDemoViewModel: ObservableObject {
    @Published var attributedText = NSAttributedString(string: "Loading Markdown...")
    @Published var status = "Preparing Markdown demo..."
    @Published var isLoading = false

    private static let syntaxNameMapping: [String: String] = [
        "py": "python",
        "ts": "typescript",
        "kt": "kotlin",
        "c++": "cpp",
        "c#": "csharp",
        "docker": "dockerfile",
        "make": "makefile",
        "dotenv": "env",
        "proto": "protobuf",
        "protobuf": "protobuf",
        "graphql": "graphql",
        "gql": "graphql",
        "nginx": "nginx",
        "conf": "nginx",
        "gitignore": "gitignore",
        "diff": "diff",
        "patch": "diff",
        "rb": "ruby",
        "ruby": "ruby",
        "hcl": "hcl",
        "tf": "terraform",
        "terraform": "terraform",
        "vue": "vue",
        "svelte": "svelte",
    ]

    private static let yamlNonZeroWidthFile = "yaml(non zero width).json"
    private var didLoad = false
    private var engine: HighlightEngine?

    deinit {
        engine?.close()
    }

    func loadIfNeeded(theme: HighlightTheme) async {
        guard !didLoad else { return }
        didLoad = true
        isLoading = true
        defer { isLoading = false }

        do {
            let startedAt = DispatchTime.now().uptimeNanoseconds
            let syntaxDirectory = try Self.requiredBundleDirectory("syntaxes")
            let filesDirectory = try Self.requiredBundleDirectory("files")
            let markdownURL = filesDirectory.appendingPathComponent("example.md")
            let markdown = try String(contentsOf: markdownURL, encoding: .utf8)

            let engine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
            self.engine = engine
            try registerStyleNames(engine)
            try engine.defineMacro("ANDROID")
            let compiledCount = try compileCommonSyntaxes(in: syntaxDirectory, engine: engine)

            attributedText = render(markdown: markdown, engine: engine, theme: theme)
            let elapsedMillis = Int((DispatchTime.now().uptimeNanoseconds - startedAt) / 1_000_000)
            status = "Compiled \(compiledCount) syntax rule files in \(elapsedMillis) ms | File: example.md"
        } catch {
            status = "Markdown failed: \(error)"
            attributedText = NSAttributedString(string: "Markdown failed: \(error)")
        }
    }

    private func render(markdown: String, engine: HighlightEngine, theme: HighlightTheme) -> NSAttributedString {
        let output = NSMutableAttributedString()
        let lines = markdown.components(separatedBy: "\n")
        var isInCodeBlock = false
        var codeLanguage = ""
        var codeLines: [String] = []

        for line in lines {
            if line.hasPrefix("```") {
                if isInCodeBlock {
                    appendCodeBlock(codeLines.joined(separator: "\n"), language: codeLanguage, to: output, engine: engine, theme: theme)
                    codeLines.removeAll()
                    codeLanguage = ""
                    isInCodeBlock = false
                } else {
                    isInCodeBlock = true
                    codeLanguage = String(line.dropFirst(3)).trimmingCharacters(in: .whitespacesAndNewlines)
                }
                continue
            }

            if isInCodeBlock {
                codeLines.append(line)
            } else {
                appendMarkdownLine(line, to: output, theme: theme)
            }
        }

        if isInCodeBlock {
            appendCodeBlock(codeLines.joined(separator: "\n"), language: codeLanguage, to: output, engine: engine, theme: theme)
        }

        return output
    }

    private func appendMarkdownLine(_ line: String, to output: NSMutableAttributedString, theme: HighlightTheme) {
        let trimmed = line.trimmingCharacters(in: .whitespaces)
        let font: UIFont
        let text: String

        if trimmed.hasPrefix("# ") {
            font = .boldSystemFont(ofSize: 24)
            text = String(trimmed.dropFirst(2))
        } else if trimmed.hasPrefix("## ") {
            font = .boldSystemFont(ofSize: 20)
            text = String(trimmed.dropFirst(3))
        } else if trimmed.hasPrefix("### ") {
            font = .boldSystemFont(ofSize: 17)
            text = String(trimmed.dropFirst(4))
        } else {
            font = .systemFont(ofSize: 15)
            text = line
        }

        let paragraph = NSMutableParagraphStyle()
        paragraph.lineSpacing = 2
        paragraph.paragraphSpacing = trimmed.isEmpty ? 8 : 4

        output.append(NSAttributedString(
            string: text + "\n",
            attributes: [
                .font: font,
                .foregroundColor: theme.textUIColor,
                .paragraphStyle: paragraph,
            ]
        ))
    }

    private func appendCodeBlock(
        _ code: String,
        language: String,
        to output: NSMutableAttributedString,
        engine: HighlightEngine,
        theme: HighlightTheme
    ) {
        let syntaxName = Self.syntaxNameMapping[language] ?? language
        let highlight: DocumentHighlight?
        if !syntaxName.isEmpty, let analyzer = try? engine.createAnalyzer(syntaxName: syntaxName) {
            highlight = try? analyzer.analyzeText(code)
            analyzer.close()
        } else {
            highlight = nil
        }

        let block = NSMutableAttributedString(
            string: code + "\n\n",
            attributes: [
                .font: UIFont.monospacedSystemFont(ofSize: 14, weight: .regular),
                .foregroundColor: theme.textUIColor,
                .backgroundColor: theme.backgroundUIColor.withAlphaComponent(theme.isDark ? 0.85 : 0.12),
            ]
        )
        apply(highlight: highlight, to: block, source: code, theme: theme)
        output.append(block)
    }

    private func apply(highlight: DocumentHighlight?, to attributed: NSMutableAttributedString, source: String, theme: HighlightTheme) {
        guard let highlight else { return }
        let lines = source.components(separatedBy: "\n")
        let lineStarts = utf16LineStarts(in: lines)

        for lineIndex in highlight.lines.indices {
            guard lineIndex < lines.count else { continue }
            for span in highlight.lines[lineIndex].spans {
                guard let range = nsRange(for: span, lines: lines, lineStarts: lineStarts) else { continue }
                attributed.addAttribute(.foregroundColor, value: theme.foreground(for: span.style), range: range)
            }
        }
    }

    private func compileCommonSyntaxes(in directory: URL, engine: HighlightEngine) throws -> Int {
        let sources = try FileManager.default.contentsOfDirectory(
            at: directory,
            includingPropertiesForKeys: [.isRegularFileKey],
            options: [.skipsHiddenFiles]
        )
        .filter { $0.pathExtension == "json" && !$0.lastPathComponent.hasSuffix("-inlineStyle.json") && $0.lastPathComponent != Self.yamlNonZeroWidthFile }
        .sorted { $0.lastPathComponent < $1.lastPathComponent }
        .map { try SyntaxSource(fileName: $0.lastPathComponent, json: String(contentsOf: $0, encoding: .utf8)) }

        var pending = sources
        var compiledCount = 0
        while !pending.isEmpty {
            var progressed = false
            var nextPending: [SyntaxSource] = []

            for source in pending {
                do {
                    try engine.compileSyntax(fromJson: source.json)
                    compiledCount += 1
                    progressed = true
                } catch let error as SyntaxCompileError where error.code == SyntaxCompileError.importSyntaxNotFound {
                    nextPending.append(source)
                }
            }

            if !progressed {
                break
            }
            pending = nextPending
        }

        return compiledCount
    }

    private func registerStyleNames(_ engine: HighlightEngine) throws {
        try engine.registerStyleName("keyword", id: HighlightTheme.styleKeyword)
        try engine.registerStyleName("string", id: HighlightTheme.styleString)
        try engine.registerStyleName("number", id: HighlightTheme.styleNumber)
        try engine.registerStyleName("comment", id: HighlightTheme.styleComment)
        try engine.registerStyleName("class", id: HighlightTheme.styleClass)
        try engine.registerStyleName("method", id: HighlightTheme.styleMethod)
        try engine.registerStyleName("variable", id: HighlightTheme.styleVariable)
        try engine.registerStyleName("punctuation", id: HighlightTheme.stylePunctuation)
        try engine.registerStyleName("annotation", id: HighlightTheme.styleAnnotation)
        try engine.registerStyleName("preprocessor", id: HighlightTheme.stylePreprocessor)
        try engine.registerStyleName("macro", id: HighlightTheme.styleMacro)
        try engine.registerStyleName("lifetime", id: HighlightTheme.styleLifetime)
        try engine.registerStyleName("selector", id: HighlightTheme.styleSelector)
        try engine.registerStyleName("builtin", id: HighlightTheme.styleBuiltin)
        try engine.registerStyleName("url", id: HighlightTheme.styleURL)
        try engine.registerStyleName("property", id: HighlightTheme.styleProperty)
    }

    private static func requiredBundleDirectory(_ name: String) throws -> URL {
        if let url = Bundle.main.url(forResource: name, withExtension: nil), isDirectory(url) {
            return url
        }
        if let resourceURL = Bundle.main.resourceURL {
            let url = resourceURL.appendingPathComponent(name, isDirectory: true)
            if isDirectory(url) {
                return url
            }
        }
        throw NSError(domain: "MarkdownDemo", code: 1, userInfo: [NSLocalizedDescriptionKey: "Missing bundled resource: \(name)"])
    }

    private static func isDirectory(_ url: URL) -> Bool {
        var isDirectory: ObjCBool = false
        return FileManager.default.fileExists(atPath: url.path, isDirectory: &isDirectory) && isDirectory.boolValue
    }

    private func utf16LineStarts(in lines: [String]) -> [Int] {
        var starts: [Int] = []
        var offset = 0
        for line in lines {
            starts.append(offset)
            offset += line.utf16.count + 1
        }
        return starts
    }

    private func nsRange(for span: TokenSpan, lines: [String], lineStarts: [Int]) -> NSRange? {
        let line = span.range.start.line
        guard lines.indices.contains(line), lineStarts.indices.contains(line) else { return nil }
        let lineText = lines[line]
        let startColumn = max(0, min(span.range.start.column, lineText.count))
        let endColumn = max(startColumn, min(span.range.end.column, lineText.count))
        guard let startOffset = utf16Offset(in: lineText, column: startColumn),
              let endOffset = utf16Offset(in: lineText, column: endColumn) else { return nil }
        return NSRange(location: lineStarts[line] + startOffset, length: endOffset - startOffset)
    }

    private func utf16Offset(in line: String, column: Int) -> Int? {
        let index = line.index(line.startIndex, offsetBy: min(column, line.count))
        guard let utf16Index = index.samePosition(in: line.utf16) else { return nil }
        return line.utf16.distance(from: line.utf16.startIndex, to: utf16Index)
    }
}

private struct SyntaxSource {
    let fileName: String
    let json: String
}
