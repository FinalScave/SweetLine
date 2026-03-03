import lib from "libsweetline.so"

export namespace sweetline {

  /**
   * 文本位置描述
   */
  export class TextPosition {
    /**
     * 文字所处行，起始为0
     */
    public line: number = 0;
    /**
     * 文字所处列，起始为0
     */
    public column: number = 0;
    /**
     * 文字在全文中的索引，起始为0
     */
    public index: number = -1;

    public constructor(line: number = 0, column: number = 0, index: number = -1) {
      this.line = line;
      this.column = column;
      this.index = index;
    }
  }

  /**
   * 文字的范围区间描述
   */
  export class TextRange {
    /**
     * 起始位置
     */
    public start: TextPosition = new TextPosition();
    /**
     * 结束位置
     */
    public end: TextPosition = new TextPosition();

    public constructor(start: TextPosition = new TextPosition(), end: TextPosition = new TextPosition()) {
      this.start = start;
      this.end = end;
    }
  }

  /**
   * 语法规则中直接包含的样式定义
   */
  export class InlineStyle {
    public static readonly STYLE_BOLD: number = 1;
    public static readonly STYLE_ITALIC: number = InlineStyle.STYLE_BOLD << 1;
    public static readonly STYLE_STRIKE_THROUGH: number = InlineStyle.STYLE_ITALIC << 1;

    /**
     * 前景色
     */
    public foreground: number = 0;
    /**
     * 背景色
     */
    public background: number = 0;
    /**
     * 是否加粗显示
     */
    public isBold: boolean = false;
    /**
     * 是否斜体显示
     */
    public isItalic: boolean = false;
    /**
     * 是否需要显示删除线
     */
    public isStrikethrough: boolean = false;
  }

  /**
   * 每一个高亮块
   */
  export class TokenSpan {
    /**
     * 高亮区域
     */
    public range: TextRange = new TextRange();
    /**
     * 高亮样式ID (非 inlineStyle 模式下才有该字段)
     */
    public styleId: number = 0;
    /**
     * 高亮块样式详细信息，inlineStyle 模式下才有该字段
     */
    public inlineStyle: InlineStyle | null = null;

    constructor(range: TextRange, styleId: number) {
      this.range = range;
      this.styleId = styleId;
    }

    public static withInlineStyle(range: TextRange, inlineStyle: InlineStyle): TokenSpan {
      const span = new TokenSpan(range, 0);
      span.inlineStyle = inlineStyle;
      return span;
    }
  }

  /**
   * 每一行的高亮块序列
   */
  export class LineHighlight {
    /**
     * 高亮序列
     */
    public spans: Array<TokenSpan> = [];
  }

  /**
   * 整个文本内容的高亮
   */
  export class DocumentHighlight {
    /**
     * 每一行的高亮序列
     */
    public lines: Array<LineHighlight> = [];
  }

  /**
   * 行作用域划线分析状态
   */
  export class LineScopeState {
    /**
     * 行所处嵌套层级
     */
    public nestingLevel: number = -1;
    /**
     * 行所处作用域划线状态: 0=START, 1=END, 2=CONTENT
     */
    public scopeState: number = 2;
    /**
     * 作用域划线所处列
     */
    public scopeColumn: number = 0;
    /**
     * 该行缩进等级
     */
    public indentLevel: number = 0;

    public constructor(nestingLevel: number = -1, scopeState: number = 2, scopeColumn: number = 0, indentLevel: number = 0) {
      this.nestingLevel = nestingLevel;
      this.scopeState = scopeState;
      this.scopeColumn = scopeColumn;
      this.indentLevel = indentLevel;
    }
  }

  /**
   * 分支点（如 else/case 的位置）
   */
  export class BranchPoint {
    public line: number = 0;
    public column: number = 0;

    public constructor(line: number = 0, column: number = 0) {
      this.line = line;
      this.column = column;
    }
  }

