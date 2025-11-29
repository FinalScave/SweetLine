import { sweetline } from "@qiplat/sweetline";


@Entry
@Component
struct Index {
  @State message: string = 'Hello World';

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
          let config = new sweetline.HighlightConfig();
          let engine = new sweetline.HighlightEngine(config);
          engine.registerStyleName("keyword", 1);
          let document = sweetline.Document.create("aaa.t", "AAA\nhhh");
          this.message = document.getLine(1);
        })
    }
    .height('100%')
    .width('100%')
  }
}