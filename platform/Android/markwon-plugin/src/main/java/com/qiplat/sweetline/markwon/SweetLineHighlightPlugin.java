package com.qiplat.sweetline.markwon;

import android.text.Spannable;

import androidx.annotation.NonNull;

import com.qiplat.sweetline.Document;
import com.qiplat.sweetline.DocumentAnalyzer;
import com.qiplat.sweetline.SpannableStyleFactory;
import com.qiplat.sweetline.TextAnalyzer;

import org.commonmark.node.FencedCodeBlock;

import io.noties.markwon.AbstractMarkwonPlugin;
import io.noties.markwon.MarkwonConfiguration;
import io.noties.markwon.MarkwonSpansFactory;
import io.noties.markwon.MarkwonVisitor;
import io.noties.markwon.RenderProps;
import io.noties.markwon.SpanFactory;

public class SweetLineHighlightPlugin extends AbstractMarkwonPlugin {

    private final CodeBackground background;
    private final SpannableStyleFactory styleFactory;

    public SweetLineHighlightPlugin(CodeBackground background, SpannableStyleFactory styleFactory) {
        this.background = background;
        this.styleFactory = styleFactory;
    }

    @Override
    public void configure(@NonNull Registry registry) {
    }
    
    @Override
    public void configureVisitor(@NonNull MarkwonVisitor.Builder builder) {
        builder.on(FencedCodeBlock.class, new FencedCodeBlockVisitor());
    }
    
    @Override
    public void configureSpansFactory(@NonNull MarkwonSpansFactory.Builder builder) {
        builder.setFactory(FencedCodeBlock.class, new CodeBlockSpanFactory());
    }

    private class FencedCodeBlockVisitor implements MarkwonVisitor.NodeVisitor<FencedCodeBlock> {
        @Override
        public void visit(@NonNull MarkwonVisitor visitor, @NonNull FencedCodeBlock fencedCodeBlock) {
            final String literal = fencedCodeBlock.getLiteral();
            final String info = fencedCodeBlock.getInfo();
            if (info != null) {
                TextAnalyzer textAnalyzer = SweetLineGlobal.getEngineInstance().createAnalyzerByExtension(info);
                Spannable highlightedCode = textAnalyzer.analyzeTextAsSpannable(literal, styleFactory);
                visitor.builder().append(highlightedCode);
            } else {
                visitor.builder().append(literal);
            }
            visitor.ensureNewLine();
        }
    }

    private class CodeBlockSpanFactory implements SpanFactory {
        @Override
        public Object getSpans(@NonNull MarkwonConfiguration configuration, @NonNull RenderProps props) {
            return new CodeBackgroundSpan(background);
        }
    }
}