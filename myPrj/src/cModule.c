#include "quickjs-libc.h"
#include "cutils.h"

static JSValue js_my_adder(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv)
{
    int a = 0;
    int b = 0;

    if (JS_ToInt32(ctx, &a, argv[0]) || JS_ToInt32(ctx, &b, argv[1]))
    {
        printf("%s err param\n",__func__);
        return JS_UNDEFINED;
    }

    return JS_NewInt32(ctx, a + b);
}

static JSValue js_my_multi(JSContext *ctx, JSValueConst this_val,
                              int argc, JSValueConst *argv)
{
    int a = 0;
    int b = 0;

    if (JS_ToInt32(ctx, &a, argv[0]) || JS_ToInt32(ctx, &b, argv[1]))
    {
        printf("%s err param\n",__func__);
        return JS_UNDEFINED;
    }

    return JS_NewInt32(ctx, a * b);
}

static const JSCFunctionListEntry js_mmath_funcs[] = {
    JS_CFUNC_DEF("add", 2, js_my_adder),
    JS_CFUNC_DEF("multi", 2, js_my_multi),
};

static int js_mmath_init(JSContext *ctx, JSModuleDef *m)
{
    return JS_SetModuleExportList(ctx, m, js_mmath_funcs,
                                  countof(js_mmath_funcs));
}


JSModuleDef *js_init_module_mmath(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_mmath_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_mmath_funcs, countof(js_mmath_funcs));
    JS_AddModuleExport(ctx, m, "add");
    JS_AddModuleExport(ctx, m, "multi");
    return m;
}
