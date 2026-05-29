using System.Runtime.InteropServices;

namespace SweetLine;

/// <summary>
/// Managed document with incremental update support.
/// </summary>
/// <remarks>
/// Use <see cref="Dispose"/> (or <c>using</c>) to deterministically release native resources.
/// </remarks>
public sealed class Document : IDisposable {
	private IntPtr _handle;
	private bool _disposed;

	/// <summary>
	/// Creates a managed document.
	/// </summary>
	/// <param name="uri">Document URI.</param>
	/// <param name="content">Document content.</param>
	public Document(string uri, string content) {
		ArgumentNullException.ThrowIfNull(uri);
		ArgumentNullException.ThrowIfNull(content);

		SweetLineNative.Initialize();
		_handle = SweetLineNative.CreateDocument(uri, content);
		if (_handle == IntPtr.Zero) {
			throw new InvalidOperationException("Failed to create SweetLine document.");
		}
	}

	~Document() {
		Dispose(false);
	}

	/// <summary>
	/// Closes and releases the native document handle.
	/// </summary>
	public void Close() {
		Dispose();
	}

	/// <summary>
	/// Releases native resources associated with this document.
	/// </summary>
	public void Dispose() {
		Dispose(true);
		GC.SuppressFinalize(this);
	}

	internal IntPtr GetHandleOrThrow() {
		EnsureOpen();
		return _handle;
	}

	private void Dispose(bool disposing) {
		if (_disposed) {
			return;
		}

		_disposed = true;
		if (_handle != IntPtr.Zero) {
			try {
				SweetLineNative.FreeDocument(_handle);
			} catch when (!disposing) {
				// Do not allow finalizer path to throw.
			}

			_handle = IntPtr.Zero;
		}
	}

	private void EnsureOpen() {
		if (_disposed) {
			throw new ObjectDisposedException(nameof(Document));
		}
	}
}

/// <summary>
/// SweetLine highlight engine.
/// </summary>
/// <remarks>
/// The engine compiles syntax rules, creates analyzers, and manages document analyzers.
/// </remarks>
public sealed class HighlightEngine : IDisposable {
	private IntPtr _handle;
	private bool _disposed;

	/// <summary>
	/// Creates a highlight engine with the given configuration.
	/// </summary>
	/// <param name="config">Highlight configuration.</param>
	public HighlightEngine(HighlightConfig config) {
		SweetLineNative.Initialize();
		_handle = SweetLineNative.CreateEngine(config.ShowIndex, config.InlineStyle);
		if (_handle == IntPtr.Zero) {
			throw new InvalidOperationException("Failed to create SweetLine engine.");
		}
	}

	/// <summary>
	/// Creates a highlight engine with default configuration.
	/// </summary>
	public HighlightEngine()
		: this(new HighlightConfig()) {
	}

	~HighlightEngine() {
		Dispose(false);
	}

	/// <summary>
	/// Registers a style name mapping.
	/// </summary>
	/// <param name="styleName">Style name.</param>
	/// <param name="styleId">Style ID.</param>
	public void RegisterStyleName(string styleName, int styleId) {
		ArgumentNullException.ThrowIfNull(styleName);
		EnsureOpen();

		int error = SweetLineNative.EngineRegisterStyleName(_handle, styleName, styleId);
		SweetLineNative.ThrowIfError(error, "register style name");
	}

	/// <summary>
	/// Gets the registered style name by style ID.
	/// </summary>
	/// <param name="styleId">Style ID.</param>
	/// <returns>Style name, or <see langword="null"/> if not found.</returns>
	public string? GetStyleName(int styleId) {
		EnsureOpen();

		IntPtr valuePtr = SweetLineNative.EngineGetStyleName(_handle, styleId);
		return valuePtr == IntPtr.Zero ? null : Marshal.PtrToStringUTF8(valuePtr);
	}

	/// <summary>
	/// Defines a macro for <c>#ifdef</c> conditional compilation in syntax import.
	/// </summary>
	/// <param name="macroName">Macro name.</param>
	public void DefineMacro(string macroName) {
		ArgumentNullException.ThrowIfNull(macroName);
		EnsureOpen();

		int error = SweetLineNative.EngineDefineMacro(_handle, macroName);
		SweetLineNative.ThrowIfError(error, "define macro");
	}

