using System.Reflection;
using System.Runtime.InteropServices;

namespace SweetLine;

/// <summary>
/// P/Invoke binding layer for SweetLine native library.
/// </summary>
/// <remarks>
/// Handles library resolving and exposes C API functions with C#-style names.
/// </remarks>
internal static class SweetLineNative {
	/// <summary>
	/// Native layout for <c>sl_syntax_error_t</c>.
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	internal readonly struct SyntaxErrorNative {
		internal readonly int ErrorCode;
		internal readonly IntPtr ErrorMessage;
	}

	private const string NativeLibraryName = "sweetline";

	private static readonly string[] RelativeSearchPaths =
	[
		"../../../../cmake-build-release-visual-studio/bin/Release",
		"../../../../cmake-build-release-visual-studio/bin/Debug",
		"../../../../cmake-build-release-visual-studio/bin",
		"../../../../cmake-build-release/bin",
		"../../../../cmake-build-debug-visual-studio/bin/Debug",
		"../../../../cmake-build-debug-visual-studio/bin",
		"../../../../cmake-build-debug/bin",
		"../../../../cmake-build-debug/lib",
		"../../../../build/bin",
		"../../../../build/lib",
		"../../../../build/mac/lib"
	];

	static SweetLineNative() {
		try {
			NativeLibrary.SetDllImportResolver(typeof(SweetLineNative).Assembly, ResolveLibrary);
		} catch (InvalidOperationException) {
			// Resolver might already be registered by host application.
		}
	}

	/// <summary>
	/// Ensures the static constructor has run.
	/// </summary>
	internal static void Initialize() {
		// Intentionally empty. Calling this ensures the static constructor has executed.
	}

	/// <summary>
	/// Throws <see cref="InvalidOperationException"/> when native error code is non-zero.
	/// </summary>
	internal static void ThrowIfError(int errorCode, string action) {
		if (errorCode == (int)SweetLineErrorCode.Ok) {
			return;
		}

		throw new InvalidOperationException($"Failed to {action}. Native error code: {errorCode}.");
	}

	/// <summary>
	/// Throws <see cref="SyntaxCompileError"/> when syntax compile result contains error.
	/// </summary>
	internal static void ThrowIfSyntaxError(SyntaxErrorNative syntaxError) {
		if (syntaxError.ErrorCode == (int)SweetLineErrorCode.Ok) {
			return;
		}

		string message = syntaxError.ErrorMessage == IntPtr.Zero
			? string.Empty
			: Marshal.PtrToStringUTF8(syntaxError.ErrorMessage) ?? string.Empty;

		throw new SyntaxCompileError(syntaxError.ErrorCode, message);
	}

