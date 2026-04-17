import { sweetline } from "@qiplat/sweetline";
import { ResourceUtils } from "./ResourceUtils";
import { HighlightTheme } from "./HighlightTheme";

const TAG = "SampleHighlight";

interface SpanItem {
  text: string;
  color: number;
}

const EXT_SYNTAX_MAP: Map<string, string> = new Map([
  [".t", "tiecode.json"],
  [".c", "c.json"],
  [".cpp", "cpp.json"],
  [".cs", "csharp.json"],
  [".dart", "dart.json"],
  [".go", "go.json"],
  [".groovy", "groovy.json"],
  [".html", "html.json"],
  [".java", "java.json"],
  [".js", "javascript.json"],
  [".json", "json-sweetline.json"],
  [".jsonc", "jsonc.json"],
  [".json5", "json5.json"],
  [".kt", "kotlin.json"],
  [".lua", "lua.json"],
  [".m", "objc.json"],
  [".php", "php.json"],
  [".ps1", "powershell.json"],
  [".py", "python.json"],
  [".rs", "rust.json"],
  [".sh", "shell.json"],
  [".sql", "sql.json"],
  [".swift", "swift.json"],
  [".toml", "toml.json"],
  [".ts", "typescript.json"],
  [".vb", "vb.json"],
  [".xml", "xml.json"],
  [".yaml", "yaml.json"],
  [".md", "markdown.json"],
  [".wenyan", "wenyan.json"],
  [".myu", "iapp.json"],
  [".css", "css.json"],
  [".scss", "scss.json"],
  [".less", "less.json"],
  [".cmake", "cmake.json"],
  [".dockerfile", "dockerfile.json"],
  [".mk", "makefile.json"],
  [".properties", "properties.json"],
  [".env", "env.json"],
  [".proto", "protobuf.json"],
  [".graphql", "graphql.json"],
  [".gql", "graphql.json"],
  [".nginx", "nginx.json"],
  [".conf", "nginx.json"],
  [".gitignore", "gitignore.json"],
  [".diff", "diff.json"],
  [".patch", "diff.json"],
  [".rb", "ruby.json"],
  [".rake", "ruby.json"],
  [".gemspec", "ruby.json"],
  [".ru", "ruby.json"],
  [".hcl", "hcl.json"],
  [".tf", "terraform.json"],
  [".tfvars", "terraform.json"],
  [".tfbackend", "terraform.json"],
  [".vue", "vue.json"],
  [".svelte", "svelte.json"],
]);

@Entry
@Component
struct Index {
  private inlineStyleEngine: sweetline.HighlightEngine =
    new sweetline.HighlightEngine(new sweetline.HighlightConfig(true, true));
  private commonEngine: sweetline.HighlightEngine =
    new sweetline.HighlightEngine(new sweetline.HighlightConfig(true, false));
  private documentAnalyzer: sweetline.DocumentAnalyzer | null = null;
  private themes: HighlightTheme[] = HighlightTheme.builtinThemes();
  @State currentThemeIndex: number = 0;
  @State currentTheme: HighlightTheme = HighlightTheme.sweetLineDark();
  @State exampleFiles: string[] = [];
  @State selectedFileIndex: number = 0;
  @State highlightSpans: SpanItem[] = [];
  @State analyzeTime: string = '';
  private renderVersion: number = 0;

  aboutToAppear(): void {
    this.commonEngine.registerStyleName("keyword", 1);
    this.commonEngine.registerStyleName("string", 2);
    this.commonEngine.registerStyleName("number", 3);
    this.commonEngine.registerStyleName("comment", 4);
    this.commonEngine.registerStyleName("class", 5);
    this.commonEngine.registerStyleName("method", 6);
    this.commonEngine.registerStyleName("variable", 7);
    this.commonEngine.registerStyleName("punctuation", 8);
    this.commonEngine.registerStyleName("annotation", 9);
    this.commonEngine.registerStyleName("preprocessor", 10);
    this.commonEngine.registerStyleName("macro", 11);
    this.commonEngine.registerStyleName("lifetime", 12);
    this.commonEngine.registerStyleName("selector", 13);
    this.commonEngine.registerStyleName("builtin", 14);
    this.commonEngine.registerStyleName("url", 15);
    this.commonEngine.registerStyleName("property", 16);

    this.commonEngine.defineMacro("OHOS");
    this.inlineStyleEngine.defineMacro("OHOS");

    this.loadExampleFiles();
  }

