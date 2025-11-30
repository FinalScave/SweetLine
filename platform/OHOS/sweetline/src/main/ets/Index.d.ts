import List from "@ohos.util.List";

export declare namespace sweetline {
  export class TextPosition {
    public line: number;
    public column: number;
    public index: number;

    public constructor();
    public constructor(line: number, column: number);
    public constructor(line: number, column: number, index: number);
  }

  export class TextRange {
    public start: TextPosition;
    public end: TextPosition;

    public constructor(start: TextPosition , end: TextPosition);
  }

  export class TokenSpan {
    public range: TextRange;
    public style: number;

    constructor(range: TextRange, style: number);
  }

  export class LineHighlight {
    public spans: List<TokenSpan>;
  }

  export class DocumentHighlight {
    public lines: List<LineHighlight>;
  }

  export class HighlightConfig {
    public showIndex: boolean;

    public static withIndex(): HighlightConfig;
  }

  export class Document {
    public constructor(uri: string, content: string);

    public getUri(): string;
    public totalChars(): number;
    public getLineCharCount(line: number): number;
    public charIndexOfLine(line: number): number;
    public charIndexToPosition(index: number): TextPosition;
    public getLineCount(): number;
    public getLine(line: number): string;
    public getText(): string;
  }

  export class SyntaxRule {
    public getName(): string;
    public getFileExtensions(): string[];
  }

  export class DocumentAnalyzer {
    public analyze(): DocumentHighlight;
    public analyzeChanges(range: TextRange, newText: string): DocumentHighlight;
    public analyzeLine(line: number): LineHighlight;
    public getDocument(): Document;
  }

  export class HighlightEngine {
    public constructor(config: HighlightConfig);

    public registerStyleName(styleName: string, styleId: number): boolean;
    public getStyleName(styleId: number): string;
    public compileSyntaxFromJson(json: string): SyntaxRule;
    public compileSyntaxFromFile(path: string): SyntaxRule;
    public loadDocument(document: Document): DocumentAnalyzer;
    public removeDocument(uri: string): boolean;
  }
}