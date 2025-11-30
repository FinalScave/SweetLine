package com.qiplat.sweetline.markwon;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.text.Layout;
import android.text.style.LeadingMarginSpan;
import android.text.style.LineBackgroundSpan;

public class CodeBackgroundSpan implements LeadingMarginSpan, LineBackgroundSpan {
    private final CodeBackground background;
    
    public CodeBackgroundSpan(CodeBackground background) {
        this.background = background;
    }
    
    @Override
    public void drawLeadingMargin(Canvas canvas, Paint paint,
                                  int x, int dir, int top, int baseline,
                                  int bottom, CharSequence text,
                                  int start, int end, boolean first,
                                  Layout layout) {
        // TODO: 绘制行号
    }
    
    @Override
    public int getLeadingMargin(boolean first) {
        RectF paddings = background.getPaddings();
        return (int) (paddings.left + paddings.right);
    }
    
    @Override
    public void drawBackground(Canvas canvas, Paint paint, 
                              int left, int right, int top, 
                              int baseline, int bottom, 
                              CharSequence text, int start, 
                              int end, int lineNumber) {
        final int save = canvas.save();
        try {
            final Drawable background = this.background.getBackground();
            if (background != null) {
                background.setBounds(left, top, right, bottom);
                background.draw(canvas);
            } else {
                paint.setColor(this.background.getBackgroundColor());
                canvas.drawRect(left, top, right, bottom, paint);
            }
        } finally {
            canvas.restoreToCount(save);
        }
    }
}