export declare namespace sweetline {

  /**
   * Text position descriptor
   */
  export class TextPosition {
    /**
     * Line number (0-based)
     */
    public line: number;
    /**
     * Column number (0-based)
     */
    public column: number;
    /**
     * Character index in the full text (0-based)
     */
    public index: number;

    public constructor();
    public constructor(line: number, column: number);
    public constructor(line: number, column: number, index: number);
  }

  /**
   * Text range descriptor
   */
  export class TextRange {
    /**
     * Start position
     */
    public start: TextPosition;
    /**
     * End position
     */
    public end: TextPosition;

    public constructor(start: TextPosition, end: TextPosition);
  }

  export class SyntaxCompileError extends Error {
    public static readonly ERR_JSON_PROPERTY_MISSED: number;
    public static readonly ERR_JSON_PROPERTY_INVALID: number;
    public static readonly ERR_PATTERN_INVALID: number;
    public static readonly ERR_STATE_INVALID: number;
    public static readonly ERR_JSON_INVALID: number;
    public static readonly ERR_FILE_NOT_EXISTS: number;
    public static readonly ERR_FILE_INVALID: number;
    public static readonly ERR_IMPORT_SYNTAX_NOT_FOUND: number;
    public static readonly ERR_STATE_REFERENCE_NOT_FOUND: number;
    public static readonly ERR_INLINE_STYLE_REFERENCE_NOT_FOUND: number;

    public errorCode: number;

    public constructor(errorCode: number, message: string);
  }

  /**
   * Inline style definition embedded in syntax rules
   */
  export class InlineStyle {
    public static readonly STYLE_BOLD: number;
    public static readonly STYLE_ITALIC: number;
    public static readonly STYLE_STRIKE_THROUGH: number;

    /**
     * Foreground color
     */
    public foreground: number;
    /**
     * Background color
     */
    public background: number;
    /**
     * Whether to display in bold
     */
    public isBold: boolean;
    /**
     * Whether to display in italic
     */
    public isItalic: boolean;
    /**
     * Whether to display with strikethrough
     */
    public isStrikethrough: boolean;
  }

  /**
   * Each highlight token span
   */
  export class TokenSpan {
    /**
     * Highlight range
     */
    public range: TextRange;
    /**
     * Highlight style ID (only in non-inlineStyle mode))
     */
    public styleId: number;
    /**
     * Detailed style info for the token span (only in inlineStyle mode)
     */
    public inlineStyle: InlineStyle | null;

    constructor(range: TextRange, styleId: number);
    public static withInlineStyle(range: TextRange, inlineStyle: InlineStyle): TokenSpan;
  }

  /**
   * Highlight token span sequence for each line
   */
  export class LineHighlight {
    /**
     * Highlight sequence
     */
    public spans: Array<TokenSpan>;
  }

  /**
   * Highlight result for the entire document
   */
  export class DocumentHighlight {
    /**
     * Highlight sequence for each line
     */
    public lines: Array<LineHighlight>;
  }

  /**
   * Line range descriptor (0-based)
   */
  export class LineRange {
    /**
     * Start line number
     */
    public startLine: number;
    /**
     * Line count
     */
    public lineCount: number;

    public constructor(startLine?: number, lineCount?: number);
  }

  /**
   * Highlight slice for the specified line range
   */
  export class DocumentHighlightSlice {
    /**
     * Slice start line
     */
    public startLine: number;
    /**
     * Total line count after patch
     */
    public totalLineCount: number;
    /**
     * Highlight sequence for slice lines
     */
    public lines: Array<LineHighlight>;
  }

  /**
   * Bracket token kind.
   */
  export class BracketTokenKind {
    public static readonly OPEN: number;
    public static readonly CLOSE: number;
  }

  /**
   * Bracket match state.
   */
  export class BracketMatchState {
    public static readonly MATCHED: number;
    public static readonly UNMATCHED: number;
    public static readonly UNKNOWN: number;
  }

  /**
   * Single bracket token.
   */
  export class BracketToken {
    /**
     * Bracket token range
     */
    public range: TextRange;
    /**
     * Nesting depth after opening or before closing
     */
    public depth: number;
    /**
     * Token kind, see BracketTokenKind
     */
    public kind: number;
    /**
     * Match state, see BracketMatchState
     */
    public matchState: number;
    /**
     * Matched partner range, null when unavailable
     */
    public partnerRange: TextRange | null;

    public constructor();
    public constructor(range: TextRange, depth: number, kind: number, matchState: number, partnerRange: TextRange | null);
  }

  /**
   * Bracket token sequence for each line.
   */
  export class LineBracketPairs {
    /**
     * Bracket token sequence
     */
    public tokens: Array<BracketToken>;
  }

  /**
   * Bracket pair analysis result.
   */
  export class BracketPairResult {
    /**
     * Actual start line of the returned slice
     */
    public startLine: number;
    /**
     * Total document line count
     */
    public totalLineCount: number;
    /**
     * Bracket tokens grouped by line
     */
    public lines: Array<LineBracketPairs>;
  }