  build() {
    Column() {
      // Top toolbar
      Row() {
        // Theme name
        Text(this.currentTheme.name)
          .fontSize(12)
          .fontColor(this.argbToColor(this.currentTheme.textColor))
          .padding({ left: 8, right: 8 })
          .layoutWeight(1)
          .onClick(() => {
            this.showThemeDialog();
          })

        // File selector
        Select(this.exampleFiles.map((file, index) => {
          return { value: file } as SelectOption;
        }))
          .selected(this.selectedFileIndex)
          .value(this.exampleFiles.length > 0 ? this.exampleFiles[this.selectedFileIndex] : '')
          .font({ size: 12 })
          .fontColor(this.argbToColor(this.currentTheme.textColor))
          .selectedOptionFont({ size: 12 })
          .optionFont({ size: 12 })
          .constraintSize({ maxWidth: 200 })
          .onSelect((index: number) => {
            this.selectedFileIndex = index;
            this.highlightFile(this.exampleFiles[index]);
          })
      }
      .width('100%')
      .height(48)
      .padding({ left: 8, right: 8 })
      .justifyContent(FlexAlign.SpaceBetween)

      // Code highlight display area
      Scroll() {
        Text() {
          ForEach(this.highlightSpans, (item: SpanItem, index: number) => {
            Span(item.text)
              .fontColor(this.argbToColor(item.color))
              .fontSize(12)
              .fontFamily('monospace')
          }, (item: SpanItem, index: number) => `${this.renderVersion}_${index}`)
        }
        .width('100%')
        .padding(8)
      }
      .layoutWeight(1)
      .width('100%')
      .scrollBar(BarState.Auto)
      .edgeEffect(EdgeEffect.Spring)

      // Bottom status bar
      Row() {
        Text(this.analyzeTime)
          .fontSize(11)
          .fontColor(this.argbToColor(this.currentTheme.textColor))
          .opacity(0.7)
      }
      .width('100%')
      .height(32)
      .padding({ left: 8, right: 8 })
      .justifyContent(FlexAlign.End)
    }
    .width('100%')
    .height('100%')
    .backgroundColor(this.argbToColor(this.currentTheme.backgroundColor))
  }

  /**
   * Show theme selection dialog
   */
  private showThemeDialog(): void {
    let sheets: Array<SheetInfo> = this.themes.map((theme, index) => {
      let info: SheetInfo = {
        title: theme.name,
        action: () => {
          this.currentThemeIndex = index;
          this.currentTheme = this.themes[index];
          this.reHighlightCurrentFile();
        }
      };
      return info;
    });
    ActionSheet.show({
      title: 'Select Highlight Theme',
      message: '',
      sheets: sheets,
      cancel: () => {
      }
    });
  }

  /**
   * Load example file list
   */
  private loadExampleFiles(): void {
    try {
      const context = this.getUIContext().getHostContext();
      if (context == undefined) {
        return;
      }
      const rawFileList = context.resourceManager.getRawFileListSync("");
      let files: string[] = [];
      for (const name of rawFileList) {
        if (!name.startsWith("example") && name !== "json-sweetline.json") {
          continue;
        }
        const ext = this.getFileExtension(name);
        if (EXT_SYNTAX_MAP.has(ext)) {
          files.push(name);
        }
      }
      files.sort();
      this.exampleFiles = files;
      if (files.length > 0) {
        this.highlightFile(files[0]);
      }
    } catch (error) {
      console.error(TAG, `Failed to list rawfile files: ${error}`);
    }
  }

  /**
   * Re-highlight the currently selected file
   */
  private reHighlightCurrentFile(): void {
    if (this.selectedFileIndex >= 0 && this.selectedFileIndex < this.exampleFiles.length) {
      this.highlightFile(this.exampleFiles[this.selectedFileIndex]);
    }
  }

  /**
   * Highlight the specified file
   */
  private highlightFile(testFileName: string): void {
    const ext = this.getFileExtension(testFileName);
    const syntaxFileName = EXT_SYNTAX_MAP.get(ext);
    if (!syntaxFileName) {
      console.warn(TAG, `Syntax rule file not found for the given test file`);
      return;
    }
    this.highlight(testFileName, syntaxFileName, false);
  }

