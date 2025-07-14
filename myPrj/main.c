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

static void dumpMemory(JSRuntime *rt)
{
    JSMemoryUsage stats;
    JS_ComputeMemoryUsage(rt, &stats);
    printf("%s malloc_size %ld  memory_used_size %ld  obj_count %ld  obj_size %ld\n",
         __func__, stats.malloc_size, stats.memory_used_size, stats.obj_count, stats.obj_size);
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
    dumpMemory(rt);
    my_class_create(ctx);

    runJsScript(ctx, demoScript);  // 执行 JS 片段

    js_load_array(ctx);  // 执行字节码

    eval_file(ctx, "../../JSBinFiles/helloworld.js", JS_EVAL_TYPE_MODULE);
    JS_RunGC(rt);
    eval_file(ctx, "../../JSBinFiles/cCallbackTest.js", JS_EVAL_TYPE_MODULE);
    freeButtons(ctx);

    JS_RunGC(rt);

    // 运行bin文件
    printf("\n%s  >>>>\n", "run bin文件");
    load_binary_file(ctx, "../../JSBinFiles/bin.bin");

    printf("\nfree context & runtime\n");
    js_std_loop(ctx);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    printf("\n");
    return 0;
}


