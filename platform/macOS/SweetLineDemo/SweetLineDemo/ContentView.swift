//
//  ContentView.swift
//  SweetLineDemo
//
//  Created by xiue233 on 2026/5/12.
//

import SwiftUI

struct ContentView: View {
    @StateObject private var viewModel = DemoViewModel()

    var body: some View {
        VStack(spacing: 0) {
            toolbar

            Divider()

            codeArea

            Divider()

            statusBar
        }
        .frame(minWidth: 920, minHeight: 640)
        .task {
            await viewModel.warmupIfNeeded()
        }
    }

    private var toolbar: some View {
        HStack(spacing: 12) {
            Text("File")
                .font(.callout.weight(.semibold))

            Picker("File", selection: $viewModel.selectedFileName) {
                Text("Select a file").tag("")
                ForEach(viewModel.exampleFiles, id: \.self) { file in
                    Text(file).tag(file)
                }
            }
            .labelsHidden()
            .frame(width: 260)
            .disabled(viewModel.isWarmingUp || viewModel.exampleFiles.isEmpty)
            .onChange(of: viewModel.selectedFileName) { _, newValue in
                viewModel.highlightSelectedFile(newValue)
            }

            Text("Theme")
                .font(.callout.weight(.semibold))

            Picker("Theme", selection: $viewModel.selectedThemeIndex) {
                ForEach(viewModel.themes.indices, id: \.self) { index in
                    Text(viewModel.themes[index].name).tag(index)
                }
            }
            .labelsHidden()
            .frame(width: 190)

            Button("Open...") {
                viewModel.openExternalFile()
            }
            .disabled(viewModel.isWarmingUp)

            Spacer()
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }

    private var codeArea: some View {
        GeometryReader { proxy in
            let theme = viewModel.currentTheme
            let contentSize = HighlightedCodeView.preferredSize(
                source: viewModel.sourceCode,
                minimumSize: proxy.size
            )

            ScrollView([.horizontal, .vertical]) {
                HighlightedCodeView(
                    source: viewModel.sourceCode,
                    highlight: viewModel.highlight,
                    indentGuides: viewModel.indentGuides,
                    theme: theme
                )
                .frame(width: contentSize.width, height: contentSize.height, alignment: .topLeading)
            }
            .background(theme.background)
        }
    }

    private var statusBar: some View {
        HStack {
            if viewModel.isWarmingUp {
                ProgressView()
                    .controlSize(.small)
            }
            Text(viewModel.status)
                .font(.caption.monospaced())
                .lineLimit(1)
            Spacer()
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
    }
}

#Preview {
    ContentView()
}
