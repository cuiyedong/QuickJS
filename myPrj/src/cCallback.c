/*
 * C Callback
 *
 * 本文件主要展示C与JS的交互，以BUTTON控件的创建为例
 */
// TODO 什么时候finalizer
#include "quickjs-libc.h"
#include "cutils.h"
#include "quickjs.h"
#include <math.h>
#include "dlist.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#define  WIDGET(X)   WIDGET_##X

typedef enum
{
    WIDGET(BUTTON) = 1,
    WIDGET(IMAGE),

    WIDGET(INVALID)
} GuiWidget;

typedef struct _jsval_node
{
    JSValue value;
    list_head_t node;
} JSValNode;

// opqua
typedef struct _button_data
{
    int x;
    JSValue clickCb;
} ButtonData;

typedef struct _button_widet
{
    list_head_t node;     // Button 列表节点

    bool alive;           // 是否还live
    ButtonData opaque;    // 不透明指针
    list_head_t jsValueHead;   // JSValue C节点，需要free
} Button;

static list_head_t sButtonList = LIST_HEAD_INIT(sButtonList);

/**
 *  ============================================================
 *                      widget class
 *  ============================================================
 */
static JSClassID js_widget_class_id;

static void js_widget_finalizer(JSRuntime *rt, JSValue val)
{
    printf("%s %p\n", __func__, val.u.ptr);
}

// 类的定义，包括析构函数
static JSClassDef js_widget_class = {
    "Widget",
    .finalizer = js_widget_finalizer,
};

static int createClassWithProto(JSContext *ctx)
{
    // 只建立一次此object
    JSValue proto = JS_GetClassProto(ctx, js_widget_class_id);
    if (JS_IsObject(proto))
    {
        JS_FreeValue(ctx, proto);
        return 0;
    }

    /* create the widget class */
    JS_NewClassID(&js_widget_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_widget_class_id, &js_widget_class);

    // 创建一个object并把成员和方法绑定在object上
    proto = JS_NewObject(ctx);

    // 将此object设置为类的原型
    JS_SetClassProto(ctx, js_widget_class_id, proto);
    return 0;
}

/**
 *  ============================================================
 *                      module part
 *  ============================================================
 */

static JSValue js_mock_click(JSContext *ctx, JSValueConst this_val,
                           int argc, JSValueConst *argv)
{
    JSValue button = JS_DupValue(ctx, this_val);
    ButtonData* btnOpa = JS_GetOpaque(button, js_widget_class_id);
    if (!btnOpa || JS_IsUndefined(btnOpa->clickCb))
        return JS_FALSE;

    JSValue args[2];
    if (argc != 2)
    {
        args[0] = JS_NewInt32(ctx, 30);  // 模拟按下的坐标 (30, 40)
        args[1] = JS_NewInt32(ctx, 40);
    }
    else
    {
        args[0] = JS_DupValue(ctx, argv[0]);  // 模拟按下的坐标 (30, 40)
        args[1] = JS_DupValue(ctx, argv[1]);
    }

    JSValue ret_val = JS_Call(ctx, btnOpa->clickCb, button, 2, (JSValueConst *)args);

    JS_FreeValue(ctx, button);
    JS_FreeValue(ctx, args[0]);
    JS_FreeValue(ctx, args[1]);

    return ret_val;
}

