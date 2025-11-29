package com.qiplat.sweetline.demo;

import android.util.SparseIntArray;

import java.util.ArrayList;
import java.util.List;

public class HighlightTheme {
    public final String name;
    public final int backgroundColor;
    public final int textColor;
    public final SparseIntArray colorMap;

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

    private HighlightTheme(String name, int backgroundColor, int textColor, SparseIntArray colorMap) {
        this.name = name;
        this.backgroundColor = backgroundColor;
        this.textColor = textColor;
        this.colorMap = colorMap;
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
        SparseIntArray map = new SparseIntArray();
        map.append(STYLE_KEYWORD,      0xFF569CD6);
        map.append(STYLE_STRING,       0xFFBD63C5);
        map.append(STYLE_NUMBER,       0xFFE4FAD5);
        map.append(STYLE_COMMENT,      0xFF60AE6F);
        map.append(STYLE_CLASS,        0xFF4EC9B0);
        map.append(STYLE_METHOD,       0xFF9CDCFE);
        map.append(STYLE_VARIABLE,     0xFF9B9BC8);
        map.append(STYLE_PUNCTUATION,  0xFFD69D85);
        map.append(STYLE_ANNOTATION,   0xFFFFFD9B);
        map.append(STYLE_PREPROCESSOR, 0xFF569CD6);
        map.append(STYLE_MACRO,        0xFF9B9BC8);
        map.append(STYLE_LIFETIME,     0xFF4EC9B0);
        map.append(STYLE_SELECTOR,     0xFF4EC9B0);
        map.append(STYLE_BUILTIN,      0xFF569CD6);
        return new HighlightTheme("SweetLine Dark", 0xFF1E1E1E, 0xFFD4D4D4, map);
    }

    public static HighlightTheme monokai() {
        SparseIntArray map = new SparseIntArray();
        map.append(STYLE_KEYWORD,      0xFFF92672);
        map.append(STYLE_STRING,       0xFFE6DB74);
        map.append(STYLE_NUMBER,       0xFFAE81FF);
        map.append(STYLE_COMMENT,      0xFF75715E);
        map.append(STYLE_CLASS,        0xFFA6E22E);
        map.append(STYLE_METHOD,       0xFFA6E22E);
        map.append(STYLE_VARIABLE,     0xFFF8F8F2);
        map.append(STYLE_PUNCTUATION,  0xFFF8F8F2);
        map.append(STYLE_ANNOTATION,   0xFFE6DB74);
        map.append(STYLE_PREPROCESSOR, 0xFFF92672);
        map.append(STYLE_MACRO,        0xFFAE81FF);
        map.append(STYLE_LIFETIME,     0xFFFD971F);
        map.append(STYLE_SELECTOR,     0xFFA6E22E);
        map.append(STYLE_BUILTIN,      0xFF66D9EF);
        return new HighlightTheme("Monokai", 0xFF272822, 0xFFF8F8F2, map);
    }

    public static HighlightTheme dracula() {
        SparseIntArray map = new SparseIntArray();
        map.append(STYLE_KEYWORD,      0xFFFF79C6);
        map.append(STYLE_STRING,       0xFFF1FA8C);
        map.append(STYLE_NUMBER,       0xFFBD93F9);
        map.append(STYLE_COMMENT,      0xFF6272A4);
        map.append(STYLE_CLASS,        0xFF8BE9FD);
        map.append(STYLE_METHOD,       0xFF50FA7B);
        map.append(STYLE_VARIABLE,     0xFFF8F8F2);
        map.append(STYLE_PUNCTUATION,  0xFFF8F8F2);
        map.append(STYLE_ANNOTATION,   0xFFFFB86C);
        map.append(STYLE_PREPROCESSOR, 0xFFFF79C6);
        map.append(STYLE_MACRO,        0xFFBD93F9);
        map.append(STYLE_LIFETIME,     0xFFFFB86C);
        map.append(STYLE_SELECTOR,     0xFF50FA7B);
        map.append(STYLE_BUILTIN,      0xFF8BE9FD);
        return new HighlightTheme("Dracula", 0xFF282A36, 0xFFF8F8F2, map);
    }

