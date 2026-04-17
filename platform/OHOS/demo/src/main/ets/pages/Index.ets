import { sweetline } from "@qiplat/sweetline";
import { ResourceUtils } from "./ResourceUtils";
import { HighlightTheme } from "./HighlightTheme";

const TAG = "SampleHighlight";

interface SpanItem {
  text: string;
  color: number;
}

const EXCLUDED_COMMON_SYNTAX_FILES: string[] = [
  "yaml(non zero width).json",
];

function isDemoSampleFile(fileName: string): boolean {
  return fileName.startsWith("example") || fileName === "json-sweetline.json";
}

function isCommonSyntaxFile(fileName: string): boolean {
  return fileName.endsWith(".json")
    && !fileName.startsWith("example")
    && !fileName.endsWith("-inlineStyle.json")
    && !EXCLUDED_COMMON_SYNTAX_FILES.includes(fileName);
}

@Entry
@Component
struct Index {
  private commonEngine: sweetline.HighlightEngine =
    new sweetline.HighlightEngine(new sweetline.HighlightConfig(true, false));
  private documentAnalyzer: sweetline.DocumentAnalyzer | null = null;
  private themes: HighlightTheme[] = HighlightTheme.builtinThemes();
  private hasPreparedSyntaxes: boolean = false;
  private isPreparingSyntaxes: boolean = false;
  private syntaxWarmupSummary: string = '';
  @State currentThemeIndex: number = 0;
  @State currentTheme: HighlightTheme = HighlightTheme.sweetLineDark();
  @State exampleFiles: string[] = [];
  @State selectedFileIndex: number = 0;
  @State highlightSpans: SpanItem[] = [];
  @State analyzeTime: string = '';
  private renderVersion: number = 0;

