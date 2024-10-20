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
#include "lobby.h"
#include "debug.h"

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

static inline void sq_setinteger(HSQUIRRELVM v, const SQChar* name, const SQInteger& value) {
    sq_pushstring(v, name, -1);
    sq_pushinteger(v, value);
    sq_newslot(v, -3, SQFalse);
}
static inline void sq_setfloat(HSQUIRRELVM v, const SQChar* name, const SQFloat& value) {
    sq_pushstring(v, name, -1);
    sq_pushfloat(v, value);
    sq_newslot(v, -3, SQFalse);
}
static inline void sq_setbool(HSQUIRRELVM v, const SQChar* name, const SQBool& value) {
    sq_pushstring(v, name, -1);
    sq_pushbool(v, value);
    sq_newslot(v, -3, SQFalse);
}
static inline void sq_setfunc(HSQUIRRELVM v, const SQChar* name, const SQFUNCTION& func) {
    sq_pushstring(v, name, -1);
    sq_newclosure(v, func, 0);
    sq_newslot(v, -3, SQFalse);
}

template <typename L>
static inline bool sq_edit(HSQUIRRELVM v, const SQChar* name, const L& lambda) {
    sq_pushstring(v, name, -1);
    if (expect(SQ_SUCCEEDED(sq_get(v, -2)), true)) {
        lambda(v);
        sq_pop(v, 1);
        return true;
    }
    return false;
}

template <typename L>
static inline void sq_createtable(HSQUIRRELVM v, const SQChar* name, const L& lambda) {
    sq_pushstring(v, name, -1);
    sq_newtable(v);
    lambda(v);
    sq_newslot(v, -3, SQFalse);
}

template <typename L>
static inline void sq_createclass(HSQUIRRELVM v, const SQChar* name, const L& lambda) {
    sq_pushstring(v, name, -1);
    sq_newclass(v, SQFalse);
    lambda(v);
    sq_newslot(v, -3, SQFalse);
}

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

SQInteger CompileBuffer(HSQUIRRELVM v) {
    const SQChar* filename;
    SQObject* pObject;

    if (
        sq_gettop(v) != 3 || 
        SQ_FAILED(sq_getstring(v, 2, &filename)) || 
        SQ_FAILED(sq_getuserdata(v, 3, (SQUserPointer *)&pObject, NULL))
    ) {
        return sq_throwerror(v, _SC("invalid arguments...expected: <filename> <*pObject>.\n"));
    }

    if (EmbedData embed = get_new_file_data(filename)) {
        if (SQ_FAILED(sq_compilebuffer(v, (const SQChar*)embed.data, embed.length, _SC("compiled from buffer"), SQFalse))) {
            sq_throwexception(v, "CompileBuffer");
            return sq_throwerror(v, _SC("Failed to compile script from buffer.\n"));
        }

        sq_getstackobj(v, -1, pObject);

        sq_pop(v, 1);
    }

    return SQ_OK;
}

SQInteger sq_print(HSQUIRRELVM v) {
    const SQChar* str;
    if (sq_gettop(v) != 2 || 
        SQ_FAILED(sq_getstring(v, 2, &str))
    ) {
        return sq_throwerror(v, "Invalid arguments,expected:<string>");
    }

    log_printf("%s", str);
    return 0;
}

SQInteger sq_fprint(HSQUIRRELVM v) {
    const SQChar* str;
    const SQChar* path;
    if (sq_gettop(v) != 3 ||
        SQ_FAILED(sq_getstring(v, 2, &path)) ||
        SQ_FAILED(sq_getstring(v, 3, &str))
    ) {
        return sq_throwerror(v, _SC("invalid arguments...expected: <file> <string>.\n"));
    }
#if SQUNICODE
    if (FILE* file = _wfopen(path, L"a")) {
        log_fprintf(file, "%ls", str);
        fclose(file);
    }
#else
    if (FILE* file = fopen(path, "a")) {
        log_fprintf(file, "%s", str);
        fclose(file);
    }
#endif
    return 0;
}

