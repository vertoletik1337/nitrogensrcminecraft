#include "jni.h"
#include <windows.h>
#include <iostream>
#include <math.h>

#pragma comment(lib, "jvm.lib")

JavaVM* g_jvm = nullptr;

// Консоль для отладки
void CreateConsole() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "--- Nitrogen Debug Console ---" << std::endl;
}

void HackThread(HMODULE hModule) {
    CreateConsole();
    
    if (JNI_GetCreatedJavaVMs(&g_jvm, 1, nullptr) != JNI_OK) {
        std::cout << "[ERROR] Could not find Java VM!" << std::endl;
        return;
    }

    JNIEnv* env;
    g_jvm->AttachCurrentThread((void**)&env, nullptr);
    std::cout << "[SUCCESS] Attached to JVM Thread" << std::endl;

    // Ищем классы (Mojang Mappings 1.20.1)
    jclass mcClass = env->FindClass("net/minecraft/client/Minecraft");
    if (!mcClass) std::cout << "[ERROR] Minecraft class not found!" << std::endl;

    jclass entityClass = env->FindClass("net/minecraft/world/entity/Entity");
    if (!entityClass) std::cout << "[ERROR] Entity class not found!" << std::endl;

    float currentHitbox = 0.6f;

    while (!(GetAsyncKeyState(VK_END))) {
        // Получаем Instance
        jmethodID getInstance = env->GetStaticMethodID(mcClass, "m_91087_", "()Lnet/minecraft/client/Minecraft;");
        jobject mc = env->CallStaticObjectMethod(mcClass, getInstance);

        if (mc) {
            // Получаем игрока
            jfieldID playerField = env->GetFieldID(mcClass, "f_91074_", "Lnet/minecraft/client/player/LocalPlayer;");
            jobject localPlayer = env->GetObjectField(mc, playerField);

            if (localPlayer) {
                // Хитбоксы на стрелки
                if (GetAsyncKeyState(VK_UP) & 1) {
                    currentHitbox += 0.2f;
                    std::cout << "[DEBUG] Hitbox expanded to: " << currentHitbox << std::endl;
                }
                if (GetAsyncKeyState(VK_DOWN) & 1) {
                    currentHitbox -= 0.2f;
                    std::cout << "[DEBUG] Hitbox shrinked to: " << currentHitbox << std::endl;
                }

                // Логика Аима/Триггера на R
                if (GetAsyncKeyState('R')) {
                    std::cout << "[DEBUG] 'R' pressed - Scanning entities..." << std::endl;
                    // Сюда добавим перебор сущностей, когда поймем, что база работает
                }
            } else {
                // Если ты в главном меню, игрока не будет - это нормально
                static bool msgShown = false;
                if (!msgShown) { std::cout << "[INFO] Waiting for player to join world..." << std::endl; msgShown = true; }
            }
        }
        
        Sleep(50); // Увеличил задержку, чтобы майн точно не вис
    }

    std::cout << "Exiting..." << std::endl;
    g_jvm->DetachCurrentThread();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE h, DWORD r, LPVOID lp) {
    if (r == DLL_PROCESS_ATTACH) CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, h, 0, 0);
    return TRUE;
}