  /**
   * 单条缩进划线（纵向线段）
   */
  export class IndentGuideLine {
    /**
     * 划线所在列（字符列）
     */
    public column: number = 0;
    /**
     * 起始行号
     */
    public startLine: number = 0;
    /**
     * 结束行号
     */
    public endLine: number = 0;
    /**
     * 嵌套层级（0-based）
     */
    public nestingLevel: number = 0;
    /**
     * 关联的 ScopeRule id（匹配对模式），-1=缩进模式
     */
    public scopeRuleId: number = -1;
    /**
     * 分支点列表（如 else/case 的行列位置）
     */
    public branches: Array<BranchPoint> = [];

    public constructor(column: number = 0, startLine: number = 0, endLine: number = 0,
                       nestingLevel: number = 0, scopeRuleId: number = -1) {
      this.column = column;
      this.startLine = startLine;
      this.endLine = endLine;
      this.nestingLevel = nestingLevel;
      this.scopeRuleId = scopeRuleId;
    }
  }

  /**
   * 缩进划线分析结果
   */
  export class IndentGuideResult {
    /**
     * 所有纵向划线
     */
    public guideLines: Array<IndentGuideLine> = [];
    /**
     * 每行的块状态
     */
    public lineStates: Array<LineScopeState> = [];
  }

  /**
   * 文本行元数据信息
   */
  export class TextLineInfo {
    /**
     * 行号索引
     */
    public line: number = 0;
    /**
     * 行起始高亮状态
     */
    public startState: number = 0;
    /**
     * 行在整个文本中起始字符偏移 (不是字节)，用于计算高亮块(TokenSpan) index，HighlightConfig中未开启 showIndex 时 无需该字段
     */
    public startCharOffset: number = 0;

    public constructor(line: number = 0, startState: number = 0, startCharOffset: number = 0) {
      this.line = line;
      this.startState = startState;
      this.startCharOffset = startCharOffset;
    }
  }

  /**
   * 单行语法高亮分析结果
   */
  export class LineAnalyzeResult {
    /**
     * 当前行高亮序列
     */
    public highlight: LineHighlight = new LineHighlight();
    /**
     * 行分析完毕后结束状态
     */
    public endState: number = 0;
    /**
     * 当前行总计分析的字符总数，不包含换行符
     */
    public charCount: number = 0;
  }

  /**
   * 高亮配置
   */
  export class HighlightConfig {
    /**
     * 分析的高亮信息是否携带index，不携带的情况下每个TokenSpan只有line和column
     */
    public showIndex: boolean = false;
    /**
     * 是否支持内联样式，即不需要外部注册高亮样式，直接在语法规则json中定义高亮样式，高亮分析结果中直接包含高亮样式(前景色、加粗等），而不是返回样式ID
     */
    public inlineStyle: boolean = false;
    /**
     * Tab宽度，用于缩进划线的缩进等级计算 (1 tab = tabSize 个空格)
     */
    public tabSize: number = 4;

    public constructor(showIndex: boolean = false, inlineStyle: boolean = false, tabSize: number = 4) {
      this.showIndex = showIndex;
      this.inlineStyle = inlineStyle;
      this.tabSize = tabSize;
    }

    protected toNativeBits(): number {
      let bits = 0;
      if (this.showIndex) {
        bits |= 1;
      }
      if (this.inlineStyle) {
        bits |= 1 << 1;
      }
      // tabSize 编码到 bit8~bit15
      bits |= (this.tabSize & 0xFF) << 8;
      return bits;
    }
  }

  /**
   * 支持增量更新的托管文档
   */
  export class Document {
    protected nativeHandle: number = 0;

    public constructor(uri?: string, content?: string) {
      if (uri == undefined || content == undefined) {
        return;
      }
      this.nativeHandle = lib.Document_Create(uri, content);
    }

    /**
     * 获取托管文档的Uri
     */
    public getUri(): string {
      return lib.Document_GetUri(this.nativeHandle);
    }

