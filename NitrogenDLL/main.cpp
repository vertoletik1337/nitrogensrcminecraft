#include "jni.h"
#include <windows.h>
#include <math.h>
#include <vector>
#pragma comment(lib, "jvm.lib")
JavaVM* g_jvm = nullptr;

// Структура для хранения позиции
struct Vec3 { double x, y, z; };

// Функция получения расстояния
double GetDist(Vec3 a, Vec3 b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

void HackThread(HMODULE hModule) {
    if (JNI_GetCreatedJavaVMs(&g_jvm, 1, nullptr) != JNI_OK) return;
    JNIEnv* env;
    g_jvm->AttachCurrentThread((void**)&env, nullptr);

    float hitboxExpand = 0.0f;

    while (!(GetAsyncKeyState(VK_END))) {
        // --- HITBOX EXPANDER ---
        if (GetAsyncKeyState(VK_UP) & 1) hitboxExpand += 0.1f;
        if (GetAsyncKeyState(VK_DOWN) & 1) hitboxExpand -= 0.1f;

        // --- AIMBOT + CRITICAL TRIGGERBOT ---
        if (GetAsyncKeyState('R')) {
            jclass mcClass = env->FindClass("net/minecraft/client/Minecraft");
            jmethodID getInstance = env->GetStaticMethodID(mcClass, "getInstance", "()Lnet/minecraft/client/Minecraft;");
            jobject mc = env->CallStaticObjectMethod(mcClass, getInstance);

            // Получаем игрока
            jfieldID playerField = env->GetFieldID(mcClass, "player", "Lnet/minecraft/client/player/LocalPlayer;");
            jobject localPlayer = env->GetObjectField(mc, playerField);

            // Получаем мир
            jfieldID worldField = env->GetFieldID(mcClass, "level", "Lnet/minecraft/client/multiplayer/ClientLevel;");
            jobject level = env->GetObjectField(mc, worldField);

            // Получаем контроллер (для атаки)
            jfieldID conField = env->GetFieldID(mcClass, "gameMode", "Lnet/minecraft/client/multiplayer/MultiPlayerGameMode;");
            jobject gameMode = env->GetObjectField(mc, conField);

            if (localPlayer && level && gameMode) {
                jclass levelClass = env->GetObjectClass(level);
                jmethodID getPlayers = env->GetMethodID(levelClass, "players", "()Ljava/util/List;");
                jobject playerList = env->CallObjectMethod(level, getPlayers);

                jclass listClass = env->FindClass("java/util/List");
                jmethodID listSize = env->GetMethodID(listClass, "size", "()I");
                jmethodID listGet = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
                int size = env->CallIntMethod(playerList, listSize);

                jclass entityClass = env->FindClass("net/minecraft/world/entity/Entity");
                jmethodID getPos = env->GetMethodID(entityClass, "position", "()Lnet/minecraft/world/phys/Vec3;");

                for (int i = 0; i < size; i++) {
                    jobject targetPlayer = env->CallObjectMethod(playerList, listGet, i);
                    if (env->IsSameObject(localPlayer, targetPlayer)) continue;

                    // Логика проверки на падение (Криты)
                    jclass lpClass = env->GetObjectClass(localPlayer);
                    jfieldID fallField = env->GetFieldID(lpClass, "fallDistance", "F");
                    float fallDist = env->GetFloatField(localPlayer, fallField);

                    jmethodID getDelta = env->GetMethodID(lpClass, "getDeltaMovement", "()Lnet/minecraft/world/phys/Vec3;");
                    jobject deltaVec = env->CallObjectMethod(localPlayer, getDelta);
                    jclass vec3Class = env->FindClass("net/minecraft/world/phys/Vec3");
                    jfieldID yField = env->GetFieldID(vec3Class, "y", "D");
                    double dy = env->GetDoubleField(deltaVec, yField);

                    // Условие: падаем вниз
                    if (fallDist > 0.0f && dy < 0.0) {
                        jclass gmClass = env->GetObjectClass(gameMode);
                        jmethodID attackMethod = env->GetMethodID(gmClass, "attack", "(Lnet/minecraft/world/entity/player/Player;Lnet/minecraft/world/entity/Entity;)V");
                        
                        // Прямой удар (Triggerbot)
                        env->CallVoidMethod(gameMode, attackMethod, localPlayer, targetPlayer);
                    }
                }
            }
        }
        Sleep(10);
    }

    g_jvm->DetachCurrentThread();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE h, DWORD r, LPVOID lp) {
    if (r == DLL_PROCESS_ATTACH) CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, h, 0, 0);
    return TRUE;
}
