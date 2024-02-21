/***
 * 1. 支持字节码的加载、运行
 * 2. 支持.js文件的加载、运行
 * 3. 支持bin文件的生成、加载、运行
 *
 * TODO:
 * 1. .js call cModule
 * 2. c callback js func
 * 3. std loop
*/

#include "quickjs-libc.h"
#include <stdbool.h>
#include "cModule.h"
#include <string.h>

static void js_load_array(JSContext* ctx);
static void  js_load_jsFile(JSContext* ctx);
static void js_load_bin(JSContext* ctx);
static int eval_buf(JSContext *ctx, const void *buf, int buf_len,
                    const char *filename, int eval_flags);

static JSContext *JS_NewCustomContext(JSRuntime *rt)
{
    printf("======  start  ======\n");
    JSContext *ctx = JS_NewContextRaw(rt);
    if (!ctx)
        return NULL;
    JS_AddIntrinsicBaseObjects(ctx);
    JS_AddIntrinsicDate(ctx);
    JS_AddIntrinsicEval(ctx);
    JS_AddIntrinsicStringNormalize(ctx);
    JS_AddIntrinsicRegExp(ctx);
    JS_AddIntrinsicJSON(ctx);
    JS_AddIntrinsicProxy(ctx);
    JS_AddIntrinsicMapSet(ctx);
    JS_AddIntrinsicTypedArrays(ctx);
    JS_AddIntrinsicPromise(ctx);
    JS_AddIntrinsicBigInt(ctx);

    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");
    js_init_module_mmath(ctx, "mmath");

    const char *str = "import * as std from 'std';\n"
                "import * as os from 'os';\n"
                "import * as mmath from 'mmath';\n"
                "globalThis.std = std;\n"
                "globalThis.os = os;\n"
                "globalThis.mmath = mmath;\n";
    eval_buf(ctx, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);
    return ctx;
}

int main(int argc, char **argv)
{
    JSRuntime *rt;
    JSContext *ctx;
    rt = JS_NewRuntime();
    js_std_set_worker_new_context_func(JS_NewCustomContext);
    js_std_init_handlers(rt);
    JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);
    ctx = JS_NewCustomContext(rt);
    js_std_add_helpers(ctx, argc, argv);


char script[] = R"(
    import * as std from "std"
    import * as mmath from "mmath"

    std.puts("\x1b[J");
    console.log("====  generate binary  ====");
    console.log("Hello World");
    console.log("4 + 5 = ", mmath.add(4,5))
  )";


  JS_Eval(ctx, script, strlen(script), "<test>", JS_EVAL_TYPE_MODULE);

    // qjsc -c 字节码
    printf("\n>>>>  qjsc 字节码  <<<<\n");
    js_load_array(ctx);

    // 运行.js文件
    printf("\n>>>>  qjsc .js  <<<<\n");
    js_load_jsFile(ctx);

    // 运行bin文件
    printf("\n>>>>  qjsc .bin  <<<<\n");
    js_load_bin(ctx);


    js_std_loop(ctx);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 0;
}

/***
 *  qjsc -c 生成的binary数组 加载
 *  step 1:qjsc -c myPrj/JSBinFiles/helloworld.js -o myPrj/JSBinFiles/bin.c
 *  step 2:js_std_eval_binary
 */
