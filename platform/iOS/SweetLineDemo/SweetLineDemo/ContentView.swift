//
//  ContentView.swift
//  SweetLineDemo
//
//  Created by xiue233 on 2026/5/12.
//

import SwiftUI
import UniformTypeIdentifiers

struct ContentView: View {
    @StateObject private var viewModel = DemoViewModel()
    @State private var isImporterPresented = false

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                controls

                Divider()

                codeArea

                Divider()

                statusBar
            }
            .navigationTitle("SweetLine")
            .navigationBarTitleDisplayMode(.inline)
            .task {
                await viewModel.warmupIfNeeded()
            }
            .fileImporter(isPresented: $isImporterPresented, allowedContentTypes: [.data]) { result in
                switch result {
                case let .success(url):
                    viewModel.openImportedFile(url)
                case let .failure(error):
                    viewModel.status = "Open failed: \(error.localizedDescription)"
                }
            }
        }
    }

    private var controls: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 12) {
                controlPicker(title: "File") {
                    Picker("File", selection: $viewModel.selectedSampleID) {
                        Text("Select a file").tag("")
                        ForEach(viewModel.demoSamples) { sample in
                            Text(sample.displayName).tag(sample.id)
                        }
                    }
                    .pickerStyle(.menu)
                    .frame(minWidth: 190)
                    .disabled(viewModel.isWarmingUp || viewModel.demoSamples.isEmpty)
                    .onChange(of: viewModel.selectedSampleID) { _, newValue in
                        viewModel.highlightSelectedSample(newValue)
                    }
                }

                controlPicker(title: "Theme") {
                    Picker("Theme", selection: $viewModel.selectedThemeIndex) {
                        ForEach(viewModel.themes.indices, id: \.self) { index in
                            Text(viewModel.themes[index].name).tag(index)
                        }
                    }
                    .pickerStyle(.menu)
                    .frame(minWidth: 170)
                }

                Button {
                    isImporterPresented = true
                } label: {
                    Label("Open", systemImage: "folder")
                }
                .buttonStyle(.bordered)
                .disabled(viewModel.isWarmingUp)

                NavigationLink {
                    MarkdownDemoView(theme: viewModel.currentTheme)
                } label: {
                    Label("Markdown", systemImage: "doc.richtext")
                }
                .buttonStyle(.bordered)
                .disabled(viewModel.isWarmingUp)

                if viewModel.isWarmingUp {
                    ProgressView()
                }
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 8)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.bar)
    }

    private func controlPicker<Content: View>(title: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(title)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)
            content()
        }
    }

    private var codeArea: some View {
        HighlightedCodeView(
            source: viewModel.sourceCode,
            highlight: viewModel.highlight,
            indentGuides: viewModel.indentGuides,
            theme: viewModel.currentTheme,
            onTextChange: viewModel.applyTextEdit(range:replacementText:)
        )
        .background(viewModel.currentTheme.background)
    }

    private var statusBar: some View {
        HStack(spacing: 8) {
            if viewModel.isWarmingUp {
                ProgressView()
                    .controlSize(.small)
            }
            Text(viewModel.status)
                .font(.caption.monospaced())
                .lineLimit(2)
                .minimumScaleFactor(0.75)
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 7)
        .background(.bar)
    }
}

#Preview {
    ContentView()
}
    