    /**
     * 文档字符总数
     */
    public totalChars(): number {
      return lib.Document_TotalChars(this.nativeHandle);
    }

    /**
     * 获取指定行的字符总数
     */
    public getLineCharCount(line: number): number {
      return lib.Document_GetLineCharCount(this.nativeHandle, line);
    }

    /**
     * 计算指定行在全文中的起始索引
     */
    public charIndexOfLine(line: number): number {
      return lib.Document_CharIndexOfLine(this.nativeHandle, line);
    }

    /**
     * 将字符索引转换为行列位置
     */
    public charIndexToPosition(index: number): TextPosition {
      const array: Int32Array = lib.Document_CharIndexToPosition(this.nativeHandle, index);
      return new TextPosition(array[0], array[1], index);
    }

    /**
     * 获取总行数
     */
    public getLineCount(): number {
      return lib.Document_GetLineCount(this.nativeHandle);
    }

    /**
     * 获取指定行的文本信息
     */
    public getLine(line: number): string {
      return lib.Document_GetLine(this.nativeHandle, line);
    }

    /**
     * 获取完整文本
     */
    public getText(): string {
      return lib.Document_GetText(this.nativeHandle);
    }

    public __getNativeHandle(): number {
      return this.nativeHandle;
    }

    public static __createNative(nativeHandle: number): Document {
      const document = new Document();
      document.nativeHandle = nativeHandle;
      return document;
    }
  }

  /**
   * 语法规则
   */
  export class SyntaxRule {
    protected nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    /**
     * 获取语法规则的名称
     */
    public getName(): string {
      return lib.SyntaxRule_GetName(this.nativeHandle);
    }

    /**
     * 获取语法规则支持的文件扩展名
     */
    public getFileExtensions(): string[] {
      return lib.SyntaxRule_GetFileExtensions(this.nativeHandle);
    }

    public __getNativeHandle(): number {
      return this.nativeHandle;
    }

    public static __createNative(nativeHandle: number): SyntaxRule {
      return new SyntaxRule(nativeHandle);
    }
  }

  /**
   * 纯文本高亮分析器，不支持增量更新，适用于全量分析的场景
   */
  export class TextAnalyzer {
    private nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    /**
     * 分析一段文本内容，并返回整段文本的高亮结果
     * @param text 整段文本内容
     * @returns 高亮结果
     */
    public analyzeText(text: string): DocumentHighlight {
      if (this.nativeHandle == 0) {
        return new DocumentHighlight();
      }
      let buffer: Int32Array = lib.TextAnalyzer_AnalyzeText(this.nativeHandle, text);
      return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * 分析单行文本
     * @param text 单行文本内容
     * @param info 当前行元数据信息
     * @returns 单行高亮分析结果
     */
    public analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult {
      if (this.nativeHandle == 0) {
        return new LineAnalyzeResult();
      }
      let buffer: Int32Array = lib.TextAnalyzer_AnalyzeLine(
        this.nativeHandle, text, info.line, info.startState, info.startCharOffset);
      return NativeBufferPack.readLineAnalyzeResult(buffer);
    }

    /**
     * 对一段文本进行缩进划线分析（内部会先进行高亮分析）
     * @param text 整段文本内容
     * @returns 缩进划线分析结果
     */
    public analyzeIndentGuides(text: string): IndentGuideResult {
      if (this.nativeHandle == 0) {
        return new IndentGuideResult();
      }
      let buffer: Int32Array = lib.TextAnalyzer_AnalyzeIndentGuides(this.nativeHandle, text);
      return NativeBufferPack.readIndentGuideResult(buffer);
    }

    public static __createNative(nativeHandle: number): TextAnalyzer {
      return new TextAnalyzer(nativeHandle);
    }
  }

