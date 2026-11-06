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
    {3, 0x2616EC8, {0xF81D0FFE, 0xA90157F6}, {0x52800020, 0xD65F03C0}, "CropInfoUpdater.IsWeatherWatering"},
    {5, 0x26254A4, {0x6DBA23E9, 0xF9000BFE}, {0x52800060, 0xD65F03C0}, "FarmUtil.LotProductQualityIndex"},
    {6, 0x2622C80, {0xA9BE57FE, 0xA9014FF4}, {0x52800020, 0xD65F03C0}, "CropInfoUpdater.IsMaxGrowPhase"},
    {8, 0x2740C74, {0xF81D0FFE, 0xA90157F6}, {0xD65F03C0, 0xD65F03C0}, "AnimalData.RunAway"},
    {9, 0x2861EAC, {0xA9BF4FFE, 0xAA0003F3}, {0x52800000, 0xD65F03C0}, "NpcInfo.GetGift"},
    {9, 0x2861F34, {0xA9BF4FFE, 0xAA0003F3}, {0x52800000, 0xD65F03C0}, "NpcInfo.GetTalked"},
    {11, 0x25ECF2C, {0xA9BF4FFE, 0xB9402408}, {0xD65F03C0, 0xD65F03C0}, "MineTrapHole.ApplyCellHit"},
    {11, 0x25BDFB8, {0xD10243FF, 0xA90757FE}, {0xD65F03C0, 0xD65F03C0}, "MineFallStone.StartHitFallStone"},
};
#define NPATCH 8

struct HookDef { uint32_t rva; uint32_t exp[4]; const char* name; };
static const HookDef H_PMUPDATE = {0x28B6194, {0xF81C0FFE, 0xA9015FF8, 0xA90257F6, 0xA9034FF4}, "PartMapManager.ExternalUpdate"};
static const HookDef H_ADDITEM = {0x26BE4EC, {0xA9BB6FFE, 0xA90167FA, 0xA9025FF8, 0xA90357F6}, "ItemInfo.AddHaveItem"};
static const HookDef H_SETHP = {0x26C9A98, {0xD10143FF, 0xF90013FE, 0xA90357F6, 0xA9044FF4}, "PlayerInfo.set_Hp"};
static const HookDef H_FIGHT = {0x28E4D78, {0x6DBB2BEB, 0x6D0123E9, 0xA9025FFE, 0xA90357F6}, "FishingController.UpdateFighting"};
static const HookDef H_ANIMDAY = {0x2741604, {0xA9BD5FFE, 0xA90157F6, 0xA9024FF4, 0x90015A55}, "AnimalData.OnDayFinished"};
static const HookDef H_TIMELATE = {0x2657A00, {0xFC1D0FE8, 0xA90157FE, 0xA9024FF4, 0xD0016195}, "TimeScheduleController.LateUpdate"};
static const HookDef H_APPLYHIT = {0x25E7B5C, {0xD10103FF, 0xA90257FE, 0xA9034FF4, 0xB0016514}, "MineCrashableObjects.ApplyHit"};
static const HookDef H_SPEED = {0x2709670, {0xF81E0FFE, 0xA9014FF4, 0x90015C14, 0x395CA288}, "ActionStateMove.GetSpeed"};
static const HookDef H_PLUPDATE = {0x26CB0A8, {0xF81E0FFE, 0xA9014FF4, 0xD0015DF4, 0x39521A88}, "Player.Update"};

#define FISH_SUCCESS 505158168
#define OFF_GETFT 0x2566BB0
#define OFF_REQFT 0x28B8228
#define OFF_EATFEED 0x27420E8
#define OFF_WARP 0x27189C8
#define OFF_ISWARPEND 0x2718BC4
#define OFF_SETKIN 0x2718B18
#define OFF_GETPOS 0x24FD740
#define OFF_GETROT 0x24FD7E8
#define OFF_FISHSTATE 0x28E1114

// ---------------------------------------------------------------------------
// Harvest Moon: Home Sweet Home - mod menu v2 (LGL Mod Menu compatible)
// ---------------------------------------------------------------------------

#define TAG "HSHMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

#define MAXF 40
static uintptr_t g_base = 0;
static volatile bool g_f[MAXF];
static volatile int g_speed = 1;
static volatile uint32_t g_tpUid = 0;
static volatile int g_frame = 0;
static void* g_pm = nullptr;
static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;

