import { sweetline } from "@qiplat/sweetline";
import { ResourceUtils } from "./ResourceUtils";
import { HighlightTheme } from "./HighlightTheme";

const TAG = "SampleHighlight";
const SYNTAX_RAWFILE_DIR = "syntaxes";
const EXAMPLE_RAWFILE_DIR = "examples";
const SYNTAX_SAMPLE_FILE = "json-sweetline.json";

interface SpanItem {
  text: string;
  color: number;
}

interface BracketRenderToken {
  start: number;
  end: number;
  color: number;
}

const EXCLUDED_COMMON_SYNTAX_FILES: string[] = [
  "yaml(non zero width).json",
];

function isCommonSyntaxFile(fileName: string): boolean {
  return fileName.endsWith(".json")
    && !fileName.endsWith("-inlineStyle.json")
    && !EXCLUDED_COMMON_SYNTAX_FILES.includes(fileName);
}

function rawfilePath(directory: string, fileName: string): string {
  return `${directory}/${fileName}`;
}

@Entry
@Component
struct Index {
  private commonEngine: sweetline.HighlightEngine =
    new sweetline.HighlightEngine(new sweetline.HighlightConfig(true, false));
  private documentAnalyzer: sweetline.DocumentAnalyzer | null = null;
  private readonly bracketPalette: number[] = [
    0xFF7DD3FC,
    0xFFF9A8D4,
    0xFFFDE047,
    0xFF86EFAC,
    0xFFC4B5FD,
    0xFFFDBA74,
  ];
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
      const syntaxFileList = ResourceUtils.listRawFileNames(context, SYNTAX_RAWFILE_DIR);
      const syntaxFiles: string[] = [];
      for (const fileName of syntaxFileList) {
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
      this.loadExampleFiles(undefined, syntaxFileList);
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
      syntaxJsonMap.set(syntaxFile, await ResourceUtils.readRawTextFile(context, rawfilePath(SYNTAX_RAWFILE_DIR, syntaxFile)));
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
  private loadExampleFiles(exampleFileList?: string[], syntaxFileList?: string[]): void {
    try {
      let files: string[] = [];
      let exampleFiles: string[] = [];
      let syntaxFiles: string[] = [];
      if (exampleFileList !== undefined) {
        exampleFiles = exampleFileList;
      } else {
        const context = this.getUIContext().getHostContext();
        if (context != undefined) {
          exampleFiles = ResourceUtils.listRawFileNames(context, EXAMPLE_RAWFILE_DIR);
        }
      }
      if (syntaxFileList !== undefined) {
        syntaxFiles = syntaxFileList;
      } else {
        const context = this.getUIContext().getHostContext();
        if (context != undefined) {
          syntaxFiles = ResourceUtils.listRawFileNames(context, SYNTAX_RAWFILE_DIR);
        }
      }
      for (const name of exampleFiles) {
        files.push(name);
      }
      if (syntaxFiles.includes(SYNTAX_SAMPLE_FILE)) {
        files.push(SYNTAX_SAMPLE_FILE);
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

  private resolveDemoSampleRawfilePath(fileName: string): string {
    if (fileName === SYNTAX_SAMPLE_FILE) {
      return rawfilePath(SYNTAX_RAWFILE_DIR, fileName);
    }
    return rawfilePath(EXAMPLE_RAWFILE_DIR, fileName);
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
      const testCode = await ResourceUtils.readRawTextFile(context, this.resolveDemoSampleRawfilePath(testFileName));
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
      const brackets = this.documentAnalyzer.analyzeBracketPairs();
      let analyzeElapsed = Date.now() - analyzeStart;
      console.info(TAG, `====analyze: ${analyzeElapsed}ms`);
      this.analyzeTime = this.syntaxWarmupSummary.length > 0
        ? `${this.syntaxWarmupSummary} | Load: ${loadTime}ms | Analyze: ${analyzeElapsed}ms`
        : `Load: ${loadTime}ms | Analyze: ${analyzeElapsed}ms`;

      if (highlight) {
        this.buildSpanItems(testCode, highlight, brackets);
      }
    } catch (e) {
      console.error(TAG, `Highlight analysis error: ${e}`);
    }
  }

  /**
   * Build highlight results into SpanItem array for rendering
   */
  private buildSpanItems(fullText: string,
                         highlight: sweetline.DocumentHighlight,
                         brackets: sweetline.BracketPairResult | null): void {
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

    let lineBracketMap = new Map<number, sweetline.LineBracketPairs>();
    if (brackets !== null) {
      for (let i = 0; i < brackets.lines.length; i++) {
        lineBracketMap.set(brackets.startLine + i, brackets.lines[i]);
      }
    }

    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
      const lineText = lines[lineNum];
      const lineBrackets = lineBracketMap.get(lineNum);

      if (lineNum > 0) {
        items.push({ text: '\n', color: this.currentTheme.textColor });
      }

      const lineHighlight = lineHighlightMap.get(lineNum);
      if (!lineHighlight) {
        // No highlight token for this line, use default color
        if (lineText.length > 0) {
          this.pushTextWithBracketColors(items, lineText, 0, lineText.length,
            this.currentTheme.textColor, lineBrackets);
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
          this.pushTextWithBracketColors(items, lineText, lastCol, startCol,
            this.currentTheme.textColor, lineBrackets);
        }

        this.pushTextWithBracketColors(items, lineText, startCol, endCol,
          this.currentTheme.getColor(span.styleId), lineBrackets);
        lastCol = endCol;
      }

      if (lastCol < lineText.length) {
        this.pushTextWithBracketColors(items, lineText, lastCol, lineText.length,
          this.currentTheme.textColor, lineBrackets);
      }
    }

    this.renderVersion++;
    this.highlightSpans = items;
  }

  private pushTextWithBracketColors(items: SpanItem[],
                                    lineText: string,
                                    startCol: number,
                                    endCol: number,
                                    baseColor: number,
                                    lineBrackets?: sweetline.LineBracketPairs): void {
    const start = this.clampColumn(startCol, lineText.length);
    const end = this.clampColumn(endCol, lineText.length);
    if (end <= start) {
      return;
    }
    const tokens = this.collectBracketTokens(lineText, start, end, lineBrackets);
    if (tokens.length === 0) {
      items.push({ text: lineText.substring(start, end), color: baseColor });
      return;
    }

    let cursor = start;
    for (let i = 0; i < tokens.length; i++) {
      const token = tokens[i];
      if (token.start > cursor) {
        items.push({ text: lineText.substring(cursor, token.start), color: baseColor });
      }
      items.push({ text: lineText.substring(token.start, token.end), color: token.color });
      cursor = Math.max(cursor, token.end);
    }
    if (cursor < end) {
      items.push({ text: lineText.substring(cursor, end), color: baseColor });
    }
  }

  private collectBracketTokens(lineText: string,
                               startCol: number,
                               endCol: number,
                               lineBrackets?: sweetline.LineBracketPairs): BracketRenderToken[] {
    let tokens: BracketRenderToken[] = [];
    if (lineBrackets === undefined || lineBrackets.tokens.length === 0) {
      return tokens;
    }
    for (let i = 0; i < lineBrackets.tokens.length; i++) {
      const token = lineBrackets.tokens[i];
      const tokenStart = this.clampColumn(token.range.start.column, lineText.length);
      const tokenEnd = this.clampColumn(token.range.end.column, lineText.length);
      if (tokenEnd <= startCol || tokenStart >= endCol) {
        continue;
      }
      const clippedStart = Math.max(startCol, tokenStart);
      const clippedEnd = Math.min(endCol, tokenEnd);
      if (clippedEnd <= clippedStart) {
        continue;
      }
      tokens.push({
        start: clippedStart,
        end: clippedEnd,
        color: this.bracketColor(token)
      });
    }
    return tokens;
  }

  private bracketColor(token: sweetline.BracketToken): number {
    if (token.matchState === sweetline.BracketMatchState.UNMATCHED) {
      return 0xFFFF6B6B;
    }
    const color = this.bracketPalette[Math.abs(token.depth) % this.bracketPalette.length];
    if (token.matchState === sweetline.BracketMatchState.UNKNOWN) {
      return (color & 0x00FFFFFF) | 0xAA000000;
    }
    return color;
  }

  private clampColumn(value: number, maxValue: number): number {
    return Math.max(0, Math.min(value, maxValue));
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
