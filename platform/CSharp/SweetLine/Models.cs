namespace SweetLine;

/// <summary>
/// Error codes from SweetLine C API (<c>sl_error_t</c>).
/// </summary>
public enum SweetLineErrorCode {
	/// <summary>No error.</summary>
	Ok = 0,
	/// <summary>Invalid native handle.</summary>
	HandleInvalid = 1,
	/// <summary>Missing property in syntax rule JSON.</summary>
	JsonPropertyMissed = -1,
	/// <summary>Invalid property value in syntax rule JSON.</summary>
	JsonPropertyInvalid = -2,
	/// <summary>Invalid regex pattern in syntax rule JSON.</summary>
	PatternInvalid = -3,
	/// <summary>Invalid state in syntax rule JSON.</summary>
	StateInvalid = -4,
	/// <summary>Malformed syntax rule JSON.</summary>
	JsonInvalid = -5,
	/// <summary>File IO error while reading syntax file.</summary>
	FileIoError = -6,
	/// <summary>Syntax file content is empty.</summary>
	FileEmpty = -7
}

/// <summary>
/// Highlight configuration.
/// </summary>
/// <param name="ShowIndex">Whether analysis result includes character index.</param>
/// <param name="InlineStyle">Whether to return inline style instead of style ID.</param>
public readonly record struct HighlightConfig(bool ShowIndex = false, bool InlineStyle = false);

/// <summary>
/// Text position descriptor.
/// </summary>
/// <param name="Line">Line number (0-based).</param>
/// <param name="Column">Column number (0-based).</param>
/// <param name="Index">Character index in full text (0-based).</param>
public readonly record struct TextPosition(int Line, int Column, int Index = 0);

/// <summary>
/// Text range descriptor.
/// </summary>
/// <param name="Start">Start position.</param>
/// <param name="End">End position.</param>
public readonly record struct TextRange(TextPosition Start, TextPosition End);

/// <summary>
/// Text line metadata for single-line analysis.
/// </summary>
/// <param name="Line">Line index.</param>
/// <param name="StartState">Start highlight state of the line.</param>
/// <param name="StartCharOffset">Start character offset in full text.</param>
public readonly record struct TextLineInfo(int Line, int StartState, int StartCharOffset = 0);

/// <summary>
/// Line range descriptor (0-based).
/// </summary>
/// <param name="StartLine">Start line number.</param>
/// <param name="LineCount">Line count.</param>
public readonly record struct LineRange(int StartLine, int LineCount);

/// <summary>
/// Line scope state for indent guide analysis.
/// </summary>
/// <param name="NestingLevel">Nesting level of the line.</param>
/// <param name="ScopeState">Scope state: 0=START, 1=END, 2=CONTENT.</param>
/// <param name="ScopeColumn">Column of the scope marker.</param>
/// <param name="IndentLevel">Indentation level of the line.</param>
public readonly record struct LineScopeState(int NestingLevel, int ScopeState, int ScopeColumn, int IndentLevel);

/// <summary>
/// Inline style definition embedded in syntax rules.
/// </summary>
public sealed class InlineStyle {
	/// <summary>Font attribute bitmask for bold.</summary>
	public const int StyleBold = 1;
	/// <summary>Font attribute bitmask for italic.</summary>
	public const int StyleItalic = StyleBold << 1;
	/// <summary>Font attribute bitmask for strikethrough.</summary>
	public const int StyleStrikeThrough = StyleItalic << 1;

	/// <summary>Foreground color.</summary>
	public int Foreground { get; }
	/// <summary>Background color.</summary>
	public int Background { get; }
	/// <summary>Whether to display in bold.</summary>
	public bool IsBold { get; }
	/// <summary>Whether to display in italic.</summary>
	public bool IsItalic { get; }
	/// <summary>Whether to display with strikethrough.</summary>
	public bool IsStrikethrough { get; }

	/// <summary>
	/// Constructs an inline style with explicit booleans.
	/// </summary>
	public InlineStyle(int foreground, int background, bool isBold, bool isItalic, bool isStrikethrough) {
		Foreground = foreground;
		Background = background;
		IsBold = isBold;
		IsItalic = isItalic;
		IsStrikethrough = isStrikethrough;
	}

	/// <summary>
	/// Constructs an inline style from a font attribute bitmask.
	/// </summary>
	public InlineStyle(int foreground, int background, int fontAttributes)
		: this(
			foreground,
			background,
			(fontAttributes & StyleBold) != 0,
			(fontAttributes & StyleItalic) != 0,
			(fontAttributes & StyleStrikeThrough) != 0) {
	}
}

