import lib from "libsweetline.so"

export namespace sweetline {

  /**
   * Text position descriptor
   */
  export class TextPosition {
    /**
     * Line number (0-based)
     */
    public line: number = 0;
    /**
     * Column number (0-based)
     */
    public column: number = 0;
    /**
     * Character index in the full text (0-based)
     */
    public index: number = -1;

    public constructor(line: number = 0, column: number = 0, index: number = -1) {
      this.line = line;
      this.column = column;
      this.index = index;
    }
  }

  /**
   * Text range descriptor
   */
  export class TextRange {
    /**
     * Start position
     */
    public start: TextPosition = new TextPosition();
    /**
     * End position
     */
    public end: TextPosition = new TextPosition();

    public constructor(start: TextPosition = new TextPosition(), end: TextPosition = new TextPosition()) {
      this.start = start;
      this.end = end;
    }
  }

  /**
   * Inline style definition embedded in syntax rules
   */
  export class InlineStyle {
    public static readonly STYLE_BOLD: number = 1;
    public static readonly STYLE_ITALIC: number = InlineStyle.STYLE_BOLD << 1;
    public static readonly STYLE_STRIKE_THROUGH: number = InlineStyle.STYLE_ITALIC << 1;

    /**
     * Foreground color
     */
    public foreground: number = 0;
    /**
     * Background color
     */
    public background: number = 0;
    /**
     * Whether to display in bold
     */
    public isBold: boolean = false;
    /**
     * Whether to display in italic
     */
    public isItalic: boolean = false;
    /**
     * Whether to display with strikethrough
     */
    public isStrikethrough: boolean = false;
  }

  /**
   * Each highlight token span
   */
  export class TokenSpan {
    /**
     * Highlight range
     */
    public range: TextRange = new TextRange();
    /**
     * Highlight style ID (only in non-inlineStyle mode))
     */
    public styleId: number = 0;
    /**
     * Detailed style info for the token span (only in inlineStyle mode)
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
   * Highlight token span sequence for each line
   */
  export class LineHighlight {
    /**
     * Highlight sequence
     */
    public spans: Array<TokenSpan> = [];
  }

  /**
   * Highlight result for the entire document
   */
  export class DocumentHighlight {
    /**
     * Highlight sequence for each line
     */
    public lines: Array<LineHighlight> = [];
  }

  /**
   * Line range descriptor (0-based)
   */
  export class LineRange {
    /**
     * Start line number
     */
    public startLine: number = 0;
    /**
     * Line count
     */
    public lineCount: number = 0;

    public constructor(startLine: number = 0, lineCount: number = 0) {
      this.startLine = startLine;
      this.lineCount = lineCount;
    }
  }

  /**
   * Highlight slice for the specified line range
   */
  export class DocumentHighlightSlice {
    /**
     * Slice start line
     */
    public startLine: number = 0;
    /**
     * Total line count after patch
     */
    public totalLineCount: number = 0;
    /**
     * Highlight sequence for slice lines
     */
    public lines: Array<LineHighlight> = [];
  }

  /**
   * Line scope state for indent guide analysis
   */
  export class LineScopeState {
    /**
     * Nesting level of the line
     */
    public nestingLevel: number = -1;
    /**
     * Scope state of the line: 0=START, 1=END, 2=CONTENT
     */
    public scopeState: number = 2;
    /**
     * Column of the scope marker
     */
    public scopeColumn: number = 0;
    /**
     * Indentation level of the line
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
   * Branch point (e.g. position of else/case)
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
   * Single indent guide line (vertical line segment)
   */
  export class IndentGuideLine {
    /**
     * Column of the guide line (character column)
     */
    public column: number = 0;
    /**
     * Start line number
     */
    public startLine: number = 0;
    /**
     * End line number
     */
    public endLine: number = 0;
    /**
     * Nesting level (0-based)
     */
    public nestingLevel: number = 0;
    /**
     * Associated ScopeRule ID (matching pair mode), -1 for indentation mode
     */
    public scopeRuleId: number = -1;
    /**
     * Branch point list (line/column positions of else/case etc.)
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
   * Indent guide analysis result
   */
  export class IndentGuideResult {
    /**
     * All vertical guide lines
     */
    public guideLines: Array<IndentGuideLine> = [];
    /**
     * Block state for each line
     */
    public lineStates: Array<LineScopeState> = [];
  }

  /**
   * Text line metadata
   */
  export class TextLineInfo {
    /**
     * Line index
     */
    public line: number = 0;
    /**
     * Start highlight state of the line
     */
    public startState: number = 0;
    /**
     * Start character offset in the full text (not bytes), used for computing TokenSpan index; not needed when showIndex is disabled in HighlightConfig
     */
    public startCharOffset: number = 0;

    public constructor(line: number = 0, startState: number = 0, startCharOffset: number = 0) {
      this.line = line;
      this.startState = startState;
      this.startCharOffset = startCharOffset;
    }
  }

