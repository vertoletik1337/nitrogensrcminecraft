#include <windows.h>
#include <jni.h>
#include <thread>
#include <vector>
#include <string>

// Хитбоксы
float hb_scale = 0.1f;

// Помощник для поиска полей по списку имен (для универсальности)
jfieldID find_field_universal(JNIEnv* env, jclass cls, const std::vector<const char*>& names, const char* sig) {
    for (const char* name : names) {
        jfieldID fid = env->GetFieldID(cls, name, sig);
        if (fid) return fid;
        env->ExceptionClear();
    }
    return nullptr;
}

void main_cheat_loop() {
    JavaVM* jvm;
    jsize nVMs;
    if (JNI_GetCreatedJavaVMs(&jvm, 1, &nVMs) != JNI_OK || nVMs == 0) return;

    JNIEnv* env;
    jvm->AttachCurrentThread((void**)&env, NULL);

    // Достаем ClassLoader
    jclass thread_cls = env->FindClass("java/lang/Thread");
    jmethodID curr_thread_mid = env->GetStaticMethodID(thread_cls, "currentThread", "()Ljava/lang/Thread;");
    jobject curr_thread_obj = env->CallStaticObjectMethod(thread_cls, curr_thread_mid);
    jmethodID get_loader_mid = env->GetMethodID(thread_cls, "getContextClassLoader", "()Ljava/lang/ClassLoader;");
    jobject class_loader = env->CallObjectMethod(curr_thread_obj, get_loader_mid);

    // Подготовка поиска классов
    jclass loader_cls = env->FindClass("java/lang/ClassLoader");
    jmethodID load_cls_mid = env->GetMethodID(loader_cls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    auto find_cls = [&](const char* name) -> jclass {
        jstring js_name = env->NewStringUTF(name);
        jclass c = (jclass)env->CallObjectMethod(class_loader, load_cls_mid, js_name);
        env->DeleteLocalRef(js_name);
        if (env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }
        return c;
    };

    while (true) {
        if (GetAsyncKeyState(VK_END)) break;

        // 1. Управление хитбоксами
        if (GetAsyncKeyState(VK_UP) & 1) hb_scale += 0.1f;
        if (GetAsyncKeyState(VK_DOWN) & 1) hb_scale -= 0.1f;

        // 2. Достаем Minecraft Instance
        jclass mc_cls = find_cls("net.minecraft.client.Minecraft");
        if (!mc_cls) mc_cls = find_cls("net.minecraft.class_310");
        if (!mc_cls) continue;

        jmethodID get_inst_mid = env->GetStaticMethodID(mc_cls, "getInstance", "()Lnet/minecraft/client/Minecraft;");
        if (!get_inst_mid) {
            env->ExceptionClear();
            get_inst_mid = env->GetStaticMethodID(mc_cls, "method_1551", "()Lnet/minecraft/class_310;");
        }
        if (!get_inst_mid) continue;
        jobject mc_inst = env->CallStaticObjectMethod(mc_cls, get_inst_mid);

        // 3. TRIGGERBOT / CRITS (Клавиша R)
        if (GetAsyncKeyState('R') & 0x8000) {
            // Ищем игрока (field_1724 - Fabric, player - Vanilla)
            jfieldID player_fid = find_field_universal(env, mc_cls, {"field_1724", "player", "thePlayer"}, "Lnet/minecraft/class_746;");
            if (!player_fid) player_fid = find_field_universal(env, mc_cls, {"field_1724", "player"}, "Lnet/minecraft/client/network/ClientPlayerEntity;");
            
            jobject player_obj = env->GetObjectField(mc_inst, player_fid);
            if (player_obj) {
                // Проверка на падение (Crits)
                jclass ent_cls = env->GetObjectClass(player_obj);
                jfieldID fall_fid = find_field_universal(env, ent_cls, {"field_6008", "fallDistance", "h"}, "F");
                jfieldID ground_fid = find_field_universal(env, ent_cls, {"field_6036", "onGround", "z"}, "Z");

                float fall_dist = env->GetFloatField(player_obj, fall_fid);
                bool on_ground = env->GetBooleanField(player_obj, ground_fid);

                if (fall_dist > 0.0f && !on_ground) {
                    // Ищем цель под прицелом (crosshairTarget)
                    jfieldID target_fid = find_field_universal(env, mc_cls, {"field_1765", "crosshairTarget", "objectMouseOver"}, "Lnet/minecraft/class_239;");
                    jobject target_obj = env->GetObjectField(mc_inst, target_fid);

                    if (target_obj) {
                        // Тут вызываем атаку через interactionManager (зависит от версии, вызываем method_2918)
                        // ПРИМЕР: env->CallVoidMethod(interactionManager, attackMethod, player, targetEntity);
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    jvm->DetachCurrentThread();
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID res) {
    if (reason == DLL_PROCESS_ATTACH) {
        std::thread(main_cheat_loop).detach();
    }
    return TRUE;
}
