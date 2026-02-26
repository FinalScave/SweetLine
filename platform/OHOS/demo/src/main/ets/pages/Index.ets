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

    this.commonEngine.defineMacro("OHOS");
    this.inlineStyleEngine.defineMacro("OHOS");

    this.loadExampleFiles();
  }

  build() {
    Column() {
      // 顶部工具栏
      Row() {
        // 主题名称
        Text(this.currentTheme.name)
          .fontSize(12)
          .fontColor(this.argbToColor(this.currentTheme.textColor))
          .padding({ left: 8, right: 8 })
          .layoutWeight(1)
          .onClick(() => {
            this.showThemeDialog();
          })

        // 文件选择
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

      // 代码高亮显示区域
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

      // 底部状态栏
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
   * 显示主题选择对话框
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
      title: '选择高亮主题',
      message: '',
      sheets: sheets,
      cancel: () => {
      }
    });
  }

  /**
   * 加载示例文件列表
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
      console.error(TAG, `列出rawfile文件失败: ${error}`);
    }
  }

  /**
   * 重新高亮当前选中的文件
   */
  private reHighlightCurrentFile(): void {
    if (this.selectedFileIndex >= 0 && this.selectedFileIndex < this.exampleFiles.length) {
      this.highlightFile(this.exampleFiles[this.selectedFileIndex]);
    }
  }

  /**
   * 高亮指定的文件
   */
  private highlightFile(testFileName: string): void {
    const ext = this.getFileExtension(testFileName);
    const syntaxFileName = EXT_SYNTAX_MAP.get(ext);
    if (!syntaxFileName) {
      console.warn(TAG, `未找到 ${testFileName} 对应的语法规则文件`);
      return;
    }
    this.highlight(testFileName, syntaxFileName, false);
  }

  /**
   * 执行高亮分析
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

      this.analyzeTime = `编译: ${compileTime}ms | 加载: ${loadTime}ms | 分析: ${analyzeElapsed}ms`;

      if (highlight) {
        this.buildSpanItems(testCode, highlight, inlineStyle);
      }
    } catch (e) {
      console.error(TAG, `高亮分析异常: ${e}`);
    }
  }

  /**
   * 将高亮结果构建为渲染用的 SpanItem 数组
   */
  private buildSpanItems(fullText: string, highlight: sweetline.DocumentHighlight, inlineStyle: boolean): void {
    let items: SpanItem[] = [];
    const lines = fullText.split('\n');

    // 按实际行号建立映射，highlight.lines 中不包含无 token 的行
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
        // 该行无高亮 token，整行使用默认颜色
        if (lineText.length > 0) {
          items.push({ text: lineText, color: this.currentTheme.textColor });
        }
        continue;
      }

      let lastCol = 0;
      for (let spanIdx = 0; spanIdx < lineHighlight.spans.length; spanIdx++) {
        const span = lineHighlight.spans[spanIdx];
        const startCol = span.range.start.column;
        // 跨行 span 限制 endCol 不超过当前行长度
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
   * 获取文件扩展名
   */
  private getFileExtension(fileName: string): string {
    const dotIndex = fileName.lastIndexOf('.');
    if (dotIndex >= 0) {
      return fileName.substring(dotIndex);
    }
    return '';
  }

  /**
   * 将ARGB整数颜色值转换为ArkTS Color
   */
  private argbToColor(argb: number): string {
    const a = (argb >>> 24) & 0xFF;
    const r = (argb >>> 16) & 0xFF;
    const g = (argb >>> 8) & 0xFF;
    const b = argb & 0xFF;
    return `rgba(${r},${g},${b},${(a / 255).toFixed(2)})`;
  }
}
