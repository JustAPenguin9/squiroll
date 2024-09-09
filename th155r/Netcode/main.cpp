#include <string.h>
#include <limits.h>
#include <squirrel.h>

#include "netcode.h"
#include "util.h"
#include "AllocMan.h"
#include "Autopunch.h"
#include "PatchUtils.h"
#include "log.h"
#include "file_replacement.h"

#if !MINGW_COMPAT
#pragma comment (lib, "Ws2_32.lib")
#endif

const uintptr_t base_address = (uintptr_t)GetModuleHandleA(NULL);
uintptr_t libact_base_address = 0;

//SQVM Rx4DACE4 initialized at Rx124710
HSQUIRRELVM VM;

void Cleanup() {
    autopunch_cleanup();
}

void Debug() {
    AllocConsole();
    (void)freopen("CONIN$", "r", stdin);
    (void)freopen("CONOUT$", "w", stdout);
    (void)freopen("CONOUT$", "w", stderr);
    SetConsoleTitleW(L"th155r debug");
}

typedef void* thisfastcall act_script_plugin_load_t(
    void* self,
    thisfastcall_edx(int dummy_edx,)
    const char* plugin_path
);

hostent* WSAAPI my_gethostbyname(const char* name) {
    log_printf("LOBBY HOST: %s\n", name);
    return gethostbyname(name);
}

void patch_se_libact(void* base_address);
void patch_se_lobby(void* base_address);
void patch_se_upnp(void* base_address);
void patch_se_information(void* base_address);
void patch_se_trust(void* base_address);

template <const uintptr_t& base, uintptr_t offset>
void* thisfastcall patch_act_script_plugin(
    void* self,
    thisfastcall_edx(int dummy_edx,)
    const char* plugin_path
) {
    void* base_address = based_pointer<act_script_plugin_load_t>(base, offset)(
        self,
        thisfastcall_edx(dummy_edx,)
        plugin_path
    );
    
    if (base_address) {
        debug_printf("Applying patches for \"%s\" at %p\n", plugin_path, base_address);
        if (!strcmp(plugin_path, "data/plugin/se_libact.dll")) {
            patch_se_libact(base_address);
        }
        else if (!strcmp(plugin_path, "data/plugin/se_lobby.dll")) {
            patch_se_lobby(base_address);
        }
        else if (!strcmp(plugin_path, "data/plugin/se_upnp.dll")) {
            patch_se_upnp(base_address);
        }
        else if (!strcmp(plugin_path, "data/plugin/se_information.dll")) {
            patch_se_information(base_address);
        }
        else if (!strcmp(plugin_path, "data/plugin/se_trust.dll")) {
            patch_se_trust(base_address);
        }
    }
    
    return base_address;
}

void patch_se_libact(void* base_address) {
    libact_base_address = (uintptr_t)base_address;
    
#if ALLOCATION_PATCH_TYPE == PATCH_SQUIRREL_ALLOCS
    //hotpatch_rel32(based_pointer(base_address, 0xC4BE5), my_malloc);
    //hotpatch_rel32(based_pointer(base_address, 0xC4C89), my_realloc);
    //hotpatch_rel32(based_pointer(base_address, 0xC4C75), my_free);
#elif ALLOCATION_PATCH_TYPE == PATCH_ALL_ALLOCS
    hotpatch_jump(based_pointer(base_address, 0x134632), my_malloc);
    hotpatch_jump(based_pointer(base_address, 0x12C67B), my_calloc);
    hotpatch_jump(based_pointer(base_address, 0x13BD53), my_realloc);
    hotpatch_jump(based_pointer(base_address, 0x12C6D8), my_free);
    hotpatch_jump(based_pointer(base_address, 0x138F44), my_recalloc);
    hotpatch_jump(based_pointer(base_address, 0x141C26), my_msize);
#endif

    hotpatch_rel32(based_pointer(base_address, 0x15F7C), &patch_act_script_plugin<libact_base_address, 0x1CC60>);
}

void patch_se_lobby(void* base_address) {
#if ALLOCATION_PATCH_TYPE == PATCH_ALL_ALLOCS
    hotpatch_jump(based_pointer(base_address, 0x41007), my_malloc);
    hotpatch_jump(based_pointer(base_address, 0x402F8), my_calloc);
    hotpatch_jump(based_pointer(base_address, 0x41055), my_realloc);
    hotpatch_jump(based_pointer(base_address, 0x40355), my_free);
    hotpatch_jump(based_pointer(base_address, 0x4DCF1), my_recalloc);
    hotpatch_jump(based_pointer(base_address, 0x53F76), my_msize);
#endif

    hotpatch_import(based_pointer(base_address, 0x1292AC), my_gethostbyname);
}

