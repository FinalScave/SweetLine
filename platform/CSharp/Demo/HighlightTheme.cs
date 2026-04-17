using System.Collections.Generic;

namespace Demo;

internal sealed class HighlightTheme {
	public const int StyleKeyword = 1;
	public const int StyleString = 2;
	public const int StyleNumber = 3;
	public const int StyleComment = 4;
	public const int StyleClass = 5;
	public const int StyleMethod = 6;
	public const int StyleVariable = 7;
	public const int StylePunctuation = 8;
	public const int StyleAnnotation = 9;
	public const int StylePreprocessor = 10;
	public const int StyleMacro = 11;
	public const int StyleLifetime = 12;
	public const int StyleSelector = 13;
	public const int StyleBuiltin = 14;
	public const int StyleUrl = 15;
	public const int StyleProperty = 16;

	public string Name { get; }
	public int BackgroundColor { get; }
	public int TextColor { get; }
	public IReadOnlyDictionary<int, int> ColorMap { get; }

	private HighlightTheme(string name, int backgroundColor, int textColor, Dictionary<int, int> colorMap) {
		Name = name;
		BackgroundColor = backgroundColor;
		TextColor = textColor;
		ColorMap = colorMap;
	}

	public int GetColor(int styleId) {
		return ColorMap.TryGetValue(styleId, out int color) ? color : TextColor;
	}

	public static List<HighlightTheme> BuiltinThemes() {
		return
		[
			SweetLineDark(),
			Monokai(),
			Dracula(),
			OneDark(),
			SolarizedDark(),
			Nord(),
			GitHubDark()
		];
	}

	public static string[] ThemeNames(IReadOnlyList<HighlightTheme> themes) {
		string[] names = new string[themes.Count];
		for (int i = 0; i < themes.Count; i++) {
			names[i] = themes[i].Name;
		}

		return names;
	}

	private static HighlightTheme SweetLineDark() {
		Dictionary<int, int> map = new() {
			[StyleKeyword] = unchecked((int)0xFF569CD6),
			[StyleString] = unchecked((int)0xFFBD63C5),
			[StyleNumber] = unchecked((int)0xFFE4FAD5),
			[StyleComment] = unchecked((int)0xFF60AE6F),
			[StyleClass] = unchecked((int)0xFF4EC9B0),
			[StyleMethod] = unchecked((int)0xFF9CDCFE),
			[StyleVariable] = unchecked((int)0xFF9B9BC8),
			[StylePunctuation] = unchecked((int)0xFFD69D85),
			[StyleAnnotation] = unchecked((int)0xFFFFFD9B),
			[StylePreprocessor] = unchecked((int)0xFF569CD6),
			[StyleMacro] = unchecked((int)0xFF9B9BC8),
			[StyleLifetime] = unchecked((int)0xFF4EC9B0),
			[StyleSelector] = unchecked((int)0xFF4EC9B0),
			[StyleBuiltin] = unchecked((int)0xFF569CD6),
			[StyleUrl] = unchecked((int)0xFF4FC1FF),
			[StyleProperty] = unchecked((int)0xFF9CDCFE)
		};

		return new HighlightTheme(
			"SweetLine Dark",
			unchecked((int)0xFF1E1E1E),
			unchecked((int)0xFFD4D4D4),
			map);
	}

	private static HighlightTheme Monokai() {
		Dictionary<int, int> map = new() {
			[StyleKeyword] = unchecked((int)0xFFF92672),
			[StyleString] = unchecked((int)0xFFE6DB74),
			[StyleNumber] = unchecked((int)0xFFAE81FF),
			[StyleComment] = unchecked((int)0xFF75715E),
			[StyleClass] = unchecked((int)0xFFA6E22E),
			[StyleMethod] = unchecked((int)0xFFA6E22E),
			[StyleVariable] = unchecked((int)0xFFF8F8F2),
			[StylePunctuation] = unchecked((int)0xFFF8F8F2),
			[StyleAnnotation] = unchecked((int)0xFFE6DB74),
			[StylePreprocessor] = unchecked((int)0xFFF92672),
			[StyleMacro] = unchecked((int)0xFFAE81FF),
			[StyleLifetime] = unchecked((int)0xFFFD971F),
			[StyleSelector] = unchecked((int)0xFFA6E22E),
			[StyleBuiltin] = unchecked((int)0xFF66D9EF),
			[StyleUrl] = unchecked((int)0xFF66D9EF),
			[StyleProperty] = unchecked((int)0xFFA6E22E)
		};

		return new HighlightTheme(
			"Monokai",
			unchecked((int)0xFF272822),
			unchecked((int)0xFFF8F8F2),
			map);
	}

	private static HighlightTheme Dracula() {
		Dictionary<int, int> map = new() {
			[StyleKeyword] = unchecked((int)0xFFFF79C6),
			[StyleString] = unchecked((int)0xFFF1FA8C),
			[StyleNumber] = unchecked((int)0xFFBD93F9),
			[StyleComment] = unchecked((int)0xFF6272A4),
			[StyleClass] = unchecked((int)0xFF8BE9FD),
			[StyleMethod] = unchecked((int)0xFF50FA7B),
			[StyleVariable] = unchecked((int)0xFFF8F8F2),
			[StylePunctuation] = unchecked((int)0xFFF8F8F2),
			[StyleAnnotation] = unchecked((int)0xFFFFB86C),
			[StylePreprocessor] = unchecked((int)0xFFFF79C6),
			[StyleMacro] = unchecked((int)0xFFBD93F9),
			[StyleLifetime] = unchecked((int)0xFFFFB86C),
			[StyleSelector] = unchecked((int)0xFF50FA7B),
			[StyleBuiltin] = unchecked((int)0xFF8BE9FD),
			[StyleUrl] = unchecked((int)0xFF8BE9FD),
			[StyleProperty] = unchecked((int)0xFF50FA7B)
		};

		return new HighlightTheme(
			"Dracula",
			unchecked((int)0xFF282A36),
			unchecked((int)0xFFF8F8F2),
			map);
	}

