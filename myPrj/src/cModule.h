#ifndef __CMODULE_H
#define __CMODULE_H

JSModuleDef *js_init_module_mmath(JSContext *ctx, const char *module_name);
JSValue my_class_create(JSContext *ctx);
JSModuleDef *js_init_module_gui(JSContext *ctx, const char *module_name);
void freeButtons(JSContext *ctx);
#endif