void patch_se_upnp(void* base_address) {
#if ALLOCATION_PATCH_TYPE == PATCH_ALL_ALLOCS
    hotpatch_jump(based_pointer(base_address, 0x1C4BB), my_malloc);
    hotpatch_jump(based_pointer(base_address, 0x21229), my_calloc);
    hotpatch_jump(based_pointer(base_address, 0x1C509), my_realloc);
    hotpatch_jump(based_pointer(base_address, 0x215AD), my_free);
    hotpatch_jump(based_pointer(base_address, 0x2D3BF), my_recalloc);
    hotpatch_jump(based_pointer(base_address, 0x32413), my_msize);
#endif
}

void patch_se_information(void* base_address) {
#if ALLOCATION_PATCH_TYPE == PATCH_ALL_ALLOCS
    hotpatch_jump(based_pointer(base_address, 0x1B853), my_malloc);
    hotpatch_jump(based_pointer(base_address, 0x1BBE9), my_calloc);
    hotpatch_jump(based_pointer(base_address, 0x1B8A1), my_realloc);
    hotpatch_jump(based_pointer(base_address, 0x1BFAE), my_free);
    hotpatch_jump(based_pointer(base_address, 0x25FF0), my_recalloc);
    hotpatch_jump(based_pointer(base_address, 0x2B406), my_msize);
#endif
}

void patch_se_trust(void* base_address) {
#if ALLOCATION_PATCH_TYPE == PATCH_ALL_ALLOCS
    hotpatch_jump(based_pointer(base_address, 0x5BA7), my_malloc);
    hotpatch_jump(based_pointer(base_address, 0x5C92), my_calloc);
    hotpatch_jump(based_pointer(base_address, 0x936E), my_realloc);
    hotpatch_jump(based_pointer(base_address, 0x5B6D), my_free);
    hotpatch_jump(based_pointer(base_address, 0x78AB), my_recalloc);
    hotpatch_jump(based_pointer(base_address, 0x933B), my_msize);
#endif

    static constexpr const uint8_t data[] = { 0x31, 0xC0 };
    mem_write(based_pointer(base_address, 0x15CC), data, sizeof(data));
}

#define sq_vm_malloc_call_addr (0x186745_R)
#define sq_vm_realloc_call_addr (0x18675A_R)
#define sq_vm_free_call_addr (0x186737_R)

#define malloc_base_addr (0x312D61_R)
#define calloc_base_addr (0x3122EA_R)
#define realloc_base_addr (0x312DAF_R)
#define free_base_addr (0x312347_R)
#define recalloc_base_addr (0x3182DF_R)
#define msize_base_addr (0x31ED30_R)

#define WSASend_import_addr (0x3884D0_R)
#define WSASendTo_import_addr (0x3884D4_R)
#define WSARecvFrom_import_addr (0x3884D8_R)
#define bind_import_addr (0x3884E0_R)
#define closesocket_import_addr (0x388514_R)

#define sq_vm_init_call_addrA (0x00D6AD_R)
#define sq_vm_init_call_addrB (0x055EFA_R)
#define sq_vm_init_addr (0x024710_R)

#define patch_act_script_plugin_hook_addr (0x127ADC_R)

#define file_replacement_hook_addr (0x23FAA_R)
#define file_replacement_read_addrA (0x2DFA1_R)
#define file_replacement_read_addrB (0x2DFED_R)
#define CloseHandle_import_addr (0x3881DC_R)

void patch_autopunch() {
    //hotpatch_import(WSARecvFrom_import_addr, my_WSARecvFrom);
    //hotpatch_import(WSASendTo_import_addr, my_WSASendTo);
    //hotpatch_import(WSASend_import_addr, my_WSASend);
    //hotpatch_import(bind_import_addr, my_bind);
    //hotpatch_import(closesocket_import_addr, my_closesocket);

    //autopunch_init();
}

