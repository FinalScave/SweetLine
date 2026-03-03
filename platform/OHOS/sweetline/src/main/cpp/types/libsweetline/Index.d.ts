// Document
export const Document_Create: (uri: string, content: string) => number;
export const Document_Delete: (handle: number) => void;
export const Document_GetUri: (handle: number) => string;
export const Document_TotalChars: (handle: number) => number;
export const Document_GetLineCharCount: (handle: number, line: number) => number;
export const Document_CharIndexOfLine: (handle: number, line: number) => number;
export const Document_CharIndexToPosition: (handle: number, index: number) => Int32Array;
export const Document_GetLineCount: (handle: number) => number;
export const Document_GetLine: (handle: number, line: number) => string;
export const Document_GetText: (handle: number) => string;

// SyntaxRule
export const SyntaxRule_GetName: (handle: number) => string;
export const SyntaxRule_GetFileExtensions: (handle: number) => string[];

// TextAnalyzer
export const TextAnalyzer_Delete: (handle: number) => void;
export const TextAnalyzer_AnalyzeText: (handle: number, text: string) => Int32Array;
export const TextAnalyzer_AnalyzeLine: (handle: number, text: string, line: number, startState: number, startCharOffset: number) => Int32Array;
export const TextAnalyzer_AnalyzeIndentGuides: (handle: number, text: string) => Int32Array;

// DocumentAnalyzer
export const DocumentAnalyzer_Delete: (handle: number) => void;
export const DocumentAnalyzer_Analyze: (handle: number) => Int32Array;
export const DocumentAnalyzer_AnalyzeChanges: (handle: number, startLine: number, startColumn: number, endLine: number, endColumn: number, newText: string) => Int32Array;
export const DocumentAnalyzer_AnalyzeChanges2: (handle: number, startIndex: number, endIndex: number, newText: string) => Int32Array;
export const DocumentAnalyzer_AnalyzeIndentGuides: (handle: number) => Int32Array;
export const DocumentAnalyzer_GetDocument: (handle: number) => number;

// HighlightEngine
export const HighlightEngine_Create: (configBits: number) => number;
export const HighlightEngine_Delete: (handle: number) => void;
export const HighlightEngine_RegisterStyleName: (handle: number, styleName: string, styleId: number) => boolean;
export const HighlightEngine_GetStyleName: (handle: number, styleId: number) => string;
export const HighlightEngine_DefineMacro: (handle: number, macroName: string) => boolean;
export const HighlightEngine_UndefineMacro: (handle: number, macroName: string) => boolean;
export const HighlightEngine_CompileSyntaxFromJson: (handle: number, json: string) => number;
export const HighlightEngine_CompileSyntaxFromFile: (handle: number, path: string) => number;
export const HighlightEngine_CreateAnalyzerByName: (handle: number, syntaxName: string) => number;
export const HighlightEngine_CreateAnalyzerByExtension: (handle: number, extension: string) => number;
export const HighlightEngine_LoadDocument: (handle: number, documentHandle: number) => number;
export const HighlightEngine_RemoveDocument: (handle: number, uri: string) => boolean;
