#ifndef SWEETLINE_C_API_H
#define SWEETLINE_C_API_H
#include <cstddef>
#include <cstdint>

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

/// 创建引擎托管文档(Document)
/// @param uri 文档Uri
/// @param text 文档内容
/// @return 托管文档句柄
SL_API intptr_t sl_create_document(const char* uri, const char* text);

/// 销毁托管文档
/// @param document_handle 托管文档句柄
/// @return 返回错误码，错误码详见 @see {sl_error_t}，销毁成功时返回 @see {SL_OK}
SL_API sl_error_t sl_free_document(intptr_t document_handle);

/// 创建SweetLine高亮引擎
/// @param show_index 高亮分析结果是否呈现字符索引(index)，如果不呈现则只返回line和column
/// @return 高亮引擎句柄
SL_API intptr_t sl_create_engine(bool show_index);

/// 销毁高亮引擎
/// @param engine_handle 高亮引擎句柄
/// @return 返回错误码，销毁成功时返回 @see {SL_OK}
SL_API sl_error_t sl_free_engine(intptr_t engine_handle);

/// 编译高亮规则(直接根据json配置内容编译)
/// @param engine_handle 高亮引擎句柄
/// @param syntax_json 语法规则json配置
/// @return 返回编译语法规则错误信息(如果有错误)
SL_API sl_syntax_error_t sl_engine_compile_json(intptr_t engine_handle, const char* syntax_json);

/// 编译高亮规则(读取文本json配置文件内容编译)
/// @param engine_handle 高亮引擎句柄
/// @param syntax_file 语法规则json文件路径
/// @return 返回编译语法规则错误信息(如果有错误)
SL_API sl_syntax_error_t sl_engine_compile_file(intptr_t engine_handle, const char* syntax_file);

/// 注册样式名称，高亮规则中的样式名称最终会映射到注册的样式ID
/// @param engine_handle 高亮引擎句柄
/// @param style_name 高亮样式名称
/// @param style_id 高亮样式ID
/// @return 返回错误码，注册成功时返回 @see {SL_OK}
SL_API sl_error_t sl_engine_register_style_name(intptr_t engine_handle, const char* style_name, int32_t style_id);

/// 通过样式ID获取样式名称
/// @param engine_handle 高亮引擎句柄
/// @param style_id 高亮样式ID
/// @return 返回对应ID在引擎中注册的高亮样式名称
SL_API const char* sl_engine_get_style_name(intptr_t engine_handle, int32_t style_id);

/// 根域语法规则名称创建纯文本高亮分析器(不支持增量分析)
/// @param engine_handle 高亮引擎句柄
/// @param syntax_name 语法规则名称
/// @return 纯文本高亮分析器句柄
SL_API intptr_t sl_engine_create_text_analyzer(intptr_t engine_handle, const char* syntax_name);

/// 根域文件后缀名创建纯文本高亮分析器(不支持增量分析)
/// @param engine_handle 高亮引擎句柄
/// @param extension 文件后缀名
/// @return 纯文本高亮分析器句柄
SL_API intptr_t sl_engine_create_text_analyzer2(intptr_t engine_handle, const char* extension);

/// 对整段文本进行全量高亮分析
/// @param analyzer_handle 纯文本高亮分析器句柄
/// @param text 整段文本内容
/// @param data_size 分析结果数组的大小，分析完毕后设置给该参数
/// @return 分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// [高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]...[高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]
/// 其中每个字段都为4字节整数
/// @endcode
SL_API int32_t* sl_text_analyze(intptr_t analyzer_handle, const char* text, int32_t* data_size);

/// 对单行文本进行高亮分析并获得返回的单行分析结果
/// @param analyzer_handle 纯文本高亮分析器句柄
/// @param text 单文本内容
/// @param line_info 当前行的元数据信息，必须指定为长度为3的 int32_t 数组，其中每个元素规定如下:
/// @code
/// line_info[0] = 当前行号
/// line_info[1] = 当前起始高亮状态，一般由上一行分析结果的 end_state 得到，第 0 行状态为 0
/// line_info[2] = 当前行累计字符总数，不包含换行符
/// @endcode
/// @param data_size 分析结果数组的大小，分析完毕后设置给该参数
/// @return 分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// result[0] = 高亮块数量
/// [高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]...[高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]
/// 其中每个字段都为4字节整数
/// @endcode
SL_API int32_t* sl_text_analyze_line(intptr_t analyzer_handle, const char* text, int32_t* line_info, int32_t* data_size);

/// 加载托管文档并获得文档高亮分析器句柄(支持增量分析)
/// @param engine_handle 高亮引擎句柄
/// @param document_handle 托管文档句柄
/// @return 文档高亮分析器句柄
SL_API intptr_t sl_engine_load_document(intptr_t engine_handle, intptr_t document_handle);

/// 对托管文档进行全量高亮分析(一般首次加载文档后调用一次)
/// @param analyzer_handle 文档高亮分析器句柄
/// @param data_size 分析结果数组的大小，分析完毕后设置给该参数
/// @return 分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// [高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]...[高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]
/// 其中每个字段都为4字节整数
/// @endcode
SL_API int32_t* sl_document_analyze(intptr_t analyzer_handle, int32_t* data_size);

/// 对托管文档进行增量高亮分析(文档发生变化时调用)
/// @param analyzer_handle 文档高亮分析器句柄
/// @param changes_range 变更区域，数组排列结构: [变更起始行],[变更起始列],[变更结束行],[变更结束列]
/// @param new_text 变更后文本
/// @param data_size 分析结果数组的大小，分析完毕后设置给该参数
/// @return 整个文档完整的分析结果，按字节顺序紧密排列，结构如下:
/// @code
/// [高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]...[高亮起始行],[高亮起始列],[高亮结束行],[高亮结束列],[高亮样式ID]
/// 其中每个字段都为4字节整数
/// @endcode
SL_API int32_t* sl_document_analyze_incremental(intptr_t analyzer_handle, size_t* changes_range, const char* new_text, int32_t* data_size);

#ifdef __cplusplus
}
#endif

#endif //SWEETLINE_C_API_H
