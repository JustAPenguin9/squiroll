#include <squirrel.h>

#include "util.h"
#include "AllocMan.h"
#include "Autopunch.h"
#include "PatchUtils.h"
#include "log.h"

const uintptr_t base_address = (uintptr_t)GetModuleHandleA(NULL);

//SQVM Rx4DACE4 initialized at Rx124710
HSQUIRRELVM* VM;

void GetSqVM(){
    VM = (HSQUIRRELVM*)0x4DACE4_R; 
}

void Cleanup() {
    autopunch_cleanup();
}

void Debug() {
    AllocConsole();
    (void)freopen("CONIN$","r",stdin);
    (void)freopen("CONOUT$","w",stdout);
    (void)freopen("CONOUT$","w",stderr);
    SetConsoleTitleW(L"th155r debug");
}

#define sq_vm_malloc_call_addr (0x186745_R)
#define sq_vm_realloc_call_addr (0x18675A_R)
#define sq_vm_free_call_addr (0x186737_R)

#define malloc_base_addr (0x312D61_R)
#define calloc_base_addr (0x3122EA_R)
#define realloc_base_addr (0x312DAF_R)
#define free_base_addr (0x312347_R)

#define WSASend_import_addr (0x3884D0_R)
#define WSASendTo_import_addr (0x3884D4_R)
#define WSARecvFrom_import_addr (0x3884D8_R)
#define bind_import_addr (0x3884DC_R)
#define closesocket_import_addr (0x388514_R)

void patch_autopunch() {
    //hotpatch_import((void*)recvfrom_addr,(void*)my_recvfrom);
    hotpatch_import((void*)WSASendTo_import_addr,(void*)my_sendto);
    hotpatch_import((void*)WSASend_import_addr,(void*)my_send);
    hotpatch_import((void*)bind_import_addr,(void*)my_bind);//crashing when connecting/spectating 
    hotpatch_import((void*)closesocket_import_addr,(void*)my_closesocket);

    autopunch_init();
}

void patch_allocman() {
#if ALLOCATION_PATCH_TYPE == PATCH_SQUIRREL_ALLOCS
    hotpatch_rel32((void*)sq_vm_malloc_call_addr, (void*)my_malloc);
    hotpatch_rel32((void*)sq_vm_realloc_call_addr, (void*)my_realloc);
    hotpatch_rel32((void*)sq_vm_free_call_addr, (void*)my_free);
#elif ALLOCATION_PATCH_TYPE == PATCH_ALL_ALLOCS
    hotpatch_jump((void*)malloc_base_addr, (void*)my_malloc);
    hotpatch_jump((void*)calloc_base_addr, (void*)my_calloc);
    hotpatch_jump((void*)realloc_base_addr, (void*)my_realloc);
    hotpatch_jump((void*)free_base_addr, (void*)my_free);
#endif
}

// Initialization code shared by th155r and thcrap use
// Executes before the start of the process
void common_init() {
#ifndef NDEBUG
    Debug();
#endif
    //patch_allocman();
    patch_autopunch();

}

void yes_tampering() {
    hotpatch_ret((void*)0x12E820_R, 0);
    hotpatch_ret((void*)0x130630_R, 0);
    hotpatch_ret((void*)0x132AF0_R, 0);
}

extern "C" {
    // FUNCTION REQUIRED FOR THE LIBRARY
    // th155r init function
    dll_export int stdcall netcode_init(int32_t param) {
        yes_tampering();
        common_init();
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