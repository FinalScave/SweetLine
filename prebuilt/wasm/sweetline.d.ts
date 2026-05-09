export namespace SweetLineBindings {
    /**
     * Text position descriptor
     */
    export class TextPosition {
        /**
         * Line number (0-based)
         */
        line: number;
        /**
         * Column number (0-based)
         */
        column: number;
        /**
         * Character index in the full text (0-based)
         */
        index: number;
    }

    /**
     * Text range descriptor
     */
    export class TextRange {
        /**
         * Start position
         */
        start: TextPosition;
        /**
         * End position
         */
        end: TextPosition;
    }

    export class SyntaxCompileError extends Error {
        static readonly ERR_JSON_PROPERTY_MISSED: number;
        static readonly ERR_JSON_PROPERTY_INVALID: number;
        static readonly ERR_PATTERN_INVALID: number;
        static readonly ERR_STATE_INVALID: number;
        static readonly ERR_JSON_INVALID: number;
        static readonly ERR_FILE_NOT_EXISTS: number;
        static readonly ERR_FILE_INVALID: number;
        static readonly ERR_IMPORT_SYNTAX_NOT_FOUND: number;
        static readonly ERR_STATE_REFERENCE_NOT_FOUND: number;
        static readonly ERR_INLINE_STYLE_REFERENCE_NOT_FOUND: number;
        errorCode: number;
        constructor(errorCode: number, message: string);
    }

    /**
     * Managed document with incremental update support
     */
    export class Document {
        /**
         * Constructor
         * @param uri Document URI
         * @param content Document content
         */
        constructor(uri: string, content: string);

        /**
         * Get the URI of the managed document
         */
        getUri(): string;

        /**
         * Get the full text content
         */
        getText(): string;

        /**
         * Get total character count
         */
        totalChars(): number;

        /**
         * Get the character count of a specific line
         * @param line Line index
         */
        getLineCharCount(line: number): number;

        /**
         * Get total line count
         */
        getLineCount(): number;

        /**
         * Get the text content of a specific line (including line ending)
         * @param line Line index
         */
        getLineText(line: number): string;

        /**
         * Get the character index of the line start
         * @param line Line index
         */
        charIndexOfLine(line: number): number;

        /**
         * Convert a character index to a line/column position
         * @param index Character index
         */
        charIndexToPosition(index: number): TextPosition;
    }

    /**
     * Inline style definition embedded in syntax rules
     */
    export class InlineStyle {
        /**
         * Foreground color
         */
        foreground: number;
        /**
         * Background color
         */
        background: number;
        /**
         * Whether to display in bold
         */
        isBold: boolean;
        /**
         * Whether to display in italic
         */
        isItalic: boolean;
        /**
         * Whether to display with strikethrough
         */
        isStrikethrough: boolean;
    }

    /**
     * Each highlight token span
     */
    export class TokenSpan {
        /**
         * Highlight range
         */
        range: TextRange;
        /**
         * Highlight style ID
         */
        styleId: number;
        /**
         * Detailed style info for the token span (only in inlineStyle mode)
         */
        inlineStyle: InlineStyle;
    }

    /**
     * Native c++ `std::vector(typedef List)` interface binding
     */
    export interface NativeList<T> {
        get(index: number): T
        set(index: number, element: T): void
        add(element: T): void
        remove(element: T): void
        isEmpty(): boolean
        size(): number
    }

    export interface TokenSpanList extends NativeList<TokenSpan> {
    }

    /**
     * Highlight token span sequence for each line
     */
    export class LineHighlight {
        /**
         * Highlight sequence
         */
        spans: TokenSpanList;

        /**
         * Convert to JSON string
         */
        toJson(): string;
    }

    export interface LineHighlightList extends NativeList<LineHighlight> {
    }

    /**
     * Highlight result for the entire document
     */
    export class DocumentHighlight {
        /**
         * Highlight sequence for each line
         */
        lines: LineHighlightList;

        /**
         * Convert to JSON string
         */
        toJson(): string;
    }

    /**
     * Line range descriptor (0-based)
     */
    export class LineRange {
        /**
         * Start line number
         */
        startLine: number;
        /**
         * Line count
         */
        lineCount: number;
    }

    /**
     * Highlight slice for the specified line range
     */
    export class DocumentHighlightSlice {
        /**
         * Slice start line
         */
        startLine: number;
        /**
         * Total line count after patch
         */
        totalLineCount: number;
        /**
         * Highlight sequence for slice lines
         */
        lines: LineHighlightList;
    }

    /**
     * Line scope state for indent guide analysis
     */
    export class LineScopeState {
        /**
         * Nesting level of the line
         */
        nestingLevel: number;
        /**
         * Scope state of the line: 0=START, 1=END, 2=CONTENT
         */
        scopeState: number;
        /**
         * Column of the scope marker
         */
        scopeColumn: number;
        /**
         * Indentation level of the line
         */
        indentLevel: number;
    }

    /**
     * Single indent guide line (vertical line segment)
     */
    export class IndentGuideLine {
        /**
         * Column of the guide line (character column)
         */
        column: number;
        /**
         * Start line number
         */
        startLine: number;
        /**
         * End line number
         */
        endLine: number;
        /**
         * Nesting level (0-based)
         */
        nestingLevel: number;
        /**
         * Associated ScopeRule ID (matching pair mode), -1 for indentation mode
         */
        scopeRuleId: number;
        /**
         * Branch point list (line/column positions of else/case etc.)
         */
        branches: BranchPointList;
    }

    /**
     * Branch point (e.g. position of else/case)
     */
    export class BranchPoint {
        line: number;
        column: number;
    }

    export interface BranchPointList extends NativeList<BranchPoint> {
    }

    export interface IndentGuideLineList extends NativeList<IndentGuideLine> {
    }

    export interface LineScopeStateList extends NativeList<LineScopeState> {
    }

    /**
     * Indent guide analysis result
     */
    export class IndentGuideResult {
        /**
         * All vertical guide lines
         */
        guideLines: IndentGuideLineList;
        /**
         * Scope state for each line
         */
        lineStates: LineScopeStateList;
    }

    /**
     * Text line metadata
     */
    export class TextLineInfo {
        /**
         * Line index
         */
        line: number;

        /**
         * Start highlight state of the line
         */
        startState: number;

        /**
         * Start character offset in the full text (not bytes), used for computing TokenSpan index; not needed when showIndex is disabled in HighlightConfig
         */
        startCharOffset: number;
    }

    /**
     * Single line syntax highlight analysis result
     */
    export class LineAnalyzeResult {
        /**
         * Highlight sequence of the current line
         */
        highlight: LineHighlight;

        /**
         * End state after line analysis
         */
        endState: number;

        /**
         * Total character count analyzed in the current line, excluding line ending
         */
        charCount: number;
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
        analyzeText(text: string): DocumentHighlight;

        /**
         * Analyze a single line of text
         * @param text Single line text content
         * @param info Metadata for the current line
         * @return Single line analysis result
         */
        analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult;

        /**
         * Perform indent guide analysis on a text (performs highlight analysis internally)
         * @param text Full text content
         * @return Indent guide analysis result
         */
        analyzeIndentGuides(text: string): IndentGuideResult;
    }

    /**
     * Managed document highlight analyzer with automatic patch and incremental analysis support
     */
    export class DocumentAnalyzer {
        /**
         * Perform full highlight analysis on the managed document
         * @return Highlight result
         */
        analyze(): DocumentHighlight;

        /**
         * Analyze enough lines to cover the specified visible line range
         * @param visibleRange Visible line range
         * @return Highlight slice for the specified line range
         */
        analyzeLineRange(visibleRange: LineRange): DocumentHighlightSlice;

        /**
         * Incrementally re-analyze the managed document based on patch content
         * @param range Change range of the patch
         * @param newText Patched text
         * @return Highlight result
         */
        analyzeIncremental(range: TextRange, newText: string): DocumentHighlight;

        /**
         * Incrementally re-analyze the managed document based on patch content
         * @param startOffset Start character index of the patch change
         * @param endOffset End character index of the patch change
         * @param newText Patched text
         * @return Highlight result
         */
        analyzeIncremental(startOffset: number, endOffset: number, newText: string): DocumentHighlight;

        /**
         * Incrementally re-analyze and return only highlight slice for the specified line range
         * @param range Change range of the patch
         * @param newText Patched text
         * @param visibleRange Visible line range
         * @return Highlight slice for the specified line range
         */
        analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice;

        /**
         * Get the highlight result slice for the specified visible line range
         * @param visibleRange Visible line range
         * @return Highlight slice
         */
        getHighlightSlice(visibleRange: LineRange): DocumentHighlightSlice;

        /**
         * Get the managed document instance
         */
        getDocument(): Document;

        /**
         * Perform indent guide analysis on the managed document (requires prior call to analyze or analyzeIncremental)
         * @return Indent guide analysis result
         */
        analyzeIndentGuides(): IndentGuideResult;
    }

    /**
     * Highlight configuration
     */
    export class HighlightConfig {
        /**
         * Whether the analysis result includes character index; without it, each TokenSpan only has line and column
         */
        showIndex: boolean;
        /**
         * Whether to use inline styles, i.e. style definitions are embedded directly in syntax rule JSON, and the analysis result contains style info instead of returning style IDs
         */
        inlineStyle: boolean;
        /**
         * Tab width, used for indent guide level calculation (1 tab = tabSize spaces)
         */
        tabSize: number;
    }

    /**
     * Syntax rule
     */
    export class SyntaxRule {
        /**
         * Get the name of the syntax rule
         */
        getName(): string;
    }

    /**
     * Highlight engine
     */
    export class HighlightEngine {
        /**
         * Constructor
         * @param config Highlight configuration
         */
        constructor(config: HighlightConfig);

        /**
         * Register a highlight style for name mapping
         * @param styleName Style name
         * @param styleId Style ID
         */
        registerStyleName(styleName: string, styleId: number): void;

        /**
         * Get the registered style name by style ID
         * @param styleId Style ID
         * @return Style name
         */
        getStyleName(styleId: number): string;

        /**
         * Define a macro
         * @param macroName Macro name
         */
        defineMacro(macroName: string): void;

        /**
         * Undefine a macro
         * @param macroName Macro name
         */
        undefineMacro(macroName: string): void;

        /**
         * Compile syntax rule from JSON
         * @param json JSON content of the syntax rule
         * @throws SyntaxCompileError on compilation error
         */
        compileSyntaxFromJson(json: string): void;

        /**
         * Compile syntax rule
         * @param path Syntax rule definition file path (JSON)
         * @throws SyntaxCompileError on compilation error
         */
        compileSyntaxFromFile(path: string): void;

        /**
         * Get syntax rule by name (e.g. java)
         * @param syntaxName Syntax rule name
         */
        getSyntaxRuleByName(syntaxName: string): SyntaxRule | null;

        /**
         * Get syntax rule by file name or path
         * @param fileName File name or path
         */
        getSyntaxRuleByFileName(fileName: string): SyntaxRule | null;

        /**
         * Create a text highlight analyzer by syntax rule name (no incremental analysis support, but supports single-line analysis with line state for custom incremental analysis)
         * @param syntaxName Syntax rule name (e.g. java)
         */
        createAnalyzerBySyntaxName(syntaxName: string): TextAnalyzer | null;

        /**
         * Create a text highlight analyzer by file name or path (no incremental analysis support, but supports single-line analysis with line state for custom incremental analysis)
         * @param fileName File name or path
         */
        createAnalyzerByFileName(fileName: string): TextAnalyzer | null;

        /**
         * Load a managed document and get a document highlight analyzer
         * @param document Managed document
         * @return Document highlight analyzer
         */
        loadDocument(document: Document): DocumentAnalyzer | null;

        /**
         * Remove a managed document
         * @param uri Managed document URI
         */
        removeDocument(uri: string): void;
    }
}

