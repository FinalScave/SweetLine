package com.qiplat.sweetline.markwon;

import android.graphics.RectF;
import android.graphics.drawable.Drawable;

public class CodeBackground {
    private RectF paddings;
    private int backgroundColor;
    private Drawable background;

    public CodeBackground(RectF paddings, int backgroundColor, Drawable background) {
        this.paddings = paddings;
        this.backgroundColor = backgroundColor;
        this.background = background;
    }

    public RectF getPaddings() {
        return paddings;
    }

    public int getBackgroundColor() {
        return backgroundColor;
    }

    public Drawable getBackground() {
        return background;
    }
}
