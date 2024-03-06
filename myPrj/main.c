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
#include "./src/cModule.h"
#include <string.h>
#include "runtime/rtInit.h"
#include "runtime/jsEval.h"

static void js_load_bin(JSContext* ctx);



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

    runJsScript(ctx, demoScript);

    js_load_array(ctx);

    eval_file(ctx, "../../JSBinFiles/helloworld.js", 0);

    // 运行bin文件
    load_binary_file(ctx, "../../JSBinFiles/bin.bin");


    js_std_loop(ctx);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 0;
}


