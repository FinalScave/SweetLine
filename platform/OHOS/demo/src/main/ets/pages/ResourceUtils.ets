import resourceManager from '@ohos.resourceManager';
import { util } from '@kit.ArkTS';

export class ResourceUtils {
  /**
   * Read a text file from the rawfile directory
   */
  static async readRawTextFile(context: Context | undefined, fileName: string): Promise<string> {
    try {
      if (context == undefined) {
        return '';
      }
      const resourceMgr: resourceManager.ResourceManager = context.resourceManager;
      const fileData = await resourceMgr.getRawFileContent(fileName);
      const textDecoder: util.TextDecoder = util.TextDecoder.create('utf-8');
      return textDecoder.decodeToString(fileData, { stream: false });
    } catch (error) {
      console.error(`Failed to read file: ${fileName}`, error);
      return '';
    }
  }

  static listRawFileNames(context: Context | undefined, directory: string): string[] {
    try {
      if (context == undefined) {
        return [];
      }
      const resourceMgr: resourceManager.ResourceManager = context.resourceManager;
      return resourceMgr.getRawFileListSync(directory);
    } catch (error) {
      console.error(`Failed to list rawfile directory: ${directory}`, error);
      return [];
    }
  }

  /**
   * Get string resource
   */
  static async getString(context: Context | undefined, resourceId: number): Promise<string> {
    try {
      if (context == undefined) {
        return '';
      }
      const resourceMgr: resourceManager.ResourceManager = context.resourceManager;
      return await resourceMgr.getStringValue(resourceId);
    } catch (error) {
      console.error('Failed to get string resource:', error);
      return '';
    }
  }

  /**
   * Check if a file exists in rawfile
   */
  static async isRawFileExists(context: Context | undefined, fileName: string, directory: string = ''): Promise<boolean> {
    try {
      if (context == undefined) {
        return false;
      }
      const rawFileList = ResourceUtils.listRawFileNames(context, directory);
      return rawFileList.includes(fileName);
    } catch (error) {
      console.error('Failed to check file existence:', error);
      return false;
    }
  }
}
