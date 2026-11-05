// Dibuat otomatis dari libil2cpp.so milik game (RVA terverifikasi).
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "icon.h"

struct PatchDef { int feat; uint32_t rva; uint32_t exp[2]; uint32_t code[2]; const char* name; };
static const PatchDef PATCHES[] = {
    {1, 0x2660684, {0xFC1D0FE8, 0xA90157FE}, {0xD65F03C0, 0xD65F03C0}, "GameTimeController.OnUpdate"},
    {3, 0x2616EC8, {0xF81D0FFE, 0xA90157F6}, {0x52800020, 0xD65F03C0}, "CropInfoUpdater.IsWeatherWatering"},
    {5, 0x2622D00, {0xA9BE57FE, 0xA9014FF4}, {0x52800C80, 0xD65F03C0}, "CropInfoUpdater.GetQualityPointWatering"},
    {5, 0x2622DD4, {0xF81E0FFE, 0xA9014FF4}, {0x52800C80, 0xD65F03C0}, "CropInfoUpdater.GetQualityPointFertilize"},
    {5, 0x2622E4C, {0xA9BE57FE, 0xA9014FF4}, {0x52800C80, 0xD65F03C0}, "CropInfoUpdater.GetQualityPointAreaType"},
    {5, 0x2623048, {0xA9BE57FE, 0xA9014FF4}, {0x52800C80, 0xD65F03C0}, "CropInfoUpdater.GetQualityPointSeason"},
    {6, 0x2622C80, {0xA9BE57FE, 0xA9014FF4}, {0x52800020, 0xD65F03C0}, "CropInfoUpdater.IsMaxGrowPhase"},
    {9, 0x2861EAC, {0xA9BF4FFE, 0xAA0003F3}, {0x52800000, 0xD65F03C0}, "NpcInfo.GetGift"},
    {9, 0x2861F34, {0xA9BF4FFE, 0xAA0003F3}, {0x52800000, 0xD65F03C0}, "NpcInfo.GetTalked"},
    {10, 0x25E78DC, {0xF81D0FFE, 0xA90157F6}, {0x52800140, 0xD65F03C0}, "MineCrashableObjects.GetItemNumOre"},
};
#define NPATCH 10

struct HookDef { uint32_t rva; uint32_t exp[4]; const char* name; };
static const HookDef H_PMUPDATE = {0x28B6194, {0xF81C0FFE, 0xA9015FF8, 0xA90257F6, 0xA9034FF4}, "PartMapManager.ExternalUpdate"};
static const HookDef H_ADDITEM = {0x26BE4EC, {0xA9BB6FFE, 0xA90167FA, 0xA9025FF8, 0xA90357F6}, "ItemInfo.AddHaveItem"};
static const HookDef H_SETHP = {0x26C9A98, {0xD10143FF, 0xF90013FE, 0xA90357F6, 0xA9044FF4}, "PlayerInfo.set_Hp"};
static const HookDef H_FIGHT = {0x28E4D78, {0x6DBB2BEB, 0x6D0123E9, 0xA9025FFE, 0xA90357F6}, "FishingController.UpdateFighting"};
static const HookDef H_ANIMDAY = {0x2741604, {0xA9BD5FFE, 0xA90157F6, 0xA9024FF4, 0x90015A55}, "AnimalData.OnDayFinished"};

#define OFF_GETFT 0x2566BB0
#define OFF_GETNAME 0x24FDC2C
#define OFF_REQFT 0x28B8228
#define OFF_GETFISH 0x28E67D8
#define OFF_EATFEED 0x27420E8

// ---------------------------------------------------------------------------
// Harvest Moon: Home Sweet Home - mod menu (LGL Mod Menu compatible)
// Mengganti libModMenu.so lama. Semua fitur lama + Teleport + Tambang.
// ---------------------------------------------------------------------------

#define TAG "HSHMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

