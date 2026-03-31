import AppKit

@main
final class SweetLineMacDemoApp: NSObject, NSApplicationDelegate {
    private var windowController: DemoWindowController?

    static func main() {
        let application = NSApplication.shared
        let delegate = SweetLineMacDemoApp()
        application.setActivationPolicy(.regular)
        application.delegate = delegate
        application.run()
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        let controller = DemoWindowController()
        controller.showWindow(nil)
        NSApp.activate(ignoringOtherApps: true)
        windowController = controller
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }
}
