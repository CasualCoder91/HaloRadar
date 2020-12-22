#include "framework.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "Dxguid.lib")

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "detours.h"
#pragma comment(lib, "detours.lib")

#include "EntityEx.h"
#include "CameraEx.h"
#include "D3D9Helper.h"

typedef LRESULT(__stdcall* wndProc)(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);
wndProc pWndProc = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//HRESULT APIENTRY hkDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);


HINSTANCE DllHandle;

//Endsceene Hooking
typedef HRESULT(__stdcall* endScene)(IDirect3DDevice9* pDevice);
endScene pEndScene = nullptr;

D3D9Helper d3D9Helper;

//Menu
bool showMenu = false;

//Radar
bool showRadar = false;
float scale = 1;
#define IM_MAX(A, B) (((A) >= (B)) ? (A) : (B))
struct CustomConstraints
{
    // Helper functions to demonstrate programmatic constraints
    static void Square(ImGuiSizeCallbackData* data) { data->DesiredSize.x = data->DesiredSize.y = IM_MAX(data->DesiredSize.x, data->DesiredSize.y); }
    static void Step(ImGuiSizeCallbackData* data) { float step = (float)(int)(intptr_t)data->UserData; data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step); }
};

//ESP
bool drawESP = false;
std::vector<EntityEx> entities;
uintptr_t pObjectTableBase = 0x400506EC;
uintptr_t pMasterChief = 0x40102B04;
BYTE nEntitesOffset = 0x08;
CameraEx cameraEx = CameraEx();

int keyPressed(int key) {
    return (GetAsyncKeyState(key) & 0x8000 != 0);
}

bool callbackLBUP = false;
LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    //std::cout << "TEST" << std::endl;
    //ImGuiIO& io = ImGui::GetIO();
    //io.MouseDrawCursor = true;
    //io.WantCaptureMouse = true;

    //if (callbackLBUP) {
    //    io.MouseDown[0] = false;
    //    io.MouseReleased[0] = true;
    //    callbackLBUP = false;
    //}
    //if (keyPressed(VK_LBUTTON)) {
    //    io.MouseDown[0] = true;
    //    callbackLBUP = true;
    //}
    //if (keyPressed(VK_RBUTTON)) {
    //    io.MouseDown[0] = true;
    //}
    //ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    //return true;
    //ImGuiIO& io = ImGui::GetIO();
    //if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
    //    std::cout << "TEST" << std::endl;
    //    return true;
    //}

    //ImGuiIO& io = ImGui::GetIO();
    //POINT mPos;
    //GetCursorPos(&mPos);
    //ScreenToClient(d3D9Helper.window, &mPos);
    //ImGui::GetIO().MousePos.x = mPos.x;
    //ImGui::GetIO().MousePos.y = mPos.y;

    //if (uMsg == WM_KEYUP)
    //{
    //    if (wParam == VK_DELETE)
    //    {
    //        g_ShowMenu = !g_ShowMenu;
    //    }

    //}

    //if (g_ShowMenu)
    //{
    //    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    //    return true;
    //}



    //if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
    //    //std::cout << "TEST" << std::endl;
    //    return true;
    //}
    //std::cout << "Test2" << std::endl;
    return CallWindowProc(pWndProc, hWnd, uMsg, wParam, lParam);
    //if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    //    return true;
    //return pWndProc(hWnd, uMsg, wParam, lParam);
}

std::vector<EntityEx> loadEntities() {
    //int failCounter = 0;
    std::vector<EntityEx> entities;
    //int nEntites = *(__int16*)(objectTableBase - nEntitesOffset);
    //std::cout << "#Entites according to memory " << nEntites << std::endl;
    for (int i = 0; i < 2048; i++) {
        uintptr_t pointer = *(uintptr_t*)(pObjectTableBase + 0x08 + i * 12);
        if (pointer && pointer > 0x400506EC) {
            //failCounter = 0;
            Entity* entity = (Entity*)pointer;
            __int16 typeID = *(__int16*)(pObjectTableBase + 0x08 + i * 12 - 0x02);
            //std::cout << std::hex << pointer << std::endl;
            if(entity->health > 0)
                entities.emplace_back(entity, typeID);
        }
        //else {
        //    failCounter++;
        //}
        //if (failCounter > 20)
        //    return entities;
    }
    //std::cout << "#Entites found: " << std::dec << entities.size() << std::endl;
    return entities;
}

