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

// Nomor fitur dihitung otomatis mulai dari 0 (urut sesuai daftar di bawah).

jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            OBFUSCATE("RadioButton_Kapasitas gudang_Normal,98304"),
            OBFUSCATE("RadioButton_Bahan pabrik_Normal,Hemat 75%,Hemat 69%"),
            OBFUSCATE("RadioButton_Produksi pabrik_Normal,2x,5x,10x,20x,50x"),
            OBFUSCATE("RadioButton_Penambang_Normal,2x,3x,5x,10x"),
            OBFUSCATE("RadioButton_Gerobak_Normal,2x,3x,4x,5x"),
            OBFUSCATE("RadioButton_Kargo kecepatan_Normal,2x,3x,4x,5x"),
            OBFUSCATE("RadioButton_Kargo kapasitas_Normal,1000,5000,10000")
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

// Target main lib
#define targetLibName OBFUSCATE("libil2cpp.so")

// Nilai pertama RadioButton: 1 = pilihan pertama ("Normal") bernilai 1.
// Kalau di game ternyata pilihannya bergeser, ganti jadi 0.
#define RADIO_FIRST 1

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jlong Lvalue, jboolean boolean, jstring text) {
    switch (featNum) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            // value - RADIO_FIRST = 0 untuk "Normal"; dikurangi 1 lagi supaya Normal = -1
            dc::applyFeature(featNum, value - RADIO_FIRST - 1);
            break;
    }
}

// Jalankan di thread baru supaya tidak memblokir thread utama game
void hack_thread() {
    // libil2cpp.so tidak langsung dimuat, jadi tunggu dulu
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1);
    }
    LOGI(OBFUSCATE("Done"));
}

__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
