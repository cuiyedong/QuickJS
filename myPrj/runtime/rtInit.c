#include "quickjs-libc.h"
#include "../src/cModule.h"
#include "rtInit.h"
#include "jsEval.h"

JSContext *JS_NewCustomContext(JSRuntime *rt)
{
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
    js_init_module_gui(ctx, "gui");

    const char *str = "import * as std from 'std';\n"
                "import * as os from 'os';\n"
                "import * as mmath from 'mmath';\n"
                "import * as gui from 'gui';\n"
                "globalThis.std = std;\n"
                "globalThis.os = os;\n"
                "globalThis.mmath = mmath;\n"
                "globalThis.gui = gui;\n";
    eval_buf(ctx, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);
    return ctx;
}
