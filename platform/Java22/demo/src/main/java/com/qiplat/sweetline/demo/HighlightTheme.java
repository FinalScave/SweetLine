package com.qiplat.sweetline.demo;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Built-in highlight themes with style ID to color mapping.
 */
public class HighlightTheme {

    public final String name;
    public final int backgroundColor;
    public final int textColor;
    public final Map<Integer, Integer> colorMap;

    public static final int STYLE_KEYWORD = 1;
    public static final int STYLE_STRING = 2;
    public static final int STYLE_NUMBER = 3;
    public static final int STYLE_COMMENT = 4;
    public static final int STYLE_CLASS = 5;
    public static final int STYLE_METHOD = 6;
    public static final int STYLE_VARIABLE = 7;
    public static final int STYLE_PUNCTUATION = 8;
    public static final int STYLE_ANNOTATION = 9;
    public static final int STYLE_PREPROCESSOR = 10;
    public static final int STYLE_MACRO = 11;
    public static final int STYLE_LIFETIME = 12;
    public static final int STYLE_SELECTOR = 13;
    public static final int STYLE_BUILTIN = 14;

    private HighlightTheme(String name, int backgroundColor, int textColor, Map<Integer, Integer> colorMap) {
        this.name = name;
        this.backgroundColor = backgroundColor;
        this.textColor = textColor;
        this.colorMap = colorMap;
    }

    public int getColor(int styleId) {
        return colorMap.getOrDefault(styleId, textColor);
    }

    public static List<HighlightTheme> builtinThemes() {
        List<HighlightTheme> themes = new ArrayList<>();
        themes.add(sweetLineDark());
        themes.add(monokai());
        themes.add(dracula());
        themes.add(oneDark());
        themes.add(solarizedDark());
        themes.add(nord());
        themes.add(githubDark());
        return themes;
    }

    public static String[] themeNames() {
        List<HighlightTheme> themes = builtinThemes();
        String[] names = new String[themes.size()];
        for (int i = 0; i < themes.size(); i++) {
            names[i] = themes.get(i).name;
        }
        return names;
    }

    public static HighlightTheme sweetLineDark() {
        Map<Integer, Integer> map = new HashMap<>();
        map.put(STYLE_KEYWORD,      0xFF569CD6);
        map.put(STYLE_STRING,       0xFFBD63C5);
        map.put(STYLE_NUMBER,       0xFFE4FAD5);
        map.put(STYLE_COMMENT,      0xFF60AE6F);
        map.put(STYLE_CLASS,        0xFF4EC9B0);
        map.put(STYLE_METHOD,       0xFF9CDCFE);
        map.put(STYLE_VARIABLE,     0xFF9B9BC8);
        map.put(STYLE_PUNCTUATION,  0xFFD69D85);
        map.put(STYLE_ANNOTATION,   0xFFFFFD9B);
        map.put(STYLE_PREPROCESSOR, 0xFF569CD6);
        map.put(STYLE_MACRO,        0xFF9B9BC8);
        map.put(STYLE_LIFETIME,     0xFF4EC9B0);
        map.put(STYLE_SELECTOR,     0xFF4EC9B0);
        map.put(STYLE_BUILTIN,      0xFF569CD6);
        return new HighlightTheme("SweetLine Dark", 0xFF1E1E1E, 0xFFD4D4D4, map);
    }

    public static HighlightTheme monokai() {
        Map<Integer, Integer> map = new HashMap<>();
        map.put(STYLE_KEYWORD,      0xFFF92672);
        map.put(STYLE_STRING,       0xFFE6DB74);
        map.put(STYLE_NUMBER,       0xFFAE81FF);
        map.put(STYLE_COMMENT,      0xFF75715E);
        map.put(STYLE_CLASS,        0xFFA6E22E);
        map.put(STYLE_METHOD,       0xFFA6E22E);
        map.put(STYLE_VARIABLE,     0xFFF8F8F2);
        map.put(STYLE_PUNCTUATION,  0xFFF8F8F2);
        map.put(STYLE_ANNOTATION,   0xFFE6DB74);
        map.put(STYLE_PREPROCESSOR, 0xFFF92672);
        map.put(STYLE_MACRO,        0xFFAE81FF);
        map.put(STYLE_LIFETIME,     0xFFFD971F);
        map.put(STYLE_SELECTOR,     0xFFA6E22E);
        map.put(STYLE_BUILTIN,      0xFF66D9EF);
        return new HighlightTheme("Monokai", 0xFF272822, 0xFFF8F8F2, map);
    }

    public static HighlightTheme dracula() {
        Map<Integer, Integer> map = new HashMap<>();
        map.put(STYLE_KEYWORD,      0xFFFF79C6);
        map.put(STYLE_STRING,       0xFFF1FA8C);
        map.put(STYLE_NUMBER,       0xFFBD93F9);
        map.put(STYLE_COMMENT,      0xFF6272A4);
        map.put(STYLE_CLASS,        0xFF8BE9FD);
        map.put(STYLE_METHOD,       0xFF50FA7B);
        map.put(STYLE_VARIABLE,     0xFFF8F8F2);
        map.put(STYLE_PUNCTUATION,  0xFFF8F8F2);
        map.put(STYLE_ANNOTATION,   0xFFFFB86C);
        map.put(STYLE_PREPROCESSOR, 0xFFFF79C6);
        map.put(STYLE_MACRO,        0xFFBD93F9);
        map.put(STYLE_LIFETIME,     0xFFFFB86C);
        map.put(STYLE_SELECTOR,     0xFF50FA7B);
        map.put(STYLE_BUILTIN,      0xFF8BE9FD);
        return new HighlightTheme("Dracula", 0xFF282A36, 0xFFF8F8F2, map);
    }

