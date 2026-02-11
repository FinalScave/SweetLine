#ifndef SWEETLINE_C_API_H
#define SWEETLINE_C_API_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64) || defined(WINDOWS)
  #ifdef SWEETLINE_EXPORT
    #define SL_API __declspec(dllexport)
  #else
    #define SL_API __declspec(dllimport)
  #endif
#else
  #define SL_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// 错误码
typedef enum sl_error {
  SL_OK = 0, // 没有错误
  SL_HANDLE_INVALID = 1, // 句柄不合法
  SL_JSON_PROPERTY_MISSED = -1, // 语法规则json缺少属性
  SL_JSON_PROPERTY_INVALID = -2, // 语法规则json属性值错误
  SL_PATTERN_INVALID = -3, // 语法规则json中正则表达式错误
  SL_STATE_INVALID = -4, // 语法规则json中状态错误
  SL_JSON_INVALID = -5, // 语法规则json错误
  SL_FILE_IO_ERR = -6, // 文件IO错误
  SL_FILE_EMPTY = -7, // 文件内容为空
} sl_error_t;

/// 语法规则错误信息
typedef struct sl_syntax_error {
  /// 错误码
  sl_error_t err_code;
  /// 错误信息
  const char* err_msg;
} sl_syntax_error_t;

/// 托管文档句柄
typedef struct sl_document_handle sl_document_handle;
typedef sl_document_handle* sl_document_handle_t;
/// 高亮引擎句柄
typedef struct sl_engine_handle sl_engine_handle;
typedef sl_engine_handle* sl_engine_handle_t;
/// 高亮分析器句柄
typedef struct sl_analyzer_handle sl_analyzer_handle;
typedef sl_analyzer_handle* sl_analyzer_handle_t;

/// 创建引擎托管文档(Document)
/// @param uri 文档Uri
/// @param text 文档内容
/// @return 托管文档句柄
SL_API sl_document_handle_t sl_create_document(const char* uri, const char* text);

/// 销毁托管文档
/// @param document_handle 托管文档句柄
/// @return 返回错误码，错误码详见 @see {sl_error_t}，销毁成功时返回 @see {SL_OK}
SL_API sl_error_t sl_free_document(sl_document_handle_t document_handle);

/// 创建SweetLine高亮引擎
/// @param show_index 高亮分析结果是否呈现字符索引(index)，如果不呈现则只返回line和column
/// @param inline_style 高亮分析结果是否使用内联样式，而不是只返回样式ID
/// @return 高亮引擎句柄
SL_API sl_engine_handle_t sl_create_engine(bool show_index, bool inline_style);

/// 销毁高亮引擎
/// @param engine_handle 高亮引擎句柄
/// @return 返回错误码，销毁成功时返回 @see {SL_OK}
SL_API sl_error_t sl_free_engine(sl_engine_handle_t engine_handle);

/// 定义一个宏，用于控制importSyntax的#ifdef条件编译
/// @param engine_handle 高亮引擎句柄
/// @param macro_name 宏名称
/// @return 返回错误码，成功时返回 @see {SL_OK}
SL_API sl_error_t sl_engine_define_macro(sl_engine_handle_t engine_handle, const char* macro_name);

/// 取消定义一个宏
/// @param engine_handle 高亮引擎句柄
/// @param macro_name 宏名称
/// @return 返回错误码，成功时返回 @see {SL_OK}
SL_API sl_error_t sl_engine_undefine_macro(sl_engine_handle_t engine_handle, const char* macro_name);

/// 编译高亮规则(直接根据json配置内容编译)
/// @param engine_handle 高亮引擎句柄
/// @param syntax_json 语法规则json配置
/// @return 返回编译语法规则错误信息(如果有错误)
SL_API sl_syntax_error_t sl_engine_compile_json(sl_engine_handle_t engine_handle, const char* syntax_json);

/// 编译高亮规则(读取文本json配置文件内容编译)
/// @param engine_handle 高亮引擎句柄
/// @param syntax_file 语法规则json文件路径
/// @return 返回编译语法规则错误信息(如果有错误)
SL_API sl_syntax_error_t sl_engine_compile_file(sl_engine_handle_t engine_handle, const char* syntax_file);