	/// <summary>
	/// Undefines a macro.
	/// </summary>
	/// <param name="macroName">Macro name.</param>
	public void UndefineMacro(string macroName) {
		ArgumentNullException.ThrowIfNull(macroName);
		EnsureOpen();

		int error = SweetLineNative.EngineUndefineMacro(_handle, macroName);
		SweetLineNative.ThrowIfError(error, "undefine macro");
	}

	/// <summary>
	/// Compiles a syntax rule from JSON content.
	/// </summary>
	/// <param name="syntaxJson">Syntax JSON content.</param>
	/// <exception cref="SyntaxCompileError">Thrown if compilation fails.</exception>
	public void CompileSyntaxFromJson(string syntaxJson) {
		ArgumentNullException.ThrowIfNull(syntaxJson);
		EnsureOpen();

		SweetLineNative.SyntaxErrorNative syntaxError = SweetLineNative.EngineCompileJson(_handle, syntaxJson);
		SweetLineNative.ThrowIfSyntaxError(syntaxError);
	}

	/// <summary>
	/// Compiles a syntax rule from a JSON file.
	/// </summary>
	/// <param name="path">Path to syntax JSON file.</param>
	/// <exception cref="SyntaxCompileError">Thrown if compilation fails.</exception>
	public void CompileSyntaxFromFile(string path) {
		ArgumentNullException.ThrowIfNull(path);
		EnsureOpen();

		SweetLineNative.SyntaxErrorNative syntaxError = SweetLineNative.EngineCompileFile(_handle, path);
		SweetLineNative.ThrowIfSyntaxError(syntaxError);
	}

	/// <summary>
	/// Creates a text analyzer by syntax rule name.
	/// </summary>
	/// <param name="syntaxName">Syntax name (for example, <c>java</c>).</param>
	/// <returns>Text analyzer, or <see langword="null"/> if syntax is not found.</returns>
	public TextAnalyzer? CreateAnalyzerBySyntaxName(string syntaxName) {
		ArgumentNullException.ThrowIfNull(syntaxName);
		EnsureOpen();

		IntPtr analyzerHandle = SweetLineNative.EngineCreateTextAnalyzer(_handle, syntaxName);
		return analyzerHandle == IntPtr.Zero ? null : new TextAnalyzer(this, analyzerHandle);
	}

	/// <summary>
	/// Creates a text analyzer by file name.
	/// </summary>
	/// <param name="fileName">File name or basename (for example, <c>main.cs</c> or <c>build.gradle.kts</c>).</param>
	/// <returns>Text analyzer, or <see langword="null"/> if syntax is not found.</returns>
	public TextAnalyzer? CreateAnalyzerByFileName(string fileName) {
		ArgumentNullException.ThrowIfNull(fileName);
		EnsureOpen();

		IntPtr analyzerHandle = SweetLineNative.EngineCreateTextAnalyzerByFileName(_handle, fileName);
		return analyzerHandle == IntPtr.Zero ? null : new TextAnalyzer(this, analyzerHandle);
	}

	/// <summary>
	/// Loads a managed document and creates a document analyzer for incremental analysis.
	/// </summary>
	/// <param name="document">Managed document.</param>
	/// <returns>Document analyzer, or <see langword="null"/> if load fails.</returns>
	public DocumentAnalyzer? LoadDocument(Document document) {
		ArgumentNullException.ThrowIfNull(document);
		EnsureOpen();

		IntPtr analyzerHandle = SweetLineNative.EngineLoadDocument(_handle, document.GetHandleOrThrow());
		return analyzerHandle == IntPtr.Zero ? null : new DocumentAnalyzer(this, analyzerHandle);
	}

	/// <summary>
	/// Closes and releases the native engine handle.
	/// </summary>
	public void Close() {
		Dispose();
	}

	/// <summary>
	/// Releases native resources associated with this engine.
	/// </summary>
	public void Dispose() {
		Dispose(true);
		GC.SuppressFinalize(this);
	}

	internal bool IsDisposed => _disposed;

