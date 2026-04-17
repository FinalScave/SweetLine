using System.Diagnostics;
using SweetLine;

namespace Demo;

public partial class Form1 : Form {
	private static readonly Dictionary<string, string> ExactSyntaxMap = new(StringComparer.Ordinal) {
		[".gitignore"] = "gitignore.json",
		["CMakeLists.txt"] = "cmake.json",
		["Containerfile"] = "dockerfile.json",
		["Dockerfile"] = "dockerfile.json",
		["GNUmakefile"] = "makefile.json",
		["Makefile"] = "makefile.json",
		["makefile"] = "makefile.json"
	};
	private static readonly Dictionary<string, string> SuffixSyntaxMap = new(StringComparer.Ordinal) {
		[".t"] = "tiecode.json",
		[".c"] = "c.json",
		[".cpp"] = "cpp.json",
		[".cs"] = "csharp.json",
		[".dart"] = "dart.json",
		[".go"] = "go.json",
		[".groovy"] = "groovy.json",
		[".html"] = "html.json",
		[".java"] = "java.json",
		[".js"] = "javascript.json",
		[".json"] = "json-sweetline.json",
		[".jsonc"] = "jsonc.json",
		[".json5"] = "json5.json",
		[".kt"] = "kotlin.json",
		[".lua"] = "lua.json",
		[".m"] = "objc.json",
		[".php"] = "php.json",
		[".ps1"] = "powershell.json",
		[".py"] = "python.json",
		[".rs"] = "rust.json",
		[".scala"] = "scala.json",
		[".sh"] = "shell.json",
		[".sql"] = "sql.json",
		[".swift"] = "swift.json",
		[".toml"] = "toml.json",
		[".ts"] = "typescript.json",
		[".vb"] = "vb.json",
		[".xml"] = "xml.json",
		[".yaml"] = "yaml.json",
		[".md"] = "markdown.json",
		[".wenyan"] = "wenyan.json",
		[".myu"] = "iapp.json",
		[".css"] = "css.json",
		[".scss"] = "scss.json",
		[".less"] = "less.json",
		[".cmake"] = "cmake.json",
		[".dockerfile"] = "dockerfile.json",
		[".mk"] = "makefile.json",
		[".properties"] = "properties.json",
		[".env"] = "env.json",
		[".proto"] = "protobuf.json",
		[".graphql"] = "graphql.json",
		[".gql"] = "graphql.json",
		[".nginx"] = "nginx.json",
		[".conf"] = "nginx.json",
		[".gitignore"] = "gitignore.json",
		[".diff"] = "diff.json",
		[".patch"] = "diff.json",
		[".rb"] = "ruby.json",
		[".rake"] = "ruby.json",
		[".gemspec"] = "ruby.json",
		[".ru"] = "ruby.json",
		[".hcl"] = "hcl.json",
		[".tf"] = "terraform.json",
		[".tfvars"] = "terraform.json",
		[".tfbackend"] = "terraform.json",
		[".vue"] = "vue.json",
		[".svelte"] = "svelte.json"
	};
	private static readonly Dictionary<string, string> RoutedDocumentNames = new(StringComparer.Ordinal) {
		[".gitignore"] = "example.gitignore",
		["CMakeLists.txt"] = "example.cmake",
		["Containerfile"] = "example.dockerfile",
		["Dockerfile"] = "example.dockerfile",
		["GNUmakefile"] = "example.mk",
		["Makefile"] = "example.mk",
		["makefile"] = "example.mk"
	};
	private static readonly List<string> SortedSuffixes = SuffixSyntaxMap.Keys
		.OrderByDescending(static suffix => suffix.Length)
		.ToList();

	private readonly string _syntaxesDir;
	private readonly string _examplesDir;
	private readonly List<string> _exampleFiles;
	private readonly List<HighlightTheme> _themes;
	private readonly HighlightEngine _engine;

	private ComboBox _fileCombo = null!;
	private ComboBox _themeCombo = null!;
	private Label _statusLabel = null!;
	private CodeViewControl _codeView = null!;
	private Panel _editorHost = null!;
	private HighlightTheme _currentTheme;
	private bool _suppressComboEvents;

	public Form1(string syntaxesDir, string examplesDir) {
		_syntaxesDir = syntaxesDir;
		_examplesDir = examplesDir;
		_themes = HighlightTheme.BuiltinThemes();
		_currentTheme = _themes[0];
		_exampleFiles = ListExampleFiles(_examplesDir);
		_engine = new HighlightEngine(new HighlightConfig(ShowIndex: true, InlineStyle: false));
		RegisterStyleNames(_engine);

		InitializeComponent();
		InitializeUi();
		Text = "SweetLine Demo(C# WinForms)";
		Size = new Size(800, 1280);
		StartPosition = FormStartPosition.CenterScreen;

		if (_exampleFiles.Count > 0) {
			_fileCombo.SelectedIndex = 0;
		} else {
			_statusLabel.Text = $"No demo files found under: {_examplesDir}";
		}
	}

