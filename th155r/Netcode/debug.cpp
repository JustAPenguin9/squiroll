#if __has_builtin(__builtin_offsetof)
#ifdef offsetof
#undef offsetof
#endif
#define offsetof(s, m) __builtin_offsetof(s, m)
#endif

#define IMGUI_IMPLEMENTATION
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d11.h>

#include "log.h"
#include "util.h"
#include "debug.h"
#include "PatchUtils.h"

#define DEVICE_ADDR (0x4DAE9C_R)
#define DEVICE_CONTEXT_ADDR (0x4DAE98_R)
#define FULLSCREEN_CURSOR_CALL_ADDRA (0x023B48);
#define FULLSCREEN_CURSOR_CALL_ADDRB (0x023CE0);

void sq_throwexception(HSQUIRRELVM v, const char* src) {
    log_printf("#####Squirrel exception from:%s#####\n", src);

    sq_getlasterror(v);
    const SQChar* errorMsg;
    if (SQ_SUCCEEDED(sq_getstring(v, -1, &errorMsg))) {
#if SQUNICODE
        log_printf("%ls\n", errorMsg);
#else
        log_printf("%s\n", errorMsg);
#endif
    }
    sq_pop(v, 1);

    log_printf("#####End of stack trace#####\n");
}

// extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// LRESULT CALLBACK imgui_wndproc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam){
//     if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))return true;
//     return ((WNDPROC)0x23A70_R)(hWnd, msg, wParam, lParam);
// }

void imgui_init(){
    // HWND hwnd = NULL;
    // ID3D11Device* device = *(ID3D11Device**)DEVICE_ADDR;
    // ID3D11DeviceContext* device_context = *(ID3D11DeviceContext**)DEVICE_CONTEXT_ADDR;
    // environment->get_hwnd(hwnd);
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    // ImGui_ImplWin32_Init(hwnd);
    // ImGui_ImplDX11_Init(device, device_context);
    // SetWindowLong(hwnd, GWL_WNDPROC, (LONG)imgui_wndproc);
}

void imgui_update(){
    // ImGui_ImplDX11_NewFrame();
    // ImGui_ImplWin32_NewFrame();
    // ImGui::NewFrame();

    // {
    //     static char test_text[256] = {};

    //     ImGui::Begin("Hello");
    //     ImGui::Text("I'm a window");
    //     ImGui::InputText("Textbox", test_text, sizeof(test_text));
    //     ImGui::End();
    // }
 
    // ImGui::Render();
    // ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void imgui_release(){
    // ImGui_ImplDX11_Shutdown();
    // ImGui_ImplWin32_Shutdown();
    // ImGui::DestroyContext();
}

