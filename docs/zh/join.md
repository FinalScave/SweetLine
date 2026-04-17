# 项目结构
```
├── 3dparty 第三方C++库
├── docs 文档目录
├── platform 各平台封装/绑定工程
    ├── Android Android JNI SDK 工程
    ├── Java22 Java 22 FFM 封装与 Swing Demo
    ├── CSharp .NET/C# P/Invoke 封装与 WinForms Demo
    ├── Emscripten WASM SDK 工程
    └── OHOS 鸿蒙 NAPI SDK 工程
├── prebuilt 预构建动态库
├── src C++源代码目录
    ├── core 核心代码
    └── include 对外开放的头文件
├── syntaxes 示例语法规则配置文件
└── tests 单元测试目录
```

# 开发环境
## Visual Studio
1. 直接打开该工程即可

## Clion
1. 直接打开该工程即可

## Visual Studio Code
1. 如果是Windows系统，安装 [MinGW64](https://github.com/niXman/mingw-builds-binaries/releases) 和 [CMake](https://github.com/Kitware/CMake/releases)，需要注意的是Windows系统MinGW64要下载带`posix-seh-ucrt`字段的；如果是MacOS系统，安装 Xcode 和 cmake 即可
2. 打开该工程，安装C/C++ Extension Pack(包含CMake)
3. 按 Ctrl + F5 运行
   (如果无法运行，就使用CMake插件在IDE左侧添加的CMake选项运行)

## Android Studio
1. 打开 platform/Android 工程 即可

## IntelliJ IDEA / Java 22（FFM）
1. 打开 `platform/Java22` 工程。
2. 使用 JDK 22。
3. 使用 Gradle 任务构建/运行（例如：`:sweetline:build`、`:demo:run`）。

## .NET SDK / Visual Studio（C# WinForms）
1. 打开 `platform/CSharp/CSharp.sln`，或使用 `dotnet` CLI。
2. 构建封装库：`dotnet build platform/CSharp/SweetLine/SweetLine.csproj`。
3. 运行 Demo：`dotnet run --project platform/CSharp/Demo/Demo.csproj`。

# C++命名规范
## 文件命名
文件名使用全小写命名, 可以包含下划线(_) 或连字符(-)。举例：
```
component.cpp
plugin_api.h

1> C++ 文件要以 .cpp 结尾, 头文件以 .h 结尾. 专门插入文本的文件则以 .inc 结尾
2> 内联函数直接放在.h文件中
```
## 类型命名
【类, 结构体, 类型定义 (typedef), 枚举, 类型模板参数 均为类型】，以大写字母开始, 每个单词首字母均大写, 不包含下划线。
举例:
```
// 类和结构体
class UrlTable { ...
class UrlTableTester { ...
struct UrlTableProperties { ...

// 类型定义
typedef hash_map<UrlTableProperties *, string> PropertiesMap;

// using 别名
using PropertiesMap = hash_map<UrlTableProperties *, string>;

// 枚举
enum UrlTableErrors { ...
```
## 变量命名
变量 (包括函数参数) 和数据成员名一律小写, 单词之间用下划线连接. 类的私有成员变量以下划线结尾, 类的公开变量和结构体内部的变量的就不用。
### 普通变量命名
```
string table_name;  // 好 - 用下划线.
string tablename;   // 好 - 全小写.

string tableName;  // 差 - 混合大小写
```
### 类数据成员变量
```
class TableInfo {
...
public:
   string name_end;     // 好
   string name_start_;  // 坏，public变量不加_
private:
   string table_name_;  // 好 - 后加下划线.
   string tablename_;   // 好.
   static Pool<TableInfo>* pool_;  // 好.
};
```
### 结构体成员变量
```
struct UrlTableProperties {
   string name;
   int num_entries;
   static Pool<UrlTableProperties>* pool;
};
```
### 全局变量
对全局变量没有特别要求, 少用就好, 但如果要用, 可以用 g 或其它标志作为前缀, 以便更好的区分局部变量.
### 常量命名
声明为 constexpr 或 const 的变量, 或在程序运行期间其值始终保持不变的, 命名时以 “k” 开头, 大小写混合，举例：
```
const int kDaysInAWeek = 7;
```
## 函数命名
函数名的每个单词首字母小写 (即 “小驼峰变量名”)，没有下划线，举例：
```
addTableEntry()
deleteUrl()
openFileOrDie()
```
## 枚举命名
枚举的命名使用k+大驼峰命名或全大写命名。举例：
```
enum UrlTableErrors {
   kOk = 0,
   kErrorOutOfMemory,
   kErrorMalformedInput,
};
enum AlternateUrlTableErrors {
   OK = 0,
   OUT_OF_MEMORY = 1,
   MALFORMED_INPUT = 2,
};
```
## 宏命名
命名全部大写, 单词间使用下划线。举例：
```
#define ROUND(x) ...
#define PI_ROUNDED 3.0
```

# 使用 Skills 快速编写语法规则

如果您要新增或完善语法规则，建议直接使用仓库内的 [`skills/`](../../skills)，这样可以明显提高编写速度，并让产出更一致。

推荐组合：
- [`syntax-highlighting-authoring`](../../skills/syntax-highlighting-authoring/SKILL.md)
- [`sweetline-syntax-profile`](../../skills/sweetline-syntax-profile/SKILL.md)

它们主要会帮助约束这些内容：
- 先判断当前需求是已有语法增强、全新语法、DSL 拆分还是方言拆分
- 先检查是否需要文件名路由，而不只是看扩展名
- 保持 token 拆分足够细，而不是用过大的兜底规则
- 为语法补齐覆盖度更高的 example 文件
- 将新增或大幅修改的 example 控制在推荐的 `120~150` 行范围
- 补齐语法编译覆盖、analyzer 路由覆盖以及精确的高亮断言
- 当语法需要在 demo 中展示时，同步更新 demo 侧注册

推荐流程：
1. 先阅读通用的语法编写 skill。
2. 再阅读 SweetLine 专用 profile，了解仓库目录和约束。
3. 按照其中的路由、example 和验证要求完成修改后再提交。