	private void Dispose(bool disposing) {
		if (_disposed) {
			return;
		}

		_disposed = true;
		if (_handle != IntPtr.Zero) {
			try {
				SweetLineNative.FreeEngine(_handle);
			} catch when (!disposing) {
				// Do not allow finalizer path to throw.
			}

			_handle = IntPtr.Zero;
		}
	}

	private void EnsureOpen() {
		if (_disposed) {
			throw new ObjectDisposedException(nameof(HighlightEngine));
		}
	}
}

/// <summary>
/// Plain text highlight analyzer.
/// </summary>
/// <remarks>
/// This analyzer does not support managed document incremental updates.
/// </remarks>
public sealed class TextAnalyzer : IDisposable {
	private readonly HighlightEngine _owner;
	private IntPtr _handle;
	private bool _disposed;

	internal TextAnalyzer(HighlightEngine owner, IntPtr handle) {
		_owner = owner;
		_handle = handle;
	}

	/// <summary>
	/// Analyzes full text and returns full document highlight result.
	/// </summary>
	/// <param name="text">Full text content.</param>
	/// <returns>Highlight result.</returns>
	public DocumentHighlight AnalyzeText(string text) {
		ArgumentNullException.ThrowIfNull(text);
		EnsureOpen();

		IntPtr resultPtr = SweetLineNative.TextAnalyze(_handle, text);
		if (resultPtr == IntPtr.Zero) {
			return new DocumentHighlight();
		}

		try {
			return BufferParser.ReadDocumentHighlight(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Analyzes a single line of text.
	/// </summary>
	/// <param name="text">Line text content.</param>
	/// <param name="info">Line metadata.</param>
	/// <returns>Single-line analysis result.</returns>
	public LineAnalyzeResult AnalyzeLine(string text, TextLineInfo info) {
		ArgumentNullException.ThrowIfNull(text);
		EnsureOpen();

		int[] packedLineInfo = [info.Line, info.StartState, info.StartCharOffset];
		IntPtr resultPtr = SweetLineNative.TextAnalyzeLine(_handle, text, packedLineInfo);
		if (resultPtr == IntPtr.Zero) {
			return new LineAnalyzeResult(new LineHighlight(), 0, 0);
		}

		try {
			return BufferParser.ReadLineAnalyzeResult(resultPtr, info.Line);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Performs indent guide analysis on text.
	/// </summary>
	/// <param name="text">Full text content.</param>
	/// <returns>Indent guide analysis result.</returns>
	public IndentGuideResult AnalyzeIndentGuides(string text) {
		ArgumentNullException.ThrowIfNull(text);
		EnsureOpen();

		IntPtr resultPtr = SweetLineNative.TextAnalyzeIndentGuides(_handle, text);
		if (resultPtr == IntPtr.Zero) {
			return new IndentGuideResult();
		}

		try {
			return BufferParser.ReadIndentGuideResult(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Marks this analyzer as closed.
	/// </summary>
	public void Close() {
		Dispose();
	}

	/// <summary>
	/// Marks this analyzer as disposed.
	/// </summary>
	/// <remarks>
	/// Text analyzer handle is managed by the engine in native side.
	/// </remarks>
	public void Dispose() {
		_disposed = true;
		_handle = IntPtr.Zero;
	}

	private void EnsureOpen() {
		if (_disposed) {
			throw new ObjectDisposedException(nameof(TextAnalyzer));
		}

		if (_owner.IsDisposed) {
			throw new InvalidOperationException("HighlightEngine has already been disposed.");
		}

		if (_handle == IntPtr.Zero) {
			throw new InvalidOperationException("Analyzer handle is invalid.");
		}
	}
}

/// <summary>
/// Managed document analyzer with incremental update support.
/// </summary>
public sealed class DocumentAnalyzer : IDisposable {
	private readonly HighlightEngine _owner;
	private IntPtr _handle;
	private bool _disposed;

	internal DocumentAnalyzer(HighlightEngine owner, IntPtr handle) {
		_owner = owner;
		_handle = handle;
	}

	/// <summary>
	/// Performs full highlight analysis on the managed document.
	/// </summary>
	/// <returns>Full document highlight result.</returns>
	public DocumentHighlight Analyze() {
		EnsureOpen();

		IntPtr resultPtr = SweetLineNative.DocumentAnalyze(_handle);
		if (resultPtr == IntPtr.Zero) {
			return new DocumentHighlight();
		}

		try {
			return BufferParser.ReadDocumentHighlight(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Performs highlight analysis and returns highlight slice in visible range.
	/// </summary>
	/// <param name="visibleRange">Visible line range (<c>startLine + lineCount</c>).</param>
	/// <returns>Highlight slice for visible lines.</returns>
	public DocumentHighlightSlice AnalyzeLineRange(LineRange visibleRange) {
		EnsureOpen();

		int[] packedVisibleRange = [visibleRange.StartLine, visibleRange.LineCount];
		IntPtr resultPtr = SweetLineNative.DocumentAnalyzeLineRange(_handle, packedVisibleRange);
		if (resultPtr == IntPtr.Zero) {
			return new DocumentHighlightSlice();
		}

		try {
			return BufferParser.ReadDocumentHighlightSlice(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Performs incremental highlight analysis and returns full document result.
	/// </summary>
	/// <param name="range">Change range (line/column).</param>
	/// <param name="newText">Replacement text.</param>
	/// <returns>Full document highlight result.</returns>
	public DocumentHighlight AnalyzeIncremental(TextRange range, string newText) {
		ArgumentNullException.ThrowIfNull(newText);
		EnsureOpen();

		int[] changesRange =
		[
			range.Start.Line,
			range.Start.Column,
			range.End.Line,
			range.End.Column
		];

		IntPtr resultPtr = SweetLineNative.DocumentAnalyzeIncremental(_handle, changesRange, newText);
		if (resultPtr == IntPtr.Zero) {
			return new DocumentHighlight();
		}

		try {
			return BufferParser.ReadDocumentHighlight(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Performs incremental analysis and returns highlight slice in visible range.
	/// </summary>
	/// <param name="range">Change range (line/column).</param>
	/// <param name="newText">Replacement text.</param>
	/// <param name="visibleRange">Visible line range (<c>startLine + lineCount</c>).</param>
	/// <returns>Highlight slice for visible lines.</returns>
	public DocumentHighlightSlice AnalyzeIncrementalInLineRange(TextRange range, string newText, LineRange visibleRange) {
		ArgumentNullException.ThrowIfNull(newText);
		EnsureOpen();

		int[] changesRange =
		[
			range.Start.Line,
			range.Start.Column,
			range.End.Line,
			range.End.Column
		];
		int[] packedVisibleRange = [visibleRange.StartLine, visibleRange.LineCount];

		IntPtr resultPtr = SweetLineNative.DocumentAnalyzeIncrementalInLineRange(
			_handle,
			changesRange,
			newText,
			packedVisibleRange);

		if (resultPtr == IntPtr.Zero) {
			return new DocumentHighlightSlice();
		}

		try {
			return BufferParser.ReadDocumentHighlightSlice(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Gets highlight slice from the current cached result.
	/// </summary>
	/// <param name="visibleRange">Visible line range (<c>startLine + lineCount</c>).</param>
	/// <returns>Highlight slice for visible lines.</returns>
	/// <remarks>
	/// Requires a prior call to <see cref="Analyze"/> or <see cref="AnalyzeIncremental(TextRange, string)"/>.
	/// </remarks>
	public DocumentHighlightSlice GetHighlightSlice(LineRange visibleRange) {
		EnsureOpen();

		int[] packedVisibleRange = [visibleRange.StartLine, visibleRange.LineCount];
		IntPtr resultPtr = SweetLineNative.DocumentGetHighlightSlice(_handle, packedVisibleRange);
		if (resultPtr == IntPtr.Zero) {
			return new DocumentHighlightSlice();
		}

		try {
			return BufferParser.ReadDocumentHighlightSlice(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Performs indent guide analysis on the managed document.
	/// </summary>
	/// <returns>Indent guide analysis result.</returns>
	public IndentGuideResult AnalyzeIndentGuides() {
		EnsureOpen();

		IntPtr resultPtr = SweetLineNative.DocumentAnalyzeIndentGuides(_handle);
		if (resultPtr == IntPtr.Zero) {
			return new IndentGuideResult();
		}

		try {
			return BufferParser.ReadIndentGuideResult(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Performs indent guide analysis for the specified visible line range.
	/// </summary>
	/// <param name="visibleRange">Visible line range (<c>startLine + lineCount</c>).</param>
	/// <returns>Indent guide analysis result for the specified line range.</returns>
	public IndentGuideResult AnalyzeIndentGuidesInLineRange(LineRange visibleRange) {
		EnsureOpen();

		int[] packedVisibleRange = [visibleRange.StartLine, visibleRange.LineCount];
		IntPtr resultPtr = SweetLineNative.DocumentAnalyzeIndentGuidesInLineRange(_handle, packedVisibleRange);
		if (resultPtr == IntPtr.Zero) {
			return new IndentGuideResult();
		}

		try {
			return BufferParser.ReadIndentGuideResult(resultPtr);
		} finally {
			SweetLineNative.FreeBuffer(resultPtr);
		}
	}

	/// <summary>
	/// Marks this analyzer as closed.
	/// </summary>
	public void Close() {
		Dispose();
	}

	/// <summary>
	/// Marks this analyzer as disposed.
	/// </summary>
	/// <remarks>
	/// Document analyzer handle is managed by the engine in native side.
	/// </remarks>
	public void Dispose() {
		_disposed = true;
		_handle = IntPtr.Zero;
	}

	private void EnsureOpen() {
		if (_disposed) {
			throw new ObjectDisposedException(nameof(DocumentAnalyzer));
		}

		if (_owner.IsDisposed) {
			throw new InvalidOperationException("HighlightEngine has already been disposed.");
		}

		if (_handle == IntPtr.Zero) {
			throw new InvalidOperationException("Analyzer handle is invalid.");
		}
	}
}

/// <summary>
/// Parses <c>int32_t*</c> buffers returned by SweetLine C API.
/// </summary>
internal static class BufferParser {
	/// <summary>
	/// Parses full document highlight from native buffer.
	/// </summary>
	/// <remarks>
	/// Layout:
	/// <code>
	/// buffer[0] = flags
	/// buffer[1] = stride
	/// buffer[2] = lineCount
	/// followed by lineCount line entries:
	/// lineEntry[0] = spanCount of current line
	/// followed by spanCount * stride int32 fields
	/// </code>
	/// </remarks>
	internal static DocumentHighlight ReadDocumentHighlight(IntPtr bufferPtr) {
		DocumentHighlight highlight = new();
		if (bufferPtr == IntPtr.Zero) {
			return highlight;
		}

		int flags = ReadInt(bufferPtr, 0);
		int stride = Math.Max(ReadInt(bufferPtr, 1), 0);
		int lineCount = Math.Max(ReadInt(bufferPtr, 2), 0);
		bool hasStartIndex = FlagsHasStartIndex(flags);
		bool inlineStyle = FlagsUsesInlineStyle(flags);
		if (!IsValidSpanStride(stride, hasStartIndex, inlineStyle)) {
			return highlight;
		}

		int index = 3;
		for (int line = 0; line < lineCount; line++) {
			LineHighlight lineHighlight = new();
			highlight.Lines.Add(lineHighlight);
			int spanCount = Math.Max(ReadInt(bufferPtr, index++), 0);
			for (int i = 0; i < spanCount; i++) {
				int startColumn = ReadInt(bufferPtr, index++);
				int length = ReadInt(bufferPtr, index++);
				int startIndex = hasStartIndex ? ReadInt(bufferPtr, index++) : 0;
				int endColumn = startColumn + length;
				int endIndex = hasStartIndex ? startIndex + length : 0;
				TextRange range = new(
					new TextPosition(line, startColumn, startIndex),
					new TextPosition(line, endColumn, endIndex));
				if (inlineStyle) {
					int foreground = ReadInt(bufferPtr, index++);
					int background = ReadInt(bufferPtr, index++);
					int fontAttributes = ReadInt(bufferPtr, index++);
					lineHighlight.Spans.Add(new TokenSpan(range, new InlineStyle(foreground, background, fontAttributes)));
				} else {
					int styleId = ReadInt(bufferPtr, index++);
					lineHighlight.Spans.Add(new TokenSpan(range, styleId));
				}
			}
		}

		return highlight;
	}

	/// <summary>
	/// Parses document highlight slice from native buffer.
	/// </summary>
	/// <remarks>
	/// Layout:
	/// <code>
	/// buffer[0] = flags
	/// buffer[1] = stride
	/// buffer[2] = startLine
	/// buffer[3] = totalLineCount
	/// buffer[4] = lineCount
	/// followed by lineCount line entries:
	/// lineEntry[0] = spanCount of current line
	/// followed by spanCount * stride int32 fields
	/// </code>
	/// </remarks>
	internal static DocumentHighlightSlice ReadDocumentHighlightSlice(IntPtr bufferPtr) {
		if (bufferPtr == IntPtr.Zero) {
			return new DocumentHighlightSlice();
		}

		int flags = ReadInt(bufferPtr, 0);
		int stride = Math.Max(ReadInt(bufferPtr, 1), 0);
		int startLine = ReadInt(bufferPtr, 2);
		int totalLineCount = ReadInt(bufferPtr, 3);
		int lineCount = Math.Max(ReadInt(bufferPtr, 4), 0);
		bool hasStartIndex = FlagsHasStartIndex(flags);
		bool inlineStyle = FlagsUsesInlineStyle(flags);
		if (!IsValidSpanStride(stride, hasStartIndex, inlineStyle)) {
			return new DocumentHighlightSlice(startLine, totalLineCount, []);
		}

		List<LineHighlight> lines = new(lineCount);
		for (int i = 0; i < lineCount; i++) {
			lines.Add(new LineHighlight());
		}

		int index = 5;
		for (int i = 0; i < lineCount; i++) {
			LineHighlight lineHighlight = lines[i];
			int line = startLine + i;
			int spanCount = Math.Max(ReadInt(bufferPtr, index++), 0);
			for (int s = 0; s < spanCount; s++) {
				int startColumn = ReadInt(bufferPtr, index++);
				int length = ReadInt(bufferPtr, index++);
				int startIndex = hasStartIndex ? ReadInt(bufferPtr, index++) : 0;
				int endColumn = startColumn + length;
				int endIndex = hasStartIndex ? startIndex + length : 0;
				TextRange range = new(
					new TextPosition(line, startColumn, startIndex),
					new TextPosition(line, endColumn, endIndex));
				if (inlineStyle) {
					int foreground = ReadInt(bufferPtr, index++);
					int background = ReadInt(bufferPtr, index++);
					int fontAttributes = ReadInt(bufferPtr, index++);
					lineHighlight.Spans.Add(new TokenSpan(range, new InlineStyle(foreground, background, fontAttributes)));
				} else {
					int styleId = ReadInt(bufferPtr, index++);
					lineHighlight.Spans.Add(new TokenSpan(range, styleId));
				}
			}
		}

		return new DocumentHighlightSlice(startLine, totalLineCount, lines);
	}

	/// <summary>
	/// Parses single-line analysis result from native buffer.
	/// </summary>
	/// <remarks>
	/// Layout:
	/// <code>
	/// buffer[0] = flags
	/// buffer[1] = stride
	/// buffer[2] = spanCount
	/// buffer[3] = endState
	/// buffer[4] = charCount
	/// followed by spanCount * stride int32 fields
	/// </code>
	/// </remarks>
	internal static LineAnalyzeResult ReadLineAnalyzeResult(IntPtr bufferPtr) {
		return ReadLineAnalyzeResult(bufferPtr, 0);
	}

	internal static LineAnalyzeResult ReadLineAnalyzeResult(IntPtr bufferPtr, int lineNumber) {
		if (bufferPtr == IntPtr.Zero) {
			return new LineAnalyzeResult(new LineHighlight(), 0, 0);
		}

		int flags = ReadInt(bufferPtr, 0);
		int stride = Math.Max(ReadInt(bufferPtr, 1), 0);
		int spanCount = Math.Max(ReadInt(bufferPtr, 2), 0);
		int endState = ReadInt(bufferPtr, 3);
		int charCount = ReadInt(bufferPtr, 4);
		bool hasStartIndex = FlagsHasStartIndex(flags);
		bool inlineStyle = FlagsUsesInlineStyle(flags);
		if (!IsValidSpanStride(stride, hasStartIndex, inlineStyle)) {
			return new LineAnalyzeResult(new LineHighlight(), endState, charCount);
		}

		LineHighlight lineHighlight = new();
		int index = 5;
		for (int i = 0; i < spanCount; i++) {
			int startColumn = ReadInt(bufferPtr, index++);
			int length = ReadInt(bufferPtr, index++);
			int startIndex = hasStartIndex ? ReadInt(bufferPtr, index++) : 0;
			int endColumn = startColumn + length;
			int endIndex = hasStartIndex ? startIndex + length : 0;
			TextRange range = new(
				new TextPosition(lineNumber, startColumn, startIndex),
				new TextPosition(lineNumber, endColumn, endIndex));
			if (inlineStyle) {
				int foreground = ReadInt(bufferPtr, index++);
				int background = ReadInt(bufferPtr, index++);
				int fontAttributes = ReadInt(bufferPtr, index++);
				lineHighlight.Spans.Add(new TokenSpan(range, new InlineStyle(foreground, background, fontAttributes)));
			} else {
				int styleId = ReadInt(bufferPtr, index++);
				lineHighlight.Spans.Add(new TokenSpan(range, styleId));
			}
		}

		return new LineAnalyzeResult(lineHighlight, endState, charCount);
	}

	/// <summary>
	/// Parses indent guide analysis result from native buffer.
	/// </summary>
	/// <remarks>
	/// Layout:
	/// <code>
	/// buffer[0] = startLine
	/// buffer[1] = lineStateCount
	/// buffer[2] = guideCount
	/// followed by guide data and line-state data
	/// </code>
	/// </remarks>
	internal static IndentGuideResult ReadIndentGuideResult(IntPtr bufferPtr) {
		IndentGuideResult result = new();
		if (bufferPtr == IntPtr.Zero) {
			return result;
		}

		result.StartLine = ReadInt(bufferPtr, 0);
		int lineStateCount = Math.Max(ReadInt(bufferPtr, 1), 0);
		int guideCount = Math.Max(ReadInt(bufferPtr, 2), 0);

		int index = 3;
		for (int i = 0; i < guideCount; i++) {
			int column = ReadInt(bufferPtr, index++);
			int startLine = ReadInt(bufferPtr, index++);
			int endLine = ReadInt(bufferPtr, index++);
			int flags = ReadInt(bufferPtr, index++);
			bool continuesBefore = (flags & 1) != 0;
			bool continuesAfter = (flags & (1 << 1)) != 0;
			int branchCount = Math.Max(ReadInt(bufferPtr, index++), 0);

			IndentGuideLine line = new(column, startLine, endLine, continuesBefore, continuesAfter);
			for (int j = 0; j < branchCount; j++) {
				int branchLine = ReadInt(bufferPtr, index++);
				int branchColumn = ReadInt(bufferPtr, index++);
				line.Branches.Add(new IndentGuideLine.BranchPoint(branchLine, branchColumn));
			}

			result.GuideLines.Add(line);
		}

		for (int i = 0; i < lineStateCount; i++) {
			int nestingLevel = ReadInt(bufferPtr, index++);
			int scopeState = ReadInt(bufferPtr, index++);
			int scopeColumn = ReadInt(bufferPtr, index++);
			int indentLevel = ReadInt(bufferPtr, index++);
			result.LineStates.Add(new LineScopeState(nestingLevel, scopeState, scopeColumn, indentLevel));
		}

		return result;
	}

	private static int ReadInt(IntPtr bufferPtr, int index) {
		return Marshal.ReadInt32(bufferPtr, index * sizeof(int));
	}

	private static bool IsValidSpanStride(int stride, bool hasStartIndex, bool inlineStyle) {
		int expected = 2 + (hasStartIndex ? 1 : 0) + (inlineStyle ? 3 : 1);
		return stride == expected;
	}

	private static bool FlagsUsesInlineStyle(int flags) {
		return (flags & (1 << 1)) != 0;
	}

	private static bool FlagsHasStartIndex(int flags) {
		return (flags & 1) != 0;
	}
}
