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
static const HookDef H_GETHAVE = {0x26BFEF8, {0xA9BD5FFE, 0xA90157F6, 0xA9024FF4, 0xD0015E57}, "ItemInfo.GetHaveItem"};
static const HookDef H_ENVLATE = {0x265770C, {0xD10183FF, 0xFD0013E8, 0xA9035FFE, 0xA90457F6}, "EnvironmentManager.ExternalLateUpdate"};
static const HookDef H_FIINIT = {0x246ED30, {0xD10403FF, 0xFD004BEA, 0x6D0A23E9, 0xA90B6FFE}, "FieldItemEntity.Init"};
static const HookDef H_FIDESTROY = {0x246F20C, {0xA9BD5FFE, 0xA90157F6, 0xA9024FF4, 0x900170D4}, "FieldItemEntity.OnDestroy"};
static const HookDef H_MINEUPD = {0x25BF0C4, {0xD10143FF, 0xFD000BE8, 0xF9000FFE, 0xA9025FF8}, "MineController.ExternalUpdate"};
static const HookDef H_CAP = {0x26BD928, {0xA9BE57FE, 0xA9014FF4, 0x90015E75, 0x39502EA8}, "InventoryInfo.GetCapacity"};
static const HookDef H_CROPUPD = {0x2615650, {0xD10103FF, 0xF9000BFE, 0xA90257F6, 0xA9034FF4}, "CropFieldController.ExternalUpdate"};
static const HookDef H_REMOVE = {0x26BFBF8, {0xF81C0FFE, 0xA9015FF8, 0xA90257F6, 0xA9034FF4}, "ItemInfo.RemoveItem"};
static const HookDef H_DOWSE = {0x2857D3C, {0xD10143FF, 0xFD0013E8, 0xA90357FE, 0xA9044FF4}, "DowsingMapController.DoDowSing"};
static const HookDef H_FISHCS = {0x28E1114, {0xD10243FF, 0x6D0233ED, 0x6D032BEB, 0x6D0423E9}, "FishingController.ChangeState"};
static const HookDef H_AHUPD = {0x26051C8, {0xA9BE57FE, 0xA9014FF4, 0xF0016415, 0xD0014994}, "AnimalHouseManager.ExternalUpdate"};
static const HookDef H_ADDBAG = {0x26C24E0, {0xF81C0FFE, 0xA9015FF8, 0xA90257F6, 0xA9034FF4}, "ItemInfo.AddItemToBagImpl"};

#define GETFISH_START 0x28E67D8
#define GETFISH_SIZE 280
#define FISH_SUCCESS 12
#define INV_ITEMBAG 0
#define FISH_FIGHTING 11
#define OFF_RESOLVE 0x22B724C
struct FsnEntry { int v; const char* n; };
static const FsnEntry FSN[] = {
    {0, "Wait"},
    {1, "Ready"},
    {2, "TipsWait"},
    {3, "Tips"},
    {4, "SelectBait"},
    {5, "SelectPos"},
    {6, "BaitAnim"},
    {7, "WaitHit"},
    {8, "Hit"},
    {9, "CancelCasting"},
    {10, "MissHit"},
    {11, "Fighting"},
    {12, "Success"},
    {13, "SuccessLp"},
    {14, "SuccessOut"},
    {15, "Failure"},
    {16, "TryBackSelect"},
};
#define NFSN (int)(sizeof(FSN)/sizeof(FSN[0]))
struct KeepRange { uint32_t start; uint32_t size; };
static const KeepRange KEEP[] = {
    {0x26174E8, 384},
    {0x2617790, 564},
    {0x26183CC, 200},
    {0x28EFD08, 272},
};
#define NKEEP (int)(sizeof(KEEP)/sizeof(KEEP[0]))
#define OFF_GETFT 0x2566BB0
#define OFF_REQFT 0x28B8228
#define OFF_EATFEED 0x27420E8
#define OFF_WARP 0x27189C8
#define OFF_ISWARPEND 0x2718BC4
#define OFF_SETKIN 0x2718B18
#define OFF_GETPOS 0x24FD740
#define OFF_GETROT 0x24FD7E8
#define OFF_FISHSTATE 0x28E1114
#define OFF_ADDMONEY 0x26C10E8
#define OFF_ADVTIME 0x2658C20
#define OFF_ATTACHCAM 0x26CA5FC
#define OFF_PICKTHIS 0x246F180
#define OFF_GETFURROW 0x26167C0
#define OFF_FURROWENABLE 0x26259E0
#define OFF_PREHARVEST 0x26179C4
#define OFF_HARVEST 0x2617A48
#define OFF_FURROWRC 0x26199AC
#define OFF_SETFLOOR 0x25BE944
#define OFF_CREATENEXT 0x25BEA64
#define OFF_ADDITEM 0x26BE4EC
#define OFF_GETHAVE 0x26BFEF8
#define OFF_CLEANDIRT 0x26075D8
#define OFF_GREATQ 0x26C2B0C

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
static volatile int g_rideSpeed = 1;
static volatile int g_moneyReq = 0;
static volatile int g_timeReq = 0;
static void* g_itemInfo = nullptr;
static void* g_env = nullptr;
static void* g_timeCtl = nullptr;
static void* g_pm = nullptr;
static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;