/// 注册样式名称，高亮规则中的样式名称最终会映射到注册的样式ID
/// @param engine_handle 高亮引擎句柄
/// @param style_name 高亮样式名称
/// @param style_id 高亮样式ID
/// @return 返回错误码，注册成功时返回 @see {SL_OK}
SL_API sl_error_t sl_engine_register_style_name(sl_engine_handle_t engine_handle, const char* style_name, int32_t style_id);

/// 通过样式ID获取样式名称
/// @param engine_handle 高亮引擎句柄
/// @param style_id 高亮样式ID
/// @return 返回对应ID在引擎中注册的高亮样式名称
SL_API const char* sl_engine_get_style_name(sl_engine_handle_t engine_handle, int32_t style_id);

/// 根域语法规则名称创建纯文本高亮分析器(不支持增量分析)
/// @param engine_handle 高亮引擎句柄
/// @param syntax_name 语法规则名称
/// @return 纯文本高亮分析器句柄
SL_API sl_analyzer_handle_t sl_engine_create_text_analyzer(sl_engine_handle_t engine_handle, const char* syntax_name);

/// 根域文件后缀名创建纯文本高亮分析器(不支持增量分析)
/// @param engine_handle 高亮引擎句柄
/// @param extension 文件后缀名
/// @return 纯文本高亮分析器句柄
SL_API sl_analyzer_handle_t sl_engine_create_text_analyzer2(sl_engine_handle_t engine_handle, const char* extension);

/// 对整段文本进行全量高亮分析
/// @param analyzer_handle 纯文本高亮分析器句柄
/// @param text 整段文本内容
/// @return 分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// result[0] = 高亮块数量
/// result[1] = 每个高亮块包含的整数字段数量
/// 后续数据包含 result[0] * result[1] 个整数字段，可按 result[0] 数量for循环读取高亮结果，后续字段排列示例：
/// 如果开启了内联样式支持(result[1] = 9), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,foregroundColor,backgroundColor,fontAttributes]
/// ...
/// 其中最后一个字段 fontAttributes 为字体显示样式，目前有加粗、斜体、显示删除线三种样式，获取方式：
/// if ((fontAttributes & 1) != 0) => 加粗
/// if ((fontAttributes & (1 << 1)) != 0) => 斜体
/// if ((fontAttributes & (1 << 2)) != 0) => 显示删除线
/// 如果没有开启内联样式支持(result[1] = 7), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,styleId]
/// ...
/// 自行根据 styleId 进行样式配置
/// @endcode
/// 需要注意的是，返回值使用完毕之后需要调用 sl_free_buffer 进行释放
SL_API int32_t* sl_text_analyze(sl_analyzer_handle_t analyzer_handle, const char* text);

/// 对单行文本进行高亮分析并获得返回的单行分析结果
/// @param analyzer_handle 纯文本高亮分析器句柄
/// @param text 单文本内容
/// @param line_info 当前行的元数据信息，必须指定为长度为3的 int32_t 数组，其中每个元素规定如下:
/// @code
/// line_info[0] = 当前行号
/// line_info[1] = 当前起始高亮状态，一般由上一行分析结果的 end_state 得到，第 0 行状态为 0
/// line_info[2] = 当前行累计字符总数，不包含换行符
/// @endcode
/// @return 分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// result[0] = 高亮块数量
/// result[1] = 每个高亮块包含的整数字段数量
/// result[2] = 当前行分析结束时的状态ID
/// result[3] = 当前行分析的字符总数(不包含换行符)
/// 后续数据包含 result[0] * result[1] 个整数字段，可按 result[0] 数量for循环读取高亮结果，后续字段排列示例：
/// 如果开启了内联样式支持(result[1] = 9), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,foregroundColor,backgroundColor,fontAttributes]
/// ...
/// 其中最后一个字段 fontAttributes 为字体显示样式，目前有加粗、斜体、显示删除线三种样式，获取方式：
/// if ((fontAttributes & 1) != 0) => 加粗
/// if ((fontAttributes & (1 << 1)) != 0) => 斜体
/// if ((fontAttributes & (1 << 2)) != 0) => 显示删除线
/// 如果没有开启内联样式支持(result[1] = 7), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,styleId]
/// ...
/// 自行根据 styleId 进行样式配置
/// @endcode
/// 需要注意的是，返回值使用完毕之后需要调用 sl_free_buffer 进行释放
SL_API int32_t* sl_text_analyze_line(sl_analyzer_handle_t analyzer_handle, const char* text, int32_t* line_info);

