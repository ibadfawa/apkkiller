#include <stdio.h>
#include <iostream>
#include <vector>
#include <jni.h>
#include <thread>
#include <future>
#include <unordered_map>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <whale.h>

#include "ElfImg.h"
#include "Utils.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "APKKiller", __VA_ARGS__)

#define apk_asset_path "original.apk" // assets/original.apk
#define apk_fake_name "original.apk" // /data/data/<package_name>/cache/original.apk
std::vector<std::vector<uint8_t>> apk_signatures {{0x30,0x82,0x03,0x49,0x30,0x82,0x02,0x31,0xA0,0x03,0x02,0x01,0x02,0x02,0x04,0x44,0x26,0xCB,0x55,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,0x05,0x00,0x30,0x54,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x06,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x08,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x07,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x0A,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x0B,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x03,0x13,0x03,0x70,0x72,0x6F,0x30,0x20,0x17,0x0D,0x32,0x30,0x31,0x32,0x31,0x31,0x30,0x36,0x31,0x36,0x30,0x34,0x5A,0x18,0x0F,0x32,0x31,0x32,0x30,0x31,0x31,0x31,0x37,0x30,0x36,0x31,0x36,0x30,0x34,0x5A,0x30,0x54,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x06,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x08,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x07,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x0A,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x0B,0x13,0x03,0x70,0x72,0x6F,0x31,0x0C,0x30,0x0A,0x06,0x03,0x55,0x04,0x03,0x13,0x03,0x70,0x72,0x6F,0x30,0x82,0x01,0x22,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x82,0x01,0x0F,0x00,0x30,0x82,0x01,0x0A,0x02,0x82,0x01,0x01,0x00,0x82,0xB5,0xE2,0x20,0x08,0x22,0x5B,0x82,0xE8,0x74,0x30,0x3A,0x83,0x19,0xE6,0x4E,0xD3,0xFA,0x73,0x93,0xE2,0x0D,0xCB,0xE3,0x3F,0x99,0xD9,0xC5,0x95,0x70,0xC5,0xD5,0x72,0x12,0x31,0x06,0x54,0x30,0x4A,0x90,0xFB,0xAA,0xD5,0xBD,0xC9,0x2E,0x92,0x16,0xE4,0x99,0x44,0xC5,0xCF,0x98,0x51,0xE0,0x54,0x59,0x2F,0x24,0x65,0x2C,0x31,0x66,0x84,0xB8,0x7F,0x0E,0xE4,0x41,0xCB,0x23,0xC7,0xF1,0x86,0xA8,0x18,0x91,0x07,0x6D,0x3E,0x92,0x59,0x4E,0x36,0x38,0xFC,0xE2,0xA7,0xDD,0xCE,0x4A,0xE3,0xEC,0xBF,0x54,0x1A,0x4C,0xFB,0xFB,0x97,0x76,0xC5,0x30,0x90,0xB8,0xAD,0xE2,0x2F,0x3A,0x03,0x5E,0xA8,0x0D,0x6D,0x46,0xB7,0xC9,0x8A,0x51,0x52,0x8D,0x44,0x49,0xBD,0x19,0x04,0x0B,0x34,0xEB,0xF4,0x23,0x3B,0x0A,0xAD,0x5C,0x3D,0x05,0xC9,0x8A,0x9B,0x18,0x42,0x3F,0xE6,0xE4,0xA5,0xB7,0xE0,0xF1,0xA0,0x1C,0x8A,0xCC,0x03,0x07,0xFB,0xA6,0x24,0xBD,0xB7,0x68,0x1E,0xAC,0xA2,0xC4,0x7C,0x19,0xEF,0x71,0x35,0x2E,0x09,0xDC,0x3F,0x28,0xEE,0x94,0xA9,0xB2,0x0C,0x37,0x25,0x7D,0xB0,0xF7,0x8A,0xC2,0x53,0x65,0xEB,0xFD,0xB9,0xD4,0xD0,0xE0,0x3F,0x61,0x3F,0x9E,0x7D,0x4C,0x45,0xD2,0xB3,0x17,0x26,0x5C,0xC9,0x0B,0x31,0xBB,0x7B,0x4A,0xF9,0x66,0x07,0x10,0x89,0x5C,0xB8,0x70,0xDC,0x35,0xEA,0x9E,0xC8,0xF9,0x88,0x1C,0xD9,0x94,0xF5,0xCB,0xA2,0xB0,0xE1,0x1E,0xB1,0x19,0x6B,0x66,0xDE,0x44,0xA5,0x66,0xD8,0x1C,0x94,0x40,0x53,0x79,0x44,0x15,0x0C,0x29,0x02,0x03,0x01,0x00,0x01,0xA3,0x21,0x30,0x1F,0x30,0x1D,0x06,0x03,0x55,0x1D,0x0E,0x04,0x16,0x04,0x14,0xCE,0x51,0x0D,0xA1,0x8F,0x4D,0xCB,0x5E,0x1B,0xD3,0x9F,0x68,0x68,0x21,0x20,0x9A,0x5F,0x2F,0xB2,0x8E,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,0x05,0x00,0x03,0x82,0x01,0x01,0x00,0x20,0x1E,0xC0,0xFF,0x93,0xBF,0xE2,0x11,0xBB,0x98,0xB5,0x51,0xB6,0xA0,0xD6,0x37,0x4E,0x26,0x66,0x8F,0xED,0x45,0xCD,0xDE,0x72,0x6B,0x4C,0x3C,0x9D,0x82,0x6E,0xAD,0xD4,0x0B,0x24,0x8D,0x84,0xE2,0x97,0xF8,0x32,0x20,0xAA,0x68,0xA4,0x1F,0x28,0x6A,0xA2,0x44,0x53,0x73,0x1A,0x7D,0xF0,0x41,0x2F,0x36,0x42,0x6A,0x94,0x21,0x8C,0xD4,0x5A,0x72,0x9B,0x9D,0x2F,0xD6,0xBC,0xB2,0x52,0xEE,0x31,0xED,0x7A,0x81,0x4B,0x26,0xE8,0x1B,0x57,0x6F,0xDE,0x1B,0x77,0xBD,0x29,0xA8,0xE5,0xE3,0xD1,0xCF,0x9E,0x9B,0xE2,0x6A,0x04,0xB2,0xA9,0xDB,0x53,0xAB,0x98,0x4C,0x2A,0x87,0x07,0xEA,0x09,0x79,0x5C,0x85,0x7B,0x99,0x7C,0x48,0xBB,0xC6,0x9B,0x41,0xF4,0xD5,0x8B,0x60,0x05,0x00,0x94,0x88,0x97,0x5A,0xAE,0xEB,0xA2,0x80,0xBA,0x3F,0x88,0xCB,0xA8,0x01,0x1C,0xBF,0x0E,0xE5,0xE1,0x45,0xE4,0x27,0x9F,0xD8,0x0A,0x2A,0xFE,0x9E,0x2E,0xFE,0x56,0x5A,0x6D,0x64,0x22,0x47,0xEB,0x86,0x63,0x70,0xA9,0xFC,0xBA,0x86,0x76,0x62,0x78,0x2D,0xF5,0x7F,0x75,0xDC,0xB8,0x51,0x6D,0xEC,0xE0,0x66,0x91,0xF2,0x49,0xA9,0x43,0xF3,0xA4,0xEA,0x32,0x34,0xA3,0x86,0xC1,0x1B,0x1E,0x98,0xAB,0x2A,0x7F,0xE1,0x4F,0xA5,0xA3,0xBA,0x14,0x8B,0x13,0x48,0x62,0xDD,0xF5,0x54,0x39,0xB7,0x60,0x03,0x13,0xB4,0xAE,0x6A,0x9D,0x92,0xF1,0xAE,0x9C,0xC2,0xAC,0xCD,0x3A,0xDD,0x53,0x0F,0x66,0x88,0x6F,0xD9,0xE4,0xC8,0xEA,0xFD,0xF4,0xB0,0x2C,0x80,0xB1,0x09,0xA7,0x55,0xF2,0x19}}; // if you fill this, it will ignore the m_APKSign from APKKiller.java

