import 'dart:ui';

class HighlightTheme {
  const HighlightTheme(
    this.name,
    this.backgroundColor,
    this.textColor,
    this.colorMap,
  );

  static const int styleKeyword = 1;
  static const int styleString = 2;
  static const int styleNumber = 3;
  static const int styleComment = 4;
  static const int styleClass = 5;
  static const int styleMethod = 6;
  static const int styleVariable = 7;
  static const int stylePunctuation = 8;
  static const int styleAnnotation = 9;
  static const int stylePreprocessor = 10;
  static const int styleMacro = 11;
  static const int styleLifetime = 12;
  static const int styleSelector = 13;
  static const int styleBuiltin = 14;
  static const int styleUrl = 15;
  static const int styleProperty = 16;

  final String name;
  final int backgroundColor;
  final int textColor;
  final Map<int, int> colorMap;

  int getColor(int? styleId) {
    if (styleId == null) {
      return textColor;
    }
    return colorMap[styleId] ?? textColor;
  }

  Color get background => _argbToColor(backgroundColor);
  Color get text => _argbToColor(textColor);

  Color get gutterBackground => blend(background, text, 0.08);
  Color get separator => blend(background, text, 0.14);
  Color get lineNumber => blend(text, background, 0.48);
  Color get indentGuide => blend(text, background, 0.34);
  Color get toolbarSurface => blend(background, const Color(0xFFFFFFFF), 0.035);
  Color get statusSurface => blend(background, const Color(0xFFFFFFFF), 0.02);
  Color get accent => _argbToColor(getColor(styleKeyword));

  static List<HighlightTheme> builtinThemes() {
    return <HighlightTheme>[
      sweetLineDark(),
      monokai(),
      dracula(),
      oneDark(),
      solarizedDark(),
      nord(),
      githubDark(),
    ];
  }

  static HighlightTheme sweetLineDark() {
    return HighlightTheme('SweetLine Dark', 0xFF1E1E1E, 0xFFD4D4D4, {
      styleKeyword: 0xFF569CD6,
      styleString: 0xFFBD63C5,
      styleNumber: 0xFFE4FAD5,
      styleComment: 0xFF60AE6F,
      styleClass: 0xFF4EC9B0,
      styleMethod: 0xFF9CDCFE,
      styleVariable: 0xFF9B9BC8,
      stylePunctuation: 0xFFD69D85,
      styleAnnotation: 0xFFFFFD9B,
      stylePreprocessor: 0xFF569CD6,
      styleMacro: 0xFF9B9BC8,
      styleLifetime: 0xFF4EC9B0,
      styleSelector: 0xFF4EC9B0,
      styleBuiltin: 0xFF569CD6,
      styleUrl: 0xFF4FC1FF,
      styleProperty: 0xFF9CDCFE,
    });
  }

  static HighlightTheme monokai() {
    return HighlightTheme('Monokai', 0xFF272822, 0xFFF8F8F2, {
      styleKeyword: 0xFFF92672,
      styleString: 0xFFE6DB74,
      styleNumber: 0xFFAE81FF,
      styleComment: 0xFF75715E,
      styleClass: 0xFFA6E22E,
      styleMethod: 0xFFA6E22E,
      styleVariable: 0xFFF8F8F2,
      stylePunctuation: 0xFFF8F8F2,
      styleAnnotation: 0xFFE6DB74,
      stylePreprocessor: 0xFFF92672,
      styleMacro: 0xFFAE81FF,
      styleLifetime: 0xFFFD971F,
      styleSelector: 0xFFA6E22E,
      styleBuiltin: 0xFF66D9EF,
      styleUrl: 0xFF66D9EF,
      styleProperty: 0xFFA6E22E,
    });
  }

  static HighlightTheme dracula() {
    return HighlightTheme('Dracula', 0xFF282A36, 0xFFF8F8F2, {
      styleKeyword: 0xFFFF79C6,
      styleString: 0xFFF1FA8C,
      styleNumber: 0xFFBD93F9,
      styleComment: 0xFF6272A4,
      styleClass: 0xFF8BE9FD,
      styleMethod: 0xFF50FA7B,
      styleVariable: 0xFFF8F8F2,
      stylePunctuation: 0xFFF8F8F2,
      styleAnnotation: 0xFFFFB86C,
      stylePreprocessor: 0xFFFF79C6,
      styleMacro: 0xFFBD93F9,
      styleLifetime: 0xFFFFB86C,
      styleSelector: 0xFF50FA7B,
      styleBuiltin: 0xFF8BE9FD,
      styleUrl: 0xFF8BE9FD,
      styleProperty: 0xFF50FA7B,
    });
  }