  aboutToAppear(): void {
    if (this.hasPreparedSyntaxes || this.isPreparingSyntaxes) {
      return;
    }
    this.registerStyleNames(this.commonEngine);
    this.commonEngine.defineMacro("OHOS");
    this.isPreparingSyntaxes = true;
    this.analyzeTime = 'Compiling demo syntaxes...';
    void this.prepareDemoSyntaxes();
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

  private registerStyleNames(engine: sweetline.HighlightEngine): void {
    engine.registerStyleName("keyword", 1);
    engine.registerStyleName("string", 2);
    engine.registerStyleName("number", 3);
    engine.registerStyleName("comment", 4);
    engine.registerStyleName("class", 5);
    engine.registerStyleName("method", 6);
    engine.registerStyleName("variable", 7);
    engine.registerStyleName("punctuation", 8);
    engine.registerStyleName("annotation", 9);
    engine.registerStyleName("preprocessor", 10);
    engine.registerStyleName("macro", 11);
    engine.registerStyleName("lifetime", 12);
    engine.registerStyleName("selector", 13);
    engine.registerStyleName("builtin", 14);
    engine.registerStyleName("url", 15);
    engine.registerStyleName("property", 16);
  }

  private async prepareDemoSyntaxes(): Promise<void> {
    try {
      const context = this.getUIContext().getHostContext();
      if (context == undefined) {
        this.analyzeTime = 'Failed to access demo resources';
        return;
      }
      const rawFileList = context.resourceManager.getRawFileListSync("");
      const syntaxFiles: string[] = [];
      for (const fileName of rawFileList) {
        if (isCommonSyntaxFile(fileName)) {
          syntaxFiles.push(fileName);
        }
      }
      syntaxFiles.sort();

      const startedAt = Date.now();
      const compiledCount = await this.precompileCommonSyntaxes(context, syntaxFiles);
      const elapsed = Date.now() - startedAt;
      this.syntaxWarmupSummary = `Compiled ${compiledCount} syntax rule files in ${elapsed}ms`;
      this.analyzeTime = this.syntaxWarmupSummary;
      this.hasPreparedSyntaxes = true;
      this.loadExampleFiles(rawFileList);
    } catch (error) {
      const detail = this.describeError(error);
      this.analyzeTime = `Failed to compile demo syntaxes: ${detail}`;
      console.error(TAG, `Failed to precompile demo syntaxes: ${detail}`);
    } finally {
      this.isPreparingSyntaxes = false;
    }
  }

  private async precompileCommonSyntaxes(context: Context | undefined, syntaxFiles: string[]): Promise<number> {
    let syntaxJsonMap: Map<string, string> = new Map();
    for (const syntaxFile of syntaxFiles) {
      syntaxJsonMap.set(syntaxFile, await ResourceUtils.readRawTextFile(context, syntaxFile));
    }

    let pending: string[] = syntaxFiles.slice();
    let compiledCount = 0;
    while (pending.length > 0) {
      let nextPending: string[] = [];
      let madeProgress = false;

      for (const syntaxFile of pending) {
        try {
          const syntaxJson = syntaxJsonMap.get(syntaxFile);
          this.commonEngine.compileSyntaxFromJson(syntaxJson !== undefined ? syntaxJson : '');
          compiledCount++;
          madeProgress = true;
        } catch (error) {
          if (error instanceof sweetline.SyntaxCompileError
            && error.errorCode === sweetline.SyntaxCompileError.ERR_IMPORT_SYNTAX_NOT_FOUND) {
            nextPending.push(syntaxFile);
            continue;
          }
          throw new Error(`${syntaxFile}: ${this.describeError(error)}`);
        }
      }

      if (!madeProgress && nextPending.length > 0) {
        throw new Error(`Unresolved syntax imports: ${nextPending.join(', ')}`);
      }
      pending = nextPending;
    }
    return compiledCount;
  }

  private describeError(error: Object): string {
    if (error instanceof Error) {
      return error.message;
    }
    return `${error}`;
  }

  /**
   * Load example file list
   */
  private loadExampleFiles(rawFileList?: string[]): void {
    try {
      let files: string[] = [];
      let sourceFiles: string[] = [];
      if (rawFileList !== undefined) {
        sourceFiles = rawFileList;
      } else {
        const context = this.getUIContext().getHostContext();
        if (context != undefined) {
          sourceFiles = context.resourceManager.getRawFileListSync("");
        }
      }
      for (const name of sourceFiles) {
        if (isDemoSampleFile(name)) {
          files.push(name);
        }
      }
      files.sort();
      this.exampleFiles = files;
      if (files.length > 0) {
        this.selectedFileIndex = 0;
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
    if (!this.hasPreparedSyntaxes) {
      return;
    }
    void this.highlight(testFileName);
  }

  /**
   * Perform highlight analysis
   */
  private async highlight(testFileName: string): Promise<void> {
    try {
      const context = this.getUIContext().getHostContext();
      const testCode = await ResourceUtils.readRawTextFile(context, testFileName);
      const document = new sweetline.Document(testFileName, testCode);

      let loadStart = Date.now();
      this.documentAnalyzer = this.commonEngine.loadDocument(document);
      let loadTime = Date.now() - loadStart;
      console.info(TAG, `====loadDocument: ${loadTime}ms`);
      if (this.documentAnalyzer == null) {
        this.analyzeTime = this.syntaxWarmupSummary.length > 0
          ? `${this.syntaxWarmupSummary} | No matching syntax for ${testFileName}`
          : `No matching syntax for ${testFileName}`;
        this.renderVersion++;
        this.highlightSpans = testCode.length > 0 ? [{ text: testCode, color: this.currentTheme.textColor }] : [];
        return;
      }

      let analyzeStart = Date.now();
      const highlight = this.documentAnalyzer.analyze();
      let analyzeElapsed = Date.now() - analyzeStart;
      console.info(TAG, `====analyze: ${analyzeElapsed}ms`);
      this.analyzeTime = this.syntaxWarmupSummary.length > 0
        ? `${this.syntaxWarmupSummary} | Load: ${loadTime}ms | Analyze: ${analyzeElapsed}ms`
        : `Load: ${loadTime}ms | Analyze: ${analyzeElapsed}ms`;

      if (highlight) {
        this.buildSpanItems(testCode, highlight);
      }
    } catch (e) {
      console.error(TAG, `Highlight analysis error: ${e}`);
    }
  }

  /**
   * Build highlight results into SpanItem array for rendering
   */
  private buildSpanItems(fullText: string, highlight: sweetline.DocumentHighlight): void {
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

        items.push({
          text: lineText.substring(startCol, endCol),
          color: this.currentTheme.getColor(span.styleId)
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
