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
        fs.rmSync(rawfileDir, { recursive: true, force: true });
        fs.mkdirSync(rawfileDir, { recursive: true });

        const syntaxRawfileDir = path.join(rawfileDir, 'syntaxes');
        const exampleRawfileDir = path.join(rawfileDir, 'examples');

        // syntaxes/*.json -> rawfile/syntaxes/*.json
        const syntaxesDir = path.join(projectRoot, 'syntaxes');
        copyFlatFiles(syntaxesDir, syntaxRawfileDir);

        // tests/files/* -> rawfile/examples/*
        const testFilesDir = path.join(projectRoot, 'tests', 'files');
        copyFlatFiles(testFilesDir, exampleRawfileDir);
      });
    }
  };
}

function copyFlatFiles(srcDir: string, destDir: string): void {
  if (!fs.existsSync(srcDir)) {
    return;
  }
  fs.mkdirSync(destDir, { recursive: true });
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