#define MAXF 32
static uintptr_t g_base = 0;
static volatile bool g_f[MAXF];
static volatile int g_tpReq = 0;
static void* g_pm = nullptr;
static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;

// ---------------------------------------------------------------- memory ---
static uintptr_t findBase() {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;
    char line[600];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "libil2cpp.so")) {
            unsigned long long s = 0;
            if (sscanf(line, "%llx-", &s) == 1) { base = (uintptr_t)s; break; }
        }
    }
    fclose(fp);
    return base;
}

static bool writeCode(uintptr_t addr, const void* data, size_t n) {
    uintptr_t page = addr & ~(uintptr_t)4095;
    size_t len = ((addr + n + 4095) & ~(uintptr_t)4095) - page;
    if (mprotect((void*)page, len, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) return false;
    memcpy((void*)addr, data, n);
    __builtin___clear_cache((char*)addr, (char*)(addr + n));
    mprotect((void*)page, len, PROT_READ | PROT_EXEC);
    return true;
}

// -------------------------------------------------------------- patches ----
static uint32_t g_saved[NPATCH][2];
static bool g_applied[NPATCH];

static void setPatches(int feat, bool on) {
    if (!g_base) return;
    pthread_mutex_lock(&g_mu);
    for (int i = 0; i < NPATCH; i++) {
        const PatchDef& p = PATCHES[i];
        if (p.feat != feat) continue;
        uintptr_t a = g_base + p.rva;
        if (on && !g_applied[i]) {
            uint32_t cur[2];
            memcpy(cur, (void*)a, 8);
            if (cur[0] != p.exp[0] || cur[1] != p.exp[1]) { LOGI("patch mismatch %s", p.name); continue; }
            g_saved[i][0] = cur[0]; g_saved[i][1] = cur[1];
            if (writeCode(a, p.code, 8)) g_applied[i] = true;
        } else if (!on && g_applied[i]) {
            if (writeCode(a, g_saved[i], 8)) g_applied[i] = false;
        }
    }
    pthread_mutex_unlock(&g_mu);
}

// ---------------------------------------------------------------- hooks ----
static uint8_t* g_tramp = nullptr;
static size_t g_trampUsed = 0;

static bool isPcRelNotAdrp(uint32_t w) {
    if ((w & 0x9F000000) == 0x10000000) return true;       // ADR
    if ((w & 0x7C000000) == 0x14000000) return true;       // B / BL
    if ((w & 0xFF000010) == 0x54000000) return true;       // B.cond
    if ((w & 0x7E000000) == 0x34000000) return true;       // CBZ / CBNZ
    if ((w & 0x7E000000) == 0x36000000) return true;       // TBZ / TBNZ
    if ((w & 0x3B000000) == 0x18000000) return true;       // LDR literal
    return false;
}

static bool installHook(const HookDef& h, void* repl, void** orig) {
    uintptr_t t = g_base + h.rva;
    uint32_t ins[4];
    memcpy(ins, (void*)t, 16);
    for (int i = 0; i < 4; i++) {
        if (ins[i] != h.exp[i]) { LOGI("hook mismatch %s", h.name); return false; }
    }
    if (!g_tramp) {
        void* m = mmap(nullptr, 16384, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (m == MAP_FAILED) { LOGI("mmap failed"); return false; }
        g_tramp = (uint8_t*)m;
    }
    uint32_t* tp = (uint32_t*)(g_tramp + g_trampUsed);
    int n = 0;
    for (int i = 0; i < 4; i++) {
        uint32_t w = ins[i];
        if ((w & 0x9F000000) == 0x90000000) {              // ADRP -> absolute load
            uint32_t rd = w & 0x1F;
            int64_t imm = (((int64_t)((w >> 5) & 0x7FFFF)) << 2) | ((w >> 29) & 3);
            if (imm & (1 << 20)) imm -= (1 << 21);
            uint64_t val = ((uint64_t)(t + 4 * i) & ~(uint64_t)0xFFF) + (uint64_t)(imm * 4096);
            tp[n++] = 0x58000040 | rd;                     // LDR Xd, #8
            tp[n++] = 0x14000003;                          // B +12
            memcpy(&tp[n], &val, 8); n += 2;
        } else if (isPcRelNotAdrp(w)) {
            LOGI("hook not relocatable %s", h.name);
            return false;
        } else {
            tp[n++] = w;
        }
    }
    uint64_t back = (uint64_t)(t + 16);
    tp[n++] = 0x58000051;                                  // LDR X17, #8
    tp[n++] = 0xD61F0220;                                  // BR  X17
    memcpy(&tp[n], &back, 8); n += 2;
    __builtin___clear_cache((char*)tp, (char*)(tp + n));
    g_trampUsed += (size_t)((n * 4 + 15) & ~15);

    uint32_t patch[4];
    patch[0] = 0x58000051;
    patch[1] = 0xD61F0220;
    memcpy(&patch[2], &repl, 8);
    if (!writeCode(t, patch, 16)) return false;
    *orig = (void*)tp;
    return true;
}

// ------------------------------------------------------------- teleport ----
static std::vector<uint32_t> g_uids;
static std::vector<std::string> g_names;
static int g_idx = -1;

static std::string pkgName() {
    char buf[256] = {0};
    FILE* f = fopen("/proc/self/cmdline", "r");
    if (f) { size_t r = fread(buf, 1, 255, f); (void)r; fclose(f); }
    return std::string(buf);
}

static std::string utf16ToUtf8(void* s) {
    std::string r;
    if (!s) return r;
    int len = *(int*)((char*)s + 0x10);
    if (len < 0 || len > 300) return r;
    const uint16_t* c = (const uint16_t*)((char*)s + 0x14);
    for (int i = 0; i < len; i++) {
        uint32_t ch = c[i];
        if (ch < 0x80) r += (char)ch;
        else if (ch < 0x800) { r += (char)(0xC0 | (ch >> 6)); r += (char)(0x80 | (ch & 0x3F)); }
        else { r += (char)(0xE0 | (ch >> 12)); r += (char)(0x80 | ((ch >> 6) & 0x3F)); r += (char)(0x80 | (ch & 0x3F)); }
    }
    return r;
}

static std::string outDir() {
    std::string pkg = pkgName();
    std::string d1 = "/storage/emulated/0/Android/data/" + pkg;
    std::string d2 = d1 + "/files";
    mkdir(d1.c_str(), 0775);
    mkdir(d2.c_str(), 0775);
    return d2;
}

static void buildList() {
    g_uids.clear();
    g_names.clear();
    typedef void* (*t_getFT)(void*);
    typedef void* (*t_getName)(void*, uint32_t, void*);
    void* acc = ((t_getFT)(g_base + OFF_GETFT))(nullptr);
    if (!acc) return;
    char* dict = *(char**)((char*)acc + 0xB0);
    if (!dict) return;
    char* entries = *(char**)(dict + 0x18);
    int count = *(int*)(dict + 0x20);
    if (!entries || count <= 0 || count > 5000) return;
    for (int i = 0; i < count; i++) {
        char* e = entries + 0x20 + i * 16;
        int next = *(int*)(e + 4);
        uint32_t key = *(uint32_t*)(e + 8);
        if (next < -1 || key == 0) continue;
        g_uids.push_back(key);
    }
    for (size_t i = 0; i < g_uids.size(); i++) {
        void* s = ((t_getName)(g_base + OFF_GETNAME))(acc, g_uids[i], nullptr);
        g_names.push_back(utf16ToUtf8(s));
    }
    std::string path = outDir() + "/ft_list.txt";
    FILE* f = fopen(path.c_str(), "w");
    if (f) {
        for (size_t i = 0; i < g_uids.size(); i++)
            fprintf(f, "%zu\t%u\t0x%X\t%s\n", i + 1, g_uids[i], g_uids[i], g_names[i].c_str());
        fclose(f);
    }
    LOGI("fast travel list: %zu", g_uids.size());
}

static void doTeleport(int dir) {
    typedef void (*t_ReqFT)(void*, uint32_t, void*);
    if (!g_pm) return;
    if (g_uids.empty()) buildList();
    if (g_uids.empty()) return;
    int n = (int)g_uids.size();
    if (g_idx < 0) g_idx = (dir > 0) ? 0 : n - 1;
    else g_idx = (g_idx + dir + n) % n;
    uint32_t uid = g_uids[g_idx];
    ((t_ReqFT)(g_base + OFF_REQFT))(g_pm, uid, nullptr);
    std::string path = outDir() + "/ft_current.txt";
    FILE* f = fopen(path.c_str(), "w");
    if (f) { fprintf(f, "%d/%d\t%u\t%s\n", g_idx + 1, n, uid, g_names[g_idx].c_str()); fclose(f); }
}

// ---------------------------------------------------------- hook bodies ----
typedef void (*t_PMUpdate)(void*, float, void*);
typedef bool (*t_AddItem)(void*, uint32_t, int, int*, int, bool, void*);
typedef void (*t_SetHp)(void*, int, void*);
typedef void (*t_Fight)(void*, float, void*);
typedef void (*t_AnimDay)(void*, bool, void*);
static t_PMUpdate o_PMUpdate = nullptr;
static t_AddItem o_AddItem = nullptr;
static t_SetHp o_SetHp = nullptr;
static t_Fight o_Fight = nullptr;
static t_AnimDay o_AnimDay = nullptr;

static void h_PMUpdate(void* self, float dt, void* mi) {
    g_pm = self;
    int req = g_tpReq;
    if (req != 0) { g_tpReq = 0; doTeleport(req); }
    o_PMUpdate(self, dt, mi);
}

static bool h_AddItem(void* self, uint32_t id, int num, int* overflow, int type, bool upd, void* mi) {
    if (g_f[4] && num > 0) num = 99;
    return o_AddItem(self, id, num, overflow, type, upd, mi);
}

static void h_SetHp(void* self, int value, void* mi) {
    if (g_f[2]) {
        int cur = *(int*)((char*)self + 0x34);
        if (value < cur) value = cur;
    }
    o_SetHp(self, value, mi);
}

static void h_Fight(void* self, float dt, void* mi) {
    if (g_f[7]) {
        typedef void (*t_GetFish)(void*, void*);
        ((t_GetFish)(g_base + OFF_GETFISH))(self, nullptr);
        return;
    }
    o_Fight(self, dt, mi);
}

static void h_AnimDay(void* self, bool dirt, void* mi) {
    if (g_f[8]) {
        typedef void (*t_Eat)(void*, bool, bool, void*);
        ((t_Eat)(g_base + OFF_EATFEED))(self, true, true, nullptr);
        dirt = false;
    }
    o_AnimDay(self, dirt, mi);
}

static void* worker(void*) {
    for (int i = 0; i < 600 && !g_base; i++) {
        g_base = findBase();
        if (!g_base) usleep(500000);
    }
    if (!g_base) return nullptr;
    LOGI("libil2cpp base %p", (void*)g_base);
    sleep(5);
    installHook(H_PMUPDATE, (void*)h_PMUpdate, (void**)&o_PMUpdate);
    installHook(H_ADDITEM, (void*)h_AddItem, (void**)&o_AddItem);
    installHook(H_SETHP, (void*)h_SetHp, (void**)&o_SetHp);
    installHook(H_FIGHT, (void*)h_Fight, (void**)&o_Fight);
    installHook(H_ANIMDAY, (void*)h_AnimDay, (void**)&o_AnimDay);
    for (int f = 1; f < MAXF; f++) if (g_f[f]) setPatches(f, true);
    return nullptr;
}

// ------------------------------------------------------------------ JNI ----
static void toast(JNIEnv* env, jobject ctx, const char* msg) {
    jclass tc = env->FindClass("android/widget/Toast");
    jmethodID mk = env->GetStaticMethodID(tc, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    jobject t = env->CallStaticObjectMethod(tc, mk, ctx, env->NewStringUTF(msg), 0);
    if (t) env->CallVoidMethod(t, env->GetMethodID(tc, "show", "()V"));
}

static jobjectArray makeArray(JNIEnv* env, const char* const* items, int n) {
    jclass sc = env->FindClass("java/lang/String");
    jobjectArray r = env->NewObjectArray(n, sc, env->NewStringUTF(""));
    for (int i = 0; i < n; i++) env->SetObjectArrayElement(r, i, env->NewStringUTF(items[i]));
    return r;
}

static const char* const FEATURES[] = {
    "Category_Mod Menu",
    "1_Toggle_Freeze Time",
    "2_Toggle_Infinite Stamina",
    "3_Toggle_Auto Watered Crop",
    "4_Toggle_Get Item x99",
    "5_Toggle_Max Quality Crops",
    "6_Toggle_Crop Grew Overnight",
    "7_Toggle_Easy Fishing (Auto Pull)",
    "8_Toggle_Animal Care",
    "9_Toggle_Unlimited Gifts/Talks",
    "Category_Tambang",
    "10_Toggle_Mine Ore x10",
    "Category_Teleport",
    "11_Button_Teleport Titik Berikutnya",
    "12_Button_Teleport Titik Sebelumnya",
};

static const char* const SETTINGS[] = {
    "Category_Settings",
    "-1_Toggle_Save feature preferences",
    "-3_Toggle_Auto size vertically",
};

static jstring JNICALL n_Icon(JNIEnv* env, jobject) { return env->NewStringUTF(ICON_B64); }
static jstring JNICALL n_IconWebViewData(JNIEnv*, jobject) { return nullptr; }
static jboolean JNICALL n_IsGameLibLoaded(JNIEnv*, jobject) { return JNI_TRUE; }

static void JNICALL n_Init(JNIEnv* env, jobject, jobject ctx, jobject title, jobject subtitle) {
    jmethodID setText = env->GetMethodID(env->GetObjectClass(title), "setText", "(Ljava/lang/CharSequence;)V");
    env->CallVoidMethod(title, setText, env->NewStringUTF("Home Sweet Home"));
    env->CallVoidMethod(subtitle, setText, env->NewStringUTF("mod by deepth"));
    toast(env, ctx, "Mod menu siap");
}

static jobjectArray JNICALL n_SettingsList(JNIEnv* env, jobject) {
    return makeArray(env, SETTINGS, (int)(sizeof(SETTINGS) / sizeof(SETTINGS[0])));
}

static jobjectArray JNICALL n_GetFeatureList(JNIEnv* env, jobject) {
    return makeArray(env, FEATURES, (int)(sizeof(FEATURES) / sizeof(FEATURES[0])));
}

static void JNICALL n_Changes(JNIEnv* env, jclass, jobject ctx, jint featNum, jstring, jint, jboolean b, jstring) {
    if (featNum >= 1 && featNum <= 10) {
        g_f[featNum] = (b != JNI_FALSE);
        setPatches(featNum, b != JNI_FALSE);
    } else if (featNum == 11 || featNum == 12) {
        if (!g_pm) { toast(env, ctx, "Teleport belum siap, masuk ke dunia game dulu"); return; }
        g_tpReq = (featNum == 11) ? 1 : -1;
    }
}

// Java Main.Start() hanya memanggil CheckOverlayPermission; menu baru muncul
// kalau native memanggil Main.StartWithoutPermission setelah izin overlay ada.
static void startMenu(JNIEnv* env, jobject ctx) {
    jclass mc = env->FindClass("com/android/support/Main");
    jmethodID m = env->GetStaticMethodID(mc, "StartWithoutPermission", "(Landroid/content/Context;)V");
    env->CallStaticVoidMethod(mc, m, ctx);
}

static void JNICALL n_CheckOverlayPermission(JNIEnv* env, jclass, jobject ctx) {
    jclass vc = env->FindClass("android/os/Build$VERSION");
    jint sdk = env->GetStaticIntField(vc, env->GetStaticFieldID(vc, "SDK_INT", "I"));
    if (sdk < 23) { startMenu(env, ctx); return; }
    jclass st = env->FindClass("android/provider/Settings");
    jmethodID can = env->GetStaticMethodID(st, "canDrawOverlays", "(Landroid/content/Context;)Z");
    if (env->CallStaticBooleanMethod(st, can, ctx)) { startMenu(env, ctx); return; }
    toast(env, ctx, "Izinkan tampil di atas aplikasi lain, lalu buka ulang game");
    jclass cc = env->GetObjectClass(ctx);
    jstring pkg = (jstring)env->CallObjectMethod(ctx, env->GetMethodID(cc, "getPackageName", "()Ljava/lang/String;"));
    jclass uc = env->FindClass("android/net/Uri");
    jmethodID parse = env->GetStaticMethodID(uc, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
    const char* p = env->GetStringUTFChars(pkg, nullptr);
    std::string s = std::string("package:") + p;
    env->ReleaseStringUTFChars(pkg, p);
    jobject uri = env->CallStaticObjectMethod(uc, parse, env->NewStringUTF(s.c_str()));
    jstring action = (jstring)env->GetStaticObjectField(st, env->GetStaticFieldID(st, "ACTION_MANAGE_OVERLAY_PERMISSION", "Ljava/lang/String;"));
    jclass ic = env->FindClass("android/content/Intent");
    jobject intent = env->NewObject(ic, env->GetMethodID(ic, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V"), action, uri);
    env->CallObjectMethod(intent, env->GetMethodID(ic, "addFlags", "(I)Landroid/content/Intent;"), 0x10000000);
    env->CallVoidMethod(ctx, env->GetMethodID(cc, "startActivity", "(Landroid/content/Intent;)V"), intent);
    sleep(5);
    exit(0);
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;

    jclass menu = env->FindClass("com/android/support/Menu");
    if (!menu) return JNI_ERR;
    JNINativeMethod mm[] = {
        {(char*)"Icon", (char*)"()Ljava/lang/String;", (void*)n_Icon},
        {(char*)"IconWebViewData", (char*)"()Ljava/lang/String;", (void*)n_IconWebViewData},
        {(char*)"IsGameLibLoaded", (char*)"()Z", (void*)n_IsGameLibLoaded},
        {(char*)"Init", (char*)"(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V", (void*)n_Init},
        {(char*)"SettingsList", (char*)"()[Ljava/lang/String;", (void*)n_SettingsList},
        {(char*)"GetFeatureList", (char*)"()[Ljava/lang/String;", (void*)n_GetFeatureList},
    };
    if (env->RegisterNatives(menu, mm, 6) != 0) return JNI_ERR;

    jclass prefs = env->FindClass("com/android/support/Preferences");
    if (!prefs) return JNI_ERR;
    JNINativeMethod pm[] = {
        {(char*)"Changes", (char*)"(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/String;)V", (void*)n_Changes},
    };
    if (env->RegisterNatives(prefs, pm, 1) != 0) return JNI_ERR;

    jclass mainc = env->FindClass("com/android/support/Main");
    if (!mainc) return JNI_ERR;
    JNINativeMethod mn[] = {
        {(char*)"CheckOverlayPermission", (char*)"(Landroid/content/Context;)V", (void*)n_CheckOverlayPermission},
    };
    if (env->RegisterNatives(mainc, mn, 1) != 0) return JNI_ERR;

    pthread_t t;
    pthread_create(&t, nullptr, worker, nullptr);
    pthread_detach(t);
    return JNI_VERSION_1_6;
}