JavaVM *g_vm;
JNIEnv *g_env;
std::string g_pkgName;
jstring g_apkPath;
jobject g_proxy, g_pkgMgr;

const char *getProcessName() {
    FILE *f = fopen("/proc/self/cmdline", "rb");
    if (f) {
        char *buf = new char[64];
        fread(buf, sizeof(char), 64, f);
        fclose(f);
        return buf;
    }
    return 0;
}

std::string getPackageName(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getPackageNameMethod = g_env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    return g_env->GetStringUTFChars((jstring) g_env->CallObjectMethod(obj, getPackageNameMethod), 0);
}

jobject getApplicationContext(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getApplicationContextMethod = g_env->GetMethodID(contextClass, "getApplicationContext", "()Landroid/content/Context;");
    return g_env->CallObjectMethod(obj, getApplicationContextMethod);
}

jobject getPackageManager(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getPackageManagerMethod = g_env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    return g_env->CallObjectMethod(obj, getPackageManagerMethod);
}

class Reference {
public:
    JNIEnv *env;
    jobject reference;
public:
    Reference(JNIEnv *env, jobject obj) {
        this->env = env;
        this->reference = env->NewGlobalRef(obj);
    }

    Reference(jobject reference) {
        this->env = g_env;
        this->reference = reference;
    }