    public static HighlightTheme oneDark() {
        Map<Integer, Integer> map = new HashMap<>();
        map.put(STYLE_KEYWORD,      0xFFC678DD);
        map.put(STYLE_STRING,       0xFF98C379);
        map.put(STYLE_NUMBER,       0xFFD19A66);
        map.put(STYLE_COMMENT,      0xFF5C6370);
        map.put(STYLE_CLASS,        0xFFE5C07B);
        map.put(STYLE_METHOD,       0xFF61AFEF);
        map.put(STYLE_VARIABLE,     0xFFE06C75);
        map.put(STYLE_PUNCTUATION,  0xFFABB2BF);
        map.put(STYLE_ANNOTATION,   0xFFE5C07B);
        map.put(STYLE_PREPROCESSOR, 0xFFC678DD);
        map.put(STYLE_MACRO,        0xFFD19A66);
        map.put(STYLE_LIFETIME,     0xFF56B6C2);
        map.put(STYLE_SELECTOR,     0xFFE5C07B);
        map.put(STYLE_BUILTIN,      0xFF56B6C2);
        return new HighlightTheme("One Dark", 0xFF282C34, 0xFFABB2BF, map);
    }

    public static HighlightTheme solarizedDark() {
        Map<Integer, Integer> map = new HashMap<>();
        map.put(STYLE_KEYWORD,      0xFF859900);
        map.put(STYLE_STRING,       0xFF2AA198);
        map.put(STYLE_NUMBER,       0xFFD33682);
        map.put(STYLE_COMMENT,      0xFF586E75);
        map.put(STYLE_CLASS,        0xFFB58900);
        map.put(STYLE_METHOD,       0xFF268BD2);
        map.put(STYLE_VARIABLE,     0xFFCB4B16);
        map.put(STYLE_PUNCTUATION,  0xFF839496);
        map.put(STYLE_ANNOTATION,   0xFFB58900);
        map.put(STYLE_PREPROCESSOR, 0xFF859900);
        map.put(STYLE_MACRO,        0xFFCB4B16);
        map.put(STYLE_LIFETIME,     0xFFD33682);
        map.put(STYLE_SELECTOR,     0xFF268BD2);
        map.put(STYLE_BUILTIN,      0xFF268BD2);
        return new HighlightTheme("Solarized Dark", 0xFF002B36, 0xFF839496, map);
    }

    public static HighlightTheme nord() {
        Map<Integer, Integer> map = new HashMap<>();
        map.put(STYLE_KEYWORD,      0xFF81A1C1);
        map.put(STYLE_STRING,       0xFFA3BE8C);
        map.put(STYLE_NUMBER,       0xFFB48EAD);
        map.put(STYLE_COMMENT,      0xFF616E88);
        map.put(STYLE_CLASS,        0xFF8FBCBB);
        map.put(STYLE_METHOD,       0xFF88C0D0);
        map.put(STYLE_VARIABLE,     0xFFD8DEE9);
        map.put(STYLE_PUNCTUATION,  0xFFECEFF4);
        map.put(STYLE_ANNOTATION,   0xFFEBCB8B);
        map.put(STYLE_PREPROCESSOR, 0xFF81A1C1);
        map.put(STYLE_MACRO,        0xFFB48EAD);
        map.put(STYLE_LIFETIME,     0xFFEBCB8B);
        map.put(STYLE_SELECTOR,     0xFF8FBCBB);
        map.put(STYLE_BUILTIN,      0xFF5E81AC);
        return new HighlightTheme("Nord", 0xFF2E3440, 0xFFD8DEE9, map);
    }

    public static HighlightTheme githubDark() {
        Map<Integer, Integer> map = new HashMap<>();
        map.put(STYLE_KEYWORD,      0xFFFF7B72);
        map.put(STYLE_STRING,       0xFFA5D6FF);
        map.put(STYLE_NUMBER,       0xFF79C0FF);
        map.put(STYLE_COMMENT,      0xFF8B949E);
        map.put(STYLE_CLASS,        0xFFFFA657);
        map.put(STYLE_METHOD,       0xFFD2A8FF);
        map.put(STYLE_VARIABLE,     0xFFFFA657);
        map.put(STYLE_PUNCTUATION,  0xFFC9D1D9);
        map.put(STYLE_ANNOTATION,   0xFFFFA657);
        map.put(STYLE_PREPROCESSOR, 0xFFFF7B72);
        map.put(STYLE_MACRO,        0xFF79C0FF);
        map.put(STYLE_LIFETIME,     0xFFFFA657);
        map.put(STYLE_SELECTOR,     0xFF7EE787);
        map.put(STYLE_BUILTIN,      0xFF79C0FF);
        return new HighlightTheme("GitHub Dark", 0xFF0D1117, 0xFFC9D1D9, map);
    }
}