SQInteger sq_log_foreach(HSQUIRRELVM v) {
    if (sq_gettop(v) != 2 || SQ_FAILED(sq_gettype(v, 2))) return sq_throwerror(v, _SC("Invalid arguments... expected: <object>.\n"));
    if (SQ_SUCCEEDED(sq_tostring(v, 2))) { 
        const SQChar *objStr;
        sq_getstring(v, -1, &objStr);  
        log_printf(">>>>>>%s<<<<<<\n", objStr);
        sq_pop(v, 1); 
    } else {
        SQUserPointer ptr;
        sq_getuserpointer(v, 2, &ptr);
        log_printf(">>>>>>%p<<<<<<\n", ptr);
    }

    sq_pushnull(v);
    while (SQ_SUCCEEDED(sq_next(v, 2))) {
        //Key|value
        // -2|-1
        const SQChar* key;
        sq_getstring(v, -2, &key);

        const SQChar *valstr;
        sq_tostring(v, -1);{
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
                    log_printf(">%s : %s : %s<\n", key, valstr, className);
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
                    log_printf(">%s : %s : %s<\n", key, valstr, className);
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
    return SQ_OK;
}


SQInteger r_resync_get(HSQUIRRELVM v) {
    sq_pushbool(v, resyncing);
    return 1;
}

static inline void set_ping_constants(HSQUIRRELVM v) {
    sq_setbool(v, _SC("enabled"), get_ping_enabled());
    sq_setinteger(v, _SC("X"), get_ping_x());
    sq_setinteger(v, _SC("Y"), get_ping_y());
    sq_setfloat(v, _SC("SX"), get_ping_scale_x());
    sq_setfloat(v, _SC("SY"), get_ping_scale_y());
    uint32_t color = get_ping_color();
    sq_setfloat(v, _SC("blue"), (float)(uint8_t)color / 255.0f);
    sq_setfloat(v, _SC("green"), (float)(uint8_t)(color >> 8) / 255.0f);
    sq_setfloat(v, _SC("red"), (float)(uint8_t)(color >> 16) / 255.0f);
    sq_setfloat(v, _SC("alpha"), (float)(uint8_t)(color >> 24) / 255.0f);
    sq_setbool(v, _SC("ping_in_frames"), get_ping_frames());
}

static inline void set_inputp1_constants(HSQUIRRELVM v) {
    sq_setbool(v, _SC("enabled"), get_inputp1_enabled());
    sq_setinteger(v, _SC("X"), get_inputp1_x());
    sq_setinteger(v, _SC("Y"), get_inputp1_y());
    sq_setfloat(v, _SC("SX"), get_inputp1_scale_x());
    sq_setfloat(v, _SC("SY"), get_inputp1_scale_y());
    sq_setinteger(v, _SC("offset"), get_inputp1_offset());
    sq_setinteger(v, _SC("list_max"), get_inputp1_count());
    sq_setbool(v, _SC("spacing"), get_inputp1_spacing());
    uint32_t color = get_inputp1_color();
    sq_setfloat(v, _SC("blue"), (float)(uint8_t)color / 255.0f);
    sq_setfloat(v, _SC("green"), (float)(uint8_t)(color >> 8) / 255.0f);
    sq_setfloat(v, _SC("red"), (float)(uint8_t)(color >> 16) / 255.0f);
    sq_setfloat(v, _SC("alpha"), (float)(uint8_t)(color >> 24) / 255.0f);
    int32_t timer = get_inputp1_timer();
    sq_setinteger(v, _SC("timer"), timer > 0 ? timer : 0);
    sq_setbool(v, _SC("raw_input"), get_inputp1_raw_input());
}

static inline void set_inputp2_constants(HSQUIRRELVM v) {
    sq_setbool(v, _SC("enabled"), get_inputp2_enabled());
    sq_setinteger(v, _SC("X"), get_inputp2_x());
    sq_setinteger(v, _SC("Y"), get_inputp2_y());
    sq_setfloat(v, _SC("SX"), get_inputp2_scale_x());
    sq_setfloat(v, _SC("SY"), get_inputp2_scale_y());
    sq_setinteger(v, _SC("offset"), get_inputp2_offset());
    sq_setinteger(v, _SC("list_max"), get_inputp2_count());
    sq_setbool(v, _SC("spacing"), get_inputp2_spacing());
    uint32_t color = get_inputp2_color();
    sq_setfloat(v, _SC("blue"), (float)(uint8_t)color / 255.0f);
    sq_setfloat(v, _SC("green"), (float)(uint8_t)(color >> 8) / 255.0f);
    sq_setfloat(v, _SC("red"), (float)(uint8_t)(color >> 16) / 255.0f);
    sq_setfloat(v, _SC("alpha"), (float)(uint8_t)(color >> 24) / 255.0f);
    int32_t timer = get_inputp2_timer();
    sq_setinteger(v, _SC("timer"), timer > 0 ? timer : 0);
    sq_setbool(v, _SC("raw_input"), get_inputp2_raw_input());
}

SQInteger update_ping_constants(HSQUIRRELVM v) {
    sq_pushroottable(v);

    sq_edit(v, _SC("setting"), [](HSQUIRRELVM v) {
        sq_edit(v, _SC("ping"), set_ping_constants);
    });

    sq_pop(v, 1);
    return 0;
}

SQInteger update_input_constants(HSQUIRRELVM v) {
    sq_pushroottable(v);

    sq_edit(v, _SC("setting"), [](HSQUIRRELVM v) {
        sq_edit(v, _SC("input_display"), [](HSQUIRRELVM v) {
            sq_edit(v, _SC("p1"), set_inputp1_constants);
            sq_edit(v, _SC("p2"), set_inputp2_constants);
        });
    });

    sq_pop(v, 1);
    return 0;
}

SQInteger start_direct_punch_wait(HSQUIRRELVM v) {
    send_lobby_punch_wait();
    return 0;
}

extern "C" {
    dll_export int stdcall init_instance_v2(HostEnvironment* environment) {
        if (
            environment &&
            environment->get_squirrel_vm(v) &&
            environment->get_kite_api(KITE)
        ) {
            //imgui_init(environment);
            // put any important initialization stuff here,
            // like adding squirrel globals/funcs/etc.
            sq_pushroottable(v);

            // setting table setup
            sq_createtable(v, _SC("setting"), [](HSQUIRRELVM v) {
                sq_setinteger(v, _SC("version"), PLUGIN_VERSION);
                sq_createtable(v, _SC("ping"), [](HSQUIRRELVM v) {
                    sq_setfunc(v, _SC("update_consts"), update_ping_constants);
                    set_ping_constants(v);
                });
                sq_createtable(v, _SC("input_display"), [](HSQUIRRELVM v) {
                    sq_setfunc(v, _SC("update_consts"), update_input_constants);
                    sq_createtable(v, _SC("p1"), set_inputp1_constants);
                    sq_createtable(v, _SC("p2"), set_inputp2_constants);
                });
            });

            // rollback table setup
            sq_createtable(v, _SC("rollback"), [](HSQUIRRELVM v) {
                sq_setfunc(v, _SC("resyncing"), r_resync_get);
            });

            sq_createtable(v, _SC("punch"), [](HSQUIRRELVM v) {
                sq_setfunc(v, _SC("init_wait"), start_direct_punch_wait);
            });

            // debug table setup
            sq_createtable(v, _SC("debug"), [](HSQUIRRELVM v) {
                sq_setfunc(v, _SC("print"), sq_print);
                sq_setfunc(v, _SC("fprint"), sq_fprint);
                sq_setfunc(v, _SC("dump"), sq_log_foreach);
            });

            // modifications to the manbow table
            sq_edit(v, _SC("manbow"), [](HSQUIRRELVM v) {
                sq_setfunc(v, _SC("CompileBuffer"), CompileBuffer);
            });

            //this changes the item array in the config menu :)
            //yes i know it's beautiful you don't have to tell me
            // sq_edit(v, _SC("menu"), [](HSQUIRRELVM v) {
            //     sq_edit(v, _SC("config"), [](HSQUIRRELVM v) {
            //         sq_edit(v, _SC("item"), [](HSQUIRRELVM v) {
            //             sq_pushstring(v, _SC("misc"), -1);
            //             if (SQ_SUCCEEDED(sq_arrayappend(v, -2))) {
            //                 log_printf("Succesfully added to array\n");
            //             }
            //         });
            //     });
            // });

            sq_pop(v, 1);
            return 1;
        }
        return -1;
    }

    dll_export int stdcall release_instance() {
        //imgui_release();
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
        //imgui_update();
        return 0;
    }
}