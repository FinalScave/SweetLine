// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "SweetLineApple",
    platforms: [
        .iOS(.v14),
        .macOS(.v11),
    ],
    products: [
        .library(name: "SweetLine", targets: ["SweetLine"]),
    ],
    targets: [
        .binaryTarget(
            name: "SweetLineCoreIOS",
            path: "Vendor/iOS/SweetLineCoreIOS.xcframework"
        ),
        .binaryTarget(
            name: "SweetLineCoreMacOS",
            path: "Vendor/macOS/SweetLineCoreMacOS.xcframework"
        ),
        .target(
            name: "SweetLine",
            dependencies: [
                .target(name: "SweetLineCoreIOS", condition: .when(platforms: [.iOS])),
                .target(name: "SweetLineCoreMacOS", condition: .when(platforms: [.macOS])),
            ],
            path: "SweetLine",
            linkerSettings: [
                .linkedLibrary("iconv", .when(platforms: [.iOS])),
            ]
        ),
        .testTarget(
            name: "SweetLineTests",
            dependencies: ["SweetLine"],
            path: "Tests/SweetLineTests"
        ),
    ],
    swiftLanguageVersions: [.v5]
)