	protected override void OnFormClosed(FormClosedEventArgs e) {
		_engine.Dispose();
		base.OnFormClosed(e);
	}

	private void InitializeUi() {
		SuspendLayout();
		Controls.Clear();

		FlowLayoutPanel toolbar = new() {
			Dock = DockStyle.Top,
			Height = 44,
			Padding = new Padding(8, 8, 8, 4),
			WrapContents = false,
			AutoScroll = true,
			FlowDirection = FlowDirection.LeftToRight
		};

		Label fileLabel = new() { Text = "File:", AutoSize = true, Margin = new Padding(0, 8, 4, 0) };
		_fileCombo = new ComboBox {
			DropDownStyle = ComboBoxStyle.DropDownList,
			Width = 300,
			Margin = new Padding(0, 4, 12, 0)
		};
		_fileCombo.Items.AddRange(_exampleFiles.Cast<object>().ToArray());
		_fileCombo.SelectedIndexChanged += (_, _) => {
			if (!_suppressComboEvents) {
				HighlightSelectedFile();
			}
		};

		Label themeLabel = new() { Text = "Theme:", AutoSize = true, Margin = new Padding(0, 8, 4, 0) };
		_themeCombo = new ComboBox {
			DropDownStyle = ComboBoxStyle.DropDownList,
			Width = 190,
			Margin = new Padding(0, 4, 12, 0)
		};
		_themeCombo.Items.AddRange(HighlightTheme.ThemeNames(_themes));
		_themeCombo.SelectedIndex = 0;
		_themeCombo.SelectedIndexChanged += (_, _) => {
			if (_suppressComboEvents) {
				return;
			}

			int idx = _themeCombo.SelectedIndex;
			if (idx >= 0 && idx < _themes.Count) {
				_currentTheme = _themes[idx];
				_codeView.SetTheme(_currentTheme);
			}
		};

		Button openButton = new() {
			Text = "Open...",
			AutoSize = true,
			Margin = new Padding(0, 4, 0, 0)
		};
		openButton.Click += (_, _) => OpenExternalFile();

		toolbar.Controls.Add(fileLabel);
		toolbar.Controls.Add(_fileCombo);
		toolbar.Controls.Add(themeLabel);
		toolbar.Controls.Add(_themeCombo);
		toolbar.Controls.Add(openButton);

		_editorHost = new Panel
		{
			Dock = DockStyle.Fill,
			AutoScroll = true
		};
		_codeView = new CodeViewControl { Location = new Point(0, 0) };
		_codeView.SetTheme(_currentTheme);
		_editorHost.Controls.Add(_codeView);

		_statusLabel = new Label {
			Dock = DockStyle.Bottom,
			Height = 28,
			Padding = new Padding(8, 5, 8, 0),
			Text = " "
		};

		Controls.Add(_editorHost);
		Controls.Add(_statusLabel);
		Controls.Add(toolbar);
		ResumeLayout(performLayout: true);
	}

	private void HighlightSelectedFile() {
		int idx = _fileCombo.SelectedIndex;
		if (idx < 0 || idx >= _exampleFiles.Count) {
			return;
		}

		string fileName = _exampleFiles[idx];
		string? syntaxFileName = ResolveSyntaxFileName(fileName);
		if (syntaxFileName is null) {
			_statusLabel.Text = $"No syntax mapping for file: {fileName}";
			return;
		}

		string syntaxPath = Path.Combine(_syntaxesDir, syntaxFileName);
		string examplePath = Path.Combine(_examplesDir, fileName);
		if (!File.Exists(syntaxPath)) {
			_statusLabel.Text = $"Syntax file not found: {syntaxPath}";
			return;
		}

		try {
			HighlightFile(examplePath, syntaxPath, ResolveDocumentFileName(fileName));
		} catch (Exception ex) {
			_statusLabel.Text = $"Error: {ex.Message}";
		}
	}