    jobject get() {
        auto referenceClass = env->FindClass("java/lang/ref/Reference");
        auto get = env->GetMethodID(referenceClass, "get", "()Ljava/lang/Object;");
        return env->CallObjectMethod(reference, get);
    }
};

class WeakReference : public Reference {
public:
    WeakReference(JNIEnv *env, jobject weakReference) : Reference(env, weakReference) {
    }

    WeakReference(jobject weakReference) : Reference(weakReference) {
    }

    static jobject Create(jobject obj) {
        auto weakReferenceClass = g_env->FindClass("java/lang/ref/WeakReference");
        auto weakReferenceClassConstructor = g_env->GetMethodID(weakReferenceClass, "<init>", "(Ljava/lang/Object;)V");
        return g_env->NewObject(weakReferenceClass, weakReferenceClassConstructor, obj);
    }
};

class ArrayList {
private:
    JNIEnv *env;
    jobject arrayList;
public:
    ArrayList(JNIEnv *env, jobject arrayList) {
        this->env = env;
        this->arrayList = arrayList;
    }

    ArrayList(jobject arrayList) {
        this->env = g_env;
        this->arrayList = arrayList;
    }

    jobject getObj() {
        return arrayList;
    }

    jobject get(int index) {
        auto arrayListClass = env->FindClass("java/util/ArrayList");
        auto getMethod = env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
        return env->CallObjectMethod(arrayList, getMethod, index);
    }

    void set(int index, jobject value) {
        auto arrayListClass = env->FindClass("java/util/ArrayList");
        auto setMethod = env->GetMethodID(arrayListClass, "set", "(ILjava/lang/Object;)Ljava/lang/Object;");
        env->CallObjectMethod(arrayList, setMethod, index, value);
    }

    int size() {
        auto arrayListClass = env->FindClass("java/util/ArrayList");
        auto sizeMethod = env->GetMethodID(arrayListClass, "size", "()I");
        return env->CallIntMethod(arrayList, sizeMethod);
    }
};

class ArrayMap {
private:
    JNIEnv *env;
    jobject arrayMap;
public:
    ArrayMap(JNIEnv *env, jobject arrayMap) {
        this->env = env;
        this->arrayMap = arrayMap;
    }

    ArrayMap(jobject arrayMap) {
        this->env = g_env;
        this->arrayMap = arrayMap;
    }

    jobject getObj() {
        return arrayMap;
    }

    jobject valueAt(int index) {
        auto arrayMapClass = env->FindClass("android/util/ArrayMap");
        auto valueAtMethod = env->GetMethodID(arrayMapClass, "valueAt", "(I)Ljava/lang/Object;");
        return env->CallObjectMethod(arrayMap, valueAtMethod, index);
    }

    jobject setValueAt(int index, jobject value) {
        auto arrayMapClass = env->FindClass("android/util/ArrayMap");
        auto setValueAtMethod = env->GetMethodID(arrayMapClass, "setValueAt", "(ILjava/lang/Object;)Ljava/lang/Object;");
        return env->CallObjectMethod(arrayMap, setValueAtMethod, index, value);
    }

    int size() {
        auto arrayMapClass = env->FindClass("android/util/ArrayMap");
        auto sizeMethod = env->GetMethodID(arrayMapClass, "size", "()I");
        return env->CallIntMethod(arrayMap, sizeMethod);
    }
};

class Method {
private:
    JNIEnv *env;
    jobject method;
    jmethodID getNameMethod;
    jmethodID invokeMethod;

