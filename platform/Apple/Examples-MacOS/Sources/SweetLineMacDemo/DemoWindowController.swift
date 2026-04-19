import AppKit
import SweetLineMacOS
import SweetLineDemoSupport

final class DemoWindowController: NSWindowController, NSToolbarDelegate {
    private enum ToolbarItemIdentifier {
        static let sample = NSToolbarItem.Identifier("sample")
        static let theme = NSToolbarItem.Identifier("theme")
    }

    private let codeView = CodeView(frame: .zero)
    private let statusBarView = DemoStatusBarView(frame: .zero)
    private let themePopup = NSPopUpButton(frame: .zero, pullsDown: false)
    private let sampleLabel = NSTextField(labelWithString: "")

    private var currentModel: DemoRenderModel?
    private var availableThemes: [DemoTheme] = DemoSampleSupport.builtinThemes

    convenience init() {
        let window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 1180, height: 760),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )
        self.init(window: window)
    }

    override init(window: NSWindow?) {
        super.init(window: window)
        configureWindow()
        configureContent()
        loadDemo()
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        nil
    }

    func loadDemo() {
        do {
            apply(state: try DemoSampleSupport.loadDefaultState())
        } catch {
            let fallbackSample = DemoSampleSupport.builtinSamples.first ?? DemoSample(fileName: "example.swift")
            apply(state: DemoSampleSupport.makeLoadState(sample: fallbackSample))
        }
    }

    @objc func didChangeTheme(_ sender: NSPopUpButton) {
        guard let selectedIndex = sender.indexOfSelectedItem as Int?, selectedIndex >= 0, selectedIndex < availableThemes.count else {
            return
        }
        let theme = availableThemes[selectedIndex]
        guard let model = currentModel else {
            return
        }

        let nextModel = DemoSampleSupport.selectTheme(id: theme.id, in: model)
        apply(state: .ready(nextModel))
    }

    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        [ToolbarItemIdentifier.sample, .flexibleSpace, ToolbarItemIdentifier.theme]
    }

    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        [ToolbarItemIdentifier.sample, .flexibleSpace, ToolbarItemIdentifier.theme]
    }

    func toolbar(
        _ toolbar: NSToolbar,
        itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier,
        willBeInsertedIntoToolbar flag: Bool
    ) -> NSToolbarItem? {
        let item = NSToolbarItem(itemIdentifier: itemIdentifier)

        switch itemIdentifier {
        case ToolbarItemIdentifier.sample:
            sampleLabel.font = .systemFont(ofSize: 13, weight: .medium)
            sampleLabel.textColor = .labelColor
            item.label = "Sample"
            item.view = sampleLabel
        case ToolbarItemIdentifier.theme:
            themePopup.target = self
            themePopup.action = #selector(didChangeTheme(_:))
            item.label = "Theme"
            item.view = themePopup
        default:
            return nil
        }

        return item
    }

    private func configureWindow() {
        guard let window else {
            return
        }
        window.title = "SweetLine macOS Demo"
        window.center()
        window.toolbarStyle = .unified

        let toolbar = NSToolbar(identifier: "SweetLineMacDemoToolbar")
        toolbar.delegate = self
        toolbar.displayMode = .iconOnly
        window.toolbar = toolbar
    }

    private func configureContent() {
        guard let window else {
            return
        }

        let rootView = NSView(frame: NSRect(x: 0, y: 0, width: 1180, height: 760))
        rootView.translatesAutoresizingMaskIntoConstraints = false

        let scrollView = NSScrollView(frame: .zero)
        scrollView.translatesAutoresizingMaskIntoConstraints = false
        scrollView.hasVerticalScroller = true
        scrollView.hasHorizontalScroller = true
        scrollView.borderType = .noBorder
        scrollView.drawsBackground = false
        scrollView.documentView = codeView

        codeView.translatesAutoresizingMaskIntoConstraints = false

        rootView.addSubview(scrollView)
        rootView.addSubview(statusBarView)

        NSLayoutConstraint.activate([
            scrollView.leadingAnchor.constraint(equalTo: rootView.leadingAnchor),
            scrollView.trailingAnchor.constraint(equalTo: rootView.trailingAnchor),
            scrollView.topAnchor.constraint(equalTo: rootView.topAnchor),
            scrollView.bottomAnchor.constraint(equalTo: statusBarView.topAnchor),

            statusBarView.leadingAnchor.constraint(equalTo: rootView.leadingAnchor),
            statusBarView.trailingAnchor.constraint(equalTo: rootView.trailingAnchor),
            statusBarView.bottomAnchor.constraint(equalTo: rootView.bottomAnchor),
        ])

        window.contentView = rootView
    }

    private func apply(state: DemoLoadState) {
        switch state {
        case .ready(let model):
            currentModel = model
            availableThemes = model.availableThemes
            sampleLabel.stringValue = "Sample: \(model.sample.fileName)"
            updateThemeMenu(themes: model.availableThemes, selectedName: model.selectedTheme.name)
            codeView.apply(model: makeCodeRenderModel(from: model))
            statusBarView.applyReadyStatus(model.status)
        case .failed(let message, let sampleName, let availableThemes):
            currentModel = nil
            self.availableThemes = availableThemes
            sampleLabel.stringValue = "Sample: \(sampleName)"
            updateThemeMenu(themes: availableThemes, selectedName: availableThemes.first?.name)
            codeView.applyError(message)
            statusBarView.applyErrorStatus(message)
        }
    }

    private func makeCodeRenderModel(from model: DemoRenderModel) -> CodeRenderModel {
        CodeRenderModel(
            sourceText: model.sourceText,
            highlight: model.highlight,
            indentGuides: model.indentGuides,
            theme: HighlightTheme(
                name: model.selectedTheme.name,
                backgroundColorARGB: model.selectedTheme.backgroundColorARGB,
                textColorARGB: model.selectedTheme.textColorARGB,
                colorMap: model.selectedTheme.colorMap
            )
        )
    }

    private func updateThemeMenu(themes: [DemoTheme], selectedName: String?) {
        themePopup.removeAllItems()
        themePopup.addItems(withTitles: themes.map(\.name))
        if let selectedName {
            themePopup.selectItem(withTitle: selectedName)
        }
    }
}