  /**
   * Single line syntax highlight analysis result
   */
  export class LineAnalyzeResult {
    /**
     * Highlight sequence of the current line
     */
    public highlight: LineHighlight = new LineHighlight();
    /**
     * End state after line analysis
     */
    public endState: number = 0;
    /**
     * Total character count analyzed in the current line, excluding line ending
     */
    public charCount: number = 0;
  }

  /**
   * Highlight configuration
   */
  export class HighlightConfig {
    /**
     * Whether the analysis result includes character index; without it, each TokenSpan only has line and column
     */
    public showIndex: boolean = false;
    /**
     * Whether to use inline styles, i.e. style definitions are embedded directly in syntax rule JSON, and the analysis result contains style info instead of returning style IDs
     */
    public inlineStyle: boolean = false;
    /**
     * Tab width, used for indent guide level calculation (1 tab = tabSize spaces))
     */
    public tabSize: number = 4;

    public constructor(showIndex: boolean = false, inlineStyle: boolean = false, tabSize: number = 4) {
      this.showIndex = showIndex;
      this.inlineStyle = inlineStyle;
      this.tabSize = tabSize;
    }

    public toNativeBits(): number {
      let bits = 0;
      if (this.showIndex) {
        bits |= 1;
      }
      if (this.inlineStyle) {
        bits |= 1 << 1;
      }
      // Encode tabSize into bit8~bit15
      bits |= (this.tabSize & 0xFF) << 8;
      return bits;
    }
  }

  /**
   * Managed document with incremental update support
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
     * Get the URI of the managed document
     */
    public getUri(): string {
      return lib.Document_GetUri(this.nativeHandle);
    }

    /**
     * Total character count of the document
     */
    public totalChars(): number {
      return lib.Document_TotalChars(this.nativeHandle);
    }

    /**
     * Get the total character count of a specific line
     */
    public getLineCharCount(line: number): number {
      return lib.Document_GetLineCharCount(this.nativeHandle, line);
    }

    /**
     * Calculate the start character index of a specific line in the full text
     */
    public charIndexOfLine(line: number): number {
      return lib.Document_CharIndexOfLine(this.nativeHandle, line);
    }

    /**
     * Convert a character index to a line/column position
     */
    public charIndexToPosition(index: number): TextPosition {
      const array: Int32Array = lib.Document_CharIndexToPosition(this.nativeHandle, index);
      return new TextPosition(array[0], array[1], index);
    }

    /**
     * Get the total line count
     */
    public getLineCount(): number {
      return lib.Document_GetLineCount(this.nativeHandle);
    }

    /**
     * Get the text content of a specific line
     */
    public getLine(line: number): string {
      return lib.Document_GetLine(this.nativeHandle, line);
    }

    /**
     * Get the full text content
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
   * Syntax rule
   */
  export class SyntaxRule {
    protected nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    /**
     * Get the name of the syntax rule
     */
    public getName(): string {
      return lib.SyntaxRule_GetName(this.nativeHandle);
    }

