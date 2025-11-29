package com.qiplat.sweetline;

public class TokenSpan {
    public TextRange range;
    public int style;
    public String styleName;

    public TokenSpan(TextRange range, int style) {
        this.range = range;
        this.style = style;
    }

    @Override
    public String toString() {
        return "TokenSpan{" +
                "range=" + range +
                ", style=" + style +
                ", styleName='" + styleName + '\'' +
                '}';
    }
}
