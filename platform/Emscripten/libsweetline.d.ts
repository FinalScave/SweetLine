export namespace sweetline {
    /**
     * 文本位置描述
     */
    export class TextPosition {
        /**
         * 文字所处行，起始为0
         */
        line: number;
        /**
         * 文字所处列，起始为0
         */
        column: number;
        /**
         * 文字在全文中的索引，起始为0
         */
        index: number;
    }

    /**
     * 文字的范围区间描述
     */
    export class TextRange {
        /**
         * 起始位置
         */
        start: TextPosition;
        /**
         * 结束位置
         */
        end: TextPosition;
    }

    /**
     * 支持增量更新的托管文档
     */
    export class Document {
        /**
         * 构造函数
         * @param uri 文档的Uri
         * @param content 文档内容
         */
        constructor(uri: string, content: string);

        /**
         * 获取托管文档的Uri
         */
        getUri(): string;
    }

    /**
     * 语法规则中直接包含的样式定义
     */
    export class InlineStyle {
        /**
         * 前景色
         */
        foreground: number;
        /**
         * 背景色
         */
        background: number;
        /**
         * 是否粗体显示
         */
        isBold: boolean;
        /**
         * 是否斜体显示
         */
        isItalic: boolean;
        /**
         * 是否需要显示删除线
         */
        isStrikethrough: boolean;
    }

    /**
     * 每一个高亮块
     */
    export class TokenSpan {
        /**
         * 高亮区域
         */
        range: TextRange;
        /**
         * 高亮样式ID
         */
        styleId: number;
        /**
         * 高亮块样式详细信息，inlineStyle 模式下才有该字段
         */
        inlineStyle: InlineStyle;
    }

    export class TokenSpanList {
        get(index: number): TokenSpan;
        set(index: number, element: TokenSpan): void;
        add(element: TokenSpan): void;
        remove(element: TokenSpan): void;
        isEmpty(): boolean;
        size(): number;
    }

    /**
     * 每一行的高亮块序列
     */
    export class LineHighlight {
        /**
         * 高亮序列
         */
        spans: TokenSpanList;

        /**
         * 转为Json字符串
         */
        toJson(): string;
    }

    export class LineHighlightList {
        get(index: number): LineHighlight;
        set(index: number, element: LineHighlight): void;
        add(element: LineHighlight): void;
        remove(element: LineHighlight): void;
        isEmpty(): boolean;
        size(): number;
    }

    /**
     * 整个文本内容的高亮
     */
    export class DocumentHighlight {
        /**
         * 每一行的高亮序列
         */
        lines: LineHighlightList;

        /**
         * 转为Json字符串
         */
        toJson(): string;
    }

    /**
     * 行范围描述（0-based）
     */
    export class LineRange {
        /**
         * 起始行号
         */
        startLine: number;
        /**
         * 行数量
         */
        lineCount: number;
    }

    /**
     * 指定行区域高亮切片
     */
    export class DocumentHighlightSlice {
        /**
         * 切片起始行
         */
        startLine: number;
        /**
         * patch后文档总行数
         */
        totalLineCount: number;
        /**
         * 切片行高亮序列
         */
        lines: LineHighlightList;
    }

    /**
     * 行作用域划线分析状态
     */
    export class LineScopeState {
        /**
         * 行所处嵌套层级
         */
        nestingLevel: number;
        /**
         * 行所处作用域划线状态: 0=START, 1=END, 2=CONTENT
         */
        scopeState: number;
        /**
         * 作用域划线所处列
         */
        scopeColumn: number;
        /**
         * 该行缩进等级
         */
        indentLevel: number;
    }

    /**
     * 单条缩进划线（纵向线段）
     */
    export class IndentGuideLine {
        /**
         * 划线所在列（字符列）
         */
        column: number;
        /**
         * 起始行号
         */
        startLine: number;
        /**
         * 结束行号
         */
        endLine: number;
        /**
         * 嵌套层级（0-based）
         */
        nestingLevel: number;
        /**
         * 关联的 ScopeRule id（匹配对模式），-1=缩进模式
         */
        scopeRuleId: number;
        /**
         * 分支点列表（如 else/case 的行列位置）
         */
        branches: BranchPointList;
    }

    /**
     * 分支点（如 else/case 的位置）
     */
    export class BranchPoint {
        line: number;
        column: number;
    }

    export class BranchPointList {
        get(index: number): BranchPoint;
        set(index: number, element: BranchPoint): void;
        add(element: BranchPoint): void;
        remove(element: BranchPoint): void;
        isEmpty(): boolean;
        size(): number;
    }

    export class Int32List {
        get(index: number): number;
        set(index: number, element: number): void;
        add(element: number): void;
        remove(element: number): void;
        isEmpty(): boolean;
        size(): number;
    }

    export class IndentGuideLineList {
        get(index: number): IndentGuideLine;
        set(index: number, element: IndentGuideLine): void;
        add(element: IndentGuideLine): void;
        remove(element: IndentGuideLine): void;
        isEmpty(): boolean;
        size(): number;
    }

    export class LineScopeStateList {
        get(index: number): LineScopeState;
        set(index: number, element: LineScopeState): void;
        add(element: LineScopeState): void;
        remove(element: LineScopeState): void;
        isEmpty(): boolean;
        size(): number;
    }

    /**
     * 缩进划线分析结果
     */
    export class IndentGuideResult {
        /**
         * 所有纵向划线
         */
        guideLines: IndentGuideLineList;
        /**
         * 每行的作用域状态
         */
        lineStates: LineScopeStateList;
    }

    /**
     * 文本行元数据信息
     */
    export class TextLineInfo {
        /**
         * 行号索引
         */
        line: number;

        /**
         * 行起始高亮状态
         */
        startState: number;

        /**
         * 行在整个文本中起始字符偏移 (不是字节)，用于计算高亮块(TokenSpan) index，HighlightConfig中未开启 showIndex 时 无需该字段
         */
        startCharOffset: number;
    }

    /**
     * 单行语法高亮分析结果
     */
    export class LineAnalyzeResult {
        /**
         * 当前行高亮序列
         */
        highlight: LineHighlight;

        /**
         * 行分析完毕后结束状态
         */
        endState: number;

        /**
         * 当前行总计分析的字符总数，不包含换行符
         */
        charCount: number;
    }

    /**
     * 纯文本高亮分析器，不支持增量更新，适用于全量分析的场景
     */
    export class TextAnalyzer {
        /**
         * 分析一段文本内容，并返回整段文本的高亮结果
         * @param text 整段文本内容
         * @return 高亮结果
         */
        analyzeText(text: string): DocumentHighlight;

        /**
         * 分析单行文本
         * @param text 单行文本内容
         * @param info 当前行元数据信息
         * @return 单行高亮分析结果
         */
        analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult;

        /**
         * 对一段文本进行缩进划线分析（内部会先进行高亮分析）
         * @param text 整段文本内容
         * @return 缩进划线分析结果
         */
        analyzeIndentGuides(text: string): IndentGuideResult;
    }

    /**
     * 托管文档高亮分析器，支持自动patch文本进行增量分析
     */
    export class DocumentAnalyzer {
        /**
         * 对整个托管文档进行高亮分析
         * @return 整个托管文档的高亮结果
         */
        analyze(): DocumentHighlight;

        /**
         * 根据patch内容重新分析整个托管文档的高亮结果
         * @param range patch的变更范围
         * @param newText patch的文本
         * @return 整个托管文档的高亮结果
         */
        analyzeIncremental(range: TextRange, newText: string): DocumentHighlight;

        /**
         * 根据patch内容重新分析整个托管文档的高亮结果
         * @param startOffset patch变更的起始字符索引
         * @param endOffset patch变更的结束字符索引
         * @param newText patch的文本
         * @return 整个托管文档的高亮结果
         */
        analyzeIncremental(startOffset: number, endOffset: number, newText: string): DocumentHighlight;

        /**
         * 根据patch内容重新分析并仅返回指定行区域高亮切片
         * @param range patch的变更范围
         * @param newText patch的文本
         * @param visibleRange 可见行范围
         * @return 指定行区域高亮切片
         */
        analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice;

        /**
         * 对托管文档进行缩进划线分析（需先调用 analyze 或 analyzeIncremental）
         * @return 缩进划线分析结果
         */
        analyzeIndentGuides(): IndentGuideResult;
    }

    /**
     * 高亮配置
     */
    export class HighlightConfig {
        /**
         * 分析的高亮信息是否携带index，不携带的情况下每个TokenSpan只有line和column
         */
        showIndex: boolean;
        /**
         * 是否支持内联样式，即不需要外部注册高亮样式，直接在语法规则json中定义高亮样式，高亮分析结果中直接包含高亮样式(前景色、加粗等），而不是返回样式ID
         */
        inlineStyle: boolean;
        /**
         * Tab宽度，用于缩进划线的缩进等级计算 (1 tab = tabSize 个空格)
         */
        tabSize: number;
    }

    /**
     * 语法规则
     */
    export class SyntaxRule {
        /**
         * 获取语法规则的名称
         */
        getName(): string;
    }

    /**
     * 高亮引擎
     */
    export class HighlightEngine {
        /**
         * 构造函数
         * @param config 高亮配置
         */
        constructor(config: HighlightConfig);

        /**
         * 注册一个高亮样式，用于名称映射
         * @param styleName 样式名称
         * @param styleId 样式id
         */
        registerStyleName(styleName: string, styleId: number): void;

        /**
         * 通过样式id获取注册的样式名称
         * @param styleId 样式id
         * @return 样式名称
         */
        getStyleName(styleId: number): string;

        /**
         * 定义一个宏
         * @param macroName 宏名称
         */
        defineMacro(macroName: string): void;

        /**
         * 取消定义宏
         * @param macroName 宏名称
         */
        undefineMacro(macroName: string): void;

        /**
         * 通过json编译语法规则
         * @param json 语法规则文件的json
         * @throws 编译错误时会抛出 SyntaxRuleParseError
         */
        compileSyntaxFromJson(json: string): SyntaxRule;

        /**
         * 编译语法规则
         * @param path 语法规则定义文件路径(json)
         * @throws 编译错误时会抛出 SyntaxRuleParseError
         */
        compileSyntaxFromFile(path: string): SyntaxRule;

        /**
         * 获取指定名称的语法规则(如 java)
         * @param syntaxName 语法规则名称
         */
        getSyntaxRuleByName(syntaxName: string): SyntaxRule;

        /**
         * 获取指定后缀名匹配的的语法规则(如 .t)
         * @param extension 后缀名
         */
        getSyntaxRuleByExtension(extension: string): SyntaxRule;

        /**
         * 根据语法规则名称创建一个文本高亮分析器(不支持增量分析,但可以分析单行并获得行状态,可以在上层自行实现增量分析)
         * @param syntaxName 语法规则名称(如 java)
         */
        createAnalyzerByName(syntaxName: string): TextAnalyzer;

        /**
         * 根据文件后缀名创建一个文本高亮分析器(不支持增量分析,但可以分析单行并获得行状态,可以在上层自行实现增量分析)
         * @param extension 文件后缀名(如 .t)
         */
        createAnalyzerByExtension(extension: string): TextAnalyzer;

        /**
         * 加载托管文档对象获得文档高亮分析器
         * @param document 托管文档对象
         * @return 文档高亮分析器
         */
        loadDocument(document: Document): DocumentAnalyzer;

        /**
         * 移除托管文档
         * @param uri 托管文档Uri
         */
        removeDocument(uri: string): void;
    }
}