  /**
   * Line scope state for indent guide analysis
   */
  export class LineScopeState {
    /**
     * Nesting level of the line
     */
    public nestingLevel: number;
    /**
     * Scope state of the line: 0=START, 1=END, 2=CONTENT
     */
    public scopeState: number;
    /**
     * Column of the scope marker
     */
    public scopeColumn: number;
    /**
     * Indentation level of the line
     */
    public indentLevel: number;

    public constructor();
    public constructor(nestingLevel: number, scopeState: number, scopeColumn: number, indentLevel: number);
  }

  /**
   * Branch point (e.g. position of else/case)
   */
  export class BranchPoint {
    public line: number;
    public column: number;

    public constructor();
    public constructor(line: number, column: number);
  }

  /**
   * Single indent guide line (vertical line segment)
   */
  export class IndentGuideLine {
    /**
     * Column of the guide line (character column)
     */
    public column: number;
    /**
     * Start line number
     */
    public startLine: number;
    /**
     * End line number
     */
    public endLine: number;
    /**
     * Whether the guide continues from before the returned slice
     */
    public continuesBefore: boolean;
    /**
     * Whether the guide continues after the returned slice
     */
    public continuesAfter: boolean;
    /**
     * Branch point list (line/column positions of else/case etc.)
     */
    public branches: Array<BranchPoint>;

    public constructor();
    public constructor(column: number, startLine: number, endLine: number, continuesBefore: boolean, continuesAfter: boolean);
  }

  /**
   * Indent guide analysis result
   */
  export class IndentGuideResult {
    /**
     * Actual start line of the returned slice
     */
    public startLine: number;
    /**
     * All vertical guide lines
     */
    public guideLines: Array<IndentGuideLine>;
    /**
     * Block state for each line
     */
    public lineStates: Array<LineScopeState>;
  }

  /**
   * Text line metadata
   */
  export class TextLineInfo {
    /**
     * Line index
     */
    public line: number;
    /**
     * Start highlight state of the line
     */
    public startState: number;
    /**
     * Start character offset in the full text
     */
    public startCharOffset: number;

    public constructor(line: number, startState: number);
    public constructor(line: number, startState: number, startCharOffset: number);
  }

  /**
   * Single line syntax highlight analysis result
   */
  export class LineAnalyzeResult {
    /**
     * Highlight sequence of the current line
     */
    public highlight: LineHighlight;
    /**
     * End state after line analysis
     */
    public endState: number;
    /**
     * Total character count analyzed in the current line, excluding line ending
     */
    public charCount: number;
  }

  /**
   * Highlight configuration
   */
  export class HighlightConfig {
    /**
     * Whether the analysis result includes character index; without it, each TokenSpan only has line and column
     */
    public showIndex: boolean;
    /**
     * Whether to use inline styles, i.e. style definitions are embedded directly in syntax rule JSON, and the analysis result contains style info instead of returning style IDs
     */
    public inlineStyle: boolean;
    /**
     * Tab width, used for indent guide level calculation (1 tab = tabSize spaces))
     */
    public tabSize: number;

    public constructor(showIndex: boolean, inlineStyle: boolean);
    public constructor(showIndex: boolean, inlineStyle: boolean, tabSize: number);
  }

  /**
   * Managed document with incremental update support
   */
  export class Document {
    public constructor(uri: string, content: string);

    /**
     * Get the URI of the managed document
     */
    public getUri(): string;
    /**
     * Total character count of the document
     */
    public totalChars(): number;
    /**
     * Get the total character count of a specific line
     */
    public getLineCharCount(line: number): number;
    /**
     * Calculate the start character index of a specific line in the full text
     */
    public charIndexOfLine(line: number): number;
    /**
     * Convert a character index to a line/column position
     */
    public charIndexToPosition(index: number): TextPosition;
    /**
     * Get the total line count
     */
    public getLineCount(): number;
    /**
     * Get the text content of a specific line
     */
    public getLine(line: number): string;
    /**
     * Get the full text content
     */
    public getText(): string;
    /**
     * Release the native document handle
     */
    public close(): void;
  }

  /**
   * Syntax rule
   */
  export class SyntaxRule {
    /**
     * Get the name of the syntax rule
     */
    public getName(): string;
    /**
     * Get the exact file names supported by the syntax rule
     */
    public getFileNames(): string[];
    /**
     * Get the file suffixes supported by the syntax rule
     */
    public getFileSuffixes(): string[];
    /**
     * Release the native syntax rule handle
     */
    public close(): void;
  }

  /**
   * Plain text highlight analyzer, no incremental update support, suitable for full analysis scenarios
   */
  export class TextAnalyzer {
    /**
     * Analyze a text and return the highlight result for the entire text
     * @param text Full text content
     * @return Highlight result
     */
    public analyzeText(text: string): DocumentHighlight;
    /**
     * Analyze a single line of text
     * @param text Single line text content
     * @param info Metadata for the current line
     * @return Single line analysis result
     */
    public analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult;
    /**
     * Perform indent guide analysis on a text.
     * @param text Full text content
     * @returns Indent guide analysis result
     */
    public analyzeIndentGuides(text: string): IndentGuideResult;
    /**
     * Perform bracket pair analysis on a text.
     * @param text Full text content
     * @returns Bracket pair analysis result
     */
    public analyzeBracketPairs(text: string): BracketPairResult;
    /**
     * Release the native text analyzer handle
     */
    public close(): void;
  }

