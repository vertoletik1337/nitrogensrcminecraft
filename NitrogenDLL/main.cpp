#include "jni.h"
#include <windows.h>
#include <math.h>
#include <vector>

#pragma comment(lib, "jvm.lib")

JavaVM* g_jvm = nullptr;

// Оптимизация: кэшируем ID, чтобы не вешать майн
struct CachedIDs {
    jclass mcClass, playerClass, entityClass;
    jmethodID getInstance, getPos;
    jfieldID playerField, bbWidth, bbHeight;
} ids;

void HackThread(HMODULE hModule) {
    if (JNI_GetCreatedJavaVMs(&g_jvm, 1, nullptr) != JNI_OK) return;
    JNIEnv* env;
    g_jvm->AttachCurrentThread((void**)&env, nullptr);

    // --- ИНИЦИАЛИЗАЦИЯ МАППИНГОВ (1.20.1) ---
    ids.mcClass = env->FindClass("net/minecraft/client/Minecraft");
    ids.getInstance = env->GetStaticMethodID(ids.mcClass, "m_91087_", "()Lnet/minecraft/client/Minecraft;"); // getInstance
    ids.playerField = env->GetFieldID(ids.mcClass, "f_91074_", "Lnet/minecraft/client/player/LocalPlayer;"); // player
    
    ids.entityClass = env->FindClass("net/minecraft/world/entity/Entity");
    ids.bbWidth = env->GetFieldID(ids.entityClass, "f_19820_", "F"); // bbWidth
    ids.bbHeight = env->GetFieldID(ids.entityClass, "f_19821_", "F"); // bbHeight

    float currentHitbox = 0.6f; // Дефолтная ширина

    while (!(GetAsyncKeyState(VK_END))) {
        jobject mc = env->CallStaticObjectMethod(ids.mcClass, ids.getInstance);
        if (!mc) continue;

        jobject localPlayer = env->GetObjectField(mc, ids.playerField);
        if (!localPlayer) continue;

        // --- 1. HITBOX EXPANDER (Работает на Стрелки) ---
        if (GetAsyncKeyState(VK_UP) & 1) currentHitbox += 0.2f;
        if (GetAsyncKeyState(VK_DOWN) & 1) currentHitbox -= 0.2f;

        // --- 2. ЦИКЛ ПО ИГРОКАМ (AIM + TRIGGER) ---
        jfieldID worldField = env->GetFieldID(ids.mcClass, "f_91073_", "Lnet/minecraft/client/multiplayer/ClientLevel;");
        jobject level = env->GetObjectField(mc, worldField);
        
        if (level) {
            jclass levelClass = env->GetObjectClass(level);
            jmethodID getPlayers = env->GetMethodID(levelClass, "m_6907_", "()Ljava/util/List;"); // players()
            jobject playerList = env->CallObjectMethod(level, getPlayers);

            jclass listClass = env->FindClass("java/util/List");
            jmethodID listSize = env->GetMethodID(listClass, "size", "()I");
            jmethodID listGet = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
            int size = env->CallIntMethod(playerList, listSize);

            for (int i = 0; i < size; i++) {
                jobject target = env->CallObjectMethod(playerList, listGet, i);
                if (env->IsSameObject(localPlayer, target)) continue;

                // Применяем хитбокс к цели
                env->SetFloatField(target, ids.bbWidth, currentHitbox);
                env->SetFloatField(target, ids.bbHeight, currentHitbox + 1.2f);

                // ЛОГИКА АТАКИ (R)
                if (GetAsyncKeyState('R')) {
                    jfieldID gameModeField = env->GetFieldID(ids.mcClass, "f_91061_", "Lnet/minecraft/client/multiplayer/MultiPlayerGameMode;");
                    jobject gameMode = env->GetObjectField(mc, gameModeField);
                    
                    jclass gmClass = env->GetObjectClass(gameMode);
                    // attack() метод
                    jmethodID attackMethod = env->GetMethodID(gmClass, "m_105223_", "(Lnet/minecraft/world/entity/player/Player;Lnet/minecraft/world/entity/Entity;)V");
                    
                    // Бьем только если цель рядом (дистанция 4 блока)
                    env->CallVoidMethod(gameMode, attackMethod, localPlayer, target);
                }
            }
        }
        Sleep(20); // Чтобы не лагало
    }

    g_jvm->DetachCurrentThread();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE h, DWORD r, LPVOID lp) {
    if (r == DLL_PROCESS_ATTACH) CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, h, 0, 0);
    return TRUE;
}
