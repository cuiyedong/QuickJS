/**
 * define => set => add
 */

#include "quickjs-libc.h"
#include "cutils.h"
#include "quickjs.h"
#include <math.h>

#define countof(x) (sizeof(x) / sizeof((x)[0]))

/**
 *  ============================================================
 *                      module part
 *  ============================================================
 */
static JSValue js_my_adder(JSContext *ctx, JSValueConst this_val,
                           int argc, JSValueConst *argv)
{
    int a = 0;
    int b = 0;

    if (JS_ToInt32(ctx, &a, argv[0]) || JS_ToInt32(ctx, &b, argv[1]))
    {
        printf("%s err param\n", __func__);
        return JS_UNDEFINED;
    }

    return JS_NewInt32(ctx, a + b);
}

static const JSCFunctionListEntry js_mmath_funcs[] = {
    // part1: function 定义
    JS_CFUNC_DEF("add", 2, js_my_adder),

    // part2: prop 定义, mmath.pai, const R
    JS_PROP_DOUBLE_DEF("pai", 3.14, JS_PROP_CONFIGURABLE),

    // part3: object 定义
};

static int js_mmath_init(JSContext *ctx, JSModuleDef *m)
{
    JS_SetModuleExportList(ctx, m, js_mmath_funcs, countof(js_mmath_funcs));
    return 0;
}

JSModuleDef *js_init_module_mmath(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_mmath_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_mmath_funcs, countof(js_mmath_funcs));

    return m;
}

/**
 * ============================================================
 *                      class part
 * ============================================================
 */

// 不透明指针，绑在object上，js和c能够共享访问
typedef struct
{
    int x;
    int y;
} JSPointData;

// 类的定义,类的唯一ID
static JSClassID js_point_class_id;

// C 侧定义类的方法实现 js_point_get_xy
static JSValue js_point_get_xy(JSContext *ctx, JSValueConst this_val, int magic)
{
    JSPointData *s = JS_GetOpaque2(ctx, this_val, js_point_class_id);
    if (!s)
        return JS_EXCEPTION;
    if (magic == 0)
        return JS_NewInt32(ctx, s->x);
    else
        return JS_NewInt32(ctx, s->y);
}

// C 侧定义类的方法实现 js_point_set_xy  magic 用于多个JS方法，C侧同一套实现逻辑，避免重复编写
static JSValue js_point_set_xy(JSContext *ctx, JSValueConst this_val, JSValue val, int magic)
{
    JSPointData *s = JS_GetOpaque2(ctx, this_val, js_point_class_id);
    int v;
    if (!s)
        return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &v, val))
        return JS_EXCEPTION;
    if (magic == 0)
        s->x = v;
    else
        s->y = v;
    return JS_UNDEFINED;
}

// C 侧定义类的方法实现 js_point_norm
static JSValue js_point_norm(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv)
{
    JSPointData *s = JS_GetOpaque2(ctx, this_val, js_point_class_id);
    if (!s)
        return JS_EXCEPTION;
    return JS_NewFloat64(ctx, sqrt((double)s->x * s->x + (double)s->y * s->y));
}

static void js_point_finalizer(JSRuntime *rt, JSValue val)
{
    JSPointData *s = JS_GetOpaque(val, js_point_class_id);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    js_free_rt(rt, s);
    printf("%s\n", __func__);
}

// 类的定义，包括析构函数
static JSClassDef js_point_class = {
    "Point",
    .finalizer = js_point_finalizer,
};

// 类的成员和方法
static const JSCFunctionListEntry js_point_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("x", js_point_get_xy, js_point_set_xy, 0),
    JS_CGETSET_MAGIC_DEF("y", js_point_get_xy, js_point_set_xy, 1),
    JS_CFUNC_DEF("norm", 0, js_point_norm),
};

// 类的构造方法
static JSValue js_point_ctor(JSContext *ctx,
                             JSValueConst new_target,
                             int argc, JSValueConst *argv)
{
    JSPointData *s;
    JSValue obj = JS_UNDEFINED;
    JSValue proto;

    s = js_mallocz(ctx, sizeof(*s));
    if (!s)
        return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &s->x, argv[0]))
        goto fail;
    if (JS_ToInt32(ctx, &s->y, argv[1]))
        goto fail;

    // 获取class的原型
    proto = JS_GetClassProto(ctx, js_point_class_id);
    if (JS_IsException(proto))
        goto fail;

    // 并以此class为原型建立新的object
    obj = JS_NewObjectProtoClass(ctx, proto, js_point_class_id);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))
        goto fail;
    JS_SetOpaque(obj, s);

    return obj;
fail:
    js_free(ctx, s);
    JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
}

static int createClassWithProto(JSContext *ctx)
{
    // 只建立一次此object
    JSValue proto = JS_GetClassProto(ctx, js_point_class_id);
    if (JS_IsObject(proto))
    {
        JS_FreeValue(ctx, proto);
        return 0;
    }

    /* create the Point class */
    JS_NewClassID(&js_point_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_point_class_id, &js_point_class);

    // 创建一个object并把成员和方法绑定在object上
    proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, js_point_proto_funcs, countof(js_point_proto_funcs));

    // 将此object设置为类的原型
    JS_SetClassProto(ctx, js_point_class_id, proto);
    return 0;
}

void my_class_create(JSContext *ctx)
{
    createClassWithProto(ctx);

    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue point_class = JS_NewCFunction2(ctx, js_point_ctor, "Point", 2, JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global_obj, "Point", point_class);
    JS_FreeValue(ctx, global_obj);
}


