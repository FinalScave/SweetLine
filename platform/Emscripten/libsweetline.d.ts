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
         * 前景色
         */
        background: number;
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
     * 高亮分析器
     */
    export class DocumentAnalyzer {
        /**
         * 对整个文本进行高亮分析
         * @return 整个文本的高亮结果
         */
        analyze(): DocumentHighlight;

        /**
         * 根据patch内容重新分析整个文本的高亮结果
         * @param range patch的变更范围
         * @param newText patch的文本
         * @return 整个文本的高亮结果
         */
        analyzeChanges(range: TextRange, newText: string): DocumentHighlight;
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