  static HighlightTheme oneDark() {
    return HighlightTheme('One Dark', 0xFF282C34, 0xFFABB2BF, {
      styleKeyword: 0xFFC678DD,
      styleString: 0xFF98C379,
      styleNumber: 0xFFD19A66,
      styleComment: 0xFF5C6370,
      styleClass: 0xFFE5C07B,
      styleMethod: 0xFF61AFEF,
      styleVariable: 0xFFE06C75,
      stylePunctuation: 0xFFABB2BF,
      styleAnnotation: 0xFFE5C07B,
      stylePreprocessor: 0xFFC678DD,
      styleMacro: 0xFFD19A66,
      styleLifetime: 0xFF56B6C2,
      styleSelector: 0xFFE5C07B,
      styleBuiltin: 0xFF56B6C2,
      styleUrl: 0xFF61AFEF,
      styleProperty: 0xFF61AFEF,
    });
  }

  static HighlightTheme solarizedDark() {
    return HighlightTheme('Solarized Dark', 0xFF002B36, 0xFF839496, {
      styleKeyword: 0xFF859900,
      styleString: 0xFF2AA198,
      styleNumber: 0xFFD33682,
      styleComment: 0xFF586E75,
      styleClass: 0xFFB58900,
      styleMethod: 0xFF268BD2,
      styleVariable: 0xFFCB4B16,
      stylePunctuation: 0xFF839496,
      styleAnnotation: 0xFFB58900,
      stylePreprocessor: 0xFF859900,
      styleMacro: 0xFFCB4B16,
      styleLifetime: 0xFFD33682,
      styleSelector: 0xFF268BD2,
      styleBuiltin: 0xFF268BD2,
      styleUrl: 0xFF268BD2,
      styleProperty: 0xFF268BD2,
    });
  }

  static HighlightTheme nord() {
    return HighlightTheme('Nord', 0xFF2E3440, 0xFFD8DEE9, {
      styleKeyword: 0xFF81A1C1,
      styleString: 0xFFA3BE8C,
      styleNumber: 0xFFB48EAD,
      styleComment: 0xFF616E88,
      styleClass: 0xFF8FBCBB,
      styleMethod: 0xFF88C0D0,
      styleVariable: 0xFFD8DEE9,
      stylePunctuation: 0xFFECEFF4,
      styleAnnotation: 0xFFEBCB8B,
      stylePreprocessor: 0xFF81A1C1,
      styleMacro: 0xFFB48EAD,
      styleLifetime: 0xFFEBCB8B,
      styleSelector: 0xFF8FBCBB,
      styleBuiltin: 0xFF5E81AC,
      styleUrl: 0xFF88C0D0,
      styleProperty: 0xFF88C0D0,
    });
  }

  static HighlightTheme githubDark() {
    return HighlightTheme('GitHub Dark', 0xFF0D1117, 0xFFC9D1D9, {
      styleKeyword: 0xFFFF7B72,
      styleString: 0xFFA5D6FF,
      styleNumber: 0xFF79C0FF,
      styleComment: 0xFF8B949E,
      styleClass: 0xFFFFA657,
      styleMethod: 0xFFD2A8FF,
      styleVariable: 0xFFFFA657,
      stylePunctuation: 0xFFC9D1D9,
      styleAnnotation: 0xFFFFA657,
      stylePreprocessor: 0xFFFF7B72,
      styleMacro: 0xFF79C0FF,
      styleLifetime: 0xFFFFA657,
      styleSelector: 0xFF7EE787,
      styleBuiltin: 0xFF79C0FF,
      styleUrl: 0xFF79C0FF,
      styleProperty: 0xFF79C0FF,
    });
  }

  static Color _argbToColor(int argb) {
    return Color(argb);
  }

  static Color blend(Color base, Color target, double ratio) {
    final clamped = ratio.clamp(0.0, 1.0);
    return Color.fromARGB(
      (((base.a * 255) * (1 - clamped)) + ((target.a * 255) * clamped)).round(),
      (((base.r * 255) * (1 - clamped)) + ((target.r * 255) * clamped)).round(),
      (((base.g * 255) * (1 - clamped)) + ((target.g * 255) * clamped)).round(),
      (((base.b * 255) * (1 - clamped)) + ((target.b * 255) * clamped)).round(),
    );
  }
}
