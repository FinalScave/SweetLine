import { sweetline } from "@qiplat/sweetline";
import { ResourceUtils } from "./ResourceUtils";


@Entry
@Component
struct Index {
  highlightEngine: sweetline.HighlightEngine = new sweetline.HighlightEngine(sweetline.HighlightConfig.withIndex());
  documentAnalyzer?: sweetline.DocumentAnalyzer;
  @State message: string = 'Hello World';

  onPageShow(): void {
    this.highlightEngine.registerStyleName("keyword", 1);
  }

  build() {
    RelativeContainer() {
      Text(this.message)
        .id('HelloWorld')
        .fontSize($r('app.float.page_text_font_size'))
        .fontWeight(FontWeight.Bold)
        .alignRules({
          center: { anchor: '__container__', align: VerticalAlign.Center },
          middle: { anchor: '__container__', align: HorizontalAlign.Center }
        })
        .onClick(() => {
          this.highlight("tests/java-syntax.json", "tests/Test.java")
        })
    }
    .height('100%')
    .width('100%')
  }

  async highlight(syntaxName: string, fileName: string) {
    const syntaxJson = await ResourceUtils.readRawTextFile(this.getUIContext().getHostContext(), syntaxName);
    this.highlightEngine.compileSyntaxFromJson(syntaxJson);

    const codeText = await ResourceUtils.readRawTextFile(this.getUIContext().getHostContext(), fileName);
    const document = sweetline.Document.create(fileName, codeText);
    this.documentAnalyzer = this.highlightEngine.loadDocument(document);

    const highlight = this.documentAnalyzer.analyze();
    this.message = JSON.stringify(highlight);
  }
}