	private static HighlightTheme OneDark() {
		Dictionary<int, int> map = new() {
			[StyleKeyword] = unchecked((int)0xFFC678DD),
			[StyleString] = unchecked((int)0xFF98C379),
			[StyleNumber] = unchecked((int)0xFFD19A66),
			[StyleComment] = unchecked((int)0xFF5C6370),
			[StyleClass] = unchecked((int)0xFFE5C07B),
			[StyleMethod] = unchecked((int)0xFF61AFEF),
			[StyleVariable] = unchecked((int)0xFFE06C75),
			[StylePunctuation] = unchecked((int)0xFFABB2BF),
			[StyleAnnotation] = unchecked((int)0xFFE5C07B),
			[StylePreprocessor] = unchecked((int)0xFFC678DD),
			[StyleMacro] = unchecked((int)0xFFD19A66),
			[StyleLifetime] = unchecked((int)0xFF56B6C2),
			[StyleSelector] = unchecked((int)0xFFE5C07B),
			[StyleBuiltin] = unchecked((int)0xFF56B6C2),
			[StyleUrl] = unchecked((int)0xFF61AFEF),
			[StyleProperty] = unchecked((int)0xFF61AFEF)
		};

		return new HighlightTheme(
			"One Dark",
			unchecked((int)0xFF282C34),
			unchecked((int)0xFFABB2BF),
			map);
	}

	private static HighlightTheme SolarizedDark() {
		Dictionary<int, int> map = new() {
			[StyleKeyword] = unchecked((int)0xFF859900),
			[StyleString] = unchecked((int)0xFF2AA198),
			[StyleNumber] = unchecked((int)0xFFD33682),
			[StyleComment] = unchecked((int)0xFF586E75),
			[StyleClass] = unchecked((int)0xFFB58900),
			[StyleMethod] = unchecked((int)0xFF268BD2),
			[StyleVariable] = unchecked((int)0xFFCB4B16),
			[StylePunctuation] = unchecked((int)0xFF839496),
			[StyleAnnotation] = unchecked((int)0xFFB58900),
			[StylePreprocessor] = unchecked((int)0xFF859900),
			[StyleMacro] = unchecked((int)0xFFCB4B16),
			[StyleLifetime] = unchecked((int)0xFFD33682),
			[StyleSelector] = unchecked((int)0xFF268BD2),
			[StyleBuiltin] = unchecked((int)0xFF268BD2),
			[StyleUrl] = unchecked((int)0xFF268BD2),
			[StyleProperty] = unchecked((int)0xFF268BD2)
		};

		return new HighlightTheme(
			"Solarized Dark",
			unchecked((int)0xFF002B36),
			unchecked((int)0xFF839496),
			map);
	}

	private static HighlightTheme Nord() {
		Dictionary<int, int> map = new() {
			[StyleKeyword] = unchecked((int)0xFF81A1C1),
			[StyleString] = unchecked((int)0xFFA3BE8C),
			[StyleNumber] = unchecked((int)0xFFB48EAD),
			[StyleComment] = unchecked((int)0xFF616E88),
			[StyleClass] = unchecked((int)0xFF8FBCBB),
			[StyleMethod] = unchecked((int)0xFF88C0D0),
			[StyleVariable] = unchecked((int)0xFFD8DEE9),
			[StylePunctuation] = unchecked((int)0xFFECEFF4),
			[StyleAnnotation] = unchecked((int)0xFFEBCB8B),
			[StylePreprocessor] = unchecked((int)0xFF81A1C1),
			[StyleMacro] = unchecked((int)0xFFB48EAD),
			[StyleLifetime] = unchecked((int)0xFFEBCB8B),
			[StyleSelector] = unchecked((int)0xFF8FBCBB),
			[StyleBuiltin] = unchecked((int)0xFF5E81AC),
			[StyleUrl] = unchecked((int)0xFF88C0D0),
			[StyleProperty] = unchecked((int)0xFF88C0D0)
		};

		return new HighlightTheme(
			"Nord",
			unchecked((int)0xFF2E3440),
			unchecked((int)0xFFD8DEE9),
			map);
	}

	private static HighlightTheme GitHubDark() {
		Dictionary<int, int> map = new() {
			[StyleKeyword] = unchecked((int)0xFFFF7B72),
			[StyleString] = unchecked((int)0xFFA5D6FF),
			[StyleNumber] = unchecked((int)0xFF79C0FF),
			[StyleComment] = unchecked((int)0xFF8B949E),
			[StyleClass] = unchecked((int)0xFFFFA657),
			[StyleMethod] = unchecked((int)0xFFD2A8FF),
			[StyleVariable] = unchecked((int)0xFFFFA657),
			[StylePunctuation] = unchecked((int)0xFFC9D1D9),
			[StyleAnnotation] = unchecked((int)0xFFFFA657),
			[StylePreprocessor] = unchecked((int)0xFFFF7B72),
			[StyleMacro] = unchecked((int)0xFF79C0FF),
			[StyleLifetime] = unchecked((int)0xFFFFA657),
			[StyleSelector] = unchecked((int)0xFF7EE787),
			[StyleBuiltin] = unchecked((int)0xFF79C0FF),
			[StyleUrl] = unchecked((int)0xFF79C0FF),
			[StyleProperty] = unchecked((int)0xFF79C0FF)
		};

		return new HighlightTheme(
			"GitHub Dark",
			unchecked((int)0xFF0D1117),
			unchecked((int)0xFFC9D1D9),
			map);
	}
}