	/// <summary>
	/// Resolves native library path for the SweetLine binding.
	/// </summary>
	/// <remarks>
	/// Search order:
	/// 1) Explicit <c>SWEETLINE_LIB_PATH</c>
	/// 2) App base / current directory candidates
	/// 3) Runtime native folders and common build output folders
	/// 4) System default native resolution
	/// </remarks>
	private static IntPtr ResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath) {
		if (!string.Equals(libraryName, NativeLibraryName, StringComparison.OrdinalIgnoreCase)) {
			return IntPtr.Zero;
		}

		foreach (string candidate in EnumerateLibraryCandidates()) {
			if (!File.Exists(candidate)) {
				continue;
			}

			if (NativeLibrary.TryLoad(candidate, out IntPtr handle)) {
				return handle;
			}
		}

		if (NativeLibrary.TryLoad(libraryName, assembly, searchPath, out IntPtr fallbackHandle)) {
			return fallbackHandle;
		}

		throw new DllNotFoundException(
			"Cannot load native library 'sweetline'. Set SWEETLINE_LIB_PATH to a directory/file containing " +
			"the library, or place the native library next to the application.");
	}

	/// <summary>
	/// Enumerates candidate native library paths.
	/// </summary>
	private static IEnumerable<string> EnumerateLibraryCandidates() {
		string fileName = GetNativeLibraryFileName();
		List<string> list = [];
		HashSet<string> seen = new(StringComparer.OrdinalIgnoreCase);

		string? explicitPath = Environment.GetEnvironmentVariable("SWEETLINE_LIB_PATH");
		if (!string.IsNullOrWhiteSpace(explicitPath)) {
			if (File.Exists(explicitPath)) {
				TryAddCandidate(seen, list, explicitPath);
			} else {
				TryAddCandidate(seen, list, Path.Combine(explicitPath, fileName));
			}
		}

		List<string> roots = [];
		if (!string.IsNullOrWhiteSpace(AppContext.BaseDirectory)) {
			roots.Add(AppContext.BaseDirectory);
		}

		try {
			string cwd = Directory.GetCurrentDirectory();
			if (!string.IsNullOrWhiteSpace(cwd)) {
				roots.Add(cwd);
			}
		} catch {
			// Ignore if current directory cannot be read.
		}

		foreach (string root in roots) {
			TryAddCandidate(seen, list, Path.Combine(root, fileName));
			TryAddCandidate(seen, list, Path.Combine(root, "runtimes", "win-x64", "native", fileName));
			TryAddCandidate(seen, list, Path.Combine(root, "runtimes", "win-arm64", "native", fileName));

			foreach (string relativePath in RelativeSearchPaths) {
				TryAddCandidate(seen, list, Path.Combine(root, relativePath, fileName));
			}
		}

		return list;
	}

	/// <summary>
	/// Tries to add a normalized candidate path into list.
	/// </summary>
	private static void TryAddCandidate(HashSet<string> seen, List<string> list, string? path) {
		if (string.IsNullOrWhiteSpace(path)) {
			return;
		}

		try {
			string normalized = Path.GetFullPath(path);
			if (seen.Add(normalized)) {
				list.Add(normalized);
			}
		} catch {
			// Ignore invalid paths.
		}
	}

	/// <summary>
	/// Gets platform-specific native library file name.
	/// </summary>
	private static string GetNativeLibraryFileName() {
		if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) {
			return "sweetline.dll";
		}

		if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX)) {
			return "libsweetline.dylib";
		}

		return "libsweetline.so";
	}

	/// <summary>
	/// <c>sl_create_document(const char* uri, const char* text)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_create_document", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr CreateDocument(
		[MarshalAs(UnmanagedType.LPUTF8Str)] string uri,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string text);

	/// <summary>
	/// <c>sl_free_document(sl_document_handle_t)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_free_document", CallingConvention = CallingConvention.Cdecl)]
	internal static extern int FreeDocument(IntPtr documentHandle);

	/// <summary>
	/// <c>sl_create_engine(bool show_index, bool inline_style)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_create_engine", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr CreateEngine(
		[MarshalAs(UnmanagedType.I1)] bool showIndex,
		[MarshalAs(UnmanagedType.I1)] bool inlineStyle);

	/// <summary>
	/// <c>sl_free_engine(sl_engine_handle_t)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_free_engine", CallingConvention = CallingConvention.Cdecl)]
	internal static extern int FreeEngine(IntPtr engineHandle);

	/// <summary>
	/// <c>sl_engine_define_macro(sl_engine_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_define_macro", CallingConvention = CallingConvention.Cdecl)]
	internal static extern int EngineDefineMacro(
		IntPtr engineHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string macroName);

	/// <summary>
	/// <c>sl_engine_undefine_macro(sl_engine_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_undefine_macro", CallingConvention = CallingConvention.Cdecl)]
	internal static extern int EngineUndefineMacro(
		IntPtr engineHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string macroName);

	/// <summary>
	/// <c>sl_engine_compile_json(sl_engine_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_compile_json", CallingConvention = CallingConvention.Cdecl)]
	internal static extern SyntaxErrorNative EngineCompileJson(
		IntPtr engineHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string syntaxJson);

	/// <summary>
	/// <c>sl_engine_compile_file(sl_engine_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_compile_file", CallingConvention = CallingConvention.Cdecl)]
	internal static extern SyntaxErrorNative EngineCompileFile(
		IntPtr engineHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string syntaxFile);

	/// <summary>
	/// <c>sl_engine_register_style_name(sl_engine_handle_t, const char*, int32_t)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_register_style_name", CallingConvention = CallingConvention.Cdecl)]
	internal static extern int EngineRegisterStyleName(
		IntPtr engineHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string styleName,
		int styleId);

	/// <summary>
	/// <c>sl_engine_get_style_name(sl_engine_handle_t, int32_t)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_get_style_name", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr EngineGetStyleName(IntPtr engineHandle, int styleId);

	/// <summary>
	/// <c>sl_engine_create_text_analyzer(sl_engine_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_create_text_analyzer", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr EngineCreateTextAnalyzer(
		IntPtr engineHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string syntaxName);

	/// <summary>
	/// <c>sl_engine_create_text_analyzer2(sl_engine_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_create_text_analyzer2", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr EngineCreateTextAnalyzerByExtension(
		IntPtr engineHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string extension);

	/// <summary>
	/// <c>sl_text_analyze(sl_analyzer_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_text_analyze", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr TextAnalyze(
		IntPtr analyzerHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string text);

	/// <summary>
	/// <c>sl_text_analyze_line(sl_analyzer_handle_t, const char*, int32_t*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_text_analyze_line", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr TextAnalyzeLine(
		IntPtr analyzerHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string text,
		[In] int[] lineInfo);

	/// <summary>
	/// <c>sl_text_analyze_indent_guides(sl_analyzer_handle_t, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_text_analyze_indent_guides", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr TextAnalyzeIndentGuides(
		IntPtr analyzerHandle,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string text);

	/// <summary>
	/// <c>sl_engine_load_document(sl_engine_handle_t, sl_document_handle_t)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_engine_load_document", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr EngineLoadDocument(IntPtr engineHandle, IntPtr documentHandle);

	/// <summary>
	/// <c>sl_document_analyze(sl_analyzer_handle_t)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_document_analyze", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr DocumentAnalyze(IntPtr analyzerHandle);

	/// <summary>
	/// <c>sl_document_analyze_incremental(sl_analyzer_handle_t, int32_t*, const char*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_document_analyze_incremental", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr DocumentAnalyzeIncremental(
		IntPtr analyzerHandle,
		[In] int[] changesRange,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string newText);

	/// <summary>
	/// <c>sl_document_analyze_incremental_in_line_range(sl_analyzer_handle_t, int32_t*, const char*, int32_t*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_document_analyze_incremental_in_line_range", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr DocumentAnalyzeIncrementalInLineRange(
		IntPtr analyzerHandle,
		[In] int[] changesRange,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string newText,
		[In] int[] visibleRange);

	/// <summary>
	/// <c>sl_document_get_highlight_slice(sl_analyzer_handle_t, int32_t*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_document_get_highlight_slice", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr DocumentGetHighlightSlice(
		IntPtr analyzerHandle,
		[In] int[] visibleRange);

	/// <summary>
	/// <c>sl_document_analyze_indent_guides(sl_analyzer_handle_t)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_document_analyze_indent_guides", CallingConvention = CallingConvention.Cdecl)]
	internal static extern IntPtr DocumentAnalyzeIndentGuides(IntPtr analyzerHandle);

	/// <summary>
	/// <c>sl_free_buffer(int32_t*)</c>
	/// </summary>
	[DllImport(NativeLibraryName, EntryPoint = "sl_free_buffer", CallingConvention = CallingConvention.Cdecl)]
	internal static extern void FreeBuffer(IntPtr result);
}