    public static HighlightTheme oneDark() {
        SparseIntArray map = new SparseIntArray();
        map.append(STYLE_KEYWORD,      0xFFC678DD);
        map.append(STYLE_STRING,       0xFF98C379);
        map.append(STYLE_NUMBER,       0xFFD19A66);
        map.append(STYLE_COMMENT,      0xFF5C6370);
        map.append(STYLE_CLASS,        0xFFE5C07B);
        map.append(STYLE_METHOD,       0xFF61AFEF);
        map.append(STYLE_VARIABLE,     0xFFE06C75);
        map.append(STYLE_PUNCTUATION,  0xFFABB2BF);
        map.append(STYLE_ANNOTATION,   0xFFE5C07B);
        map.append(STYLE_PREPROCESSOR, 0xFFC678DD);
        map.append(STYLE_MACRO,        0xFFD19A66);
        map.append(STYLE_LIFETIME,     0xFF56B6C2);
        map.append(STYLE_SELECTOR,     0xFFE5C07B);
        map.append(STYLE_BUILTIN,      0xFF56B6C2);
        return new HighlightTheme("One Dark", 0xFF282C34, 0xFFABB2BF, map);
    }

    public static HighlightTheme solarizedDark() {
        SparseIntArray map = new SparseIntArray();
        map.append(STYLE_KEYWORD,      0xFF859900);
        map.append(STYLE_STRING,       0xFF2AA198);
        map.append(STYLE_NUMBER,       0xFFD33682);
        map.append(STYLE_COMMENT,      0xFF586E75);
        map.append(STYLE_CLASS,        0xFFB58900);
        map.append(STYLE_METHOD,       0xFF268BD2);
        map.append(STYLE_VARIABLE,     0xFFCB4B16);
        map.append(STYLE_PUNCTUATION,  0xFF839496);
        map.append(STYLE_ANNOTATION,   0xFFB58900);
        map.append(STYLE_PREPROCESSOR, 0xFF859900);
        map.append(STYLE_MACRO,        0xFFCB4B16);
        map.append(STYLE_LIFETIME,     0xFFD33682);
        map.append(STYLE_SELECTOR,     0xFF268BD2);
        map.append(STYLE_BUILTIN,      0xFF268BD2);
        return new HighlightTheme("Solarized Dark", 0xFF002B36, 0xFF839496, map);
    }

    public static HighlightTheme nord() {
        SparseIntArray map = new SparseIntArray();
        map.append(STYLE_KEYWORD,      0xFF81A1C1);
        map.append(STYLE_STRING,       0xFFA3BE8C);
        map.append(STYLE_NUMBER,       0xFFB48EAD);
        map.append(STYLE_COMMENT,      0xFF616E88);
        map.append(STYLE_CLASS,        0xFF8FBCBB);
        map.append(STYLE_METHOD,       0xFF88C0D0);
        map.append(STYLE_VARIABLE,     0xFFD8DEE9);
        map.append(STYLE_PUNCTUATION,  0xFFECEFF4);
        map.append(STYLE_ANNOTATION,   0xFFEBCB8B);
        map.append(STYLE_PREPROCESSOR, 0xFF81A1C1);
        map.append(STYLE_MACRO,        0xFFB48EAD);
        map.append(STYLE_LIFETIME,     0xFFEBCB8B);
        map.append(STYLE_SELECTOR,     0xFF8FBCBB);
        map.append(STYLE_BUILTIN,      0xFF5E81AC);
        return new HighlightTheme("Nord", 0xFF2E3440, 0xFFD8DEE9, map);
    }

    public static HighlightTheme githubDark() {
        SparseIntArray map = new SparseIntArray();
        map.append(STYLE_KEYWORD,      0xFFFF7B72);
        map.append(STYLE_STRING,       0xFFA5D6FF);
        map.append(STYLE_NUMBER,       0xFF79C0FF);
        map.append(STYLE_COMMENT,      0xFF8B949E);
        map.append(STYLE_CLASS,        0xFFFFA657);
        map.append(STYLE_METHOD,       0xFFD2A8FF);
        map.append(STYLE_VARIABLE,     0xFFFFA657);
        map.append(STYLE_PUNCTUATION,  0xFFC9D1D9);
        map.append(STYLE_ANNOTATION,   0xFFFFA657);
        map.append(STYLE_PREPROCESSOR, 0xFFFF7B72);
        map.append(STYLE_MACRO,        0xFF79C0FF);
        map.append(STYLE_LIFETIME,     0xFFFFA657);
        map.append(STYLE_SELECTOR,     0xFF7EE787);
        map.append(STYLE_BUILTIN,      0xFF79C0FF);
        return new HighlightTheme("GitHub Dark", 0xFF0D1117, 0xFFC9D1D9, map);
    }
}