  /**
   * Highlight analyzer
   */
  export class DocumentAnalyzer {
    /**
     * Perform full highlight analysis on the text
     * @return Highlight result
     */
    public analyze(): DocumentHighlight;
    /**
     * Analyze enough lines to cover the specified visible line range
     * @param visibleRange Visible line range
     * @returns Highlight slice for the specified line range
     */
    public analyzeLineRange(visibleRange: LineRange): DocumentHighlightSlice;
    /**
     * Incrementally re-analyze the text based on patch content
     * @param range Change range of the patch
     * @param newText Patched text
     * @return Highlight result
     */
    public analyzeIncremental(range: TextRange, newText: string): DocumentHighlight;
    /**
     * Incrementally re-analyze and return only highlight slice for the specified visible line range
     * @param range Change range of the patch
     * @param newText Patched text
     * @param visibleRange Visible line range
     * @returns Highlight slice for the specified line range
     */
    public analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice;
    /**
     * Get highlight slice from the current cached result (requires prior call to analyze or analyzeIncremental)
     * @param visibleRange Visible line range
     * @returns Highlight slice for the specified line range
     */
    public getHighlightSlice(visibleRange: LineRange): DocumentHighlightSlice;
    /**
     * Incrementally re-analyze the text based on patch content (by character index)
     * @param startIndex Start index of the patch change
     * @param endIndex End index of the patch change
     * @param newText Patched text
     * @return Highlight result
     */
    public analyzeIncrementalByIndex(startIndex: number, endIndex: number, newText: string): DocumentHighlight;

    /**
     * Perform indent guide analysis on the managed document
     * @returns Indent guide analysis result
     */
    public analyzeIndentGuides(): IndentGuideResult;

    /**
     * Perform indent guide analysis for the specified visible line range
     * @param visibleRange Visible line range
     * @returns Indent guide analysis result for the specified line range
     */
    public analyzeIndentGuidesInLineRange(visibleRange: LineRange): IndentGuideResult;

    /**
     * Perform bracket pair analysis on the managed document
     * @returns Bracket pair analysis result
     */
    public analyzeBracketPairs(): BracketPairResult;

    /**
     * Perform bracket pair analysis for the specified visible line range
     * @param visibleRange Visible line range
     * @returns Bracket pair analysis result for the specified line range
     */
    public analyzeBracketPairsInLineRange(visibleRange: LineRange): BracketPairResult;

    /**
     * Get the managed document
     */
    public getDocument(): Document;
    /**
     * Release the native document analyzer handle
     */
    public close(): void;
  }

  /**
   * Highlight engine
   */
  export class HighlightEngine {
    public constructor(config: HighlightConfig);

    /**
     * Register a highlight style for name mapping
     * @param styleName Style name
     * @param styleId Style ID
     */
    public registerStyleName(styleName: string, styleId: number): boolean;
    /**
     * Get the registered style name by style ID
     * @param styleId Style ID
     * @return Style name
     */
    public getStyleName(styleId: number): string;
    /**
     * Define a macro for controlling #ifdef conditional compilation in importSyntax
     * @param macroName Macro name
     */
    public defineMacro(macroName: string): boolean;
    /**
     * Undefine a macro
     * @param macroName Macro name
     */
    public undefineMacro(macroName: string): boolean;
    /**
     * Compile syntax rule from JSON
     * @param syntaxJson JSON content of the syntax rule
     * @throws SyntaxCompileError on compilation error
     */
    public compileSyntaxFromJson(syntaxJson: string): SyntaxRule | null;
    /**
     * Compile syntax rule
     * @param path Syntax rule definition file path (JSON)
     * @throws SyntaxCompileError on compilation error
     */
    public compileSyntaxFromFile(path: string): SyntaxRule | null;
    /**
     * Create a text highlight analyzer by syntax rule name
     * @param syntaxName Syntax rule name (e.g. java)
     */
    public createAnalyzerBySyntaxName(syntaxName: string): TextAnalyzer | null;
    /**
     * Create a text highlight analyzer by file name
     * @param fileName File name or basename (e.g. main.dart)
     */
    public createAnalyzerByFileName(fileName: string): TextAnalyzer | null;
    /**
     * Load a managed document and get a document highlight analyzer
     * @param document Managed document
     * @returns Document highlight analyzer
     */
    public loadDocument(document: Document): DocumentAnalyzer | null;
    /**
     * Remove a managed document
     * @param uri Managed document URI
     */
    public removeDocument(uri: string): boolean;
    /**
     * Release the native highlight engine handle
     */
    public close(): void;
  }
}
