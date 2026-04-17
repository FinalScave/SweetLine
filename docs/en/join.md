# Project Structure
```
├── 3dparty        Third-party C++ libraries
├── docs           Documentation
├── platform       Platform-specific wrappers/bindings
    ├── Android    Android JNI SDK project
    ├── Java22     Java 22 FFM wrapper and Swing demo
    ├── CSharp     .NET/C# P/Invoke wrapper and WinForms demo
    ├── Emscripten WASM SDK project
    └── OHOS       HarmonyOS NAPI SDK project
├── prebuilt       Prebuilt shared libs
├── src            C++ source code
    ├── core       Core implementation
    └── include    Public header files
├── syntaxes       Example syntax rule configuration files
└── tests          Unit test directory
```

# Development Environment
## Visual Studio
1. Open the project directly.

## CLion
1. Open the project directly.

## Visual Studio Code
1. On Windows, install [MinGW64](https://github.com/niXman/mingw-builds-binaries/releases) and [CMake](https://github.com/Kitware/CMake/releases). Note: on Windows, download the MinGW64 version with the `posix-seh-ucrt` identifier. On macOS, install Xcode and cmake.
2. Open the project and install the C/C++ Extension Pack (includes CMake).
3. Press Ctrl + F5 to run.
   (If it fails to run, use the CMake plugin options on the left sidebar of the IDE.)

## Android Studio
1. Open the `platform/Android` project.

## IntelliJ IDEA / Java 22 (FFM)
1. Open the `platform/Java22` project.
2. Use JDK 22.
3. Build and run with Gradle tasks (for example: `:sweetline:build`, `:demo:run`).

## .NET SDK / Visual Studio (C# WinForms)
1. Open `platform/CSharp/CSharp.sln` or use the `dotnet` CLI.
2. Build wrapper library: `dotnet build platform/CSharp/SweetLine/SweetLine.csproj`.
3. Run demo app: `dotnet run --project platform/CSharp/Demo/Demo.csproj`.

# C++ Naming Conventions
## File Naming
File names use all lowercase, and may include underscores (`_`) or hyphens (`-`). Examples:
```
component.cpp
plugin_api.h

1> C++ files end with .cpp, headers end with .h. Files for text insertion end with .inc
2> Inline functions go directly in .h files
```

## Type Naming
Types (classes, structs, typedefs, enums, type template parameters) start with an uppercase letter, with each word capitalized, no underscores. Examples:
```
// Classes and structs
class UrlTable { ...
class UrlTableTester { ...
struct UrlTableProperties { ...

// Typedefs
typedef hash_map<UrlTableProperties *, string> PropertiesMap;

// Using aliases
using PropertiesMap = hash_map<UrlTableProperties *, string>;

// Enums
enum UrlTableErrors { ...
```

## Variable Naming
Variables (including function parameters) and data members are all lowercase with underscores between words. Private class members end with an underscore; public members and struct members do not.

### Regular Variables
```
string table_name;  // Good - uses underscores.
string tablename;   // Good - all lowercase.

string tableName;   // Bad - mixed case.
```

### Class Data Members
```
class TableInfo {
...
public:
   string name_end;     // Good
   string name_start_;  // Bad - public variables don't use trailing underscore
private:
   string table_name_;  // Good - trailing underscore.
   string tablename_;   // Good.
   static Pool<TableInfo>* pool_;  // Good.
};
```

### Struct Members
```
struct UrlTableProperties {
   string name;
   int num_entries;
   static Pool<UrlTableProperties>* pool;
};
```

### Global Variables
No special requirements for global variables — use them sparingly. If needed, prefix with `g` or another marker to distinguish from local variables.

### Constants
Variables declared as `constexpr` or `const`, or those whose values remain constant throughout program execution, are named with a leading "k" in mixed case. Example:
```
const int kDaysInAWeek = 7;
```

## Function Naming
Function names use lowerCamelCase (first letter of each word capitalized except the first), no underscores. Examples:
```
addTableEntry()
deleteUrl()
openFileOrDie()
```

## Enum Naming
Enum values use either k + UpperCamelCase or ALL_CAPS naming. Examples:
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

## Macro Naming
All uppercase with underscores between words. Examples:
```
#define ROUND(x) ...
#define PI_ROUNDED 3.0
```

# Using Skills for Syntax Work

If you are adding or refining syntax rules, you can use the repository skills in [`skills/`](../../skills) to speed up the work and keep the output consistent.

Recommended entry:
- [`sweetline-syntax-profile`](../../skills/sweetline-syntax-profile/SKILL.md)

What it helps enforce:
- deciding whether the task is a refinement, a new syntax, a DSL split, or a dialect split
- deciding whether the syntax needs exact file names, file suffixes, or file-name patterns
- keeping tokenization fine-grained instead of using overly broad fallback rules
- expanding or creating example files with realistic syntax coverage
- keeping new or heavily revised examples in the recommended `120` to `150` line range
- adding syntax compilation coverage, analyzer-routing coverage, and focused highlight assertions
- updating demo-side syntax registration when the syntax should appear in demos

Recommended workflow:
1. Read the general authoring skill first.
2. Read the SweetLine profile for repository-specific paths and constraints.
3. Follow the routing, example, and validation requirements before submitting changes.
