import { hapTasks } from '@ohos/hvigor-ohos-plugin';
import { HvigorNode, HvigorPlugin } from '@ohos/hvigor';
import * as fs from 'fs';
import * as path from 'path';

function copyRawFilesPlugin(): HvigorPlugin {
  return {
    pluginId: 'copyRawFilesPlugin',
    apply(node: HvigorNode): void {
      node.afterNodeEvaluate(nd => {
        const moduleDir = nd.getNodePath();
        const projectRoot = path.resolve(moduleDir, '..', '..', '..');
        const rawfileDir = path.join(moduleDir, 'src', 'main', 'resources', 'rawfile');

        if (!fs.existsSync(rawfileDir)) {
          fs.mkdirSync(rawfileDir, { recursive: true });
        }

        // syntaxes/*.json -> rawfile/*.json
        const syntaxesDir = path.join(projectRoot, 'syntaxes');
        copyFlatFiles(syntaxesDir, rawfileDir);

        // tests/files/example.* -> rawfile/example.*
        const testFilesDir = path.join(projectRoot, 'tests', 'files');
        copyFlatFiles(testFilesDir, rawfileDir);
      });
    }
  };
}

function copyFlatFiles(srcDir: string, destDir: string): void {
  if (!fs.existsSync(srcDir)) {
    return;
  }
  const entries = fs.readdirSync(srcDir, { withFileTypes: true });
  for (const entry of entries) {
    if (!entry.isFile()) {
      continue;
    }
    const srcFile = path.join(srcDir, entry.name);
    const destFile = path.join(destDir, entry.name);
    fs.copyFileSync(srcFile, destFile);
  }
}

export default {
  system: hapTasks,
  plugins: [copyRawFilesPlugin()]
}