    /**
     * Get the file extensions supported by the syntax rule
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
   * Plain text highlight analyzer, no incremental update support, suitable for full analysis scenarios
   */
  export class TextAnalyzer {
    private nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    /**
     * Analyze a text and return the highlight result for the entire text
     * @param text Full text content
     * @return Highlight result
     */
    public analyzeText(text: string): DocumentHighlight {
      if (this.nativeHandle == 0) {
        return new DocumentHighlight();
      }
      let buffer: Int32Array = lib.TextAnalyzer_AnalyzeText(this.nativeHandle, text);
      return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * Analyze a single line of text
     * @param text Single line text content
     * @param info Metadata for the current line
     * @return Single line analysis result
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
     * Perform indent guide analysis on a text (performs highlight analysis internally)）
     * @param text Full text content
     * @returns Indent guide analysis result
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
   * Highlight analyzer
   */
  export class DocumentAnalyzer {
    private nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    /**
     * Perform full highlight analysis on the text
     * @return Highlight result
     */
    public analyze(): DocumentHighlight {
      if (this.nativeHandle == 0) {
        return new DocumentHighlight();
      }
      let buffer: Int32Array = lib.DocumentAnalyzer_Analyze(this.nativeHandle);
      return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * Incrementally re-analyze the text based on patch content
     * @param range Change range of the patch
     * @param newText Patched text
     * @return Highlight result
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
     * Incrementally re-analyze the text based on patch content (by character index)
     * @param startIndex Start index of the patch change
     * @param endIndex End index of the patch change
     * @param newText Patched text
     * @return Highlight result
     */
    public analyzeIncrementalByIndex(startIndex: number, endIndex: number, newText: string): DocumentHighlight {
      if (this.nativeHandle == 0) {
        return new DocumentHighlight();
      }
      let buffer: Int32Array = lib.DocumentAnalyzer_AnalyzeChanges2(this.nativeHandle, startIndex, endIndex, newText);
      return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * Incrementally re-analyze and return only highlight slice for the specified visible line range
     * @param range Change range of the patch
     * @param newText Patched text
     * @param visibleRange Visible line range
     * @returns Highlight slice for the specified line range
     */
    public analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice {
      if (this.nativeHandle == 0) {
        return new DocumentHighlightSlice();
      }
      let buffer: Int32Array = lib.DocumentAnalyzer_AnalyzeChangesInLineRange(
        this.nativeHandle,
        range.start.line, range.start.column,
        range.end.line, range.end.column,
        newText,
        visibleRange.startLine, visibleRange.lineCount
      );
      return NativeBufferPack.readDocumentHighlightSlice(buffer);
    }

    /**
     * Perform indent guide analysis on the managed document (requires prior call to analyze or analyzeIncremental)
     * @returns Indent guide analysis result
     */
    public analyzeIndentGuides(): IndentGuideResult {
      if (this.nativeHandle == 0) {
        return new IndentGuideResult();
      }
      let buffer: Int32Array = lib.DocumentAnalyzer_AnalyzeIndentGuides(this.nativeHandle);
      return NativeBufferPack.readIndentGuideResult(buffer);
    }

    /**
     * Get the managed document
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
   * Highlight engine
   */
  export class HighlightEngine {
    private nativeHandle: number = 0;

    public constructor(config: HighlightConfig) {
      this.nativeHandle = lib.HighlightEngine_Create(config.toNativeBits());
    }

    /**
     * Register a highlight style for name mapping
     * @param styleName Style name
     * @param styleId Style ID
     */
    public registerStyleName(styleName: string, styleId: number): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_RegisterStyleName(this.nativeHandle, styleName, styleId);
    }

    /**
     * Get the registered style name by style ID
     * @param styleId Style ID
     * @return Style name
     */
    public getStyleName(styleId: number): string {
      if (this.nativeHandle == 0) {
        return '';
      }
      return lib.HighlightEngine_GetStyleName(this.nativeHandle, styleId);
    }

    /**
     * Define a macro for controlling #ifdef conditional compilation in importSyntax
     * @param macroName Macro name
     */
    public defineMacro(macroName: string): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_DefineMacro(this.nativeHandle, macroName);
    }

    /**
     * Undefine a macro
     * @param macroName Macro name
     */
    public undefineMacro(macroName: string): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_UndefineMacro(this.nativeHandle, macroName);
    }

    /**
     * Compile syntax rule from JSON
     * @param syntaxJson JSON content of the syntax rule
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
     * Compile syntax rule
     * @param path Syntax rule definition file path (JSON)
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
     * Create a text highlight analyzer by syntax rule name (no incremental analysis support, but supports single-line analysis with line state for custom incremental analysis)
     * @param syntaxName Syntax rule name (e.g. java)
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
     * Create a text highlight analyzer by file extension (no incremental analysis support, but supports single-line analysis with line state for custom incremental analysis)
     * @param extension File extension (e.g. .t)
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
     * Load a managed document and get a document highlight analyzer
     * @param document Managed document
     * @returns Document highlight analyzer
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
     * Remove a managed document
     * @param uri Managed document URI
     */
    public removeDocument(uri: string): boolean {
      if (this.nativeHandle == 0) {
        return false;
      }
      return lib.HighlightEngine_RemoveDocument(this.nativeHandle, uri);
    }
  }

  /**
   * Native layer data packet parsing utility
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

    static readDocumentHighlightSlice(buffer: Int32Array): DocumentHighlightSlice {
      const slice = new DocumentHighlightSlice();
      if (!buffer || buffer.length < 5) {
        return slice;
      }
      slice.startLine = buffer[0];
      slice.totalLineCount = buffer[1];
      const lineCount = Math.max(buffer[2], 0);
      const spanCount = Math.max(buffer[3], 0);
      const stride = Math.max(buffer[4], 0);
      for (let i = 0; i < lineCount; i++) {
        slice.lines.push(new LineHighlight());
      }
      for (let i = 0; i < spanCount; i++) {
        let baseIndex = i * stride + 5;
        if (baseIndex + 6 >= buffer.length) {
          break;
        }
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
          if (baseIndex + 8 >= buffer.length) {
            break;
          }
          let inlineStyle = new InlineStyle();
          inlineStyle.foreground = buffer[baseIndex + 6];
          inlineStyle.background = buffer[baseIndex + 7];
          NativeBufferPack.unpackInlineStyleTags(buffer[baseIndex + 8], inlineStyle);
          span = TokenSpan.withInlineStyle(range, inlineStyle);
        } else {
          span = new TokenSpan(range, buffer[baseIndex + 6]);
        }
        const localLine = startLine - slice.startLine;
        if (localLine < 0 || localLine >= slice.lines.length) {
          continue;
        }
        slice.lines[localLine].spans.push(span);
      }
      return slice;
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
