#if __INTELLISENSE__
#undef _HAS_CXX20
#define _HAS_CXX20 0
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "kite_api.h"
#include "PatchUtils.h"

#include "util.h"
#include "log.h"
#include "netcode.h"
#include "config.h"
#include "file_replacement.h"
#include "AllocMan.h"

const KiteSquirrelAPI* KITE;

struct HostEnvironment;

typedef int thisfastcall get_value_t(
    const HostEnvironment* self,
    thisfastcall_edx(int dummy_edx,)
    const char* name,
    void** out
);

// size: 0x10
struct HostEnvironmentVftable {
    void *const __validate_environment; // 0x0
    void *const __method_4; // 0x4
    void *const __method_8; // 0x4
    get_value_t *const get_value; // 0xC
    // 0x10
};

// size: unknown
struct HostEnvironment {
    const HostEnvironmentVftable* vftable;

protected:
    template<typename T = void*>
    inline bool thiscall get_value(const char* name, T* out) const {
        return SUCCEEDED(this->vftable->get_value(
            this,
            thisfastcall_edx(0,)
            name,
            (void**)out
        ));
    }

public:

    inline bool get_hwnd(HWND& out) const {
        return this->get_value("HWND", &out);
    }

    inline bool get_hinstance(HINSTANCE& out) const {
        return this->get_value("HINSTANCE", &out);
    }

    // this just returns "Windows"
    inline bool get_platform(const char*& out) const {
        return this->get_value("PLATFORM", &out);
    }

    inline bool get_squirrel_vm(HSQUIRRELVM& out) const {
        return this->get_value("HSQUIRRELVM", &out);
    }

    inline bool get_kite_api(const KiteSquirrelAPI*& out) const {
        return this->get_value("Kite_SquirrelAPI", &out);
    }

    inline bool get_file_open_read_func(void*& out) const {
        return this->get_value("Function_FileOpenRead", &out);
    }

    inline bool get_file_open_write_func(void*& out) const {
        return this->get_value("Function_FileOpenWrite", &out);
    }

    inline bool get_interface_object_graphics(void*& out) const {
        return this->get_value("InterfaceObject_Graphics", &out);
    }

    inline bool get_interface_object_memory(void*& out) const {
        return this->get_value("InterfaceObject_Memory", &out);
    }
};

// SQInteger example_function(HSQUIRRELVM v) {
//     SQBool arg;
//     if (SQ_SUCCEEDED(sq_getbool(v, 2, &arg))) {
//         log_printf("function called from Squirrel with argument: %d\n", arg);
//         sq_pushbool(v, arg); // Return the argument
//     } else {
//         log_printf("function called from Squirrel with no arguments.\n");
//         return 0;
//     }
//     return 1; // Number of return values
// }

static HSQUIRRELVM v;

SQInteger CompileBuffer(HSQUIRRELVM v) {
    const SQChar* filename;
    SQObject* pObject;

    if (sq_gettop(v) != 3 || 
        SQ_FAILED(sq_getstring(v, 2, &filename)) || 
        SQ_FAILED(sq_getuserdata(v, 3, (SQUserPointer *)&pObject, NULL))){
        return sq_throwerror(v, _SC("invalid arguments...expected: <filename> <*pObject>.\n"));
    }

    if (EmbedData embed = get_new_file_data(filename)) {
        if (SQ_FAILED(sq_compilebuffer(v, (const SQChar*)embed.data, embed.length, "compiled from buffer", SQFalse))) {
            return sq_throwerror(v, _SC("Failed to compile script from buffer.\n"));
        }

        sq_getstackobj(v, -1, pObject);

        sq_pop(v, 1);
    }

    return SQ_OK;
}

SQInteger sq_print(HSQUIRRELVM v){
    const SQChar *str;
    if (sq_gettop(v) != 2){
        return sq_throwerror(v, "Invalid arguments,expected:<string>");
    }

    if (SQ_FAILED(sq_getstring(v, 2, &str))){
        return sq_throwerror(v, "Invalid arguments,expected a string");
    }

    log_printf("%s", str);
    return 0;
}

SQInteger r_resync_get(HSQUIRRELVM v) {
    sq_pushbool(v, resyncing);
    return 1;
}

SQInteger update_ping_constants(HSQUIRRELVM v) {
    sq_pushroottable(v);

    sq_pushstring(v, _SC("setting"), -1);
    if (SQ_SUCCEEDED(sq_get(v, -2))) {
        sq_pushstring(v, _SC("ping"), -1);
        if (SQ_SUCCEEDED(sq_get(v,-2))){
            sq_pushstring(v, _SC("X"), -1);
                sq_pushinteger(v, get_ping_x());
            sq_newslot(v, -3, SQFalse);
            sq_pushstring(v, _SC("Y"), -1);
                sq_pushinteger(v, get_ping_y());
            sq_newslot(v, -3, SQFalse);
            sq_pushstring(v, _SC("SX"), -1);
                sq_pushfloat(v, get_ping_scale_x());
            sq_newslot(v, -3, SQFalse);
            sq_pushstring(v, _SC("SY"), -1);
                sq_pushfloat(v, get_ping_scale_y());
            sq_newslot(v, -3, SQFalse);
            sq_pop(v,1);
        }
        sq_pop(v, 1);
    }

    sq_pop(v, 1);
    return 0;
}