  /**
   * 高亮分析器
   */
  export class DocumentAnalyzer {
    private nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    /**
     * 对整个文本进行高亮分析
     * @returns 整个文本的高亮结果
     */
    public analyze(): DocumentHighlight {
      if (this.nativeHandle == 0) {
        return new DocumentHighlight();
      }
      let buffer: Int32Array = lib.DocumentAnalyzer_Analyze(this.nativeHandle);
      return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果
     * @param range patch的变更范围
     * @param newText patch的文本
     * @returns 整个文本的高亮结果
     */
    public analyzeIncremental(range: TextRange, newText: string): DocumentHighlight {
      if (this.nativeHandle == 0) {
        return new DocumentHighlight();
      }
      let buffer: Int32Array;
      if (range.start.index >= 0 && range.end.index >= 0) {
        buffer = lib.DocumentAnalyzer_AnalyzeChanges2(this.nativeHandle, range.start.index, range.end.index, newText);
      } else {
        buffer = lib.DocumentAnalyzer_AnalyzeChanges(this.nativeHandle, range.start.line, range.start.column,
          range.end.line, range.end.column, newText);
      }
      return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果(字符索引)
     * @param startIndex patch的起始索引
     * @param endIndex patch的结束索引
     * @param newText patch的文本
     * @returns 整个文本的高亮结果
     */
    public analyzeIncrementalByIndex(startIndex: number, endIndex: number, newText: string): DocumentHighlight {
      if (this.nativeHandle == 0) {
        return new DocumentHighlight();
      }
      let buffer: Int32Array = lib.DocumentAnalyzer_AnalyzeChanges2(this.nativeHandle, startIndex, endIndex, newText);
      return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * 对托管文档进行缩进划线分析（需先调用 analyze 或 analyzeIncremental）
     * @returns 缩进划线分析结果
     */
    public analyzeIndentGuides(): IndentGuideResult {
      if (this.nativeHandle == 0) {
        return new IndentGuideResult();
      }
      let buffer: Int32Array = lib.DocumentAnalyzer_AnalyzeIndentGuides(this.nativeHandle);
      return NativeBufferPack.readIndentGuideResult(buffer);
    }

    /**
     * 获取托管文档对象
     */
    public getDocument(): Document {
      if (this.nativeHandle == 0) {
        return new Document();
      }
      let documentHandle: number = lib.DocumentAnalyzer_GetDocument(this.nativeHandle);
      return Document.__createNative(documentHandle);
    }

    public static __createNative(nativeHandle: number): DocumentAnalyzer {
      return new DocumentAnalyzer(nativeHandle);
    }
  }

  /**
   * 高亮引擎
   */
  export class HighlightEngine {
    private nativeHandle: number = 0;

    public constructor(config: HighlightConfig) {
      this.nativeHandle = lib.HighlightEngine_Create(config.toNativeBits());
    }

    /**
     * 注册一个高亮样式，用于名称映射
     * @param styleName 样式名称
     * @param styleId 样式id
     */
    public registerStyleName(styleName: string, styleId: number): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_RegisterStyleName(this.nativeHandle, styleName, styleId);
    }

    /**
     * 通过样式id获取注册的样式名称
     * @param styleId 样式id
     * @returns 样式名称
     */
    public getStyleName(styleId: number): string {
      if (this.nativeHandle == 0) {
        return '';
      }
      return lib.HighlightEngine_GetStyleName(this.nativeHandle, styleId);
    }

    /**
     * 定义一个宏，用于控制importSyntax的#ifdef条件编译
     * @param macroName 宏名称
     */
    public defineMacro(macroName: string): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_DefineMacro(this.nativeHandle, macroName);
    }

    /**
     * 取消定义宏
     * @param macroName 宏名称
     */
    public undefineMacro(macroName: string): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_UndefineMacro(this.nativeHandle, macroName);
    }

