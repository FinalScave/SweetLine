# 项目结构
```
├── 3dparty 第三方C++库
├── docs 文档目录
├── platform 各平台C++接口桥接项目（Windows和Linux不需要）
    ├── Android Android平台SDK工程
    ├── Emscripten WASM SDK工程
    └── OHOS 鸿蒙 SDK工程
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

# 命名规范
参见[命名规范](docs/项目命名规范.md)

# 高亮引擎语法配置规则
参见[语法配置规则](docs/语法配置规则.md)