/**
 * Filesystem backend descriptor used by Emscripten FS.
 * For SweetLine's handwritten typings, only NODEFS is modeled explicitly.
 */
export interface SweetLineFSBackend {
    mount(mount: SweetLineFSMount): unknown;
}

/**
 * Mounted filesystem record returned by Emscripten FS.
 */
export interface SweetLineFSMount {
    type: SweetLineFSBackend;
    mountpoint: string;
    opts: Record<string, unknown> | null;
}

/**
 * Mount options for NODEFS.
 */
export interface SweetLineNodeFSOptions {
    root: string;
}

export interface SweetLineFileSystems {
    /**
     * Node.js-backed filesystem bridge.
     * Only available in Node-capable Emscripten environments.
     */
    NODEFS: SweetLineFSBackend;
}

export interface SweetLineFS {
    /**
     * Registered filesystem backends.
     */
    filesystems: SweetLineFileSystems;

    /**
     * Get current working directory.
     */
    cwd(): string;

    /**
     * Change current working directory.
     */
    chdir(path: string): void;

    /**
     * Create a single directory.
     */
    mkdir(path: string): void;

    /**
     * Create a directory tree recursively.
     */
    mkdirTree(path: string): void;

    /**
     * Mount a filesystem backend at the given mount point.
     */
    mount(type: SweetLineFSBackend, opts: SweetLineNodeFSOptions | Record<string, unknown> | null, mountpoint: string): SweetLineFSMount;

