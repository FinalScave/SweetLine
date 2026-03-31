// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "SweetLineApple",
    platforms: [
        .macOS(.v14),
        .iOS(.v13),
    ],
    products: [
        .library(
            name: "SweetLineMacOS",
            targets: ["SweetLineMacOS"]
        ),
    ],
    targets: [
        .binaryTarget(
            name: "SweetLineNative",
            path: "binaries/SweetLineNative.xcframework"
        ),
        .target(
            name: "SweetLineBridge",
            dependencies: ["SweetLineNative"],
            publicHeadersPath: "include"
        ),
        .target(
            name: "SweetLineCoreInternal",
            dependencies: ["SweetLineBridge"]
        ),
        .target(
            name: "SweetLineMacOS",
            dependencies: ["SweetLineCoreInternal"]
        ),
        .testTarget(
            name: "SweetLineAppleTests",
            dependencies: ["SweetLineMacOS", "SweetLineNative"],
            path: "Tests/SweetLineAppleTests"
        ),
    ],
    swiftLanguageVersions: [.v5]
)
