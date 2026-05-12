// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "SweetLine",
    platforms: [
        .macOS(.v14),
    ],
    products: [
        .library(name: "SweetLine", targets: ["SweetLine"]),
    ],
    targets: [
        .binaryTarget(
            name: "SweetLineCoreOSX",
            path: "Vendor/macOS/SweetLineCoreOSX.xcframework"
        ),
        .target(
            name: "SweetLine",
            dependencies: ["SweetLineCoreOSX"]
        ),
        .testTarget(
            name: "SweetLineTests",
            dependencies: ["SweetLine"]
        ),
    ]
)