    /**
     * Unmount a mounted filesystem.
     */
    unmount(mountpoint: string): void;

    readFile(path: string, options?: { encoding?: "utf8" | "binary"; flags?: string }): string | Uint8Array;
    writeFile(path: string, data: string | ArrayBufferView | ArrayBuffer, options?: { encoding?: "utf8" | "binary"; flags?: string }): void;
    unlink(path: string): void;
}

/**
 * Runtime module object returned by the default factory.
 * It contains all embind-exposed constructors plus selected Emscripten runtime helpers.
 */
export type SweetLineModule = typeof SweetLineBindings & {
    FS: SweetLineFS;
    addRunDependency(id: string): void;
    removeRunDependency(id: string): void;
    calledRun: boolean;
};

export interface SweetLineModuleFactoryOptions {
    locateFile?(path: string, prefix: string): string;
    print?(message?: unknown, ...optionalParams: unknown[]): void;
    printErr?(message?: unknown, ...optionalParams: unknown[]): void;
    onAbort?(reason: unknown): void;
    onRuntimeInitialized?(): void;
    preInit?: Array<() => void> | (() => void);
    preRun?: Array<(module: SweetLineModule) => void> | ((module: SweetLineModule) => void);
    postRun?: Array<(module: SweetLineModule) => void> | ((module: SweetLineModule) => void);
    monitorRunDependencies?(left: number): void;
    instantiateWasm?(
        imports: WebAssembly.Imports,
        successCallback: (instance: WebAssembly.Instance, module?: WebAssembly.Module) => void,
    ): void;
    preloadPlugins?: unknown[];
    wasmBinary?: ArrayBuffer | Uint8Array;
    arguments?: string[];
    thisProgram?: string;
    noFSInit?: boolean;
    noExitRuntime?: boolean;
    setStatus?(text: string): void;
}

declare function createSweetLine(module?: SweetLineModuleFactoryOptions): Promise<SweetLineModule>;

export default createSweetLine;
