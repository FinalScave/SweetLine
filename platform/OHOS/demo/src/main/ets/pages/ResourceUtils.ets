import resourceManager from '@ohos.resourceManager';
import { util } from '@kit.ArkTS';

export class ResourceUtils {
  /**
   * 读取rawfile目录下的文本文件
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

  /**
   * 获取string资源
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
   * 检查rawfile中的文件是否存在
   */
  static async isRawFileExists(context: Context | undefined, fileName: string): Promise<boolean> {
    try {
      if (context == undefined) {
        return false;
      }
      const resourceMgr: resourceManager.ResourceManager = context.resourceManager;
      const rawFileList = resourceMgr.getRawFileListSync("");
      return rawFileList.includes(fileName);
    } catch (error) {
      console.error('Failed to check file existence:', error);
      return false;
    }
  }
}