struct Vec3 { float x, y, z; };
struct V2I { int x, y; };
struct UniTask { void* src; int64_t tok; };

static volatile int g_pickRange = 0;
static volatile int g_harvestReq = 0;
static volatile int g_saveReq = 0;
static volatile int g_gotoReq = 0;
static volatile int g_floorReq = 0;
static void* g_mine = nullptr;
static Vec3 g_slot[3];
static int g_slotSet[3];
struct FieldRef { void* ptr; int frame; };
static FieldRef g_fields[16];
struct ItemRef { void* ptr; Vec3 pos; int next; };
static std::vector<ItemRef> g_items;

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

static const TpPoint IN[] = {
    {70, 1101, "Balai Desa (dalam)"},
    {71, 1102, "Kantor Berita (dalam)"},
    {72, 1103, "Toko Suvenir (dalam)"},
    {73, 1104, "Rumah Nelayan (dalam)"},
    {74, 1105, "Restoran (dalam)"},
    {75, 1106, "Toko Umum (dalam)"},
    {76, 1107, "Toko Kerajinan dan Pakaian (dalam)"},
    {77, 1108, "Toko Hewan (dalam)"},
    {78, 1109, "Kandang Toko Hewan (dalam)"},
    {79, 1110, "Rumah Petani Besar (dalam)"},
    {80, 1111, "Rumah Penjaga Tambang (dalam)"},
    {81, 1112, "Balai Desa (pintu belakang)"},
};
#define NIN ((int)(sizeof(IN) / sizeof(IN[0])))
static volatile uint32_t g_ftUid = 0;

static bool g_warpPending = false;
static int g_warpFrames = 0;

static void* g_player = nullptr;   // diisi oleh hook Player.Update
static void* getPlayer() { return g_player; }

static void attachCam() {
    typedef void (*t_Attach)(void*, void*);
    void* pl = getPlayer();
    if (pl) ((t_Attach)(g_base + OFF_ATTACHCAM))(pl, nullptr);
}

static void warpTo(Vec3 p, float rot) {
    typedef void (*t_Warp)(void*, Vec3, Vec3, void*);
    void* pl = getPlayer();
    if (!pl) return;
    Vec3 r = {0.0f, rot, 0.0f};
    ((t_Warp)(g_base + OFF_WARP))(pl, p, r, nullptr);
    attachCam();
    g_warpPending = true;
    g_warpFrames = 0;
}