    /**
     * 通过json编译语法规则
     * @param syntaxJson 语法规则文件的json
     */
    public compileSyntaxFromJson(syntaxJson: string): SyntaxRule | null {
      if (this.nativeHandle == 0) {
        return null;
      }
      let handle: number = lib.HighlightEngine_CompileSyntaxFromJson(this.nativeHandle, syntaxJson);
      if (handle == 0) {
        return null;
      }
      return SyntaxRule.__createNative(handle);
    }

    /**
     * 编译语法规则
     * @param path 语法规则定义文件路径(json)
     */
    public compileSyntaxFromFile(path: string): SyntaxRule | null {
      if (this.nativeHandle == 0) {
        return null;
      }
      let handle: number = lib.HighlightEngine_CompileSyntaxFromFile(this.nativeHandle, path);
      if (handle == 0) {
        return null;
      }
      return SyntaxRule.__createNative(handle);
    }

    /**
     * 根据语法规则名称创建一个文本高亮分析器(不支持增量分析,但可以分析单行并获得行状态,可以在上层自行实现增量分析)
     * @param syntaxName 语法规则名称(如 java)
     */
    public createAnalyzerByName(syntaxName: string): TextAnalyzer | null {
      if (this.nativeHandle == 0) {
        return null;
      }
      let analyzerHandle: number = lib.HighlightEngine_CreateAnalyzerByName(this.nativeHandle, syntaxName);
      if (analyzerHandle == 0) {
        return null;
      }
      return TextAnalyzer.__createNative(analyzerHandle);
    }

    /**
     * 根据文件后缀名创建一个文本高亮分析器(不支持增量分析,但可以分析单行并获得行状态,可以在上层自行实现增量分析)
     * @param extension 文件后缀名(如 .t)
     */
    public createAnalyzerByExtension(extension: string): TextAnalyzer | null {
      if (this.nativeHandle == 0) {
        return null;
      }
      let analyzerHandle: number = lib.HighlightEngine_CreateAnalyzerByExtension(this.nativeHandle, extension);
      if (analyzerHandle == 0) {
        return null;
      }
      return TextAnalyzer.__createNative(analyzerHandle);
    }

    /**
     * 加载托管文档对象获得文档高亮分析器
     * @param document 托管文档对象
     * @returns 文档高亮分析器
     */
    public loadDocument(document: Document): DocumentAnalyzer | null {
      if (this.nativeHandle == 0) {
        return null;
      }
      let handle: number = lib.HighlightEngine_LoadDocument(this.nativeHandle, document.__getNativeHandle());
      if (handle == 0) {
        return null;
      }
      return DocumentAnalyzer.__createNative(handle);
    }

    /**
     * 移除托管文档
     * @param uri 托管文档Uri
     */
    public removeDocument(uri: string): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_RemoveDocument(this.nativeHandle, uri);
    }
  }

  /**
   * Native层数据包解析工具
   */
  class NativeBufferPack {
    static unpackInlineStyleTags(tags: number, inlineStyle: InlineStyle): void {
      if ((tags & InlineStyle.STYLE_BOLD) != 0) {
        inlineStyle.isBold = true;
      }
      if ((tags & InlineStyle.STYLE_ITALIC) != 0) {
        inlineStyle.isItalic = true;
      }
      if ((tags & InlineStyle.STYLE_STRIKE_THROUGH) != 0) {
        inlineStyle.isStrikethrough = true;
      }
    }

