// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "SweetLineMacDemo",
    platforms: [
        .macOS(.v14),
    ],
    products: [
        .executable(name: "SweetLineMacDemo", targets: ["SweetLineMacDemo"]),
    ],
    dependencies: [
        .package(name: "SweetLineApple", path: ".."),
    ],
    targets: [
        .executableTarget(
            name: "SweetLineMacDemo",
            dependencies: [
                .product(name: "SweetLine", package: "SweetLineApple"),
            ],
            path: "SweetLineMacDemo",
            resources: [
                .process("Assets.xcassets"),
            ]
        ),
    ],
    swiftLanguageVersions: [.v5]
)
