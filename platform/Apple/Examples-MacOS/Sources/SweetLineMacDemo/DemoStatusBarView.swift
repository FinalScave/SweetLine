import AppKit
import SweetLineDemoSupport

final class DemoStatusBarView: NSView {
    private let label = NSTextField(labelWithString: "")

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        translatesAutoresizingMaskIntoConstraints = false
        wantsLayer = true
        layer?.backgroundColor = NSColor.windowBackgroundColor.cgColor

        label.translatesAutoresizingMaskIntoConstraints = false
        label.font = .monospacedSystemFont(ofSize: 12, weight: .regular)
        label.textColor = .secondaryLabelColor
        addSubview(label)

        NSLayoutConstraint.activate([
            heightAnchor.constraint(equalToConstant: 28),
            label.leadingAnchor.constraint(equalTo: leadingAnchor, constant: 12),
            label.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -12),
            label.centerYAnchor.constraint(equalTo: centerYAnchor),
        ])
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        nil
    }

    func applyReadyStatus(_ status: DemoStatusMetrics) {
        label.stringValue = [
            "Compile \(status.compileMicroseconds)us",
            "Analyze \(status.analyzeMicroseconds)us",
            "Lines \(status.lineCount)",
            "Theme \(status.themeName)",
        ].joined(separator: "  |  ")
        label.textColor = .secondaryLabelColor
    }

    func applyErrorStatus(_ message: String) {
        label.stringValue = message
        label.textColor = .systemRed
    }
}