    static readDocumentHighlight(buffer: Int32Array): DocumentHighlight {
      const highlight = new DocumentHighlight();
      if (!buffer || buffer.length < 2) {
        return highlight;
      }
      const spanCount = buffer[0];
      const stride = buffer[1];
      let lineHighlight = new LineHighlight();
      let line = -1;
      for (let i = 0; i < spanCount; i++) {
        let baseIndex = i * stride + 2;
        let startLine = buffer[baseIndex];
        let startColumn = buffer[baseIndex + 1];
        let startIndex = buffer[baseIndex + 2];
        let endLine = buffer[baseIndex + 3];
        let endColumn = buffer[baseIndex + 4];
        let endIndex = buffer[baseIndex + 5];

        let range = new TextRange(
          new TextPosition(startLine, startColumn, startIndex),
          new TextPosition(endLine, endColumn, endIndex)
        );

        let span: TokenSpan;
        if (stride > 7) {
          let inlineStyle = new InlineStyle();
          inlineStyle.foreground = buffer[baseIndex + 6];
          inlineStyle.background = buffer[baseIndex + 7];
          NativeBufferPack.unpackInlineStyleTags(buffer[baseIndex + 8], inlineStyle);
          span = TokenSpan.withInlineStyle(range, inlineStyle);
        } else {
          span = new TokenSpan(range, buffer[baseIndex + 6]);
        }

        if (startLine != line) {
          line = startLine;
          lineHighlight = new LineHighlight();
          highlight.lines.push(lineHighlight);
        }
        lineHighlight.spans.push(span);
      }
      return highlight;
    }

    static readLineAnalyzeResult(buffer: Int32Array): LineAnalyzeResult {
      const result = new LineAnalyzeResult();
      if (!buffer || buffer.length < 4) {
        return result;
      }
      const spanCount = buffer[0];
      const stride = buffer[1];
      result.endState = buffer[2];
      result.charCount = buffer[3];
      let lineHighlight = new LineHighlight();
      for (let i = 0; i < spanCount; i++) {
        let baseIndex = i * stride + 4;
        let startLine = buffer[baseIndex];
        let startColumn = buffer[baseIndex + 1];
        let startIndex = buffer[baseIndex + 2];
        let endLine = buffer[baseIndex + 3];
        let endColumn = buffer[baseIndex + 4];
        let endIndex = buffer[baseIndex + 5];

        let range = new TextRange(
          new TextPosition(startLine, startColumn, startIndex),
          new TextPosition(endLine, endColumn, endIndex)
        );

        let span: TokenSpan;
        if (stride > 7) {
          let inlineStyle = new InlineStyle();
          inlineStyle.foreground = buffer[baseIndex + 6];
          inlineStyle.background = buffer[baseIndex + 7];
          NativeBufferPack.unpackInlineStyleTags(buffer[baseIndex + 8], inlineStyle);
          span = TokenSpan.withInlineStyle(range, inlineStyle);
        } else {
          span = new TokenSpan(range, buffer[baseIndex + 6]);
        }
        lineHighlight.spans.push(span);
      }
      result.highlight = lineHighlight;
      return result;
    }

    static readIndentGuideResult(buffer: Int32Array): IndentGuideResult {
      const result = new IndentGuideResult();
      if (!buffer || buffer.length < 4) {
        return result;
      }
      const guideCount = buffer[0];
      // buffer[1] = stride (unused, we read dynamically)
      const lineStateCount = buffer[2];
      // buffer[3] = lineStateStride (4)

      let idx = 4;
      for (let i = 0; i < guideCount; i++) {
        let guide = new IndentGuideLine();
        guide.column = buffer[idx++];
        guide.startLine = buffer[idx++];
        guide.endLine = buffer[idx++];
        guide.nestingLevel = buffer[idx++];
        guide.scopeRuleId = buffer[idx++];
        let branchCount = buffer[idx++];
        for (let j = 0; j < branchCount; j++) {
          let bp = new BranchPoint();
          bp.line = buffer[idx++];
          bp.column = buffer[idx++];
          guide.branches.push(bp);
        }
        result.guideLines.push(guide);
      }
      for (let i = 0; i < lineStateCount; i++) {
        let state = new LineScopeState();
        state.nestingLevel = buffer[idx++];
        state.scopeState = buffer[idx++];
        state.scopeColumn = buffer[idx++];
        state.indentLevel = buffer[idx++];
        result.lineStates.push(state);
      }
      return result;
    }
  }
}
