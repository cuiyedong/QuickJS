#ifndef _JSEVAL_H
#define _JSEVAL_H

#include "quickjs-libc.h"
#include <stdbool.h>
#include <string.h>

int eval_buf(JSContext *ctx, const void *buf, int buf_len,
                    const char *filename, int eval_flags);

int eval_file(JSContext *ctx, const char *filename, int module);

/**
 * @brief quickJS C 执行 JS 片段
 */
extern char demoScript[];
JSValue runJsScript(JSContext *ctx, const char *script);

/***
 *  qjsc -c 生成的binary数组 加载
 *  step 1:qjsc -c myPrj/JSBinFiles/helloworld.js -o myPrj/JSBinFiles/bin.c
 *  step 2:js_std_eval_binary
 */
void js_load_array(JSContext* ctx);

/***
 *  qjsc -c 生成的binary数组 array2bin 将数组转换为bin文件
 *  step 1:qjsc -c myPrj/JSBinFiles/helloworld.js -o myPrj/JSBinFiles/bin.c
 *  step 2:array2bin
 *  step 3:load_binary_file
 */
int load_binary_file(JSContext *ctx, const char *filename);

#endif
