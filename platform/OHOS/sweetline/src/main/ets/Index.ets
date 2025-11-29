import List from "@ohos.util.List";
import lib from "libsweetline.so"

export namespace sweetline {
  export class TextPosition {
    public line: number = 0;
    public column: number = 0;
    public index: number = -1;

    public constructor(line: number = 0, column: number = 0, index: number = -1) {
      this.line = line;
      this.column = column;
      this.index = index;
    }
  }

  export class TextRange {
    public start: TextPosition = new TextPosition();
    public end: TextPosition = new TextPosition();

    public constructor(start: TextPosition = new TextPosition(), end: TextPosition = new TextPosition()) {
      this.start = start;
      this.end = end;
    }
  }

  export class TokenSpan {
    public range: TextRange = new TextRange();
    public style: number = 0;

    constructor(range: TextRange, style: number) {
      this.range = range;
      this.style = style;
    }
  }

  export class LineHighlight {
    public spans: List<TokenSpan> = new List();
  }

  export class DocumentHighlight {
    public lines: List<LineHighlight> = new List();
  }

  export class HighlightConfig {
    public showIndex: boolean = false;
  }

  export class Document {
    protected nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    public getLineCount(): number {
      return lib.Document_GetLineCount(this.nativeHandle);
    }

    public getLine(line: number): string {
      return lib.Document_GetLine(this.nativeHandle, line);
    }

    public getText(): string {
      return lib.Document_GetText(this.nativeHandle);
    }

    public static create(uri: string, content: string) {
      let handle: number = lib.Document_Create(uri, content);
      return new Document(handle);
    }

    public __getNativeHandle(): number {
      return this.nativeHandle;
    }

    public static __createNative(nativeHandle: number) {
      return new Document(nativeHandle);
    }
  }

  export class SyntaxRule {
    protected nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    public getName(): string {
      return lib.SyntaxRule_GetName(this.nativeHandle);
    }

    public getFileExtensions(): string[] {
      return lib.SyntaxRule_GetFileExtensions(this.nativeHandle);
    }

    public __getNativeHandle(): number {
      return this.nativeHandle;
    }

    public static __createNative(nativeHandle: number) {
      return new SyntaxRule(nativeHandle);
    }
  }

  export class DocumentAnalyzer {
    private nativeHandle: number = 0;

    protected constructor(nativeHandle: number) {
      this.nativeHandle = nativeHandle;
    }

    public analyze(): DocumentHighlight {
      let buffer: Int32Array = lib.DocumentAnalyzer_Analyze(this.nativeHandle);
      return this.readDocumentHighlight(buffer);
    }

    public analyzeChanges(range: TextRange, newText: string): DocumentHighlight {
      let buffer: Int32Array;
      if (range.start.index >= 0 && range.end.index >= 0) {
        buffer = lib.DocumentAnalyzer_AnalyzeChanges2(this.nativeHandle, range.start.index, range.end.index, newText);
      } else {
        buffer = lib.DocumentAnalyzer_AnalyzeChanges(this.nativeHandle, range.start.line, range.start.column, range.end.line, range.end.column, newText);
      }
      return this.readDocumentHighlight(buffer);
    }

    public analyzeLine(line: number): LineHighlight {
      let buffer: Int32Array = lib.DocumentAnalyzer_AnalyzeLine(this.nativeHandle, line);
      return this.readLineHighlight(buffer);
    }

    public getDocument(): Document {
      let documentHandle: number = lib.DocumentAnalyzer_GetDocument(this.nativeHandle);
      return Document.__createNative(documentHandle);
    }

    public static create(document: Document, syntaxRule: SyntaxRule, config: HighlightConfig) {
      let handle: number = lib.DocumentAnalyzer_Create(document.__getNativeHandle(), syntaxRule.__getNativeHandle(), config.showIndex);
      return new DocumentAnalyzer(handle);
    }

    public static __createNative(nativeHandle: number) {
      return new DocumentAnalyzer(nativeHandle);
    }

    private readDocumentHighlight(buffer: Int32Array): DocumentHighlight {
      let highlight = new DocumentHighlight();
      let lineHighlight = new LineHighlight();
      let spanCount = buffer.length / 7;
      let line = -1;
      for (let i = 0; i < spanCount; i++) {
        let baseIndex = i * 7;
        let startLine = buffer[baseIndex];
        let startColumn = buffer[baseIndex + 1];
        let startIndex = buffer[baseIndex + 2];
        let endLine = buffer[baseIndex + 3];
        let endColumn = buffer[baseIndex + 4];
        let endIndex = buffer[baseIndex + 5];
        let styleId = buffer[baseIndex + 6];

        if (startLine != line) {
          line = startLine;
          lineHighlight = new LineHighlight();
          highlight.lines.add(lineHighlight);
        }

        let range = new TextRange(
          new TextPosition(startLine, startColumn, startIndex),
          new TextPosition(endLine, endColumn, endIndex)
        );
        let span = new TokenSpan(range, styleId);
        lineHighlight.spans.add(span);
      }
      return highlight;
		}

    private readLineHighlight(buffer: Int32Array): LineHighlight {
      let spanCount = buffer.length / 7;
      let lineHighlight = new LineHighlight();
      for (let i = 0; i < spanCount; i++) {
        let baseIndex = i * 7;
        let startLine = buffer[baseIndex];
        let startColumn = buffer[baseIndex + 1];
        let startIndex = buffer[baseIndex + 2];
        let endLine = buffer[baseIndex + 3];
        let endColumn = buffer[baseIndex + 4];
        let endIndex = buffer[baseIndex + 5];
        let styleId = buffer[baseIndex + 6];
        let range = new TextRange(
          new TextPosition(startLine, startColumn, startIndex),
          new TextPosition(endLine, endColumn, endIndex)
        );
        let span = new TokenSpan(range, styleId);
        lineHighlight.spans.add(span);
      }
      return lineHighlight;
    }
  }

  export class HighlightEngine {
    private nativeHandle: number = 0;

    public constructor(config: HighlightConfig) {
      this.nativeHandle = lib.HighlightEngine_Create(config.showIndex);
    }

    public registerStyleName(styleName: string, styleId: number): boolean {
      return lib.HighlightEngine_RegisterStyleName(this.nativeHandle, styleName, styleId);
    }

    public getStyleName(styleId: number): string {
      return lib.HighlightEngine_GetStyleName(this.nativeHandle, styleId);
    }

    public compileSyntaxFromJson(json: string): SyntaxRule {
      let handle: number = lib.HighlightEngine_CompileSyntaxFromJson(this.nativeHandle, json);
      return SyntaxRule.__createNative(handle);
    }

    public compileSyntaxFromFile(path: string): SyntaxRule {
      let handle: number = lib.HighlightEngine_CompileSyntaxFromFile(this.nativeHandle, path);
      return SyntaxRule.__createNative(handle);
    }

    public loadDocument(document: Document): DocumentAnalyzer {
      let handle: number = lib.HighlightEngine_LoadDocument(this.nativeHandle, document.__getNativeHandle());
      return DocumentAnalyzer.__createNative(handle);
    }
  }
}