/// 加载托管文档并获得文档高亮分析器句柄(支持增量分析)
/// @param engine_handle 高亮引擎句柄
/// @param document_handle 托管文档句柄
/// @return 文档高亮分析器句柄
SL_API sl_analyzer_handle_t sl_engine_load_document(sl_engine_handle_t engine_handle, sl_document_handle_t document_handle);

/// 对托管文档进行全量高亮分析(一般首次加载文档后调用一次)
/// @param analyzer_handle 文档高亮分析器句柄
/// @return 分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// result[0] = 高亮块数量
/// result[1] = 每个高亮块包含的整数字段数量
/// 后续数据包含 result[0] * result[1] 个整数字段，可按 result[0] 数量for循环读取高亮结果，后续字段排列示例：
/// 如果开启了内联样式支持(result[1] = 9), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,foregroundColor,backgroundColor,fontAttributes]
/// ...
/// 其中最后一个字段 fontAttributes 为字体显示样式，目前有加粗、斜体、显示删除线三种样式，获取方式：
/// if ((fontAttributes & 1) != 0) => 加粗
/// if ((fontAttributes & (1 << 1)) != 0) => 斜体
/// if ((fontAttributes & (1 << 2)) != 0) => 显示删除线
/// 如果没有开启内联样式支持(result[1] = 7), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,styleId]
/// ...
/// 自行根据 styleId 进行样式配置
/// @endcode
/// 需要注意的是，返回值使用完毕之后需要调用 sl_free_buffer 进行释放
SL_API int32_t* sl_document_analyze(sl_analyzer_handle_t analyzer_handle);

/// 对托管文档进行增量高亮分析(文档发生变化时调用)
/// @param analyzer_handle 文档高亮分析器句柄
/// @param changes_range 变更区域，数组排列结构: [startLine],[startColumn],[endLine],[endColumn]
/// @param new_text 变更后文本
/// @return 整个文档完整的分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// result[0] = 高亮块数量
/// result[1] = 每个高亮块包含的整数字段数量
/// 后续数据包含 result[0] * result[1] 个整数字段，可按 result[0] 数量for循环读取高亮结果，后续字段排列示例：
/// 如果开启了内联样式支持(result[1] = 9), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,foregroundColor,backgroundColor,fontAttributes]
/// ...
/// 其中最后一个字段 fontAttributes 为字体显示样式，目前有加粗、斜体、显示删除线三种样式，获取方式：
/// if ((fontAttributes & 1) != 0) => 加粗
/// if ((fontAttributes & (1 << 1)) != 0) => 斜体
/// if ((fontAttributes & (1 << 2)) != 0) => 显示删除线
/// 如果没有开启内联样式支持(result[1] = 7), 结构如下:
/// [startLine,startColumn,startIndex,endLine,endColumn,endIndex,styleId]
/// ...
/// 自行根据 styleId 进行样式配置
/// @endcode
/// 需要注意的是，返回值使用完毕之后需要调用 sl_free_buffer 进行释放
SL_API int32_t* sl_document_analyze_incremental(sl_analyzer_handle_t analyzer_handle, int32_t* changes_range, const char* new_text);

/// 释放分析结果的内存，所有返回 int32_t* 的分析函数
/// sl_text_analyze、sl_text_analyze_line、sl_document_analyze、sl_document_analyze_incremental 的返回值都必须通过此函数释放
/// @param result 高亮分析结果
SL_API void sl_free_buffer(int32_t* result);
#ifdef __cplusplus
}
#endif

#endif //SWEETLINE_C_API_H
