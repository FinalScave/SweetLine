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
            name: "SweetLineCoreIOS",
            path: "binaries/SweetLineCoreIOS.xcframework"
        ),
        .binaryTarget(
            name: "SweetLineCoreOSX",
            path: "binaries/SweetLineCoreOSX.xcframework"
        ),
        .target(
            name: "SweetLineBridge",
            dependencies: [
                .target(name: "SweetLineCoreIOS", condition: .when(platforms: [.iOS])),
                .target(name: "SweetLineCoreOSX", condition: .when(platforms: [.macOS])),
            ],
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
            dependencies: ["SweetLineMacOS"],
            path: "Tests/SweetLineAppleTests"
        ),
    ],
    swiftLanguageVersions: [.v5]
)