    void initMethod(jobject method) {
        this->method = method;

        jclass methodClass = env->FindClass("java/lang/reflect/Method");
        getNameMethod = env->GetMethodID(methodClass, "getName", "()Ljava/lang/String;");
        invokeMethod = env->GetMethodID(methodClass, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");

        auto setAccessibleMethod = env->GetMethodID(methodClass, "setAccessible", "(Z)V");
        env->CallVoidMethod(method, setAccessibleMethod, true);
    }

public:
    Method(JNIEnv *env, jobject method) {
        this->env = env;
        initMethod(method);
    }

    Method(jobject method) {
        this->env = g_env;
        initMethod(method);
    }

    ~Method() {
        env->DeleteLocalRef(method);
    }

    const char *getName() {
        return env->GetStringUTFChars((jstring) env->CallObjectMethod(method, getNameMethod), 0);
    }

    jobject invoke(jobject object, jobjectArray args = 0) {
        return env->CallObjectMethod(method, invokeMethod, object, args);
    }
};

class Field {
private:
    JNIEnv *env;
    jobject field;
    jmethodID getMethod;
    jmethodID setMethod;
public:
    Field(JNIEnv *env, jobject field) {
        this->env = env;
        this->field = field;

        jclass fieldClass = env->FindClass("java/lang/reflect/Field");
        getMethod = env->GetMethodID(fieldClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        setMethod = env->GetMethodID(fieldClass, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V");

        auto setAccessibleMethod = env->GetMethodID(fieldClass, "setAccessible", "(Z)V");
        env->CallVoidMethod(field, setAccessibleMethod, true);
    }

    ~Field() {
        env->DeleteGlobalRef(field);
    }

    jobject getField() {
        return field;
    }

    jobject get(jobject obj) {
        return env->CallObjectMethod(field, getMethod, obj);
    }

    void set(jobject obj, jobject value) {
        env->CallVoidMethod(field, setMethod, obj, value);
    }
};

class Class {
private:
    JNIEnv *env;
    jobject clazz;

    void initClass(const char *className) {
        auto classClass = env->FindClass("java/lang/Class");
        auto forNameMethod = env->GetStaticMethodID(classClass, "forName", "(Ljava/lang/String;)Ljava/lang/Class;");
        clazz = env->NewGlobalRef(env->CallStaticObjectMethod(classClass, forNameMethod, env->NewStringUTF(className)));
    }

public:
    Class(JNIEnv *env, const char *className) {
        this->env = env;
        initClass(className);
    }

    Class(const char *className) {
        this->env = g_env;
        initClass(className);
    }

    ~Class() {
        env->DeleteGlobalRef(clazz);
    }

    jobject getClass() {
        return clazz;
    }

    Field getField(const char *fieldName) {
        auto future = std::async([&]() -> jobject {
            JNIEnv *env;
            g_vm->AttachCurrentThread(&env, NULL);

            auto classClass = env->FindClass("java/lang/Class");
            jmethodID getDeclaredFieldMethod = env->GetMethodID(classClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");

            auto fieldNameObj = env->NewStringUTF(fieldName);
            auto result = env->CallObjectMethod(clazz, getDeclaredFieldMethod, fieldNameObj);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }

            if (result) {
                result = env->NewGlobalRef(result);
            }

            env->DeleteLocalRef(fieldNameObj);
            env->DeleteLocalRef(classClass);

            g_vm->DetachCurrentThread();
            return result;
        });
        return Field(env, future.get());
    }
};

void patch_ApplicationInfo(jobject obj) {
    if (obj) {
        LOGI("-------- Patching ApplicationInfo - %p", obj);
        Class applicationInfoClass("android.content.pm.ApplicationInfo");

        auto sourceDirField = applicationInfoClass.getField("sourceDir");
        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

        sourceDirField.set(obj, g_apkPath);
        publicSourceDirField.set(obj, g_apkPath);
    }
}

void patch_LoadedApk(jobject obj) {
    if (obj) {
        LOGI("-------- Patching LoadedApk - %p", obj);
        Class loadedApkClass("android.app.LoadedApk");

        auto mApplicationInfoField = loadedApkClass.getField("mApplicationInfo");
        patch_ApplicationInfo(mApplicationInfoField.get(obj));

        auto mAppDirField = loadedApkClass.getField("mAppDir");
        auto mResDirField = loadedApkClass.getField("mResDir");

        mAppDirField.set(obj, g_apkPath);
        mResDirField.set(obj, g_apkPath);
    }
}

void patch_AppBindData(jobject obj) {
    if (obj) {
        LOGI("-------- Patching AppBindData - %p", obj);
        Class appBindDataClass("android.app.ActivityThread$AppBindData");

        auto infoField = appBindDataClass.getField("info");
        patch_LoadedApk(infoField.get(obj));

        auto appInfoField = appBindDataClass.getField("appInfo");
        patch_ApplicationInfo(appInfoField.get(obj));
    }
}

void patch_ContextImpl(jobject obj) {
    if (obj) {
        LOGI("-------- Patching ContextImpl - %p", obj);
        Class contextImplClass("android.app.ContextImpl");

        auto mPackageInfoField = contextImplClass.getField("mPackageInfo");
        patch_LoadedApk(mPackageInfoField.get(obj));

        auto mPackageManagerField = contextImplClass.getField("mPackageManager");
        mPackageManagerField.set(obj, g_proxy);
    }
}

void patch_Application(jobject obj) {
    if (obj) {
        LOGI("-------- Patching Application - %p", obj);
        Class applicationClass("android.app.Application");

        auto mLoadedApkField = applicationClass.getField("mLoadedApk");
        patch_LoadedApk(mLoadedApkField.get(obj));

        // patch_ContextImpl(getApplicationContext(obj));
    }
}

AAssetManager *g_assetManager;

void extractAsset(std::string assetName, std::string extractPath) {
    LOGI("-------- Extracting %s to %s", assetName.c_str(), extractPath.c_str());
    AAssetManager *assetManager = g_assetManager;
    AAsset *asset = AAssetManager_open(assetManager, assetName.c_str(), AASSET_MODE_UNKNOWN);
    if (!asset) {
        return;
    }

    int fd = open(extractPath.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        AAsset_close(asset);
        return;
    }

    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = AAsset_read(asset, buffer, BUFFER_SIZE)) > 0) {
        int bytesWritten = write(fd, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            AAsset_close(asset);
            close(fd);
            return;
        }
    }

