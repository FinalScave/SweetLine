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