void show_tree(HSQUIRRELVM v, SQObject Root) {
    sq_pushobject(v, Root);
    if (SQ_FAILED(sq_gettype(v, -1))){
        //sq_throwerror(v, _SC("Invalid arguments... expected: <object>.\n"));
        log_printf("root object is not delegable\n");
        return;
    }
    if (SQ_SUCCEEDED(sq_tostring(v, -1))) { 
        const SQChar *objStr;
        sq_getstring(v, -1, &objStr);  
        log_printf(">>>>>>%s<<<<<<\n", objStr);
        sq_pop(v, 1); 
    } else {
        SQUserPointer ptr;
        sq_getuserpointer(v, -1, &ptr);
        log_printf(">>>>>>%p<<<<<<\n", ptr);
    }

    sq_pushnull(v);
    while (SQ_SUCCEEDED(sq_next(v, -2))) {
        //Key|value
        // -2|-1
        const SQChar* key;
        sq_getstring(v, -2, &key);

        const SQChar *valstr;
        sq_typeof(v, -1);{
            if (SQ_FAILED(sq_getstring(v, -1, &valstr)))valstr = _SC("Unknown");
            sq_pop(v, 1);
        }
        switch (sq_gettype(v, -1)) {
            //All Value types
            /*
            NULL,
            INTEGER,
            FLOAT,
            STRING,
            CLOSURE(SQUIRREL FUNCTION),
            NATIVECLOSURE(C++ FUNCTION),
            BOOL,
            INSTANCE,
            CLASS,
            ARRAY,
            TABLE,
            USERDATA,
            USERPOINTER,
            GENERATOR,
            WEAKREF
            */
            case OT_NULL: {
                log_printf(">%s : %s : NULL\n", key, valstr);
                break;
            }

            case OT_INTEGER: {
                SQInteger val;
                sq_getinteger(v, -1, &val);
                log_printf(">%s : %s : %d\n", key, valstr, val);
                break;
            }

            case OT_FLOAT: {
                SQFloat val;
                sq_getfloat(v, -1, &val);
                log_printf(">%s : %s : %f\n", key, valstr, val);
                break;
            }

            case OT_STRING: {
                const SQChar* val;
                sq_getstring(v, -1, &val);
                log_printf(">%s : %s : %s\n", key, valstr, val);
                break;
            }

            case OT_CLOSURE: {
                SQUnsignedInteger numParams, numFreeVars;
                sq_getclosureinfo(v, -1, &numParams, &numFreeVars);
                log_printf(">%s : %s : %d arguments, %d free variables\n", key, valstr, numParams, numFreeVars);
                break;
            }

            case OT_NATIVECLOSURE: {
                SQUnsignedInteger numParams;
                sq_getclosureinfo(v, -1, &numParams, nullptr);
                log_printf(">%s : %s : %d arguments\n", key, valstr, numParams);
                break;
            }

            case OT_BOOL: {
                SQBool val;
                sq_getbool(v, -1, &val);
                log_printf(">%s : %s : %s\n", key, valstr, val ? "true" : "false");
                break;
            }

            case OT_INSTANCE: {
                sq_push(v, -1);
                const SQChar *className;
                if (SQ_SUCCEEDED(sq_gettypetag(v, -1, (SQUserPointer *)&className))) {
                    log_printf(">%s : %s : %s\n", key, valstr, className);
                } else {
                    log_printf(">%s : %s : unknown\n", key, valstr);
                }
                sq_pop(v, 1);
                break;
            }

            case OT_CLASS: {
                const SQChar *className;
                if (SQ_SUCCEEDED(
                        sq_gettypetag(v, -1, (SQUserPointer *)&className))) {
                    log_printf(">%s : %s : %s\n", key, valstr, className);
                } else {
                    log_printf(">%s : %s : unknown\n", key, valstr);
                }
                break;
            }

            case OT_ARRAY:
            case OT_TABLE: {
                SQInteger size = sq_getsize(v, -1);
                log_printf(">%s : %s : %d\n",key, valstr, size);
                break;
            }
            default: {
                SQUserPointer val;
                sq_getuserpointer(v, -1, &val);
                log_printf(">%s : %s : %p\n", key, valstr, val);
                break;
            }
        }
        sq_pop(v, 2);
    }

    sq_pop(v, 1);
    log_printf("<<<<<<>>>>>>\n");
}

bool CompileScriptBuffer(HSQUIRRELVM v, const char *Src, const char *to) {
  bool is_compiled = false;
  if (Src) {
    is_compiled = SQ_SUCCEEDED(sq_compilebuffer(v, Src, strlen(Src), _SC("repl"), SQTrue));

    if (is_compiled) {
        // MessageBox(NULL, TEXT("Script compiled succesfuly."), TEXT("Squirrel
        // Info"), MB_ICONINFORMATION);
        //TODO:
        /*
        root parsing for the to variable so
        this could work as a example "parent.nested"
        */
        sq_pushroottable(v);
        if (strcmp(to,"") == 0){

            sq_pushstring(v,_SC(to), -1);
            if( SQ_FAILED(sq_get(v, -2))){
                //future error handling
                sq_throwexception(v, "repl");
            }
        }
        sq_call(v, 1, SQFalse, SQTrue);
        sq_pop(v, -1);
    } else{
        // MessageBox(NULL, TEXT("Couldn't compile script."), TEXT("Squirrel
        // Info"), MB_ICONERROR);
    }

    return is_compiled;
  }
  return false;
}

HSQOBJECT SQGetObjectByName(HSQUIRRELVM v, const SQChar *name) {
    HSQOBJECT ret = {};
    sq_pushstring(v, name, -1);
    sq_get(v, -2);
    sq_getstackobj(v, -1, &ret);
    sq_pop(v, 1);
    return ret;
}