    AAsset_close(asset);
    close(fd);
}

void patch_PackageManager(jobject obj) {
    if (!obj) return;

    Class activityThreadClass("android.app.ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass.getField("sCurrentActivityThread");
    auto sCurrentActivityThread = sCurrentActivityThreadField.get(NULL);

    auto sPackageManagerField = activityThreadClass.getField("sPackageManager");
    g_pkgMgr = g_env->NewGlobalRef(sPackageManagerField.get(NULL));

    Class iPackageManagerClass("android.content.pm.IPackageManager");

    auto classClass = g_env->FindClass("java/lang/Class");
    auto getClassLoaderMethod = g_env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

    auto classLoader = g_env->CallObjectMethod(iPackageManagerClass.getClass(), getClassLoaderMethod);
    auto classArray = g_env->NewObjectArray(1, classClass, NULL);
    g_env->SetObjectArrayElement(classArray, 0, iPackageManagerClass.getClass());

    auto apkKillerClass = g_env->FindClass("com/kuro/APKKiller");
    auto myInvocationHandlerField = g_env->GetStaticFieldID(apkKillerClass, "myInvocationHandler", "Ljava/lang/reflect/InvocationHandler;");
    auto myInvocationHandler = g_env->GetStaticObjectField(apkKillerClass, myInvocationHandlerField);

    auto proxyClass = g_env->FindClass("java/lang/reflect/Proxy");
    auto newProxyInstanceMethod = g_env->GetStaticMethodID(proxyClass, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");
    g_proxy = g_env->NewGlobalRef(g_env->CallStaticObjectMethod(proxyClass, newProxyInstanceMethod, classLoader, classArray, myInvocationHandler));

    sPackageManagerField.set(sCurrentActivityThread, g_proxy);

    auto pm = getPackageManager(obj);
    Class applicationPackageManagerClass("android.app.ApplicationPackageManager");
    auto mPMField = applicationPackageManagerClass.getField("mPM");
    mPMField.set(pm, g_proxy);
}

void APKKill(JNIEnv *env, jclass clazz, jobject context) {
    env->PushLocalFrame(256); // We call this so that we don't need to manually delete the local refs

    g_env = env;
    g_assetManager = AAssetManager_fromJava(env, env->CallObjectMethod(context, env->GetMethodID(env->FindClass("android/content/Context"), "getAssets", "()Landroid/content/res/AssetManager;")));

    std::string apkPkg = getPackageName(context);
    g_pkgName = apkPkg;

    auto procName = getProcessName();
    LOGI("-------- Killing %s", procName);

    char apkDir[512];
    sprintf(apkDir, "/data/data/%s/cache", apkPkg.c_str());
    mkdir(apkDir, 0777);

    std::string apkPath = "/data/data/";
    apkPath += apkPkg;
    apkPath += "/cache/";
    apkPath += apk_fake_name;

    if (access(apkPath.c_str(), F_OK) == -1) {
        extractAsset(apk_asset_path, apkPath);
    }

    g_apkPath = (jstring) env->NewGlobalRef(g_env->NewStringUTF(apkPath.c_str()));

    Class activityThreadClass("android.app.ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass.getField("sCurrentActivityThread");
    auto sCurrentActivityThread = sCurrentActivityThreadField.get(NULL);

    auto mBoundApplicationField = activityThreadClass.getField("mBoundApplication");
    patch_AppBindData(mBoundApplicationField.get(sCurrentActivityThread));

    auto mInitialApplicationField = activityThreadClass.getField("mInitialApplication");
    patch_Application(mInitialApplicationField.get(sCurrentActivityThread));

    auto mAllApplicationsField = activityThreadClass.getField("mAllApplications");
    auto mAllApplications = mAllApplicationsField.get(sCurrentActivityThread);
    ArrayList list(mAllApplications);
    for (int i = 0; i < list.size(); i++) {
        auto application = list.get(i);
        patch_Application(application);
        list.set(i, application);
    }
    mAllApplicationsField.set(sCurrentActivityThread, list.getObj());

    auto mPackagesField = activityThreadClass.getField("mPackages");
    auto mPackages = mPackagesField.get(sCurrentActivityThread);
    ArrayMap mPackagesMap(mPackages);
    for (int i = 0; i < mPackagesMap.size(); i++) {
        WeakReference loadedApk(mPackagesMap.valueAt(i));
        patch_LoadedApk(loadedApk.get());
        mPackagesMap.setValueAt(i, WeakReference::Create(loadedApk.get()));
    }
    mPackagesField.set(sCurrentActivityThread, mPackagesMap.getObj());

    auto mResourcePackagesField = activityThreadClass.getField("mResourcePackages");
    auto mResourcePackages = mResourcePackagesField.get(sCurrentActivityThread);
    ArrayMap mResourcePackagesMap(mResourcePackages);
    for (int i = 0; i < mResourcePackagesMap.size(); i++) {
        WeakReference loadedApk(mResourcePackagesMap.valueAt(i));
        patch_LoadedApk(loadedApk.get());
        mResourcePackagesMap.setValueAt(i, WeakReference::Create(loadedApk.get()));
    }
    mResourcePackagesField.set(sCurrentActivityThread, mResourcePackagesMap.getObj());

    // patch_ContextImpl(getApplicationContext(context));
    patch_PackageManager(context);

    env->PopLocalFrame(0);
}

jobject processInvoke(JNIEnv *env, jclass clazz, jobject method, jobjectArray args) {
    env->PushLocalFrame(256);

    auto Integer_intValue = [&](jobject param) -> int {
        auto integerClass = env->FindClass("java/lang/Integer");
        auto intValueMethod = env->GetMethodID(integerClass, "intValue", "()I");
        return env->CallIntMethod(param, intValueMethod);
    };

    Method mMethod(env, method);
    const char *mName = mMethod.getName();
    auto mResult = mMethod.invoke(g_pkgMgr, args);

    if (!strcmp(mName, "getPackageInfo")) {
        const jobject packageInfo = mResult;
        if (packageInfo) {
            const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), 0);
            int flags = Integer_intValue(env->GetObjectArrayElement(args, 1));
            if (!strcmp(packageName, g_pkgName.c_str())) {
                if ((flags & 0x40) != 0) {
                    Class packageInfoClass(env, "android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass.getField("applicationInfo");
                    auto applicationInfo = applicationInfoField.get(packageInfo);
                    if (applicationInfo) {
                        Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");
                        auto sourceDirField = applicationInfoClass.getField("sourceDir");
                        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                        sourceDirField.set(applicationInfo, g_apkPath);
                        publicSourceDirField.set(applicationInfo, g_apkPath);
                    }
                    applicationInfoField.set(packageInfo, applicationInfo);
                    auto signaturesField = packageInfoClass.getField("signatures");

                    auto signatureClass = env->FindClass("android/content/pm/Signature");
                    auto signatureConstructor = env->GetMethodID(signatureClass, "<init>", "([B)V");
                    auto signatureArray = env->NewObjectArray(apk_signatures.size(), signatureClass, NULL);
                    for (int i = 0; i < apk_signatures.size(); i++) {
                        auto signature = env->NewByteArray(apk_signatures[i].size());
                        env->SetByteArrayRegion(signature, 0, apk_signatures[i].size(), (jbyte *) apk_signatures[i].data());
                        env->SetObjectArrayElement(signatureArray, i, env->NewObject(signatureClass, signatureConstructor, signature));
                    }
                    signaturesField.set(packageInfo, signatureArray);
                } else if ((flags & 0x8000000) != 0) {
                    Class packageInfoClass(env, "android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass.getField("applicationInfo");
                    auto applicationInfo = applicationInfoField.get(packageInfo);
                    if (applicationInfo) {
                        Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");
                        auto sourceDirField = applicationInfoClass.getField("sourceDir");
                        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                        sourceDirField.set(applicationInfo, g_apkPath);
                        publicSourceDirField.set(applicationInfo, g_apkPath);
                    }
                    applicationInfoField.set(packageInfo, applicationInfo);

                    auto signingInfoField = packageInfoClass.getField("signingInfo");
                    auto signingInfo = signingInfoField.get(packageInfo);

                    Class signingInfoClass(env, "android.content.pm.SigningInfo");
                    auto mSigningDetailsField = signingInfoClass.getField("mSigningDetails");
                    auto mSigningDetails = mSigningDetailsField.get(signingInfo);

                    Class signingDetailsClass(env, "android.content.pm.PackageParser$SigningDetails");
                    auto signaturesField = signingDetailsClass.getField("signatures");
                    auto pastSigningCertificatesField = signingDetailsClass.getField("pastSigningCertificates");

                    auto signatureClass = env->FindClass("android/content/pm/Signature");
                    auto signatureConstructor = env->GetMethodID(signatureClass, "<init>", "([B)V");
                    auto signatureArray = env->NewObjectArray(apk_signatures.size(), signatureClass, NULL);
                    for (int i = 0; i < apk_signatures.size(); i++) {
                        auto signature = env->NewByteArray(apk_signatures[i].size());
                        env->SetByteArrayRegion(signature, 0, apk_signatures[i].size(), (jbyte *) apk_signatures[i].data());
                        env->SetObjectArrayElement(signatureArray, i, env->NewObject(signatureClass, signatureConstructor, signature));
                    }
                    signaturesField.set(mSigningDetails, signatureArray);
                    pastSigningCertificatesField.set(mSigningDetails, signatureArray);
                } else {
                    Class packageInfoClass(env, "android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass.getField("applicationInfo");
                    auto applicationInfo = applicationInfoField.get(packageInfo);
                    if (applicationInfo) {
                        Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");
                        auto sourceDirField = applicationInfoClass.getField("sourceDir");
                        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                        sourceDirField.set(applicationInfo, g_apkPath);
                        publicSourceDirField.set(applicationInfo, g_apkPath);
                    }
                    applicationInfoField.set(packageInfo, applicationInfo);
                }
            }
        }
    } else if (!strcmp(mName, "getApplicationInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), 0);
        if (!strcmp(packageName, g_pkgName.c_str())) {
            auto applicationInfo = mResult;
            if (applicationInfo) {
                Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");

                auto sourceDirField = applicationInfoClass.getField("sourceDir");
                auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                sourceDirField.set(applicationInfo, g_apkPath);
                publicSourceDirField.set(applicationInfo, g_apkPath);
            }
        }
    } else if (!strcmp(mName, "getInstallerPackageName")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), 0);
        if (!strcmp(packageName, g_pkgName.c_str())) {
            mResult = env->NewStringUTF("com.android.vending");
        }
    }

    if (mResult) {
        mResult = env->NewGlobalRef(mResult);
    }
    return env->PopLocalFrame(mResult); // make sure all local refs are deleted except for the result
}