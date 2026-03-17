namespace Demo;

internal static class Program {
	[STAThread]
	static void Main() {
		string syntaxesDir = ResolveDirFromExe("syntaxes");
		string examplesDir = ResolveDirFromExe(Path.Combine("tests", "files"));

		if (!Directory.Exists(syntaxesDir)) {
			MessageBox.Show(
				$"Syntaxes directory not found:\n{syntaxesDir}",
				"SweetLine Demo",
				MessageBoxButtons.OK,
				MessageBoxIcon.Error);
			return;
		}

		if (!Directory.Exists(examplesDir)) {
			MessageBox.Show(
				$"Examples directory not found:\n{examplesDir}",
				"SweetLine Demo",
				MessageBoxButtons.OK,
				MessageBoxIcon.Error);
			return;
		}

		ApplicationConfiguration.Initialize();
		Application.Run(new Form1(syntaxesDir, examplesDir));
	}

	private static string ResolveDirFromExe(string relativePath) {
		return Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../../../", relativePath));
	}
}