bool init = false;
int x = 0;
int y = 0;

LPDIRECT3DTEXTURE9 combatFlood, flood, jackal;
HRESULT __stdcall hookedEndScene(IDirect3DDevice9* pDevice) {
    d3D9Helper.pDevice = pDevice;

    //d3D9Helper.drawFilledRectangle(menuX, menuY, menuWidth, menuHeight, D3DCOLOR_ARGB(120, 54, 162, 255));
    //for (size_t i = 0; i < lines.size();++i) {
    //    d3D9Helper.drawText(lines.at(i), menuX + padding, menuY + padding + i * lineHeight, D3DCOLOR_ARGB(255, 153, 255, 153));
    //}

    if (drawESP || showRadar) {
        entities = loadEntities();
    }

    //ESP
    if (drawESP) {
        for (EntityEx entityEx : entities) {
            if (entityEx.entity) {
                Vector3 feetCoords = cameraEx.WorldToScreen(entityEx.entity->feet);
                if (feetCoords.z < 100) {//screenCoords.x > 0 && screenCoords.y > 0 && screenCoords.x < 1 && screenCoords.y < 1 && 
                    //std::cout << screenCoords.x << ", " << screenCoords.y << ", " << screenCoords.z << std::endl;
                    Vector3 torsoCoords = cameraEx.WorldToScreen(entityEx.entity->torso);
                    float heightEntity = abs(feetCoords.y - torsoCoords.y)  * 2;
                    //text
                    std::string str = "health: " + std::to_string((int)(entityEx.entity->health * 100));
                    str = "typeID: " + std::to_string(entityEx.typeID);
                    d3D9Helper.drawText(str, torsoCoords.x -20, torsoCoords.y - heightEntity / 1.2, D3DCOLOR_ARGB(255, 153, 255, 153));
                    //box
                    D3DCOLOR boxColor = D3DCOLOR_ARGB(255, 255, 0, 0); //red
                    if (entityEx.typeID == 3680)
                        boxColor = D3DCOLOR_ARGB(255, 0, 255, 0); //green
                    float widthEntity = heightEntity / 2;
                    d3D9Helper.drawRectangle(feetCoords.x - widthEntity/2, torsoCoords.y  - heightEntity / 2, widthEntity, heightEntity, boxColor);
                }
            }
        }
    }

    //ImGui
    if (!init)
    {
        d3D9Helper.InitImGui(pDevice);
        ImGui::GetIO().ImeWindowHandle = FindWindowA(NULL, "Halo");
        std::cout << ImGui::GetIO().WantCaptureMouse << std::endl;
        std::cout << ImGui::GetIO().WantTextInput << std::endl;
        std::cout << ImGui::GetIO().WantCaptureKeyboard << std::endl;
        HWND  window = FindWindowA(NULL, "Halo");
        std::cout << window << std::endl;
        RECT rect;
        if (GetWindowRect(window, &rect))
        {
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            std::cout << "width" << width << std::endl;
            std::cout << "height" << height << std::endl;
        }

        //pWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);

        //load Textures:
        HRESULT res = D3DXCreateTextureFromFile(pDevice, "combatFlood.png", &combatFlood);
        if (res != D3D_OK)
            std::cout << "Error loading Img: " << res << std::endl;
        res = D3DXCreateTextureFromFile(pDevice, "flood.png", &flood);
        if (res != D3D_OK)
            std::cout << "Error loading Img: " << res << std::endl;
        res = D3DXCreateTextureFromFile(pDevice, "jackal.png", &flood);
        if (res != D3D_OK)
            std::cout << "Error loading Img: " << res << std::endl;

        init = true;
    }


    //ImGui Main Loop
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = false;
    if (showMenu || showRadar) {

        if (showMenu) {
            x = *(int*)(0x00718F84);
            y = *(int*)(0x00718F88);
            io.WantSetMousePos = true;
            io.MousePos.x = x * cameraEx.windowWidth / 640;
            io.MousePos.y = y * cameraEx.windowHeigth / 480;
            io.MouseDrawCursor = true;
            SetCursor(NULL);
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (showMenu) {
            ImGui::SetNextWindowPos(ImVec2(100, 200), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);

            ImGui::Begin("Halo CE Trainer",&showMenu);
            ImGui::Text("Options:");
            ImGui::Checkbox("ESP", &drawESP);
            ImGui::Checkbox("Radar", &showRadar);
            //ImGui::Button("test");
            //ImGui::Button("test");
            //ImGui::Button("test");
            //ImGui::ShowDemoWindow();
            ImGui::End();
        }

#pragma region Draw Radar
        if (showRadar) {


            ImGui::SetNextWindowPos(ImVec2(400, 200), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);

            ImGui::Begin("Radar", &showRadar);
            ImGui::DragFloat("Scale", &scale, 0.005f, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_None);

                ImGui::BeginChild("ChildL");
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
                if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
                if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
                ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
                ImVec2 canvas_midpoint = ImVec2(canvas_p0.x + canvas_sz.x / 2, canvas_p0.y + canvas_sz.y / 2);

                // Draw border and background color
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
                draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

                //circles
                int nCircles = 10;
                for (int i = 1; i < nCircles; i++) {
                    draw_list->AddCircle(canvas_midpoint, canvas_sz.x / (2*nCircles) * i, ImColor(51, 255, 0));
                }

                //Draw Points on radar
                for (EntityEx entityEx : entities) {
                    if (entityEx.entity) {
                        Vector3 coords = cameraEx.WorldToRadar(entityEx.entity->feet, scale);
                        ImVec2 coords2d = ImVec2(coords.x + canvas_midpoint.x, coords.y + canvas_midpoint.y);
                        std::string str = "health: " + std::to_string((int)(entityEx.entity->health * 100));
                        //str = "typeID: " + std::to_string(entity.typeID);
                        //d3D9Helper.drawText(str, torsoCoords.x - 20, torsoCoords.y - heightEntity / 1.2, D3DCOLOR_ARGB(255, 153, 255, 153));
                        //box
                        //D3DCOLOR boxColor = D3DCOLOR_ARGB(255, 255, 0, 0); //red
                        if (entityEx.typeID == 3680) {//marine
                            draw_list->AddCircleFilled(coords2d, 5, ImColor(0, 0, 255));//blue
                        }
                        else if (entityEx.typeID == 6000) { //Combat Flood
                            //draw_list->AddCircleFilled(coords2d, 5, ImColor(153, 255, 153));//bright green     
                            ImVec2 min = ImVec2(coords2d.x - 15*scale, coords2d.y - 19 * scale);
                            ImVec2 max = ImVec2(coords2d.x + 15 * scale, coords2d.y + 19 * scale);
                            draw_list->AddImage((void*)combatFlood, min, max);
                        }
                        else if (entityEx.typeID == 2868) { //Infection Flood
                            ImVec2 min = ImVec2(coords2d.x - 15 * scale, coords2d.y - 15 * scale);
                            ImVec2 max = ImVec2(coords2d.x + 15 * scale, coords2d.y + 15 * scale);
                            draw_list->AddImage((void*)flood, min, max);
                        }
                        else if (entityEx.typeID == 4492) { //Jackal (Shield Guy)
                            ImVec2 min = ImVec2(coords2d.x - 19 * scale, coords2d.y - 15 * scale);
                            ImVec2 max = ImVec2(coords2d.x + 19 * scale, coords2d.y + 15 * scale);
                            draw_list->AddImage((void*)jackal, min, max);
                        }

                        else {
                            draw_list->AddCircleFilled(coords2d, 5, ImColor(255, 0, 0));//red
                        }
                        //d3D9Helper.drawRectangle(feetCoords.x - widthEntity / 2, torsoCoords.y - heightEntity / 2, widthEntity, heightEntity, boxColor);
                    }
                }
                ImGui::EndChild();

            ImGui::End();
        }
#pragma endregion

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }


    return pEndScene(pDevice); // call original endScene 
}

DWORD __stdcall EjectThread(LPVOID lpParameter) {
    Sleep(100);
    FreeLibraryAndExitThread(DllHandle, 0);
    return 0;
}

void* HookVTableFunction(void* pVTable, void* fnHookFunc, int nOffset)
{
    intptr_t ptrVtable = *((intptr_t*)pVTable); // Pointer to our chosen vtable
    intptr_t ptrFunction = ptrVtable + sizeof(intptr_t) * nOffset; // The offset to the function (remember it's a zero indexed array with a size of four bytes)
    intptr_t ptrOriginal = *((intptr_t*)ptrFunction); // Save original address

    // Edit the memory protection so we can modify it
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery((LPCVOID)ptrFunction, &mbi, sizeof(mbi));
    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);

    // Overwrite the old function with our new one
    *((intptr_t*)ptrFunction) = (intptr_t)fnHookFunc;

    // Restore the protection
    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);

    // Return the original function address incase we want to call it
    return (void*)ptrOriginal;
}