static void doWarp(uint32_t uid) {
    typedef void* (*t_getFT)(void*);
    typedef float (*t_getPos)(void*, uint32_t, int, void*);
    typedef float (*t_getRot)(void*, uint32_t, void*);
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
    warpTo(p, rot);
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
        attachCam();
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
typedef int (*t_GetHave)(void*, uint32_t, int, bool, void*);
typedef void (*t_EnvLate)(void*, float, void*);
typedef void (*t_FIInit)(void*, int, Vec3, bool, uint32_t, uint32_t, void*, bool, void*);
typedef void (*t_FIDestroy)(void*, void*);
typedef void (*t_MineUpd)(void*, float, void*);
typedef int (*t_Cap)(void*, uint32_t, void*);
typedef void (*t_CropUpd)(void*, float, void*);
typedef bool (*t_Remove)(void*, uint32_t, int, int, void*);
typedef void (*t_Dowse)(void*, float, void*);
typedef void (*t_FishCS)(void*, int, void*);
typedef void (*t_AHUpd)(void*, float, void*);
typedef int (*t_AddBag)(uint32_t, int, bool, bool, bool, void*);
static t_PMUpdate o_PMUpdate = nullptr;
static t_AddItem o_AddItem = nullptr;
static t_SetHp o_SetHp = nullptr;
static t_Fight o_Fight = nullptr;
static t_AnimDay o_AnimDay = nullptr;
static t_TimeLate o_TimeLate = nullptr;
static t_ApplyHit o_ApplyHit = nullptr;
static t_Speed o_Speed = nullptr;
static t_PlUpdate o_PlUpdate = nullptr;
static t_GetHave o_GetHave = nullptr;
static t_EnvLate o_EnvLate = nullptr;
static t_FIInit o_FIInit = nullptr;
static t_FIDestroy o_FIDestroy = nullptr;
static t_MineUpd o_MineUpd = nullptr;
static t_Cap o_Cap = nullptr;
static t_CropUpd o_CropUpd = nullptr;
static t_Remove o_Remove = nullptr;
static t_Dowse o_Dowse = nullptr;
static t_FishCS o_FishCS = nullptr;
static t_AHUpd o_AHUpd = nullptr;
static t_AddBag o_AddBag = nullptr;

static void h_PlUpdate(void* self, void* mi) {
    g_player = self;
    o_PlUpdate(self, mi);
}

// ---------------------------------------------------------- icall util ----
static void* resolveIcall(const char* a, const char* b2) {
    typedef void* (*t_resolve)(const char*);
    t_resolve r = (t_resolve)(g_base + OFF_RESOLVE);
    void* p = r(a);
    if (!p && b2) p = r(b2);
    return p;
}

static bool getPlayerPos(Vec3* out) {
    void* pl = g_player;
    if (!pl) return false;
    typedef void (*t_posInj)(void*, Vec3*);
    typedef void* (*t_getTr)(void*);
    static t_posInj rigFn = nullptr, trFn = nullptr;
    static t_getTr goTr = nullptr;
    static bool tried = false;
    if (!tried) {
        tried = true;
        rigFn = (t_posInj)resolveIcall("UnityEngine.Rigidbody::get_position_Injected(UnityEngine.Vector3&)", "UnityEngine.Rigidbody::get_position_Injected");
        trFn = (t_posInj)resolveIcall("UnityEngine.Transform::get_position_Injected(UnityEngine.Vector3&)", "UnityEngine.Transform::get_position_Injected");
        goTr = (t_getTr)resolveIcall("UnityEngine.GameObject::get_transform()", "UnityEngine.GameObject::get_transform");
    }
    void* rig = *(void**)((char*)pl + 0xB0);
    if (rigFn && rig) { rigFn(rig, out); return true; }
    void* go = *(void**)((char*)pl + 0x70);
    if (trFn && goTr && go) {
        void* tr = goTr(go);
        if (tr) { trFn(tr, out); return true; }
    }
    return false;
}

static void setFps(int fps) {
    typedef void (*t_setInt)(int);
    static t_setInt fn = nullptr;
    static bool tried = false;
    if (!tried) {
        tried = true;
        fn = (t_setInt)resolveIcall("UnityEngine.Application::set_targetFrameRate(System.Int32)", "UnityEngine.Application::set_targetFrameRate");
    }
    if (fn) fn(fps);
}

// ------------------------------------------------- saved positions (file) --
static std::string outDir() {
    char buf[256] = {0};
    FILE* f = fopen("/proc/self/cmdline", "r");
    if (f) { size_t r = fread(buf, 1, 255, f); (void)r; fclose(f); }
    std::string d1 = std::string("/storage/emulated/0/Android/data/") + buf;
    std::string d2 = d1 + "/files";
    mkdir(d1.c_str(), 0775);
    mkdir(d2.c_str(), 0775);
    return d2;
}

static void savePosFile() {
    std::string path = outDir() + "/saved_pos.bin";
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return;
    for (int i = 0; i < 3; i++) { fwrite(&g_slotSet[i], 4, 1, f); fwrite(&g_slot[i], sizeof(Vec3), 1, f); }
    fclose(f);
}

static void loadPosFile() {
    std::string path = outDir() + "/saved_pos.bin";
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return;
    for (int i = 0; i < 3; i++) {
        int s = 0; Vec3 v = {0, 0, 0};
        if (fread(&s, 4, 1, f) != 1 || fread(&v, sizeof(Vec3), 1, f) != 1) break;
        g_slotSet[i] = s; g_slot[i] = v;
    }
    fclose(f);
}

// ----------------------------------------------------------- auto pickup ---
static void autoPick() {
    Vec3 p;
    if (!getPlayerPos(&p)) return;
    float r = (float)g_pickRange;
    float r2 = r * r;
    std::vector<void*> todo;
    for (size_t i = 0; i < g_items.size(); i++) {
        ItemRef& it = g_items[i];
        if (it.next > g_frame) continue;
        float dx = it.pos.x - p.x, dy = it.pos.y - p.y, dz = it.pos.z - p.z;
        if (dx * dx + dy * dy + dz * dz <= r2) { it.next = g_frame + 240; todo.push_back(it.ptr); }
    }
    typedef void (*t_Pick)(void*, void*);
    typedef bool (*t_add)(void*, uint32_t, int, int*, int, bool, void*);
    typedef int (*t_have)(void*, uint32_t, int, bool, void*);
    if (!g_itemInfo) return;
    for (size_t i = 0; i < todo.size(); i++) {
        uint32_t id = *(uint32_t*)((char*)todo[i] + 0x50);
        int before = ((t_have)(g_base + OFF_GETHAVE))(g_itemInfo, id, INV_ITEMBAG, false, nullptr);
        int ovf = 0;
        ((t_add)(g_base + OFF_ADDITEM))(g_itemInfo, id, 1, &ovf, INV_ITEMBAG, true, nullptr);
        int after = ((t_have)(g_base + OFF_GETHAVE))(g_itemInfo, id, INV_ITEMBAG, false, nullptr);
        if (after > before) ((t_Pick)(g_base + OFF_PICKTHIS))(todo[i], nullptr);
    }
}

// ------------------------------------------------------------ auto panen ---
static void harvestAll() {
    typedef void* (*t_getFurrow)(void*, int, void*);
    typedef bool (*t_enable)(void*, void*);
    typedef V2I (*t_rc)(void*, int, void*);
    typedef void (*t_pre)(void*, int, int, void*);
    typedef void* (*t_harvest)(void*, int, int, bool*, void*);
    typedef bool (*t_add)(void*, uint32_t, int, int*, int, bool, void*);
    if (!g_itemInfo) return;
    for (int fi = 0; fi < 16; fi++) {
        void* fld = g_fields[fi].ptr;
        if (!fld || (g_frame - g_fields[fi].frame) > 120) continue;
        int rows = *(int*)((char*)fld + 0x54), cols = *(int*)((char*)fld + 0x58);
        if (rows <= 0 || cols <= 0 || rows * cols > 4000) continue;
        for (int i = 0; i < rows * cols; i++) {
            void* fur = ((t_getFurrow)(g_base + OFF_GETFURROW))(fld, i, nullptr);
            if (!fur) continue;
            if (!((t_enable)(g_base + OFF_FURROWENABLE))(fur, nullptr)) continue;
            V2I rc = ((t_rc)(g_base + OFF_FURROWRC))(fld, i, nullptr);
            ((t_pre)(g_base + OFF_PREHARVEST))(fld, rc.x, rc.y, nullptr);
            bool trans = false;
            void* list = ((t_harvest)(g_base + OFF_HARVEST))(fld, rc.x, rc.y, &trans, nullptr);
            if (!list) continue;
            void* arr = *(void**)((char*)list + 0x10);
            int sz = *(int*)((char*)list + 0x18);
            if (!arr || sz <= 0 || sz > 200) continue;
            for (int k = 0; k < sz; k++) {
                int id = *(int*)((char*)arr + 0x20 + 4 * k);
                int ovf = 0;
                ((t_add)(g_base + OFF_ADDITEM))(g_itemInfo, (uint32_t)id, 1, &ovf, INV_ITEMBAG, true, nullptr);
            }
        }
    }
}

// ------------------------------------------------------------ fish log ----
static int g_fishLogCount = 0;
static const char* fishName(int st) {
    for (int i = 0; i < NFSN; i++) if (FSN[i].v == st) return FSN[i].n;
    return "?";
}
static void fishLog(const char* tag, int st) {
    if (g_fishLogCount > 600) return;
    g_fishLogCount++;
    std::string path = outDir() + "/fish_log.txt";
    FILE* f = fopen(path.c_str(), "a");
    if (!f) return;
    fprintf(f, "%d\t%s\t%d\t%s\n", g_frame, tag, st, fishName(st));
    fclose(f);
}

// ------------------------------------------------- bersihkan kompos kandang -
static void* g_ahMgr = nullptr;
static int g_ahFrame = -1000;
static volatile int g_dirtReq = 0;

static int dirtCount(void* impl) {
    void* data = *(void**)((char*)impl + 0x78);
    if (!data) return 0;
    void* list = *(void**)((char*)data + 0x20);
    if (!list) return 0;
    int n = *(int*)((char*)list + 0x18);
    return (n < 0 || n > 500) ? 0 : n;
}

static void cleanAllDirt() {
    if (!g_ahMgr || (g_frame - g_ahFrame) > 120) return;
    void* impl = *(void**)((char*)g_ahMgr + 0x30);
    if (!impl) return;
    void* dirtPos = *(void**)((char*)impl + 0x68);
    if (!dirtPos) return;
    int total = dirtCount(impl);
    if (total <= 0) return;
    typedef void* (*t_goTr)(void*);
    typedef int (*t_cnt)(void*);
    typedef void* (*t_child)(void*, int);
    typedef void* (*t_compGo)(void*);
    typedef bool (*t_act)(void*);
    typedef void (*t_clean)(void*, void*, void*);
    t_goTr goTr = (t_goTr)resolveIcall("UnityEngine.GameObject::get_transform()", "UnityEngine.GameObject::get_transform");
    t_cnt cnt = (t_cnt)resolveIcall("UnityEngine.Transform::get_childCount()", "UnityEngine.Transform::get_childCount");
    t_child child = (t_child)resolveIcall("UnityEngine.Transform::GetChild(System.Int32)", "UnityEngine.Transform::GetChild");
    t_compGo compGo = (t_compGo)resolveIcall("UnityEngine.Component::get_gameObject()", "UnityEngine.Component::get_gameObject");
    t_act act = (t_act)resolveIcall("UnityEngine.GameObject::get_activeSelf()", "UnityEngine.GameObject::get_activeSelf");
    typedef void (*t_setAct)(void*, bool);
    t_setAct setAct = (t_setAct)resolveIcall("UnityEngine.GameObject::SetActive(System.Boolean)", "UnityEngine.GameObject::SetActive");
    if (!goTr || !cnt || !child || !compGo || !act) return;
    void* tr = goTr(dirtPos);
    if (!tr) return;
    std::vector<void*> gos;
    int n = cnt(tr);
    for (int i = 0; i < n && i < 500; i++) {
        void* c = child(tr, i);
        if (!c) continue;
        int m = cnt(c);
        if (m > 0) {
            for (int j = 0; j < m && j < 20; j++) {
                void* cc = child(c, j);
                void* go = cc ? compGo(cc) : nullptr;
                if (go && act(go)) gos.push_back(go);
            }
        } else {
            void* go = compGo(c);
            if (go && act(go)) gos.push_back(go);
        }
    }
    int done = 0;
    for (size_t i = 0; i < gos.size() && done < total; i++) {
        int before = dirtCount(impl);
        if (before <= 0) break;
        ((t_clean)(g_base + OFF_CLEANDIRT))(impl, gos[i], nullptr);
        if (setAct) setAct(gos[i], false);
        done++;
    }
}

static bool g_fpsApplied = false;
static bool g_frozenSnap = false;

static void doAdvance(int mode) {
    typedef void (*t_Adv)(void*, int, int, void*);
    int h = 0, m = 0;
    if (mode == 1) h = 1;
    else if (mode == 2) h = 3;
    else if (mode == 3) {
        if (!g_timeCtl) return;
        float hr = *(float*)((char*)g_timeCtl + 0x20);
        float mn = *(float*)((char*)g_timeCtl + 0x24);
        int now = (int)hr * 60 + (int)mn;
        int delta = (360 - now + 1440) % 1440;
        if (delta == 0) delta = 1440;
        h = delta / 60;
        m = delta % 60;
    }
    ((t_Adv)(g_base + OFF_ADVTIME))(g_env, h, m, nullptr);
    g_frozenSnap = false;
}

static void h_PMUpdate(void* self, float dt, void* mi) {
    g_pm = self;
    g_frame++;
    uint32_t u = g_tpUid;
    if (u != 0) { g_tpUid = 0; doWarp(u); }
    if (g_warpPending) pollWarp();
    int mreq = g_moneyReq;
    if (mreq != 0 && g_itemInfo) {
        g_moneyReq = 0;
        typedef void (*t_AddMoney)(void*, int, void*);
        ((t_AddMoney)(g_base + OFF_ADDMONEY))(g_itemInfo, mreq, nullptr);
    }
    int treq = g_timeReq;
    if (treq != 0 && g_env) { g_timeReq = 0; doAdvance(treq); }
    if (g_ftUid != 0) {
        uint32_t fu = g_ftUid;
        g_ftUid = 0;
        typedef void (*t_ReqFT2)(void*, uint32_t, void*);
        ((t_ReqFT2)(g_base + OFF_REQFT))(self, fu, nullptr);
    }
    if (g_harvestReq) { g_harvestReq = 0; harvestAll(); }
    if (g_dirtReq) { g_dirtReq = 0; cleanAllDirt(); }
    int sv = g_saveReq;
    if (sv >= 1 && sv <= 3) {
        g_saveReq = 0;
        Vec3 p;
        if (getPlayerPos(&p)) { g_slot[sv - 1] = p; g_slotSet[sv - 1] = 1; savePosFile(); }
    }
    int gt = g_gotoReq;
    if (gt >= 1 && gt <= 3) {
        g_gotoReq = 0;
        if (g_slotSet[gt - 1]) warpTo(g_slot[gt - 1], 0.0f);
    }
    if (g_pickRange > 0 && (g_frame % 8) == 0) autoPick();
    if ((g_frame % 60) == 0) {
        if (g_f[17] && !g_fpsApplied) { setFps(60); g_fpsApplied = true; }
        else if (!g_f[17] && g_fpsApplied) { setFps(30); g_fpsApplied = false; }
        else if (g_f[17]) setFps(60);
    }
    o_PMUpdate(self, dt, mi);
}

static bool h_AddItem(void* self, uint32_t id, int num, int* overflow, int type, bool upd, void* mi) {
    g_itemInfo = self;
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

static void h_FIInit(void* self, int index, Vec3 pos, bool fixedFlag, uint32_t id, uint32_t colony, void* e, bool fixGround, void* mi) {
    bool found = false;
    for (size_t i = 0; i < g_items.size(); i++) {
        if (g_items[i].ptr == self) { g_items[i].pos = pos; g_items[i].next = 0; found = true; break; }
    }
    if (!found && g_items.size() < 3000) { ItemRef r; r.ptr = self; r.pos = pos; r.next = 0; g_items.push_back(r); }
    o_FIInit(self, index, pos, fixedFlag, id, colony, e, fixGround, mi);
}

static void h_FIDestroy(void* self, void* mi) {
    for (size_t i = 0; i < g_items.size(); i++) {
        if (g_items[i].ptr == self) { g_items.erase(g_items.begin() + i); break; }
    }
    o_FIDestroy(self, mi);
}

static void h_MineUpd(void* self, float dt, void* mi) {
    g_mine = self;
    int n = g_floorReq;
    if (n > 0) {
        g_floorReq = 0;
        typedef void (*t_SetFloor)(void*, int, void*);
        typedef UniTask (*t_Next)(void*, bool, bool, void*);
        ((t_SetFloor)(g_base + OFF_SETFLOOR))(self, n - 1, nullptr);
        ((t_Next)(g_base + OFF_CREATENEXT))(self, false, true, nullptr);
    }
    o_MineUpd(self, dt, mi);
}

static int h_Cap(void* self, uint32_t category, void* mi) {
    int v = o_Cap(self, category, mi);
    if (g_f[14]) v += 60;
    return v;
}

static void h_CropUpd(void* self, float dt, void* mi) {
    int slot = -1;
    for (int i = 0; i < 16; i++) {
        if (g_fields[i].ptr == self) { slot = i; break; }
    }
    if (slot < 0) {
        for (int i = 0; i < 16; i++) {
            if (!g_fields[i].ptr || (g_frame - g_fields[i].frame) > 600) { slot = i; break; }
        }
    }
    if (slot >= 0) { g_fields[slot].ptr = self; g_fields[slot].frame = g_frame; }
    o_CropUpd(self, dt, mi);
}

static bool h_Remove(void* self, uint32_t id, int num, int type, void* mi) {
    if (g_f[15]) {
        uintptr_t rva = (uintptr_t)__builtin_return_address(0) - g_base;
        for (int i = 0; i < NKEEP; i++) {
            if (rva >= KEEP[i].start && rva < KEEP[i].start + KEEP[i].size) return true;
        }
    }
    return o_Remove(self, id, num, type, mi);
}

static void h_Dowse(void* self, float t, void* mi) {
    if (g_f[16]) t = 100000.0f;
    o_Dowse(self, t, mi);
}

static void h_FishCS(void* self, int state, void* mi) {
    if (g_f[7]) fishLog("ChangeState", state);
    o_FishCS(self, state, mi);
}

static int h_AddBag(uint32_t id, int count, bool noticeT, bool showLog, bool playSe, void* mi) {
    if (g_f[19]) {
        uintptr_t rva = (uintptr_t)__builtin_return_address(0) - g_base;
        if (rva >= GETFISH_START && rva < GETFISH_START + GETFISH_SIZE) {
            typedef uint32_t (*t_great)(uint32_t, void*);
            uint32_t g = ((t_great)(g_base + OFF_GREATQ))(id, nullptr);
            if (g != 0 && g != id) id = g;
        }
    }
    return o_AddBag(id, count, noticeT, showLog, playSe, mi);
}

static void h_AHUpd(void* self, float dt, void* mi) {
    g_ahMgr = self;
    g_ahFrame = g_frame;
    o_AHUpd(self, dt, mi);
}

static int h_GetHave(void* self, uint32_t id, int type, bool lv, void* mi) {
    g_itemInfo = self;
    return o_GetHave(self, id, type, lv, mi);
}

static void h_EnvLate(void* self, float dt, void* mi) {
    g_env = self;
    o_EnvLate(self, dt, mi);
}

static int g_fightFrames = 0;
static int g_lastLogSt = 0;
static void h_Fight(void* self, float dt, void* mi) {
    int st = *(int*)((char*)self + 0x58);
    if (g_f[7] && st != g_lastLogSt) { g_lastLogSt = st; fishLog("UpdateFighting", st); }
    if (g_f[7] && st == FISH_FIGHTING) {
        g_fightFrames++;
        if (g_fightFrames > 25) {
            g_fightFrames = 0;
            typedef void (*t_State)(void*, int, void*);
            ((t_State)(g_base + OFF_FISHSTATE))(self, FISH_SUCCESS, nullptr);
            return;
        }
    } else {
        g_fightFrames = 0;
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
static float g_snap[6];
static const int SNAP_OFF[6] = {0x20, 0x24, 0x2C, 0x30, 0x34, 0x38};
static void h_TimeLate(void* self, float dt, void* mi) {
    g_timeCtl = self;
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
    if (*(bool*)((char*)self + 0x28)) {
        int m = g_speed;
        if (m > 1) v *= (float)m;
    } else {
        int r = g_rideSpeed;
        if (r > 1 && g_player) {
            void* ride = *(void**)((char*)g_player + 0xA8);
            if (ride && *(int*)((char*)ride + 0x10) != 0) v *= (float)r;
        }
    }
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
    installHook(H_GETHAVE, (void*)h_GetHave, (void**)&o_GetHave);
    installHook(H_ENVLATE, (void*)h_EnvLate, (void**)&o_EnvLate);
    installHook(H_FIINIT, (void*)h_FIInit, (void**)&o_FIInit);
    installHook(H_FIDESTROY, (void*)h_FIDestroy, (void**)&o_FIDestroy);
    installHook(H_MINEUPD, (void*)h_MineUpd, (void**)&o_MineUpd);
    installHook(H_CAP, (void*)h_Cap, (void**)&o_Cap);
    installHook(H_CROPUPD, (void*)h_CropUpd, (void**)&o_CropUpd);
    installHook(H_REMOVE, (void*)h_Remove, (void**)&o_Remove);
    installHook(H_DOWSE, (void*)h_Dowse, (void**)&o_Dowse);
    installHook(H_FISHCS, (void*)h_FishCS, (void**)&o_FishCS);
    installHook(H_AHUPD, (void*)h_AHUpd, (void**)&o_AHUpd);
    installHook(H_ADDBAG, (void*)h_AddBag, (void**)&o_AddBag);
    loadPosFile();
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
    "Collapse_Kebutuhan Harian",
    "CollapseAdd_3_Toggle_Auto Watered Crop",
    "CollapseAdd_6_Toggle_Crop Grew Overnight",
    "CollapseAdd_5_Toggle_Max Quality Crops",
    "CollapseAdd_46_Button_Panen Semua Tanaman",
    "CollapseAdd_15_Toggle_Bibit, Pupuk dan Pakan Tidak Habis",
    "CollapseAdd_4_Toggle_Get Item x99",
    "CollapseAdd_14_Toggle_Bawa Lebih Banyak Item (tambah slot tas)",
    "CollapseAdd_18_SeekBar_Jarak Auto Ambil Barang (0=mati)_0_30",
    "CollapseAdd_9_Toggle_Unlimited Gifts/Talks",
    "Collapse_Pemain",
    "CollapseAdd_2_Toggle_Infinite Stamina",
    "CollapseAdd_1_Toggle_Freeze Time",
    "CollapseAdd_12_SeekBar_Kecepatan Pemain_1_6",
    "CollapseAdd_13_SeekBar_Kecepatan Tunggangan_1_6",
    "Collapse_Peternakan",
    "CollapseAdd_8_Toggle_Animal Care",
    "CollapseAdd_7_Toggle_Easy Fishing (Auto Pull)",
    "CollapseAdd_19_Toggle_Ikan Selalu Kualitas Maksimal",
    "CollapseAdd_47_Button_Bersihkan Semua Kompos Kandang",
    "Collapse_Tambang",
    "CollapseAdd_10_Toggle_Batu Sekali Pukul",
    "CollapseAdd_11_Toggle_Tanpa Jebakan dan Batu Jatuh",
    "CollapseAdd_16_Toggle_Radar Tambang Lama",
    "CollapseAdd_60_Button_Lantai 1 sampai 10 (mulai lantai 1)",
    "CollapseAdd_61_Button_Lantai 11 sampai 20 (mulai lantai 11)",
    "CollapseAdd_62_Button_Lantai 21 sampai 30 (mulai lantai 21)",
    "CollapseAdd_63_Button_Lantai 31 sampai 40 (mulai lantai 31)",
    "CollapseAdd_64_Button_Lantai 41 sampai 50 (mulai lantai 41)",
    "CollapseAdd_65_Button_Lantai 51 sampai 60 (mulai lantai 51)",
    "CollapseAdd_66_Button_Lantai 61 sampai 70 (mulai lantai 61)",
    "CollapseAdd_67_Button_Lantai 71 sampai 80 (mulai lantai 71)",
    "CollapseAdd_68_Button_Lantai 81 sampai 90 (mulai lantai 81)",
    "CollapseAdd_69_Button_Lantai 91 sampai 100 (mulai lantai 91)",
    "Collapse_Uang dan Waktu",
    "CollapseAdd_40_Button_Tambah Uang +10.000",
    "CollapseAdd_41_Button_Tambah Uang +100.000",
    "CollapseAdd_42_Button_Tambah Uang +1.000.000",
    "CollapseAdd_43_Button_Maju Waktu 1 Jam",
    "CollapseAdd_44_Button_Maju Waktu 3 Jam",
    "CollapseAdd_45_Button_Lompat ke Pagi (06:00)",
    "Collapse_Teleport Luar Ruangan",
    "CollapseAdd_20_Button_Rumah",
    "CollapseAdd_21_Button_Tengah Desa",
    "CollapseAdd_22_Button_Balai Desa",
    "CollapseAdd_26_Button_Tambang (pintu luar)",
    "CollapseAdd_27_Button_Puncak Gunung",
    "CollapseAdd_28_Button_Pemandian Air Panas",
    "CollapseAdd_30_Button_Lomba Pancing 1",
    "CollapseAdd_31_Button_Lomba Pancing 2",
    "CollapseAdd_32_Button_Lomba Pancing 3",
    "Collapse_Teleport Dalam Ruangan",
    "CollapseAdd_70_Button_Balai Desa",
    "CollapseAdd_71_Button_Kantor Berita",
    "CollapseAdd_72_Button_Toko Suvenir",
    "CollapseAdd_73_Button_Rumah Nelayan",
    "CollapseAdd_74_Button_Restoran",
    "CollapseAdd_75_Button_Toko Umum",
    "CollapseAdd_76_Button_Toko Kerajinan dan Pakaian",
    "CollapseAdd_77_Button_Toko Hewan",
    "CollapseAdd_78_Button_Kandang Toko Hewan",
    "CollapseAdd_79_Button_Rumah Petani Besar",
    "CollapseAdd_80_Button_Rumah Penjaga Tambang",
    "CollapseAdd_81_Button_Balai Desa (pintu belakang)",
    "Collapse_Lainnya",
    "CollapseAdd_17_Toggle_FPS 60 (coba-coba)",
    "CollapseAdd_50_Button_Simpan Posisi 1",
    "CollapseAdd_51_Button_Simpan Posisi 2",
    "CollapseAdd_52_Button_Simpan Posisi 3",
    "CollapseAdd_53_Button_Teleport ke Posisi 1",
    "CollapseAdd_54_Button_Teleport ke Posisi 2",
    "CollapseAdd_55_Button_Teleport ke Posisi 3",
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
    env->CallVoidMethod(subtitle, setText, env->NewStringUTF("mod by NS"));
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
    } else if ((featNum >= 14 && featNum <= 17) || featNum == 19) {
        g_f[featNum] = (b != JNI_FALSE);
    } else if (featNum == 18) {
        int v = value;
        if (v < 0) v = 0;
        if (v > 30) v = 30;
        g_pickRange = v;
    } else if (featNum == 46) {
        if (!g_itemInfo || !g_pm) { toast(env, ctx, "Buka tas atau ambil satu item dulu, lalu coba lagi"); return; }
        g_harvestReq = 1;
    } else if (featNum == 47) {
        if (!g_ahMgr || (g_frame - g_ahFrame) > 120) { toast(env, ctx, "Masuk ke dalam kandang dulu"); return; }
        g_dirtReq = 1;
        toast(env, ctx, "Membersihkan kompos kandang");
    } else if (featNum >= 50 && featNum <= 52) {
        g_saveReq = featNum - 49;
        toast(env, ctx, "Posisi disimpan");
    } else if (featNum >= 53 && featNum <= 55) {
        if (!g_slotSet[featNum - 53]) { toast(env, ctx, "Slot ini masih kosong, simpan posisi dulu"); return; }
        g_gotoReq = featNum - 52;
    } else if (featNum >= 60 && featNum <= 69) {
        if (!g_mine) { toast(env, ctx, "Masuk ke dalam tambang dulu"); return; }
        g_floorReq = (featNum - 60) * 10 + 1;
        toast(env, ctx, "Pindah lantai tambang");
    } else if (featNum == 13) {
        int v = value;
        if (v < 1) v = 1;
        if (v > 6) v = 6;
        g_rideSpeed = v;
    } else if (featNum >= 40 && featNum <= 42) {
        if (!g_itemInfo) { toast(env, ctx, "Buka tas atau ambil satu item dulu, lalu coba lagi"); return; }
        g_moneyReq = (featNum == 40) ? 10000 : (featNum == 41) ? 100000 : 1000000;
    } else if (featNum >= 43 && featNum <= 45) {
        if (!g_env) { toast(env, ctx, "Belum siap, masuk ke dunia game dulu"); return; }
        g_timeReq = featNum - 42;
    } else {
        for (int i = 0; i < NIN; i++) {
            if (IN[i].feat != featNum) continue;
            if (!g_pm) { toast(env, ctx, "Teleport belum siap, masuk ke dunia game dulu"); return; }
            g_ftUid = IN[i].uid;
            toast(env, ctx, IN[i].label);
            return;
        }
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