static JSValue js_create_widget(JSContext *ctx, JSValueConst this_val,
                           int argc, JSValueConst *argv)
{
    JSValue wigetObj = JS_UNDEFINED;
    JSValue paraObj = JS_UNDEFINED;
    JSValue valeX = JS_UNDEFINED;
    JSValue clickCb = JS_UNDEFINED;

    if (argc < 2)
        return JS_UNDEFINED;

    // parse 参数1 widetId
    wigetObj = JS_DupValue(ctx, argv[0]);
    if (!JS_IsNumber(wigetObj))
        goto fail;

    int32_t widgetId = 0;
    JS_ToInt32(ctx, &widgetId, wigetObj);
    JS_FreeValue(ctx, wigetObj);
    if (widgetId <= 0 || widgetId >= WIDGET(INVALID))
        goto fail;

    // parse 参数2, obj, rect + callback
    paraObj = JS_DupValue(ctx, argv[1]);
    if (!JS_IsObject(paraObj))
        goto fail;

    valeX = JS_GetPropertyStr(ctx, paraObj, "x");
    if (!JS_IsNumber(valeX))
        goto fail;
    int32_t x = 0;
    JS_ToInt32(ctx, &x, valeX);  // 简化，不做非法判断了

    clickCb = JS_GetPropertyStr(ctx, paraObj, "onClick");
    if(!JS_IsFunction(ctx, clickCb))
        goto fail;

    // 创建button JS object
    JSValue proto = JS_GetClassProto(ctx, js_widget_class_id);
    if (JS_IsException(proto))
        goto fail;
    JSValue button = JS_NewObjectProtoClass(ctx, proto, js_widget_class_id);
    JS_FreeValue(ctx, proto);
    JS_DefinePropertyValueStr(ctx, button, "x", JS_NewInt32(ctx, x), JS_PROP_C_W_E);
    JS_SetPropertyStr(ctx, button, "mockClick", JS_NewCFunction(ctx, js_mock_click, "mockClick", 2)); // mock 一个click事件. js => c => js

    // 创建c侧button管理单元
    Button* cbutton = (Button*)malloc(sizeof(Button));
    memset(cbutton, 0, sizeof(Button));
    JS_SetOpaque(button, (void*)(&cbutton->opaque));
    cbutton->opaque.x = x;
    cbutton->opaque.clickCb = JS_DupValue(ctx, clickCb);
    cbutton->alive = true;
    init_list_head(&cbutton->jsValueHead);
    list_add_tail(&cbutton->node, &sButtonList);

    JSValNode* jsvalNode = (JSValNode*)malloc(sizeof(JSValNode));
    jsvalNode->value = button;
    list_add_tail(&jsvalNode->node, &cbutton->jsValueHead);

    JS_FreeValue(ctx, paraObj);
    JS_FreeValue(ctx, valeX);
    JS_FreeValue(ctx, clickCb);
    return button;
fail:
    JS_FreeValue(ctx, wigetObj);
    JS_FreeValue(ctx, paraObj);
    JS_FreeValue(ctx, valeX);
    JS_FreeValue(ctx, clickCb);
    return JS_UNDEFINED;
}

static JSValue js_delete_widget(JSContext *ctx, JSValueConst this_val,
                           int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_FALSE;
    }

    JSValue delObj = JS_DupValue(ctx, argv[0]);

    ButtonData* btnOpa = JS_GetOpaque(delObj, js_widget_class_id);
    if (!btnOpa || JS_IsUndefined(btnOpa->clickCb))
        return JS_FALSE;

    JS_FreeValue(ctx, btnOpa->clickCb);
    JS_SetOpaque(delObj, NULL);
    JS_FreeValue(ctx, delObj);

    Button* button = list_entry(btnOpa, Button, opaque);
    button->alive = false;
    button->opaque.clickCb = JS_UNDEFINED;

    return JS_TRUE;
}

static const JSCFunctionListEntry js_gui_funcs[] = {
    JS_PROP_INT32_DEF("BUTTON", WIDGET(BUTTON), JS_PROP_CONFIGURABLE),
    JS_PROP_INT32_DEF("IMAGE",  WIDGET(IMAGE),  JS_PROP_CONFIGURABLE),

    JS_CFUNC_DEF("createWidget", 2, js_create_widget),
    JS_CFUNC_DEF("deleteWidget", 1, js_delete_widget),
};

static int js_gui_init(JSContext *ctx, JSModuleDef *m)
{
    JS_SetModuleExportList(ctx, m, js_gui_funcs, countof(js_gui_funcs));
    return 0;
}

JSModuleDef *js_init_module_gui(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_gui_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_gui_funcs, countof(js_gui_funcs));

    createClassWithProto(ctx);
    return m;
}

static void freeButtonJsval(JSContext *ctx, list_head_t * head)
{
    JSValNode *node_p;
    struct list_head *pos;
    list_for_each(pos, head)
    {
        node_p = list_entry(pos, JSValNode, node);
        if (node_p)
        {
            // JS_FreeValue(ctx, node_p->value);
        }
    }
}

void freeButtons(JSContext *ctx)
{
    Button *node_p;
    struct list_head *pos, *next;
    list_for_each_safe(pos, next, &sButtonList)
    {
        node_p = list_entry(pos, Button, node);
        if (!node_p->alive)
        {
            freeButtonJsval(ctx, &node_p->jsValueHead);  // free 掉C侧的JSValue
            list_del(&node_p->node);   // 将此button从list删除
            free(node_p);
        }
    }
}
