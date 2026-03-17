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
	public TextAnalyzer? CreateAnalyzerByName(string syntaxName) {
		ArgumentNullException.ThrowIfNull(syntaxName);
		EnsureOpen();

		IntPtr analyzerHandle = SweetLineNative.EngineCreateTextAnalyzer(_handle, syntaxName);
		return analyzerHandle == IntPtr.Zero ? null : new TextAnalyzer(this, analyzerHandle);
	}

	/// <summary>
	/// Creates a text analyzer by file extension.
	/// </summary>
	/// <param name="extension">File extension (for example, <c>.cs</c>).</param>
	/// <returns>Text analyzer, or <see langword="null"/> if syntax is not found.</returns>
	public TextAnalyzer? CreateAnalyzerByExtension(string extension) {
		ArgumentNullException.ThrowIfNull(extension);
		EnsureOpen();

		IntPtr analyzerHandle = SweetLineNative.EngineCreateTextAnalyzerByExtension(_handle, extension);
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
			return BufferParser.ReadLineAnalyzeResult(resultPtr);
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
	/// buffer[0] = spanCount
	/// buffer[1] = stride
	/// followed by spanCount * stride int32 fields
	/// </code>
	/// </remarks>
	internal static DocumentHighlight ReadDocumentHighlight(IntPtr bufferPtr) {
		DocumentHighlight highlight = new();
		if (bufferPtr == IntPtr.Zero) {
			return highlight;
		}

		int spanCount = ReadInt(bufferPtr, 0);
		int stride = ReadInt(bufferPtr, 1);
		if (spanCount <= 0 || stride <= 0) {
			return highlight;
		}

		int totalInts = CheckedTotalLength(2, spanCount, stride);
		int[] buffer = new int[totalInts];
		Marshal.Copy(bufferPtr, buffer, 0, totalInts);

		LineHighlight? lineHighlight = null;
		int currentLine = -1;
		for (int i = 0; i < spanCount; i++) {
			int baseIndex = 2 + i * stride;
			if (!HasSpanHead(buffer, baseIndex)) {
				break;
			}

			int startLine = buffer[baseIndex];
			int startColumn = buffer[baseIndex + 1];
			int startIndex = buffer[baseIndex + 2];
			int endLine = buffer[baseIndex + 3];
			int endColumn = buffer[baseIndex + 4];
			int endIndex = buffer[baseIndex + 5];

			TokenSpan span = ReadTokenSpan(
				buffer,
				baseIndex,
				stride,
				startLine,
				startColumn,
				startIndex,
				endLine,
				endColumn,
				endIndex);

			if (startLine != currentLine || lineHighlight is null) {
				currentLine = startLine;
				lineHighlight = new LineHighlight();
				highlight.Lines.Add(lineHighlight);
			}

			lineHighlight.Spans.Add(span);
		}

		return highlight;
	}

	/// <summary>
	/// Parses document highlight slice from native buffer.
	/// </summary>
	/// <remarks>
	/// Layout:
	/// <code>
	/// buffer[0] = startLine
	/// buffer[1] = totalLineCount
	/// buffer[2] = lineCount
	/// buffer[3] = spanCount
	/// buffer[4] = stride
	/// followed by spanCount * stride int32 fields
	/// </code>
	/// </remarks>
	internal static DocumentHighlightSlice ReadDocumentHighlightSlice(IntPtr bufferPtr) {
		if (bufferPtr == IntPtr.Zero) {
			return new DocumentHighlightSlice();
		}

		int headerSize = 5;
		int startLine = ReadInt(bufferPtr, 0);
		int totalLineCount = ReadInt(bufferPtr, 1);
		int lineCount = Math.Max(ReadInt(bufferPtr, 2), 0);
		int spanCount = Math.Max(ReadInt(bufferPtr, 3), 0);
		int stride = Math.Max(ReadInt(bufferPtr, 4), 0);

		int totalInts = CheckedTotalLength(headerSize, spanCount, stride);
		int[] buffer = new int[totalInts];
		Marshal.Copy(bufferPtr, buffer, 0, totalInts);

		List<LineHighlight> lines = new(lineCount);
		for (int i = 0; i < lineCount; i++) {
			lines.Add(new LineHighlight());
		}

		for (int i = 0; i < spanCount; i++) {
			int baseIndex = headerSize + i * stride;
			if (!HasSpanHead(buffer, baseIndex)) {
				break;
			}

			int spanStartLine = buffer[baseIndex];
			int spanStartColumn = buffer[baseIndex + 1];
			int spanStartIndex = buffer[baseIndex + 2];
			int spanEndLine = buffer[baseIndex + 3];
			int spanEndColumn = buffer[baseIndex + 4];
			int spanEndIndex = buffer[baseIndex + 5];

			TokenSpan span = ReadTokenSpan(
				buffer,
				baseIndex,
				stride,
				spanStartLine,
				spanStartColumn,
				spanStartIndex,
				spanEndLine,
				spanEndColumn,
				spanEndIndex);

			int localLine = spanStartLine - startLine;
			if (localLine >= 0 && localLine < lines.Count) {
				lines[localLine].Spans.Add(span);
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
	/// buffer[0] = spanCount
	/// buffer[1] = stride
	/// buffer[2] = endState
	/// buffer[3] = charCount
	/// followed by spanCount * stride int32 fields
	/// </code>
	/// </remarks>
	internal static LineAnalyzeResult ReadLineAnalyzeResult(IntPtr bufferPtr) {
		if (bufferPtr == IntPtr.Zero) {
			return new LineAnalyzeResult(new LineHighlight(), 0, 0);
		}

		int spanCount = Math.Max(ReadInt(bufferPtr, 0), 0);
		int stride = Math.Max(ReadInt(bufferPtr, 1), 0);
		int endState = ReadInt(bufferPtr, 2);
		int charCount = ReadInt(bufferPtr, 3);

		int headerSize = 4;
		int totalInts = CheckedTotalLength(headerSize, spanCount, stride);
		int[] buffer = new int[totalInts];
		Marshal.Copy(bufferPtr, buffer, 0, totalInts);

		LineHighlight lineHighlight = new();
		for (int i = 0; i < spanCount; i++) {
			int baseIndex = headerSize + i * stride;
			if (!HasSpanHead(buffer, baseIndex)) {
				break;
			}

			int startLine = buffer[baseIndex];
			int startColumn = buffer[baseIndex + 1];
			int startIndex = buffer[baseIndex + 2];
			int endLine = buffer[baseIndex + 3];
			int endColumn = buffer[baseIndex + 4];
			int endIndex = buffer[baseIndex + 5];

			TokenSpan span = ReadTokenSpan(
				buffer,
				baseIndex,
				stride,
				startLine,
				startColumn,
				startIndex,
				endLine,
				endColumn,
				endIndex);

			lineHighlight.Spans.Add(span);
		}

		return new LineAnalyzeResult(lineHighlight, endState, charCount);
	}

	/// <summary>
	/// Parses indent guide analysis result from native buffer.
	/// </summary>
	/// <remarks>
	/// Layout:
	/// <code>
	/// buffer[0] = guideCount
	/// buffer[1] = guideStride (fixed head fields, currently 6)
	/// buffer[2] = lineStateCount
	/// buffer[3] = lineStateStride (currently 4)
	/// followed by guide data and line-state data
	/// </code>
	/// </remarks>
	internal static IndentGuideResult ReadIndentGuideResult(IntPtr bufferPtr) {
		IndentGuideResult result = new();
		if (bufferPtr == IntPtr.Zero) {
			return result;
		}

		int guideCount = Math.Max(ReadInt(bufferPtr, 0), 0);
		int lineStateCount = Math.Max(ReadInt(bufferPtr, 2), 0);

		int index = 4;
		for (int i = 0; i < guideCount; i++) {
			int column = ReadInt(bufferPtr, index++);
			int startLine = ReadInt(bufferPtr, index++);
			int endLine = ReadInt(bufferPtr, index++);
			int nestingLevel = ReadInt(bufferPtr, index++);
			int scopeRuleId = ReadInt(bufferPtr, index++);
			int branchCount = Math.Max(ReadInt(bufferPtr, index++), 0);

			IndentGuideLine line = new(column, startLine, endLine, nestingLevel, scopeRuleId);
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

	private static bool HasSpanHead(int[] buffer, int baseIndex) {
		return baseIndex >= 0 && baseIndex + 6 < buffer.Length;
	}

	private static int CheckedTotalLength(int headerSize, int itemCount, int stride) {
		if (itemCount <= 0 || stride <= 0) {
			return headerSize;
		}

		long total = headerSize + (long)itemCount * stride;
		if (total > int.MaxValue) {
			throw new InvalidOperationException("Native highlight buffer is too large.");
		}

		return (int)total;
	}

	private static TokenSpan ReadTokenSpan(
		int[] buffer,
		int baseIndex,
		int stride,
		int startLine,
		int startColumn,
		int startIndex,
		int endLine,
		int endColumn,
		int endIndex) {
		TextRange range = new(
			new TextPosition(startLine, startColumn, startIndex),
			new TextPosition(endLine, endColumn, endIndex));

		if (stride > 7 && baseIndex + 8 < buffer.Length) {
			int foreground = buffer[baseIndex + 6];
			int background = buffer[baseIndex + 7];
			int fontAttributes = buffer[baseIndex + 8];
			return new TokenSpan(range, new InlineStyle(foreground, background, fontAttributes));
		}

		int styleId = baseIndex + 6 < buffer.Length ? buffer[baseIndex + 6] : 0;
		return new TokenSpan(range, styleId);
	}
}