  /**
   * Perform highlight analysis
   */
  private async highlight(testFileName: string, syntaxFileName: string, inlineStyle: boolean): Promise<void> {
    try {
      const context = this.getUIContext().getHostContext();
      const syntaxJson = await ResourceUtils.readRawTextFile(context, syntaxFileName);
      const testCode = await ResourceUtils.readRawTextFile(context, testFileName);

      let compileStart = Date.now();
      if (inlineStyle) {
        this.inlineStyleEngine.compileSyntaxFromJson(syntaxJson);
      } else {
        this.commonEngine.compileSyntaxFromJson(syntaxJson);
      }
      let compileTime = Date.now() - compileStart;
      console.info(TAG, `====compileSyntaxFromJson: ${compileTime}ms`);

      const document = new sweetline.Document(testFileName, testCode);

      let loadStart = Date.now();
      if (inlineStyle) {
        this.documentAnalyzer = this.inlineStyleEngine.loadDocument(document);
      } else {
        this.documentAnalyzer = this.commonEngine.loadDocument(document);
      }
      let loadTime = Date.now() - loadStart;
      console.info(TAG, `====loadDocument: ${loadTime}ms`);

      let analyzeStart = Date.now();
      const highlight = this.documentAnalyzer?.analyze();
      let analyzeElapsed = Date.now() - analyzeStart;
      console.info(TAG, `====analyze: ${analyzeElapsed}ms`);

      this.analyzeTime = `Compile: ${compileTime}ms | Load: ${loadTime}ms | Analyze: ${analyzeElapsed}ms`;

      if (highlight) {
        this.buildSpanItems(testCode, highlight, inlineStyle);
      }
    } catch (e) {
      console.error(TAG, `Highlight analysis error: ${e}`);
    }
  }

  /**
   * Build highlight results into SpanItem array for rendering
   */
  private buildSpanItems(fullText: string, highlight: sweetline.DocumentHighlight, inlineStyle: boolean): void {
    let items: SpanItem[] = [];
    const lines = fullText.split('\n');

    // Build mapping by actual line numbers, highlight.lines excludes lines without tokens
    let lineHighlightMap = new Map<number, sweetline.LineHighlight>();
    for (let i = 0; i < highlight.lines.length; i++) {
      const lh = highlight.lines[i];
      if (lh.spans.length > 0) {
        const actualLine = lh.spans[0].range.start.line;
        lineHighlightMap.set(actualLine, lh);
      }
    }

    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
      const lineText = lines[lineNum];

      if (lineNum > 0) {
        items.push({ text: '\n', color: this.currentTheme.textColor });
      }

      const lineHighlight = lineHighlightMap.get(lineNum);
      if (!lineHighlight) {
        // No highlight token for this line, use default color
        if (lineText.length > 0) {
          items.push({ text: lineText, color: this.currentTheme.textColor });
        }
        continue;
      }

      let lastCol = 0;
      for (let spanIdx = 0; spanIdx < lineHighlight.spans.length; spanIdx++) {
        const span = lineHighlight.spans[spanIdx];
        const startCol = span.range.start.column;
        // Cross-line span: limit endCol to current line length
        const endCol = span.range.start.line === span.range.end.line
          ? Math.min(span.range.end.column, lineText.length)
          : lineText.length;

        if (startCol > lastCol) {
          items.push({
            text: lineText.substring(lastCol, startCol),
            color: this.currentTheme.textColor
          });
        }

        let color: number;
        if (inlineStyle && span.inlineStyle != null) {
          color = span.inlineStyle.foreground != 0 ? span.inlineStyle.foreground : this.currentTheme.textColor;
        } else {
          color = this.currentTheme.getColor(span.styleId);
        }

        items.push({
          text: lineText.substring(startCol, endCol),
          color: color
        });
        lastCol = endCol;
      }

      if (lastCol < lineText.length) {
        items.push({
          text: lineText.substring(lastCol),
          color: this.currentTheme.textColor
        });
      }
    }

    this.renderVersion++;
    this.highlightSpans = items;
  }

  /**
   * Get file extension
   */
  private getFileExtension(fileName: string): string {
    const dotIndex = fileName.lastIndexOf('.');
    if (dotIndex >= 0) {
      return fileName.substring(dotIndex);
    }
    return '';
  }

  /**
   * Convert ARGB integer color value to ArkTS Color
   */
  private argbToColor(argb: number): string {
    const a = (argb >>> 24) & 0xFF;
    const r = (argb >>> 16) & 0xFF;
    const g = (argb >>> 8) & 0xFF;
    const b = argb & 0xFF;
    return `rgba(${r},${g},${b},${(a / 255).toFixed(2)})`;
  }
}
