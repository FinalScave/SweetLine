export declare namespace sweetline {

  /**
   * 文本位置描述
   */
  export class TextPosition {
    /**
     * 文字所处行，起始为0
     */
    public line: number;
    /**
     * 文字所处列，起始为0
     */
    public column: number;
    /**
     * 文字在全文中的索引，起始为0
     */
    public index: number;

    public constructor();
    public constructor(line: number, column: number);
    public constructor(line: number, column: number, index: number);
  }

  /**
   * 文字的范围区间描述
   */
  export class TextRange {
    /**
     * 起始位置
     */
    public start: TextPosition;
    /**
     * 结束位置
     */
    public end: TextPosition;

    public constructor(start: TextPosition, end: TextPosition);
  }

  /**
   * 语法规则中直接包含的样式定义
   */
  export class InlineStyle {
    public static readonly STYLE_BOLD: number;
    public static readonly STYLE_ITALIC: number;
    public static readonly STYLE_STRIKE_THROUGH: number;

    /**
     * 前景色
     */
    public foreground: number;
    /**
     * 背景色
     */
    public background: number;
    /**
     * 是否加粗显示
     */
    public isBold: boolean;
    /**
     * 是否斜体显示
     */
    public isItalic: boolean;
    /**
     * 是否需要显示删除线
     */
    public isStrikethrough: boolean;
  }

  /**
   * 每一个高亮块
   */
  export class TokenSpan {
    /**
     * 高亮区域
     */
    public range: TextRange;
    /**
     * 高亮样式ID (非 inlineStyle 模式下才有该字段)
     */
    public styleId: number;
    /**
     * 高亮块样式详细信息，inlineStyle 模式下才有该字段
     */
    public inlineStyle: InlineStyle | null;

    constructor(range: TextRange, styleId: number);
    public static withInlineStyle(range: TextRange, inlineStyle: InlineStyle): TokenSpan;
  }

  /**
   * 每一行的高亮块序列
   */
  export class LineHighlight {
    /**
     * 高亮序列
     */
    public spans: Array<TokenSpan>;
  }

  /**
   * 整个文本内容的高亮
   */
  export class DocumentHighlight {
    /**
     * 每一行的高亮序列
     */
    public lines: Array<LineHighlight>;
  }

  /**
   * 行范围描述（0-based）
   */
  export class LineRange {
    /**
     * 起始行号
     */
    public startLine: number;
    /**
     * 行数量
     */
    public lineCount: number;

    public constructor(startLine?: number, lineCount?: number);
  }

  /**
   * 指定行区域高亮切片
   */
  export class DocumentHighlightSlice {
    /**
     * 切片起始行
     */
    public startLine: number;
    /**
     * patch 后文档总行数
     */
    public totalLineCount: number;
    /**
     * 切片行高亮序列
     */
    public lines: Array<LineHighlight>;
  }

  /**
   * 行作用域划线分析状态
   */
  export class LineScopeState {
    /**
     * 行所处嵌套层级
     */
    public nestingLevel: number;
    /**
     * 行所处作用域划线状态: 0=START, 1=END, 2=CONTENT
     */
    public scopeState: number;
    /**
     * 作用域划线所处列
     */
    public scopeColumn: number;
    /**
     * 该行缩进等级
     */
    public indentLevel: number;

    public constructor();
    public constructor(nestingLevel: number, scopeState: number, scopeColumn: number, indentLevel: number);
  }

  /**
   * 分支点（如 else/case 的位置）
   */
  export class BranchPoint {
    public line: number;
    public column: number;

    public constructor();
    public constructor(line: number, column: number);
  }

  /**
   * 单条缩进划线（纵向线段）
   */
  export class IndentGuideLine {
    /**
     * 划线所在列（字符列）
     */
    public column: number;
    /**
     * 起始行号
     */
    public startLine: number;
    /**
     * 结束行号
     */
    public endLine: number;
    /**
     * 嵌套层级（0-based）
     */
    public nestingLevel: number;
    /**
     * 关联的 ScopeRule id（匹配对模式），-1=缩进模式
     */
    public scopeRuleId: number;
    /**
     * 分支点列表（如 else/case 的行列位置）
     */
    public branches: Array<BranchPoint>;

    public constructor();
    public constructor(column: number, startLine: number, endLine: number, nestingLevel: number, scopeRuleId: number);
  }

  /**
   * 缩进划线分析结果
   */
  export class IndentGuideResult {
    /**
     * 所有纵向划线
     */
    public guideLines: Array<IndentGuideLine>;
    /**
     * 每行的块状态
     */
    public lineStates: Array<LineScopeState>;
  }

  /**
   * 文本行元数据信息
   */
  export class TextLineInfo {
    /**
     * 行号索引
     */
    public line: number;
    /**
     * 行起始高亮状态
     */
    public startState: number;
    /**
     * 行在整个文本中起始字符偏移
     */
    public startCharOffset: number;

    public constructor(line: number, startState: number);
    public constructor(line: number, startState: number, startCharOffset: number);
  }

  /**
   * 单行语法高亮分析结果
   */
  export class LineAnalyzeResult {
    /**
     * 当前行高亮序列
     */
    public highlight: LineHighlight;
    /**
     * 行分析完毕后结束状态
     */
    public endState: number;
    /**
     * 当前行总计分析的字符总数，不包含换行符
     */
    public charCount: number;
  }

  /**
   * 高亮配置
   */
  export class HighlightConfig {
    /**
     * 分析的高亮信息是否携带index，不携带的情况下每个TokenSpan只有line和column
     */
    public showIndex: boolean;
    /**
     * 是否支持内联样式，即不需要外部注册高亮样式，直接在语法规则json中定义高亮样式，高亮分析结果中直接包含高亮样式(前景色、加粗等），而不是返回样式ID
     */
    public inlineStyle: boolean;
    /**
     * Tab宽度，用于缩进划线的缩进等级计算 (1 tab = tabSize 个空格)
     */
    public tabSize: number;

    public constructor(showIndex: boolean, inlineStyle: boolean);
    public constructor(showIndex: boolean, inlineStyle: boolean, tabSize: number);
  }

  /**
   * 支持增量更新的托管文档
   */
  export class Document {
    public constructor(uri: string, content: string);

    /**
     * 获取托管文档的Uri
     */
    public getUri(): string;
    /**
     * 文档字符总数
     */
    public totalChars(): number;
    /**
     * 获取指定行的字符总数
     */
    public getLineCharCount(line: number): number;
    /**
     * 计算指定行在全文中的起始索引
     */
    public charIndexOfLine(line: number): number;
    /**
     * 将字符索引转换为行列位置
     */
    public charIndexToPosition(index: number): TextPosition;
    /**
     * 获取总行数
     */
    public getLineCount(): number;
    /**
     * 获取指定行的文本信息
     */
    public getLine(line: number): string;
    /**
     * 获取完整文本
     */
    public getText(): string;
  }

  /**
   * 语法规则
   */
  export class SyntaxRule {
    /**
     * 获取语法规则的名称
     */
    public getName(): string;
    /**
     * 获取语法规则支持的文件扩展名
     */
    public getFileExtensions(): string[];
  }

  /**
   * 纯文本高亮分析器，不支持增量更新，适用于全量分析的场景
   */
  export class TextAnalyzer {
    /**
     * 分析一段文本内容，并返回整段文本的高亮结果
     * @param text 整段文本内容
     * @returns 高亮结果
     */
    public analyzeText(text: string): DocumentHighlight;
    /**
     * 分析单行文本
     * @param text 单行文本内容
     * @param info 当前行元数据信息
     * @returns 单行高亮分析结果
     */
    public analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult;
    /**
     * 对一段文本进行缩进划线分析（内部会先进行高亮分析）
     * @param text 整段文本内容
     * @returns 缩进划线分析结果
     */
    public analyzeIndentGuides(text: string): IndentGuideResult;
  }

  /**
   * 高亮分析器
   */
  export class DocumentAnalyzer {
    /**
     * 对整个文本进行高亮分析
     * @returns 整个文本的高亮结果
     */
    public analyze(): DocumentHighlight;
    /**
     * 根据patch内容重新分析整个文本的高亮结果
     * @param range patch的变更范围
     * @param newText patch的文本
     * @returns 整个文本的高亮结果
     */
    public analyzeIncremental(range: TextRange, newText: string): DocumentHighlight;
    /**
     * 根据patch内容重新分析文本，并仅返回指定可见行区域高亮切片
     * @param range patch的变更范围
     * @param newText patch的文本
     * @param visibleRange 可见行区域
     * @returns 指定行区域高亮切片
     */
    public analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice;
    /**
     * 根据patch内容重新分析整个文本的高亮结果(字符索引)
     * @param startIndex patch的起始索引
     * @param endIndex patch的结束索引
     * @param newText patch的文本
     * @returns 整个文本的高亮结果
     */
    public analyzeIncrementalByIndex(startIndex: number, endIndex: number, newText: string): DocumentHighlight;

    /**
     * 对托管文档进行缩进划线分析（需先调用 analyze 或 analyzeIncremental）
     * @returns 缩进划线分析结果
     */
    public analyzeIndentGuides(): IndentGuideResult;

    /**
     * 获取托管文档对象
     */
    public getDocument(): Document;
  }

  /**
   * 高亮引擎
   */
  export class HighlightEngine {
    public constructor(config: HighlightConfig);

    /**
     * 注册一个高亮样式，用于名称映射
     * @param styleName 样式名称
     * @param styleId 样式id
     */
    public registerStyleName(styleName: string, styleId: number): boolean;
    /**
     * 通过样式id获取注册的样式名称
     * @param styleId 样式id
     * @returns 样式名称
     */
    public getStyleName(styleId: number): string;
    /**
     * 定义一个宏，用于控制importSyntax的#ifdef条件编译
     * @param macroName 宏名称
     */
    public defineMacro(macroName: string): boolean;
    /**
     * 取消定义宏
     * @param macroName 宏名称
     */
    public undefineMacro(macroName: string): boolean;
    /**
     * 通过json编译语法规则
     * @param syntaxJson 语法规则文件的json
     */
    public compileSyntaxFromJson(syntaxJson: string): SyntaxRule | null;
    /**
     * 编译语法规则
     * @param path 语法规则定义文件路径(json)
     */
    public compileSyntaxFromFile(path: string): SyntaxRule | null;
    /**
     * 根据语法规则名称创建一个文本高亮分析器
     * @param syntaxName 语法规则名称(如 java)
     */
    public createAnalyzerByName(syntaxName: string): TextAnalyzer | null;
    /**
     * 根据文件后缀名创建一个文本高亮分析器
     * @param extension 文件后缀名(如 .t)
     */
    public createAnalyzerByExtension(extension: string): TextAnalyzer | null;
    /**
     * 加载托管文档对象获得文档高亮分析器
     * @param document 托管文档对象
     * @returns 文档高亮分析器
     */
    public loadDocument(document: Document): DocumentAnalyzer | null;
    /**
     * 移除托管文档
     * @param uri 托管文档Uri
     */
    public removeDocument(uri: string): boolean;
  }
}
