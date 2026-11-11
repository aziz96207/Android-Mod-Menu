#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"
#include "dobby.h"
#include "DeepCorp.h"

int scoreMul = 1, coinsMul = 1;

// Do not change or translate the first text unless you know what you are doing
// Assigning feature numbers is optional. Without it, it will automatically count for you, starting from 0
// Assigned feature numbers can be like any numbers 1,3,200,10... instead in order 0,1,2,3,4,5...
// ButtonLink, Category, RichTextView and RichWebView is not counted. They can't have feature number assigned
// Toggle, ButtonOnOff and Checkbox can be switched on by default, if you add True_. Example: CheckBox_True_The Check Box
// To learn HTML, go to this page: https://www.w3schools.com/

jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            OBFUSCATE("RadioButton_Kapasitas gudang_Normal,98304",
"RadioButton_Bahan pabrik_Normal,Hemat 75%,Hemat 69%",
"RadioButton_Produksi pabrik_Normal,2x,5x,10x,20x,50x",
"RadioButton_Penambang_Normal,2x,3x,5x,10x",
"RadioButton_Gerobak_Normal,2x,3x,4x,5x",
"RadioButton_Kargo kecepatan_Normal,2x,3x,4x,5x",
"RadioButton_Kargo kapasitas_Normal,1000,5000,10000")
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

bool btnPressed = false;

//Target main lib here
#define targetLibName OBFUSCATE("libil2cpp.so")

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jlong Lvalue, jboolean boolean, jstring text) {

    switch (featNum) {const int RADIO_FIRST = 1; // ganti 0 kalau pilihan pertama bernilai 0
case 0: case 1: case 2: case 3: case 4: case 5: case 6:
    dc::applyFeature(featNum, value - RADIO_FIRST - 1);
    break;
        cabreak;
    }
}

//CharacterPlayer
void (*StartInvcibility)(void *instance, float duration);

void (*old_Update)(void *instance);
void Update(void *instance) {
    if (instance != nullptr) {
        if (btnPressed) {
            StartInvcibility(instance, 30);
            btnPressed = false;
        }
    }
    return old_Update(instance);
}

/*
 void (*old_AddScore)(void *instance, int score);
 void AddScore(void *instance, int score) {
    //default any actions
    return old_AddScore(instance, score * scoreMul);
 }
*/
// === This function was completely replaced with `install_hook_name` from dobby.h ===
// (base name, return type, ... args)
install_hook_name(AddScore, void *, void *instance, int score) {
    // default any actions

    // use orig_ for call original function
    return orig_AddScore(instance, score + scoreMul);
}

void (*old_AddCoins)(void *instance, int count);
void AddCoins(void *instance, int count) {
    return old_AddCoins(instance, count * coinsMul);
}


// we will run our hacks in a new thread so our while loop doesn't block process main thread
void hack_thread() {
    // This loop should be always enabled in unity game
    // because libil2cpp.so is not loaded into memory immediately.
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1); // Wait for target lib be loaded.
    }

    // In Android Studio, to switch between arm64-v8a and armeabi-v7a syntax highlighting,
    // You can modify the "Active ABI" in "Build Variants" to switch to another architecture for parsing.
#if defined(__aarch64__)
    //Il2Cpp: Use RVA offset
    StartInvcibility = (void (*)(void *, float)) getAbsoluteAddress(targetLibName, OBFUSCATE("0x107A3BC"));
    StartInvcibility = (void (*)(void *, float)) getAbsoluteAddress(targetLibName, OBFUSCATE("_characterPlayer_Update"));

    HOOK(targetLibName, "0x107A2FC", AddCoins, old_AddCoins);

    // HOOK(targetLibName, "0x107A2E0", AddScore, old_AddScore);
    // === This function was completely replaced with super-macro `install_hook_name` from dobby.h ===
    // don't forget set address for install_hook:
    // ! getAbsoluteAddress not have OBFUSCATE, so don't forget use his here
    install_hook_AddScore(getAbsoluteAddress(targetLibName,OBFUSCATE("0x107A2E0")));

    HOOK(targetLibName, "0x1078C44", Update, old_Update);
    //HOOK(targetLibName, "0x1079728", Kill, old_Kill);
    //HOOK(targetLibName, "_example__sym", Kill, old_Kill);
    //HOOK_NO_ORIG("libFileC.so", "0x123456", FunctionExample);
    //HOOK_NO_ORIG("libFileC.so", "_example__sym", FunctionExample);

    //PATCH(targetLibName, "0x10709AC", "E05F40B2C0035FD6");

    INST(targetLibName, "0x23558C", "AnyNameForDetect", true);

    // LOGI(OBFUSCATE("Test SYM: 0x%llx"), (uintptr_t)getAbsoluteAddress(OBFUSCATE("libil2cpp.so"), OBFUSCATE("il2cpp_init")));
#elif defined(__arm__)
    //Put your code here if you want the code to be compiled for armv7 only
#endif

    LOGI(OBFUSCATE("Done"));
}

// Functions with `__attribute__((constructor))` are executed immediately when System.loadLibrary("lib_name") is called.
// If there are multiple such functions at the same time, `constructor(priority)` (the priority is an integer)
// will determine the execution priority, otherwise the execution order is undefined behavior.
__attribute__((constructor))
void lib_main() {
    // Create a new thread so it does not block the main thread, means the game would not freeze
    // In modern C++, you should use std::thread(yourFunction).detach() instead of pthread_create
    std::thread(hack_thread).detach();
}
