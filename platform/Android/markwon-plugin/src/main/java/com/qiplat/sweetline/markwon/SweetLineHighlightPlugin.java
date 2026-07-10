package com.qiplat.sweetline.markwon;

import android.text.Spannable;

import androidx.annotation.NonNull;

import com.qiplat.sweetline.Document;
import com.qiplat.sweetline.DocumentAnalyzer;
import com.qiplat.sweetline.SpannableStyleFactory;
import com.qiplat.sweetline.TextAnalyzer;

import org.commonmark.node.FencedCodeBlock;

import java.util.HashMap;
import java.util.Map;

import io.noties.markwon.AbstractMarkwonPlugin;
import io.noties.markwon.MarkwonConfiguration;
import io.noties.markwon.MarkwonSpansFactory;
import io.noties.markwon.MarkwonVisitor;
import io.noties.markwon.RenderProps;
import io.noties.markwon.SpanFactory;

public class SweetLineHighlightPlugin extends AbstractMarkwonPlugin {

    private final CodeBackground background;
    private final SpannableStyleFactory styleFactory;
    private static final Map<String, String> syntaxNameMapping = new HashMap<>();

    static {
        syntaxNameMapping.put("py", "python");
        syntaxNameMapping.put("ts", "typescript");
        syntaxNameMapping.put("kt", "kotlin");
        syntaxNameMapping.put("c++", "cpp");
        syntaxNameMapping.put("c#", "csharp");
        syntaxNameMapping.put("docker", "dockerfile");
        syntaxNameMapping.put("make", "makefile");
        syntaxNameMapping.put("dotenv", "env");
        syntaxNameMapping.put("proto", "protobuf");
        syntaxNameMapping.put("protobuf", "protobuf");
        syntaxNameMapping.put("graphql", "graphql");
        syntaxNameMapping.put("gql", "graphql");
        syntaxNameMapping.put("nginx", "nginx");
        syntaxNameMapping.put("conf", "nginx");
        syntaxNameMapping.put("gitignore", "gitignore");
        syntaxNameMapping.put("diff", "diff");
        syntaxNameMapping.put("patch", "diff");
        syntaxNameMapping.put("rb", "ruby");
        syntaxNameMapping.put("ruby", "ruby");
        syntaxNameMapping.put("hcl", "hcl");
        syntaxNameMapping.put("tf", "terraform");
        syntaxNameMapping.put("terraform", "terraform");
        syntaxNameMapping.put("vue", "vue");
        syntaxNameMapping.put("svelte", "svelte");
    }

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
                String syntaxName = syntaxNameMapping.get(info);
                if (syntaxName == null) {
                    syntaxName = info;
                }
                try (TextAnalyzer textAnalyzer = SweetLineGlobal.getEngineInstance().createAnalyzerBySyntaxName(syntaxName)) {
                    if (textAnalyzer != null) {
                        Spannable highlightedCode = textAnalyzer.analyzeTextAsSpannable(literal, styleFactory);
                        visitor.builder().append(highlightedCode);
                    } else {
                        visitor.builder().append(literal);
                    }
                }
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
