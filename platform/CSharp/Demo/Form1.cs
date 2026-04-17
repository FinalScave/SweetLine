using System.Diagnostics;
using SweetLine;

namespace Demo;

public partial class Form1 : Form {
	private const string ExcludedDuplicateSyntaxFile = "yaml(non zero width).json";

	private readonly string _syntaxesDir;
	private readonly string _examplesDir;
	private readonly List<string> _exampleFiles;
	private readonly List<HighlightTheme> _themes;
	private readonly HighlightEngine _engine;
	private readonly int _compiledSyntaxCount;
	private readonly long _precompileUs;

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
		_engine = new HighlightEngine(new HighlightConfig(ShowIndex: true, InlineStyle: false));
		RegisterStyleNames(_engine);
		_engine.DefineMacro("WINDOWS");
		(_compiledSyntaxCount, _precompileUs) = PrecompileCommonSyntaxes(_syntaxesDir, _engine);
		_exampleFiles = ListExampleFiles(_examplesDir, _engine);

		InitializeComponent();
		InitializeUi();
		Text = "SweetLine Demo(C# WinForms)";
		Size = new Size(800, 1280);
		StartPosition = FormStartPosition.CenterScreen;

		if (_exampleFiles.Count > 0) {
			_fileCombo.SelectedIndex = 0;
		} else {
			_statusLabel.Text = $"Compiled {_compiledSyntaxCount} syntax rule files in {_precompileUs}us | No demo files found under: {_examplesDir}";
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
		string examplePath = Path.Combine(_examplesDir, fileName);
		if (!File.Exists(examplePath)) {
			_statusLabel.Text = $"Demo file not found: {examplePath}";
			return;
		}

		try {
			HighlightFile(examplePath, fileName);
		} catch (Exception ex) {
			_statusLabel.Text = $"Error: {ex.Message}";
		}
	}

	private void HighlightFile(string filePath, string documentFileName) {
		string sourceCode = File.ReadAllText(filePath);
		string fileName = Path.GetFileName(filePath);

		using Document doc = new(documentFileName, sourceCode);
		Stopwatch loadWatch = Stopwatch.StartNew();
		using DocumentAnalyzer? analyzer = _engine.LoadDocument(doc);
		loadWatch.Stop();
		if (analyzer is null) {
			_statusLabel.Text = $"No matching syntax for file: {documentFileName}";
			return;
		}

		Stopwatch analyzeWatch = Stopwatch.StartNew();
		DocumentHighlight highlight = analyzer.Analyze();
		analyzeWatch.Stop();
		long loadUs = (long)(loadWatch.Elapsed.TotalMilliseconds * 1000);
		long analyzeUs = (long)(analyzeWatch.Elapsed.TotalMilliseconds * 1000);

		IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
		int lineCount = sourceCode.Replace("\r\n", "\n").Split('\n').Length;
		_statusLabel.Text = $"Warmup: {_compiledSyntaxCount} files in {_precompileUs}us | Load: {loadUs}us | Analyze: {analyzeUs}us | Lines: {lineCount} | File: {fileName}";
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
		try {
			_suppressComboEvents = true;
			_fileCombo.SelectedIndex = -1;
			_suppressComboEvents = false;
			HighlightFile(filePath, fileName);
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

	private static List<string> ListExampleFiles(string examplesDir, HighlightEngine engine) {
		List<string> files = [];
		if (!Directory.Exists(examplesDir)) {
			return files;
		}

		foreach (string path in Directory.GetFiles(examplesDir)) {
			string name = Path.GetFileName(path);
			if (!name.StartsWith("example", StringComparison.OrdinalIgnoreCase) &&
				!name.Equals("json-sweetline.json", StringComparison.OrdinalIgnoreCase)) {
				continue;
			}

			using TextAnalyzer? analyzer = engine.CreateAnalyzerByFileName(name);
			if (analyzer is not null) {
				files.Add(name);
			}
		}

		files.Sort(StringComparer.OrdinalIgnoreCase);
		return files;
	}

	private static (int CompiledSyntaxCount, long CompileUs) PrecompileCommonSyntaxes(string syntaxesDir, HighlightEngine engine) {
		List<string> syntaxPaths = Directory
			.GetFiles(syntaxesDir, "*.json")
			.Where(static path => !Path.GetFileName(path).EndsWith("-inlineStyle.json", StringComparison.Ordinal))
			.Where(static path => !Path.GetFileName(path).Equals(ExcludedDuplicateSyntaxFile, StringComparison.Ordinal))
			.OrderBy(static path => Path.GetFileName(path), StringComparer.Ordinal)
			.ToList();
		Dictionary<string, string> syntaxJsonByFileName = syntaxPaths.ToDictionary(
			static path => Path.GetFileName(path),
			static path => File.ReadAllText(path),
			StringComparer.Ordinal);
		List<string> pending = syntaxJsonByFileName.Keys
			.OrderBy(static fileName => fileName, StringComparer.Ordinal)
			.ToList();
		Stopwatch compileWatch = Stopwatch.StartNew();
		int compiledCount = 0;

		while (pending.Count > 0) {
			bool progress = false;
			List<string> retryPending = [];

			foreach (string fileName in pending) {
				try {
					engine.CompileSyntaxFromJson(syntaxJsonByFileName[fileName]);
					compiledCount++;
					progress = true;
				} catch (SyntaxCompileError ex) when (ex.ErrorCode == SyntaxCompileError.ErrImportSyntaxNotFound) {
					retryPending.Add(fileName);
				}
			}

			if (!progress) {
				throw new InvalidOperationException($"Failed to resolve syntax dependencies: {string.Join(", ", retryPending)}");
			}

			pending = retryPending;
		}

		compileWatch.Stop();
		long compileUs = (long)(compileWatch.Elapsed.TotalMilliseconds * 1000);
		return (compiledCount, compileUs);
	}
}