void patch_allocman() {
#if ALLOCATION_PATCH_TYPE == PATCH_SQUIRREL_ALLOCS
    hotpatch_rel32(sq_vm_malloc_call_addr, my_malloc);
    hotpatch_rel32(sq_vm_realloc_call_addr, my_realloc);
    hotpatch_rel32(sq_vm_free_call_addr, my_free);
#elif ALLOCATION_PATCH_TYPE == PATCH_ALL_ALLOCS
    hotpatch_jump(malloc_base_addr, my_malloc);
    hotpatch_jump(calloc_base_addr, my_calloc);
    hotpatch_jump(realloc_base_addr, my_realloc);
    hotpatch_jump(free_base_addr, my_free);
    hotpatch_jump(recalloc_base_addr, my_recalloc);
    hotpatch_jump(msize_base_addr, my_msize);
#endif
}

void patch_file_loading() {
    hotpatch_call(file_replacement_hook_addr, file_replacement_hook);
    static constexpr uint8_t patch[] = { 0xE9, 0x8C, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
    mem_write(file_replacement_hook_addr + 5, patch, sizeof(patch));

    hotpatch_icall(file_replacement_read_addrA, file_replacement_read);
    hotpatch_icall(file_replacement_read_addrB, file_replacement_read);

    hotpatch_import(CloseHandle_import_addr, close_handle_hook);
}

typedef HSQUIRRELVM (*sq_vm_init)(void);
sq_vm_init sq_vm_init_ptr = (sq_vm_init)sq_vm_init_addr;

#define update_addr (0x008F50_R)
typedef void (*update_test)(void);
update_test update_ptr = (update_test)update_addr;

HSQUIRRELVM my_sq_vm_init(){
    VM = sq_vm_init_ptr();
    return VM;
}

#define loadplugin_addr (0x12B6F0_R)
#define init_addr (0x00D530_R)
#define init_call_addr (0x01DEF2_R)

typedef void (*init)(void);
init init_ptr = (init)init_addr;

typedef void* (*loadplugin)(const char*);
loadplugin loadplugin_ptr = (loadplugin)loadplugin_addr;

void init_instance(){
    log_printf("Hello from init!");
}

void release_instance(){
    log_printf("Hello from release!");
}

void update_frame(){
    log_printf("Hello from update!");
}

void render_preproc(){
    log_printf("Hello from preproc");
}

void render(){
    log_printf("Hello from render");
}

void my_init(){
    init_ptr();
    loadplugin_ptr("test");
}

// Initialization code shared by th155r and thcrap use
// Executes before the start of the process
void common_init() {
#ifndef NDEBUG
    Debug();
#endif
    patch_allocman();

    patch_netplay();
    hotpatch_rel32(sq_vm_init_call_addrA, my_sq_vm_init);
    //hotpatch_rel32(init_call_addr,my_init);

    //hotpatch_rel32(sq_vm_init_call_addrB, my_sq_vm_init); //not sure why its called twice but pretty sure the first call is enough

    //patch_autopunch();
    
    //hotpatch_import(CreateFileA_import_addr,my_createfileA);

    //hotpatch_import(ReadFile_import_addr,my_readfile);

    hotpatch_rel32(patch_act_script_plugin_hook_addr, &patch_act_script_plugin<base_address, 0x12DDD0>);
}

void yes_tampering() {
    hotpatch_ret(0x12E820_R, 0);
    hotpatch_ret(0x130630_R, 0);
    hotpatch_ret(0x132AF0_R, 0);
}

extern "C" {
    // FUNCTION REQUIRED FOR THE LIBRARY
    // th155r init function
    dll_export int stdcall netcode_init(int32_t param) {
        yes_tampering();
        common_init();
        patch_file_loading();
        return 0;
    }
    
    // thcrap init function
    // Thcrap already removes the tamper protection,
    // so that code is unnecessary to include here.
    dll_export void cdecl netcode_mod_init(void* param) {
        common_init();
    }
    
    // thcrap plugin init
    dll_export int stdcall thcrap_plugin_init() {
        if (HMODULE thcrap_handle = GetModuleHandleA("thcrap.dll")) {
            if (auto runconfig_game_get = (const char*(*)())GetProcAddress(thcrap_handle, "runconfig_game_get")) {
                const char* game_str = runconfig_game_get();
                if (game_str && !strcmp(game_str, "th155")) {
                    if (printf_t* log_func = (printf_t*)GetProcAddress(thcrap_handle, "log_printf")) {
                        log_printf = log_func;
                    }
                    //if (patchhook_register_t* patchhook_register_func = (patchhook_register_t*)GetProcAddress(thcrap_handle, "patchhook_register")) {
                        //patchhook_register = patchhook_register_func;
                    //}
                    return 0;
                }
            }
        }
        return 1;
    }
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
    if (dwReason == DLL_PROCESS_DETACH) {
        Cleanup();
    }
    return TRUE;
}