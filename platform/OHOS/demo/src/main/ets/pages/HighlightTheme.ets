export class HighlightTheme {
  public readonly name: string;
  public readonly backgroundColor: number;
  public readonly textColor: number;
  public readonly colorMap: Map<number, number>;

  public static readonly STYLE_KEYWORD: number = 1;
  public static readonly STYLE_STRING: number = 2;
  public static readonly STYLE_NUMBER: number = 3;
  public static readonly STYLE_COMMENT: number = 4;
  public static readonly STYLE_CLASS: number = 5;
  public static readonly STYLE_METHOD: number = 6;
  public static readonly STYLE_VARIABLE: number = 7;
  public static readonly STYLE_PUNCTUATION: number = 8;
  public static readonly STYLE_ANNOTATION: number = 9;
  public static readonly STYLE_PREPROCESSOR: number = 10;
  public static readonly STYLE_MACRO: number = 11;
  public static readonly STYLE_LIFETIME: number = 12;
  public static readonly STYLE_SELECTOR: number = 13;
  public static readonly STYLE_BUILTIN: number = 14;

  private constructor(name: string, backgroundColor: number, textColor: number, colorMap: Map<number, number>) {
    this.name = name;
    this.backgroundColor = backgroundColor;
    this.textColor = textColor;
    this.colorMap = colorMap;
  }

  public static builtinThemes(): HighlightTheme[] {
    return [
      HighlightTheme.sweetLineDark(),
      HighlightTheme.monokai(),
      HighlightTheme.dracula(),
      HighlightTheme.oneDark(),
      HighlightTheme.solarizedDark(),
      HighlightTheme.nord(),
      HighlightTheme.githubDark()
    ];
  }

  public static themeNames(): string[] {
    return HighlightTheme.builtinThemes().map(t => t.name);
  }

  public static sweetLineDark(): HighlightTheme {
    let map = new Map<number, number>();
    map.set(HighlightTheme.STYLE_KEYWORD, 0xFF569CD6);
    map.set(HighlightTheme.STYLE_STRING, 0xFFBD63C5);
    map.set(HighlightTheme.STYLE_NUMBER, 0xFFE4FAD5);
    map.set(HighlightTheme.STYLE_COMMENT, 0xFF60AE6F);
    map.set(HighlightTheme.STYLE_CLASS, 0xFF4EC9B0);
    map.set(HighlightTheme.STYLE_METHOD, 0xFF9CDCFE);
    map.set(HighlightTheme.STYLE_VARIABLE, 0xFF9B9BC8);
    map.set(HighlightTheme.STYLE_PUNCTUATION, 0xFFD69D85);
    map.set(HighlightTheme.STYLE_ANNOTATION, 0xFFFFFD9B);
    map.set(HighlightTheme.STYLE_PREPROCESSOR, 0xFF569CD6);
    map.set(HighlightTheme.STYLE_MACRO, 0xFF9B9BC8);
    map.set(HighlightTheme.STYLE_LIFETIME, 0xFF4EC9B0);
    map.set(HighlightTheme.STYLE_SELECTOR, 0xFF4EC9B0);
    map.set(HighlightTheme.STYLE_BUILTIN, 0xFF569CD6);
    return new HighlightTheme("SweetLine Dark", 0xFF1E1E1E, 0xFFD4D4D4, map);
  }

  public static monokai(): HighlightTheme {
    let map = new Map<number, number>();
    map.set(HighlightTheme.STYLE_KEYWORD, 0xFFF92672);
    map.set(HighlightTheme.STYLE_STRING, 0xFFE6DB74);
    map.set(HighlightTheme.STYLE_NUMBER, 0xFFAE81FF);
    map.set(HighlightTheme.STYLE_COMMENT, 0xFF75715E);
    map.set(HighlightTheme.STYLE_CLASS, 0xFFA6E22E);
    map.set(HighlightTheme.STYLE_METHOD, 0xFFA6E22E);
    map.set(HighlightTheme.STYLE_VARIABLE, 0xFFF8F8F2);
    map.set(HighlightTheme.STYLE_PUNCTUATION, 0xFFF8F8F2);
    map.set(HighlightTheme.STYLE_ANNOTATION, 0xFFE6DB74);
    map.set(HighlightTheme.STYLE_PREPROCESSOR, 0xFFF92672);
    map.set(HighlightTheme.STYLE_MACRO, 0xFFAE81FF);
    map.set(HighlightTheme.STYLE_LIFETIME, 0xFFFD971F);
    map.set(HighlightTheme.STYLE_SELECTOR, 0xFFA6E22E);
    map.set(HighlightTheme.STYLE_BUILTIN, 0xFF66D9EF);
    return new HighlightTheme("Monokai", 0xFF272822, 0xFFF8F8F2, map);
  }

  public static dracula(): HighlightTheme {
    let map = new Map<number, number>();
    map.set(HighlightTheme.STYLE_KEYWORD, 0xFFFF79C6);
    map.set(HighlightTheme.STYLE_STRING, 0xFFF1FA8C);
    map.set(HighlightTheme.STYLE_NUMBER, 0xFFBD93F9);
    map.set(HighlightTheme.STYLE_COMMENT, 0xFF6272A4);
    map.set(HighlightTheme.STYLE_CLASS, 0xFF8BE9FD);
    map.set(HighlightTheme.STYLE_METHOD, 0xFF50FA7B);
    map.set(HighlightTheme.STYLE_VARIABLE, 0xFFF8F8F2);
    map.set(HighlightTheme.STYLE_PUNCTUATION, 0xFFF8F8F2);
    map.set(HighlightTheme.STYLE_ANNOTATION, 0xFFFFB86C);
    map.set(HighlightTheme.STYLE_PREPROCESSOR, 0xFFFF79C6);
    map.set(HighlightTheme.STYLE_MACRO, 0xFFBD93F9);
    map.set(HighlightTheme.STYLE_LIFETIME, 0xFFFFB86C);
    map.set(HighlightTheme.STYLE_SELECTOR, 0xFF50FA7B);
    map.set(HighlightTheme.STYLE_BUILTIN, 0xFF8BE9FD);
    return new HighlightTheme("Dracula", 0xFF282A36, 0xFFF8F8F2, map);
  }

  public static oneDark(): HighlightTheme {
    let map = new Map<number, number>();
    map.set(HighlightTheme.STYLE_KEYWORD, 0xFFC678DD);
    map.set(HighlightTheme.STYLE_STRING, 0xFF98C379);
    map.set(HighlightTheme.STYLE_NUMBER, 0xFFD19A66);
    map.set(HighlightTheme.STYLE_COMMENT, 0xFF5C6370);
    map.set(HighlightTheme.STYLE_CLASS, 0xFFE5C07B);
    map.set(HighlightTheme.STYLE_METHOD, 0xFF61AFEF);
    map.set(HighlightTheme.STYLE_VARIABLE, 0xFFE06C75);
    map.set(HighlightTheme.STYLE_PUNCTUATION, 0xFFABB2BF);
    map.set(HighlightTheme.STYLE_ANNOTATION, 0xFFE5C07B);
    map.set(HighlightTheme.STYLE_PREPROCESSOR, 0xFFC678DD);
    map.set(HighlightTheme.STYLE_MACRO, 0xFFD19A66);
    map.set(HighlightTheme.STYLE_LIFETIME, 0xFF56B6C2);
    map.set(HighlightTheme.STYLE_SELECTOR, 0xFFE5C07B);
    map.set(HighlightTheme.STYLE_BUILTIN, 0xFF56B6C2);
    return new HighlightTheme("One Dark", 0xFF282C34, 0xFFABB2BF, map);
  }

  public static solarizedDark(): HighlightTheme {
    let map = new Map<number, number>();
    map.set(HighlightTheme.STYLE_KEYWORD, 0xFF859900);
    map.set(HighlightTheme.STYLE_STRING, 0xFF2AA198);
    map.set(HighlightTheme.STYLE_NUMBER, 0xFFD33682);
    map.set(HighlightTheme.STYLE_COMMENT, 0xFF586E75);
    map.set(HighlightTheme.STYLE_CLASS, 0xFFB58900);
    map.set(HighlightTheme.STYLE_METHOD, 0xFF268BD2);
    map.set(HighlightTheme.STYLE_VARIABLE, 0xFFCB4B16);
    map.set(HighlightTheme.STYLE_PUNCTUATION, 0xFF839496);
    map.set(HighlightTheme.STYLE_ANNOTATION, 0xFFB58900);
    map.set(HighlightTheme.STYLE_PREPROCESSOR, 0xFF859900);
    map.set(HighlightTheme.STYLE_MACRO, 0xFFCB4B16);
    map.set(HighlightTheme.STYLE_LIFETIME, 0xFFD33682);
    map.set(HighlightTheme.STYLE_SELECTOR, 0xFF268BD2);
    map.set(HighlightTheme.STYLE_BUILTIN, 0xFF268BD2);
    return new HighlightTheme("Solarized Dark", 0xFF002B36, 0xFF839496, map);
  }

  public static nord(): HighlightTheme {
    let map = new Map<number, number>();
    map.set(HighlightTheme.STYLE_KEYWORD, 0xFF81A1C1);
    map.set(HighlightTheme.STYLE_STRING, 0xFFA3BE8C);
    map.set(HighlightTheme.STYLE_NUMBER, 0xFFB48EAD);
    map.set(HighlightTheme.STYLE_COMMENT, 0xFF616E88);
    map.set(HighlightTheme.STYLE_CLASS, 0xFF8FBCBB);
    map.set(HighlightTheme.STYLE_METHOD, 0xFF88C0D0);
    map.set(HighlightTheme.STYLE_VARIABLE, 0xFFD8DEE9);
    map.set(HighlightTheme.STYLE_PUNCTUATION, 0xFFECEFF4);
    map.set(HighlightTheme.STYLE_ANNOTATION, 0xFFEBCB8B);
    map.set(HighlightTheme.STYLE_PREPROCESSOR, 0xFF81A1C1);
    map.set(HighlightTheme.STYLE_MACRO, 0xFFB48EAD);
    map.set(HighlightTheme.STYLE_LIFETIME, 0xFFEBCB8B);
    map.set(HighlightTheme.STYLE_SELECTOR, 0xFF8FBCBB);
    map.set(HighlightTheme.STYLE_BUILTIN, 0xFF5E81AC);
    return new HighlightTheme("Nord", 0xFF2E3440, 0xFFD8DEE9, map);
  }

  public static githubDark(): HighlightTheme {
    let map = new Map<number, number>();
    map.set(HighlightTheme.STYLE_KEYWORD, 0xFFFF7B72);
    map.set(HighlightTheme.STYLE_STRING, 0xFFA5D6FF);
    map.set(HighlightTheme.STYLE_NUMBER, 0xFF79C0FF);
    map.set(HighlightTheme.STYLE_COMMENT, 0xFF8B949E);
    map.set(HighlightTheme.STYLE_CLASS, 0xFFFFA657);
    map.set(HighlightTheme.STYLE_METHOD, 0xFFD2A8FF);
    map.set(HighlightTheme.STYLE_VARIABLE, 0xFFFFA657);
    map.set(HighlightTheme.STYLE_PUNCTUATION, 0xFFC9D1D9);
    map.set(HighlightTheme.STYLE_ANNOTATION, 0xFFFFA657);
    map.set(HighlightTheme.STYLE_PREPROCESSOR, 0xFFFF7B72);
    map.set(HighlightTheme.STYLE_MACRO, 0xFF79C0FF);
    map.set(HighlightTheme.STYLE_LIFETIME, 0xFFFFA657);
    map.set(HighlightTheme.STYLE_SELECTOR, 0xFF7EE787);
    map.set(HighlightTheme.STYLE_BUILTIN, 0xFF79C0FF);
    return new HighlightTheme("GitHub Dark", 0xFF0D1117, 0xFFC9D1D9, map);
  }

  /**
   * 根据styleId获取对应颜色值，取不到时返回默认文本颜色
   */
  public getColor(styleId: number): number {
    return this.colorMap.get(styleId) ?? this.textColor;
  }
}