typedef HRESULT(WINAPI* IDirectInputDevice_GetDeviceData_t)(IDirectInputDevice8*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
IDirectInputDevice_GetDeviceData_t pGetDeviceData = NULL;

HRESULT WINAPI HookGetDeviceData(IDirectInputDevice8* pThis, DWORD a, LPDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d)
{
    HRESULT ret = pGetDeviceData(pThis, a, b, c, d);
    if (ret == DI_OK)
    {
        for (DWORD i = 0; i < *c; i++)
        {
            if (LOBYTE(b[i].dwData) > 0)
            {
                std::cout << b[i].dwOfs << std::endl;
  /*              switch (b[i].dwOfs)
                {
                case DIK_W:
                    printf("[W]");
                    break;
                case DIK_S:
                    printf("[S]");
                    break;
                case DIK_A:
                    printf("[A]");
                    break;
                case DIK_D:
                    printf("[D]");
                    break;
                }*/
            }
        }
    }
    return ret;
}


typedef HRESULT(__stdcall* GetDeviceState_t)(IDirectInputDevice8A* device, DWORD cbData, LPVOID* lpvdata);
GetDeviceState_t pGetDeviceState;

HRESULT __stdcall hkGetDeviceState(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID* lpvData);
HRESULT __stdcall hkGetDeviceState(IDirectInputDevice8A* lpDevice, DWORD cbData, LPVOID* lpvData)
{
    HRESULT ret = pGetDeviceState(lpDevice, cbData, lpvData);

    ImGuiIO& io = ImGui::GetIO();

    if (ret == DI_OK){
        if (cbData == sizeof(DIMOUSESTATE)){

            if (((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] != 0) {
                io.MouseDown[0] = true;
                std::cout << "[LMB]" << std::endl;
            }
            else {
                io.MouseDown[0] = false;
            }

            if (((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] != 0) {
                std::cout << "[RMB]" << std::endl;
                io.MouseDown[1] = true;
            }
            else{
                io.MouseDown[1] = false;
            }
        }
        if (cbData == sizeof(DIMOUSESTATE2)) {
            if (((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] != 0) {
                std::cout << "[LMB2]" << std::endl;
                io.MouseDown[0] = true;
            }
            else {
                io.MouseDown[0] = false;
            }

            if (((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] != 0) {
                std::cout << "[RMB2]" << std::endl;
                io.MouseDown[1] = true;
            }
            else {
                io.MouseDown[1] = false;
            }
            //std::cout << ((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] << ",";
            //std::cout << ((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] << ",";
            //std::cout << ((LPDIMOUSESTATE2)lpvData)->rgbButtons[2] << ",";
            //std::cout << ((LPDIMOUSESTATE2)lpvData)->rgbButtons[3] << std::endl;
        }
        if (showMenu) { //Block mouse clicks in game if menu is shown
            ((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] = (BYTE)0;
            ((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] = (BYTE)0;
        }

    }

    return ret; //Returning instantly to fix that the game doesn't recognize any physical input of my keyboard anymore. Anyways this wont work. The game still doesnt recognize any input.
}

DWORD WINAPI Menue(HINSTANCE hModule) {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout); //sets cout to be used with our newly created console

    if (!d3D9Helper.initVTable()) {
        std::cout << "could not init d3D9Helper exiting!" << std::endl;
        Sleep(1000);
        fclose(fp);
        FreeConsole();
        CreateThread(0, 0, EjectThread, 0, 0, 0);
        return 0;
    }

    IDirectInput8* pDirectInput = NULL;
    HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
    if (DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&pDirectInput, NULL) != DI_OK)
    {
        printf("[-] Failed to acquire DirectInput handle\n");
        return -1;
    }
    CloseHandle(hInst);

    LPDIRECTINPUTDEVICE8 lpdiMouse;

    if (pDirectInput->CreateDevice(GUID_SysMouse, &lpdiMouse, NULL) != DI_OK)
    {
        pDirectInput->Release();
        return -1;
    }

    //pGetDeviceState = (GetDeviceState_t)HookVTableFunction(lpdiMouse, hkGetDeviceState, 9);
    //fnGetDeviceData = (IDirectInputDevice_GetDeviceData_t)HookVTableFunction(lpdiMouse, HookGetDeviceData, 10);

    //pGetDeviceState = (GetDeviceState_t)DetourFunction((PBYTE) (*reinterpret_cast<void***>(lpdiMouse))[9], (PBYTE)hkGetDeviceState);
    //pWndProc = (wndProc)DetourFunction((PBYTE)&DefWindowProcW, (PBYTE)hookedhWndProc);



    pEndScene = (endScene)DetourFunction((PBYTE)d3D9Helper.vTable[42], (PBYTE)hookedEndScene);
    pGetDeviceState = (GetDeviceState_t)DetourFunction((PBYTE)(*reinterpret_cast<void***>(lpdiMouse))[9], (PBYTE)hkGetDeviceState);
    pGetDeviceData = (IDirectInputDevice_GetDeviceData_t)DetourFunction((PBYTE)(*reinterpret_cast<void***>(lpdiMouse))[10], (PBYTE)HookGetDeviceData);

    while (true) {
        Sleep(50);
        if (GetAsyncKeyState(VK_NUMPAD0)) {
            showMenu = !showMenu;
            Sleep(1000);
        }
        if (GetAsyncKeyState(VK_NUMPAD1)) {
            pDirectInput->Release();
            lpdiMouse->Release();
            DetourRemove((PBYTE)pEndScene, (PBYTE)hookedEndScene);
            DetourRemove((PBYTE)pGetDeviceState, (PBYTE)hkGetDeviceState);
            DetourRemove((PBYTE)pGetDeviceData, (PBYTE)HookGetDeviceData);

            d3D9Helper.cleanup();
            break;
        }
    }
    std::cout << "ight imma head out" << std::endl;
    Sleep(1000);
    fclose(fp);
    FreeConsole();
    CreateThread(0, 0, EjectThread, 0, 0, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DllHandle = hModule;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Menue, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