extern "C" {
    dll_export int stdcall init_instance_v2(HostEnvironment* environment) {
        if (
            environment &&
            environment->get_squirrel_vm(v) &&
            environment->get_kite_api(KITE)
        ) {
            // put any important initialization stuff here,
            // like adding squirrel globals/funcs/etc.
            sq_pushroottable(v);

            //config table setup
            sq_pushstring(v, _SC("setting"), -1);
                sq_newtable(v);
                sq_pushstring(v, _SC("version"), -1);
                    sq_pushinteger(v, PLUGIN_VERSION);
                sq_newslot(v, -3, SQFalse);
                sq_pushstring(v, _SC("ping"), -1);
                    sq_newclass(v, SQFalse);
                    sq_pushstring(v, _SC("X"), -1);
                        sq_pushinteger(v, get_ping_x());
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("Y"), -1);
                        sq_pushinteger(v, get_ping_y());
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("SX"), -1);
                        sq_pushfloat(v, get_ping_scale_x());
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("SY"), -1);
                        sq_pushfloat(v, get_ping_scale_y());
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("update_consts"), -1);
                        sq_newclosure(v, update_ping_constants, 0);
                    sq_newslot(v, -3, SQFalse);
                sq_newslot(v, -3, SQFalse);
                sq_pushstring(v, _SC("input_display"), -1);
                    sq_newclass(v, SQFalse);
                    sq_pushstring(v, _SC("enabled"), -1);
                        sq_pushbool(v, SQTrue);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                     sq_pushstring(v, _SC("X"), -1);
                        sq_pushinteger(v, 10);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("Y"), -1);
                        sq_pushinteger(v, 515);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("SX"), -1);
                        sq_pushfloat(v, 1.0);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("SY"), -1);
                        sq_pushfloat(v, 1.0);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("offset"), -1);
                        sq_pushinteger(v, 30);//PLACEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("list_max"), -1);
                        sq_pushinteger(v, 12);//PLACEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3 ,SQFalse);
                    sq_pushstring(v, _SC("red"), -1);
                        sq_pushfloat(v, 0.0);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("green"), -1);
                        sq_pushfloat(v, 1.0);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("blue"), -1);
                        sq_pushfloat(v, 0.0);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                    sq_pushstring(v, _SC("alpha"), -1);
                        sq_pushfloat(v, 1.0);//PLACHEHOLDER PLEASE CHANGE
                    sq_newslot(v, -3, SQFalse);
                sq_newslot(v, -3, SQFalse);
            sq_newslot(v, -3, SQFalse);

            // rollback table setup
            sq_pushstring(v, _SC("rollback"), -1);
                sq_newtable(v);
                sq_pushstring(v, _SC("resyncing"), -1);
                    sq_newclosure(v, r_resync_get, 0);
                sq_newslot(v, -3, SQFalse);
                sq_pushstring(v, _SC("print"), -1);
                    sq_newclosure(v, sq_print, 0);
                sq_newslot(v, -3, SQFalse);
            sq_newslot(v, -3, SQFalse);

                // modifications to the manbow table
            sq_pushstring(v, _SC("manbow"), -1);
                sq_get(v,-2);
                sq_pushstring(v, _SC("CompileBuffer"), -1);
                    sq_newclosure(v, CompileBuffer, 0);
                sq_newslot(v, -3, SQFalse);
            sq_pop(v, 1);

            //this changes the item array in the config menu :)
            //yes i know it's beatiful you don't have to tell me
            // sq_pushstring(v,_SC("menu"),-1);
            // if (SQ_SUCCEEDED(sq_get(v,-2))){
            //     sq_pushstring(v,_SC("config"),-1);
            //     if (SQ_SUCCEEDED(sq_get(v,-2))){
            //         sq_pushstring(v,_SC("item"),-1);
            //         if (SQ_SUCCEEDED(sq_get(v,-2))){
            //             sq_pushstring(v,_SC("misc"),-1);
            //             if(SQ_SUCCEEDED(sq_arrayappend(v,-2))){
            //                 log_printf("Succesfully added to array");
            //             }
            //             sq_pop(v,1);
            //         }
            //         sq_pop(v,1);
            //     }
            //     sq_pop(v,1);
            // }

            sq_pop(v, 1);
            return 1;
        }
        return -1;
    }

    dll_export int stdcall release_instance() {
        return 1;
    }

    dll_export int stdcall update_frame() {
        sq_pushroottable(v);

        //saving ::network.IsPlaying to a variable
        //getting the network table
        sq_pushstring(v, _SC("network"), -1);
        if (SQ_SUCCEEDED(sq_get(v, -2))) {
            //Getting the variable
            sq_pushstring(v, _SC("IsPlaying"), -1);
            if (SQ_SUCCEEDED(sq_get(v, -2))) {
                if (sq_gettype(v, -1) == OT_BOOL) {
                    sq_getbool(v, -1, &isplaying);
                }
                //pop the variable
                sq_pop(v, 1);
            }
            //pop the network table
            sq_pop(v, 1);
        }
        //pop the roottable
        sq_pop(v, 1);

#if ALLOCATION_PATCH_TYPE != PATCH_NO_ALLOCS
        update_allocs();
#endif

        return 1;
    }

    dll_export int stdcall render_preproc() {
        return 0;
    }

    dll_export int stdcall render(int, int) {
        return 0;
    }
}