struct Vec3 { float x, y, z; };

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
struct TpPoint { int feat; uint32_t uid; const char* label; };
static const TpPoint TP[] = {
    {20, 3001, "Rumah"},
    {21, 3003, "Tengah Desa"},
    {22, 3002, "Balai Desa"},
    {23, 2108, "Toko Hewan"},
    {24, 2105, "Restoran"},
    {25, 2104, "Dermaga"},
    {26, 9002, "Tambang"},
    {27, 3004, "Puncak Gunung"},
    {28, 2202, "Pemandian Air Panas"},
    {30, 4004, "Lomba Pancing 1"},
    {31, 4005, "Lomba Pancing 2"},
    {32, 4006, "Lomba Pancing 3"},
    {33, 2110, "Rumah Petani Besar"},
    {34, 2106, "Toko Umum"},
    {35, 2103, "Toko Suvenir"},
};
#define NTP ((int)(sizeof(TP) / sizeof(TP[0])))

static bool g_warpPending = false;
static int g_warpFrames = 0;

static void* g_player = nullptr;   // diisi oleh hook Player.Update
static void* getPlayer() { return g_player; }

static void doWarp(uint32_t uid) {
    typedef void* (*t_getFT)(void*);
    typedef float (*t_getPos)(void*, uint32_t, int, void*);
    typedef float (*t_getRot)(void*, uint32_t, void*);
    typedef void (*t_Warp)(void*, Vec3, Vec3, void*);
    typedef void (*t_ReqFT)(void*, uint32_t, void*);
    void* acc = ((t_getFT)(g_base + OFF_GETFT))(nullptr);
    void* pl = getPlayer();
    if (!acc || !pl) return;
    Vec3 p;
    p.x = ((t_getPos)(g_base + OFF_GETPOS))(acc, uid, 0, nullptr);
    p.y = ((t_getPos)(g_base + OFF_GETPOS))(acc, uid, 1, nullptr);
    p.z = ((t_getPos)(g_base + OFF_GETPOS))(acc, uid, 2, nullptr);
    float rot = ((t_getRot)(g_base + OFF_GETROT))(acc, uid, nullptr);
    LOGI("warp uid %u -> %f %f %f rot %f", uid, p.x, p.y, p.z, rot);
    if (p.x == 0.0f && p.y == 0.0f && p.z == 0.0f) {
        // koordinat kosong: pakai fast travel biasa (ada layar loading)
        if (g_pm) ((t_ReqFT)(g_base + OFF_REQFT))(g_pm, uid, nullptr);
        return;
    }
    Vec3 r = {0.0f, rot, 0.0f};
    ((t_Warp)(g_base + OFF_WARP))(pl, p, r, nullptr);
    g_warpPending = true;
    g_warpFrames = 0;
}

static void pollWarp() {
    typedef bool (*t_IsWarpEnd)(void*, bool, void*);
    typedef void (*t_SetKin)(void*, bool, void*);
    void* pl = getPlayer();
    if (!pl) { g_warpPending = false; return; }
    bool done = ((t_IsWarpEnd)(g_base + OFF_ISWARPEND))(pl, true, nullptr);
    if (done || ++g_warpFrames > 900) {
        if (!done) ((t_SetKin)(g_base + OFF_SETKIN))(pl, false, nullptr);
        g_warpPending = false;
    }
}

// ---------------------------------------------------------- hook bodies ----
typedef void (*t_PMUpdate)(void*, float, void*);
typedef bool (*t_AddItem)(void*, uint32_t, int, int*, int, bool, void*);
typedef void (*t_SetHp)(void*, int, void*);
typedef void (*t_Fight)(void*, float, void*);
typedef void (*t_AnimDay)(void*, bool, void*);
typedef void (*t_TimeLate)(void*, float, void*);
typedef void (*t_ApplyHit)(void*, void*);
typedef float (*t_Speed)(void*, void*);
typedef void (*t_PlUpdate)(void*, void*);
static t_PMUpdate o_PMUpdate = nullptr;
static t_AddItem o_AddItem = nullptr;
static t_SetHp o_SetHp = nullptr;
static t_Fight o_Fight = nullptr;
static t_AnimDay o_AnimDay = nullptr;
static t_TimeLate o_TimeLate = nullptr;
static t_ApplyHit o_ApplyHit = nullptr;
static t_Speed o_Speed = nullptr;
static t_PlUpdate o_PlUpdate = nullptr;

static void h_PlUpdate(void* self, void* mi) {
    g_player = self;
    o_PlUpdate(self, mi);
}