static void js_load_array(JSContext* ctx)
{
    const uint32_t qjsc_fib_module_size = 138;

    const uint8_t qjsc_fib_module[138] = {
    0x02, 0x03, 0x32, 0x6d, 0x79, 0x50, 0x72, 0x6a,
    0x2f, 0x6a, 0x73, 0x53, 0x72, 0x63, 0x2f, 0x66,
    0x69, 0x62, 0x5f, 0x6d, 0x6f, 0x64, 0x75, 0x6c,
    0x65, 0x2e, 0x6a, 0x73, 0x06, 0x66, 0x69, 0x62,
    0x02, 0x6e, 0x0f, 0xc2, 0x03, 0x00, 0x01, 0x00,
    0x00, 0xc4, 0x03, 0x00, 0x00, 0x0e, 0x00, 0x06,
    0x01, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x08, 0x00, 0xc4, 0x03, 0x00, 0x01, 0x08,
    0xea, 0x05, 0xc0, 0x00, 0xe1, 0x29, 0x29, 0xc2,
    0x03, 0x01, 0x04, 0x01, 0x00, 0x07, 0x14, 0x0e,
    0x43, 0x06, 0x01, 0xc4, 0x03, 0x01, 0x00, 0x01,
    0x04, 0x01, 0x00, 0x1a, 0x01, 0xc6, 0x03, 0x00,
    0x01, 0x00, 0xc4, 0x03, 0x00, 0x00, 0xd1, 0xb5,
    0xa4, 0xea, 0x03, 0xb5, 0x28, 0xd1, 0xb6, 0xa9,
    0xea, 0x03, 0xb6, 0x28, 0xdd, 0xd1, 0xb6, 0x9e,
    0xef, 0xdd, 0xd1, 0xb7, 0x9e, 0xef, 0x9d, 0x28,
    0xc2, 0x03, 0x02, 0x06, 0x04, 0x1c, 0x08, 0x21,
    0x08, 0x08,
    };

    const uint32_t qjsc_hello_size = 164;

    const uint8_t qjsc_hello[164] = {
    0x02, 0x07, 0x28, 0x6d, 0x79, 0x50, 0x72, 0x6a,
    0x2f, 0x6a, 0x73, 0x53, 0x72, 0x63, 0x2f, 0x68,
    0x65, 0x6c, 0x6c, 0x6f, 0x2e, 0x6a, 0x73, 0x1e,
    0x2e, 0x2f, 0x66, 0x69, 0x62, 0x5f, 0x6d, 0x6f,
    0x64, 0x75, 0x6c, 0x65, 0x2e, 0x6a, 0x73, 0x06,
    0x66, 0x69, 0x62, 0x0e, 0x63, 0x6f, 0x6e, 0x73,
    0x6f, 0x6c, 0x65, 0x06, 0x6c, 0x6f, 0x67, 0x16,
    0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f,
    0x72, 0x6c, 0x64, 0x10, 0x66, 0x69, 0x62, 0x28,
    0x31, 0x30, 0x29, 0x3d, 0x0f, 0xc2, 0x03, 0x01,
    0xc4, 0x03, 0x00, 0x00, 0x01, 0x00, 0xc6, 0x03,
    0x00, 0x0e, 0x00, 0x06, 0x01, 0xa0, 0x01, 0x00,
    0x00, 0x00, 0x05, 0x01, 0x00, 0x30, 0x00, 0xc6,
    0x03, 0x00, 0x0c, 0x08, 0xea, 0x02, 0x29, 0x38,
    0xe4, 0x00, 0x00, 0x00, 0x42, 0xe5, 0x00, 0x00,
    0x00, 0x04, 0xe6, 0x00, 0x00, 0x00, 0x24, 0x01,
    0x00, 0x0e, 0x38, 0xe4, 0x00, 0x00, 0x00, 0x42,
    0xe5, 0x00, 0x00, 0x00, 0x04, 0xe7, 0x00, 0x00,
    0x00, 0x65, 0x00, 0x00, 0xbd, 0x0a, 0xef, 0x24,
    0x02, 0x00, 0x29, 0xc2, 0x03, 0x01, 0x05, 0x01,
    0x00, 0x04, 0x0a, 0x62,
    };

    js_std_eval_binary(ctx, qjsc_fib_module, qjsc_fib_module_size, 0);
    js_std_eval_binary(ctx, qjsc_hello, qjsc_hello_size, 0);
}

/***
 *  qjsc -c 生成的binary数组 加载
 *  step 1:qjsc -c myPrj/JSBinFiles/helloworld.js -o myPrj/JSBinFiles/bin.c
 *  step 2:js_std_eval_binary
 */
static int eval_buf(JSContext *ctx, const void *buf, int buf_len,
                    const char *filename, int eval_flags)
{
    JSValue val;
    int ret;

    if ((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
        /* for the modules, we compile then run to be able to set
           import.meta */
        val = JS_Eval(ctx, buf, buf_len, filename,
                      eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
        if (!JS_IsException(val)) {
            js_module_set_import_meta(ctx, val, true, true);
            val = JS_EvalFunction(ctx, val);
        }
    } else {
        val = JS_Eval(ctx, buf, buf_len, filename, eval_flags);
    }
    if (JS_IsException(val)) {
        js_std_dump_error(ctx);
        ret = -1;
    } else {
        ret = 0;
    }
    JS_FreeValue(ctx, val);
    return ret;
}

static int eval_file(JSContext *ctx, const char *filename, int module)
{
    uint8_t *buf;
    int ret, eval_flags;
    size_t buf_len;

    buf = js_load_file(ctx, &buf_len, filename);
    if (!buf) {
        perror(filename);
        exit(1);
    }

    if (module)
        eval_flags = JS_EVAL_TYPE_MODULE;
    else
        eval_flags = JS_EVAL_TYPE_GLOBAL;
    ret = eval_buf(ctx, buf, buf_len, filename, eval_flags);
    js_free(ctx, buf);
    return ret;
}

static void js_load_jsFile(JSContext* ctx)
{
    printf("%s\n", __func__);
    eval_file(ctx, "../../JSBinFiles/helloworld.js", 0);
}


/***
 *  qjsc -c 生成的binary数组 array2bin 将数组转换为bin文件
 *  step 1:qjsc -c myPrj/JSBinFiles/helloworld.js -o myPrj/JSBinFiles/bin.c
 *  step 2:array2bin
 *  step 3:load_binary_file
 */
int load_binary_file(JSContext *ctx, const char *filename)
{
    uint8_t *buf;
    size_t buf_len;

    if(filename == NULL)
    {
        return -1;
    }

    buf = js_load_file(ctx, &buf_len, filename); //js_load_file是用 osMalloc申请的内存,避免文件size过大推高js threshhold
    if (buf == NULL) {
        return -1;
    }

    js_std_eval_binary(ctx, buf, buf_len, 0);

    free(buf);

    return 0;
}

static void js_load_bin(JSContext* ctx)
{
    printf("%s\n", __func__);
    load_binary_file(ctx, "../../JSBinFiles/bin.bin");
}