// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "SweetLineMacDemo",
    platforms: [
        .macOS(.v14),
    ],
    dependencies: [
        .package(name: "Apple", path: ".."),
    ],
    targets: [
        .target(
            name: "SweetLineDemoSupport",
            dependencies: [
                .product(name: "SweetLineMacOS", package: "Apple"),
            ]
        ),
        .executableTarget(
            name: "SweetLineMacDemo",
            dependencies: [
                .product(name: "SweetLineMacOS", package: "Apple"),
                "SweetLineDemoSupport",
            ],
            linkerSettings: [
                .linkedLibrary("iconv"),
                .linkedLibrary("c++"),
            ]
        ),
        .testTarget(
            name: "SweetLineDemoSupportTests",
            dependencies: [
                "SweetLineDemoSupport",
                .product(name: "SweetLineMacOS", package: "Apple"),
            ],
            path: "Tests/SweetLineDemoSupportTests",
            linkerSettings: [
                .linkedLibrary("iconv"),
                .linkedLibrary("c++"),
            ]
        ),
    ],
    swiftLanguageVersions: [.v5]
)
