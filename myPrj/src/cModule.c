/**
 * define => set => add
 */

#include "quickjs-libc.h"
#include "cutils.h"
#include "quickjs.h"

static JSValue js_my_adder(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_my_multi(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static void CircleClassInit(JSContext *ctx, JSModuleDef *m);

static JSClassID CircleClassID;

static const JSCFunctionListEntry js_mmath_funcs[] = {
    // part1: function 定义
    JS_CFUNC_DEF("add", 2, js_my_adder),
    JS_CFUNC_DEF("multi", 2, js_my_multi),

    // part2: prop 定义, mmath.pai, const R
    JS_PROP_DOUBLE_DEF("pai", 3.14, JS_PROP_CONFIGURABLE),

    // part3: object 定义

};

/**
 * @brief cModule create init
 * @details 添加module export清单 function prop object
 */
static int js_mmath_init(JSContext *ctx, JSModuleDef *m)
{
    JS_SetModuleExportList(ctx, m, js_mmath_funcs, countof(js_mmath_funcs));
    CircleClassInit(ctx, m);
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
 * @brief part1: cModule 添加方法
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

static JSValue js_my_multi(JSContext *ctx, JSValueConst this_val,
                           int argc, JSValueConst *argv)
{
    int a = 0;
    int b = 0;

    if (JS_ToInt32(ctx, &a, argv[0]) || JS_ToInt32(ctx, &b, argv[1]))
    {
        printf("%s err param\n", __func__);
        return JS_UNDEFINED;
    }

    return JS_NewInt32(ctx, a * b);
}

/**
 * @brief part1: 类的添加方法
 */
static JSClassDef circle_class_def = {
    .class_name = "circleA",
    // .finalizer = JSButtonFinalizer,
};

typedef struct _circle
{
    const char *name;
    uint32_t r;
} Circle;

/* 计算圆的周长 */
static JSValue JSCircleRoundFunc(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue ret = JS_UNDEFINED;
    Circle *circle = (Circle *)JS_GetOpaque2(ctx, this_val, CircleClassID);

    if (circle == NULL)
        return JS_EXCEPTION;
    if (argc > 0)
    {
        JS_ToUint32(ctx, &circle->r, argv[0]);
    }

    return JS_NewUint32(ctx, 2 * 3.14 * circle->r);
}

static JSValue JSCircleGetR(JSContext *ctx, JSValueConst this_val)
{
    Circle *circle = (Circle *)JS_GetOpaque2(ctx, this_val, CircleClassID);
    if (circle == NULL)
    {
        return JS_EXCEPTION;
    }
    return JS_NewUint32(ctx, circle->r);
}
static JSValue JSCircleSetR(JSContext *ctx, JSValueConst this_val, JSValueConst val)
{
    Circle *circle = (Circle *)JS_GetOpaque2(ctx, this_val, CircleClassID);
    if (circle == NULL)
    {
        return JS_EXCEPTION;
    }
    int v = 0;
    JS_ToUint32(ctx, &v, val);
    circle->r = v;
    return JS_UNDEFINED;
}

static JSValue JSCircleCreate(JSContext *ctx, JSValueConst val, int argc, JSValueConst *argv)
{
  JSValue obj = JS_UNDEFINED;
  Circle * circle = (Circle*)js_malloc(ctx, sizeof(Circle));
  if(argc > 1){
    JS_ToInt32(ctx, &circle->r, argv[0]);
    circle->name = JS_ToCString(ctx, argv[1]);
  }else if(argc > 0){
    JS_ToInt32(ctx, &circle->r, argv[0]);
  }

  JSValue proto = JS_GetPropertyStr(ctx, val, "prototype");
  obj = JS_NewObjectProtoClass(ctx, JS_NULL, CircleClassID);
  JS_SetPrototype(ctx, obj, proto);
  JS_FreeValue(ctx, proto);
  JS_SetOpaque(obj, circle);
  return obj;
}

static const JSCFunctionListEntry circle_class_funcs[] = {
    JS_CFUNC_DEF("round", 0, JSCircleRoundFunc),
    JS_PROP_INT32_DEF("version", 100, JS_PROP_C_W_E),
    // JS_CGETSET_DEF("name", JSButtonGetName, JSButtonSetName),
    JS_CGETSET_DEF("r", JSCircleGetR, JSCircleSetR)};

static void CircleClassInit(JSContext *ctx, JSModuleDef *m)
{
    JS_NewClassID(&CircleClassID);                                     // 分配全局唯一class id
    JS_NewClass(JS_GetRuntime(ctx), CircleClassID, &circle_class_def); // 新建类，类名和析构

    // 建立class原型 绑定属性、枚举、方法等 然后挂到class中
    JSValue proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, circle_class_funcs, countof(circle_class_funcs));

    // 类的构造函数创建并绑定
    JSValue class = JS_NewCFunction2(ctx, JSCircleCreate, circle_class_def.class_name, 1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, class, proto);
    JS_SetClassProto(ctx, CircleClassID, proto);
    JS_SetModuleExport(ctx, m, circle_class_def.class_name, class); //设置可导出类
}