	private void HighlightFile(string filePath, string syntaxPath, string documentFileName) {
		string sourceCode = File.ReadAllText(filePath);
		string syntaxJson = File.ReadAllText(syntaxPath);
		string fileName = Path.GetFileName(filePath);

		Stopwatch compileWatch = Stopwatch.StartNew();
		_engine.CompileSyntaxFromJson(syntaxJson);
		compileWatch.Stop();
		long compileUs = (long)(compileWatch.Elapsed.TotalMilliseconds * 1000);

		using Document doc = new(documentFileName, sourceCode);
		using DocumentAnalyzer? analyzer = _engine.LoadDocument(doc);
		if (analyzer is null) {
			_statusLabel.Text = "Failed to load document.";
			return;
		}

		Stopwatch analyzeWatch = Stopwatch.StartNew();
		DocumentHighlight highlight = analyzer.Analyze();
		analyzeWatch.Stop();
		long analyzeUs = (long)(analyzeWatch.Elapsed.TotalMilliseconds * 1000);

		IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
		int lineCount = sourceCode.Replace("\r\n", "\n").Split('\n').Length;
		_statusLabel.Text = $"Compile: {compileUs}us | Analyze: {analyzeUs}us | Lines: {lineCount} | File: {fileName}";
		_codeView.SetHighlightData(sourceCode, highlight, guides);
		_editorHost.AutoScrollPosition = new Point(0, 0);
	}

	private void OpenExternalFile() {
		using OpenFileDialog dialog = new() {
			Filter = "Supported Files|*.*|All Files|*.*",
			FilterIndex = 1,
			Title = "Open Source File"
		};

		if (dialog.ShowDialog(this) != DialogResult.OK) {
			return;
		}

		string filePath = dialog.FileName;
		string fileName = Path.GetFileName(filePath);
		string? syntaxFileName = ResolveSyntaxFileName(fileName);
		if (syntaxFileName is null) {
			_statusLabel.Text = $"Unsupported file name: {fileName}";
			return;
		}

		string syntaxPath = Path.Combine(_syntaxesDir, syntaxFileName);
		if (!File.Exists(syntaxPath)) {
			_statusLabel.Text = $"Syntax file not found: {syntaxPath}";
			return;
		}

		try {
			_suppressComboEvents = true;
			_fileCombo.SelectedIndex = -1;
			_suppressComboEvents = false;
			HighlightFile(filePath, syntaxPath, ResolveDocumentFileName(fileName));
		} catch (Exception ex) {
			_statusLabel.Text = $"Error: {ex.Message}";
		}
	}

	private static void RegisterStyleNames(HighlightEngine engine) {
		engine.RegisterStyleName("keyword", HighlightTheme.StyleKeyword);
		engine.RegisterStyleName("string", HighlightTheme.StyleString);
		engine.RegisterStyleName("number", HighlightTheme.StyleNumber);
		engine.RegisterStyleName("comment", HighlightTheme.StyleComment);
		engine.RegisterStyleName("class", HighlightTheme.StyleClass);
		engine.RegisterStyleName("method", HighlightTheme.StyleMethod);
		engine.RegisterStyleName("variable", HighlightTheme.StyleVariable);
		engine.RegisterStyleName("punctuation", HighlightTheme.StylePunctuation);
		engine.RegisterStyleName("annotation", HighlightTheme.StyleAnnotation);
		engine.RegisterStyleName("preprocessor", HighlightTheme.StylePreprocessor);
		engine.RegisterStyleName("macro", HighlightTheme.StyleMacro);
		engine.RegisterStyleName("lifetime", HighlightTheme.StyleLifetime);
		engine.RegisterStyleName("selector", HighlightTheme.StyleSelector);
		engine.RegisterStyleName("builtin", HighlightTheme.StyleBuiltin);
		engine.RegisterStyleName("url", HighlightTheme.StyleUrl);
		engine.RegisterStyleName("property", HighlightTheme.StyleProperty);
	}

	private static List<string> ListExampleFiles(string examplesDir) {
		List<string> files = [];
		if (!Directory.Exists(examplesDir)) {
			return files;
		}

		foreach (string path in Directory.GetFiles(examplesDir)) {
			string name = Path.GetFileName(path);
			if (!name.StartsWith("example", StringComparison.OrdinalIgnoreCase) &&
				!name.Equals("json-sweetline.json", StringComparison.OrdinalIgnoreCase)) {
				if (ResolveSyntaxFileName(name) is null) {
					continue;
				}
			}
			if (ResolveSyntaxFileName(name) is not null) {
				files.Add(name);
			}
		}

		files.Sort(StringComparer.OrdinalIgnoreCase);
		return files;
	}

	private static string? ResolveSyntaxFileName(string fileName) {
		if (ExactSyntaxMap.TryGetValue(fileName, out string? exactSyntaxFileName)) {
			return exactSyntaxFileName;
		}

		foreach (string suffix in SortedSuffixes) {
			if (fileName.EndsWith(suffix, StringComparison.Ordinal)) {
				return SuffixSyntaxMap[suffix];
			}
		}

		return null;
	}

	private static string ResolveDocumentFileName(string fileName) {
		return RoutedDocumentNames.TryGetValue(fileName, out string? routedDocumentName)
			? routedDocumentName
			: fileName;
	}
}
