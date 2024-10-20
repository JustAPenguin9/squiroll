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


// extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// LRESULT CALLBACK imgui_wndproc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam){
//     if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))return true;
//     return ((WNDPROC)0x23A70_R)(hWnd, msg, wParam, lParam);
// }

void test(){
}

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