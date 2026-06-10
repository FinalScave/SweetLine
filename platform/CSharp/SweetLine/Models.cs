namespace SweetLine;

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
/// Bracket token kind.
/// </summary>
public enum BracketTokenKind {
	Open = 0,
	Close = 1
}

/// <summary>
/// Bracket match state.
/// </summary>
public enum BracketMatchState {
	Matched = 0,
	Unmatched = 1,
	Unknown = 2
}

/// <summary>
/// Single bracket token.
/// </summary>
public sealed class BracketToken {
	public TextRange Range { get; }
	public int Depth { get; }
	public BracketTokenKind Kind { get; }
	public BracketMatchState MatchState { get; }
	public TextRange? PartnerRange { get; }

	public BracketToken(
		TextRange range,
		int depth,
		BracketTokenKind kind,
		BracketMatchState matchState,
		TextRange? partnerRange = null) {
		Range = range;
		Depth = depth;
		Kind = kind;
		MatchState = matchState;
		PartnerRange = partnerRange;
	}
}

/// <summary>
/// Bracket token sequence for a line.
/// </summary>
public sealed class LineBracketPairs {
	public List<BracketToken> Tokens { get; } = [];
}

/// <summary>
/// Bracket pair analysis result.
/// </summary>
public sealed class BracketPairResult {
	public int StartLine { get; }
	public int TotalLineCount { get; }
	public List<LineBracketPairs> Lines { get; }

	public BracketPairResult()
		: this(0, 0, []) {
	}

	public BracketPairResult(int startLine, int totalLineCount, List<LineBracketPairs>? lines = null) {
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
	/// <summary>Whether the guide continues from before the returned slice.</summary>
	public bool ContinuesBefore { get; }
	/// <summary>Whether the guide continues after the returned slice.</summary>
	public bool ContinuesAfter { get; }
	/// <summary>Branch points on this guide line.</summary>
	public List<BranchPoint> Branches { get; }

	public IndentGuideLine(int column, int startLine, int endLine, bool continuesBefore, bool continuesAfter)
		: this(column, startLine, endLine, continuesBefore, continuesAfter, []) {
	}

	public IndentGuideLine(
		int column,
		int startLine,
		int endLine,
		bool continuesBefore,
		bool continuesAfter,
		List<BranchPoint>? branches) {
		Column = column;
		StartLine = startLine;
		EndLine = endLine;
		ContinuesBefore = continuesBefore;
		ContinuesAfter = continuesAfter;
		Branches = branches ?? [];
	}
}

/// <summary>
/// Indent guide analysis result.
/// </summary>
public sealed class IndentGuideResult {
	/// <summary>Actual start line of the returned slice.</summary>
	public int StartLine { get; set; }
	/// <summary>All vertical guide lines.</summary>
	public List<IndentGuideLine> GuideLines { get; } = [];
	/// <summary>Per-line block scope states.</summary>
	public List<LineScopeState> LineStates { get; } = [];
}

/// <summary>
/// Exception thrown when syntax rule compilation fails.
/// </summary>
public sealed class SyntaxCompileError : Exception {
	/// <summary>No error.</summary>
	public const int Ok = 0;
	/// <summary>Missing property in syntax rule JSON.</summary>
	public const int ErrJsonPropertyMissed = -1;
	/// <summary>Invalid property value in syntax rule JSON.</summary>
	public const int ErrJsonPropertyInvalid = -2;
	/// <summary>Invalid regex pattern in syntax rule JSON.</summary>
	public const int ErrPatternInvalid = -3;
	/// <summary>Invalid syntax state reference.</summary>
	public const int ErrStateInvalid = -4;
	/// <summary>Malformed syntax rule JSON.</summary>
	public const int ErrJsonInvalid = -5;
	/// <summary>Syntax file does not exist.</summary>
	public const int ErrFileNotExists = -6;
	/// <summary>Syntax file content is empty or invalid.</summary>
	public const int ErrFileInvalid = -7;
	/// <summary>Referenced importSyntax target is not compiled yet.</summary>
	public const int ErrImportSyntaxNotFound = -8;
	/// <summary>Referenced state/subState/onLineEndState target is not defined.</summary>
	public const int ErrStateReferenceNotFound = -9;
	/// <summary>Referenced inline style name is not declared in styles[].</summary>
	public const int ErrInlineStyleReferenceNotFound = -10;

	/// <summary>Error code from native compile result.</summary>
	public int ErrorCode { get; }

	public SyntaxCompileError(int errorCode, string? message)
		: base(message) {
		ErrorCode = errorCode;
	}
}