static void h_PMUpdate(void* self, float dt, void* mi) {
    g_pm = self;
    g_frame++;
    uint32_t u = g_tpUid;
    if (u != 0) { g_tpUid = 0; doWarp(u); }
    if (g_warpPending) pollWarp();
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

static int g_lastFishFrame = -1000;
static void h_Fight(void* self, float dt, void* mi) {
    if (g_f[7] && (g_frame - g_lastFishFrame) > 60) {
        g_lastFishFrame = g_frame;
        typedef void (*t_State)(void*, int, void*);
        ((t_State)(g_base + OFF_FISHSTATE))(self, FISH_SUCCESS, nullptr);
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

// Freeze Time: tahan jam game (TimeScheduleController)
static bool g_frozenSnap = false;
static float g_snap[6];
static const int SNAP_OFF[6] = {0x20, 0x24, 0x2C, 0x30, 0x34, 0x38};
static void h_TimeLate(void* self, float dt, void* mi) {
    if (g_f[1]) {
        if (!g_frozenSnap) {
            for (int i = 0; i < 6; i++) g_snap[i] = *(float*)((char*)self + SNAP_OFF[i]);
            g_frozenSnap = true;
        } else {
            for (int i = 0; i < 6; i++) *(float*)((char*)self + SNAP_OFF[i]) = g_snap[i];
        }
        dt = 0.0f;
    } else {
        g_frozenSnap = false;
    }
    o_TimeLate(self, dt, mi);
}

// Batu tambang pecah sekali pukul
static void h_ApplyHit(void* self, void* mi) {
    if (g_f[10]) {
        *(int*)((char*)self + 0x38) = 1;
        if (*(int*)((char*)self + 0x4C) > 1) *(int*)((char*)self + 0x4C) = 1;
    }
    o_ApplyHit(self, mi);
}

// Kecepatan jalan pemain (hanya pemain, bukan NPC)
static float h_Speed(void* self, void* mi) {
    float v = o_Speed(self, mi);
    int m = g_speed;
    if (m > 1 && *(bool*)((char*)self + 0x28)) v *= (float)m;
    return v;
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
    installHook(H_TIMELATE, (void*)h_TimeLate, (void**)&o_TimeLate);
    installHook(H_APPLYHIT, (void*)h_ApplyHit, (void**)&o_ApplyHit);
    installHook(H_SPEED, (void*)h_Speed, (void**)&o_Speed);
    installHook(H_PLUPDATE, (void*)h_PlUpdate, (void**)&o_PlUpdate);
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
    "10_Toggle_Batu Sekali Pukul",
    "11_Toggle_Tanpa Jebakan dan Batu Jatuh",
    "Category_Pemain",
    "12_SeekBar_Kecepatan Pemain_1_6",
    "Category_Teleport (tanpa loading)",
    "Collapse_Titik Utama",
    "CollapseAdd_20_Button_Rumah",
    "CollapseAdd_21_Button_Tengah Desa",
    "CollapseAdd_22_Button_Balai Desa",
    "CollapseAdd_23_Button_Toko Hewan (bibit dan kemah)",
    "CollapseAdd_24_Button_Restoran (pantai)",
    "CollapseAdd_25_Button_Dermaga (rumah nelayan)",
    "CollapseAdd_26_Button_Tambang (pintu luar)",
    "CollapseAdd_27_Button_Puncak Gunung",
    "CollapseAdd_28_Button_Pemandian Air Panas",
    "Collapse_Titik Tambahan (cari danau)",
    "CollapseAdd_30_Button_Lomba Pancing 1",
    "CollapseAdd_31_Button_Lomba Pancing 2",
    "CollapseAdd_32_Button_Lomba Pancing 3",
    "CollapseAdd_33_Button_Rumah Petani Besar",
    "CollapseAdd_34_Button_Toko Umum",
    "CollapseAdd_35_Button_Toko Suvenir",
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

static void JNICALL n_Changes(JNIEnv* env, jclass, jobject ctx, jint featNum, jstring, jint value, jboolean b, jstring) {
    if (featNum >= 1 && featNum <= 11) {
        g_f[featNum] = (b != JNI_FALSE);
        setPatches(featNum, b != JNI_FALSE);
    } else if (featNum == 12) {
        int v = value;
        if (v < 1) v = 1;
        if (v > 6) v = 6;
        g_speed = v;
    } else {
        for (int i = 0; i < NTP; i++) {
            if (TP[i].feat != featNum) continue;
            if (!g_pm) { toast(env, ctx, "Teleport belum siap, masuk ke dunia game dulu"); return; }
            g_tpUid = TP[i].uid;
            toast(env, ctx, TP[i].label);
            return;
        }
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