/// <summary>
/// A highlight token span.
/// </summary>
public sealed class TokenSpan {
	/// <summary>Highlight range.</summary>
	public TextRange Range { get; }
	/// <summary>Highlight style ID (valid in non-inline style mode).</summary>
	public int StyleId { get; }
	/// <summary>Detailed inline style (valid in inline style mode).</summary>
	public InlineStyle? InlineStyle { get; }

	/// <summary>
	/// Constructs a token span with style ID.
	/// </summary>
	public TokenSpan(TextRange range, int styleId) {
		Range = range;
		StyleId = styleId;
	}

	/// <summary>
	/// Constructs a token span with inline style.
	/// </summary>
	public TokenSpan(TextRange range, InlineStyle inlineStyle) {
		Range = range;
		StyleId = -1;
		InlineStyle = inlineStyle;
	}
}

/// <summary>
/// Highlight token span sequence for a line.
/// </summary>
public sealed class LineHighlight {
	public List<TokenSpan> Spans { get; } = [];
}

/// <summary>
/// Highlight result for the entire document.
/// </summary>
public sealed class DocumentHighlight {
	public List<LineHighlight> Lines { get; } = [];
}

/// <summary>
/// Highlight slice for a specified line range.
/// </summary>
public sealed class DocumentHighlightSlice {
	/// <summary>Slice start line.</summary>
	public int StartLine { get; }
	/// <summary>Total line count after patch.</summary>
	public int TotalLineCount { get; }
	/// <summary>Highlight sequence for slice lines.</summary>
	public List<LineHighlight> Lines { get; }

	/// <summary>
	/// Constructs an empty highlight slice.
	/// </summary>
	public DocumentHighlightSlice()
		: this(0, 0, []) {
	}

	/// <summary>
	/// Constructs a highlight slice.
	/// </summary>
	public DocumentHighlightSlice(int startLine, int totalLineCount, List<LineHighlight>? lines = null) {
		StartLine = startLine;
		TotalLineCount = totalLineCount;
		Lines = lines ?? [];
	}
}

/// <summary>
/// Single-line syntax highlight analysis result.
/// </summary>
public sealed class LineAnalyzeResult {
	/// <summary>Highlight sequence for the current line.</summary>
	public LineHighlight Highlight { get; }
	/// <summary>End state after line analysis.</summary>
	public int EndState { get; }
	/// <summary>Total character count analyzed in the line.</summary>
	public int CharCount { get; }

	public LineAnalyzeResult(LineHighlight highlight, int endState, int charCount) {
		Highlight = highlight;
		EndState = endState;
		CharCount = charCount;
	}
}

/// <summary>
/// Single indent guide line (vertical line segment).
/// </summary>
public sealed class IndentGuideLine {
	/// <summary>
	/// Branch point (for example, <c>else</c>/<c>case</c> positions).
	/// </summary>
	public readonly record struct BranchPoint(int Line, int Column);

	/// <summary>Column of the guide line.</summary>
	public int Column { get; }
	/// <summary>Start line number.</summary>
	public int StartLine { get; }
	/// <summary>End line number.</summary>
	public int EndLine { get; }
	/// <summary>Nesting level (0-based).</summary>
	public int NestingLevel { get; }
	/// <summary>Associated scope rule ID, -1 in indentation mode.</summary>
	public int ScopeRuleId { get; }
	/// <summary>Branch points on this guide line.</summary>
	public List<BranchPoint> Branches { get; }

	public IndentGuideLine(int column, int startLine, int endLine, int nestingLevel, int scopeRuleId)
		: this(column, startLine, endLine, nestingLevel, scopeRuleId, []) {
	}

	public IndentGuideLine(
		int column,
		int startLine,
		int endLine,
		int nestingLevel,
		int scopeRuleId,
		List<BranchPoint>? branches) {
		Column = column;
		StartLine = startLine;
		EndLine = endLine;
		NestingLevel = nestingLevel;
		ScopeRuleId = scopeRuleId;
		Branches = branches ?? [];
	}
}

/// <summary>
/// Indent guide analysis result.
/// </summary>
public sealed class IndentGuideResult {
	/// <summary>All vertical guide lines.</summary>
	public List<IndentGuideLine> GuideLines { get; } = [];
	/// <summary>Per-line block scope states.</summary>
	public List<LineScopeState> LineStates { get; } = [];
}

/// <summary>
/// Exception thrown when syntax rule compilation fails.
/// </summary>
public sealed class SyntaxCompileError : Exception {
	/// <summary>Error code from native compile result.</summary>
	public int ErrorCode { get; }

	public SyntaxCompileError(int errorCode, string? message)
		: base(message) {
		ErrorCode = errorCode;
	}
}
