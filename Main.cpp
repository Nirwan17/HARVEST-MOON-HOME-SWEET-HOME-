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
};
#define NPATCH 6

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
static const HookDef H_MSGGROUP = {0x28CCAF0, {0xD10143FF, 0xF90013FE, 0xA90357F6, 0xA9044FF4}, "MessageManager.GetMessageGroup"};

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
    {0x28E1114, 5012},
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
#define OFF_GETITEMACC 0x2567588
#define OFF_ITEMNAME 0x2511C9C
#define OFF_ITEMCAT 0x2511EAC
#define OFF_ITEMSUB 0x2511F30
#define OFF_ITEMKIND 0x2511FB4
#define OFF_ITEMSELL 0x2512608
#define OFF_GETMSG 0x28CE754
static const uint32_t G0[] = {50106,50000,50041,50137,50218,50008,50009,50016,50040,50024,50196,50237,50114,50032,50179,50160,50019,50091,50169,50048,50033,50168,59999,50056,50176,50074,50064,50052,50184,50192,50153,50225,50072,50213,50003,50088,50161,50096,50080,50208,50216,50147,50066,50027,50129,50112,50120,50200,50104,50224,50098,50128,50232,50136,50240,50203,50185,50121,50152,50144,50058,50081};
static const uint32_t G1[] = {10047,20047,30047,40047,10106,20106,30106,40106,10046,20046,30046,40046,10109,20109,30109,40109,10000,20000,30000,40000,10041,20041,30041,40041,10145,20145,30145,40145,10137,20137,30137,40137,10042,20042,30042,40042,10097,20097,30097,40097,10013,20013,30013,40013,10030,20030,30030,40030,10071,20071,30071,40071,10157,20157,30157,40157,10008,20008,30008,40008,10009,20009,30009,40009,10016,20016,30016,40016,10154,20154,30154,40154,10040,20040,30040,40040,10010,20010,30010,40010,10024,20024,30024,40024,10114,20114,30114,40114,10078,20078,30078,40078,10032,20032,30032,40032,10085,20085,30085,40085,10133,20133,30133,40133,10038,20038,30038,40038,10092,20092,30092,40092,10116,20116,30116,40116,10019,20019,30019,40019,10091,20091,30091,40091,10048,20048,30048,40048,10065,20065,30065,40065,10014,20014,30014,40014,10033,20033,30033,40033,10075,20075,30075,40075,10083,20083,30083,40083,10143,20143,30143,40143,10036,20036,30036,40036,10158,20158,30158,40158,10090,20090,30090,40090,10060,20060,30060,40060,10056,20056,30056,40056,10113,20113,30113,40113,10139,20139,30139,40139,10067,20067,30067,40067,10102,20102,30102,40102,10130,20130,30130,40130,10028,20028,30028,40028,10084,20084,30084,40084,10079,20079,30079,40079,10123,20123,40123,30123,10131,20131,30131,40131,10148,20148,30148,40148,10074,20074,30074,40074,10115,20115,30115,40115,10064,20064,30064,40064,10052,20052,30052,40052,10125,20125,40125,30125,10101,20101,30101,40101,10005,20005,30005,40005,10059,20059,30059,40059,10153,20153,30153,40153,10025,20025,30025,40025,10072,20072,30072,40072,10089,20089,30089,40089,10018,20018,30018,40018,10003,20003,30003,40003,10088,20088,30088,40088,10127,20127,30127,40127,10021,20021,30021,40021,10096,20096,30096,40096,10108,20108,30108,40108,10080,20080,30080,40080,10002,20002,30002,40002,10057,20057,30057,40057,10017,20017,30017,40017,10055,20055,30055,40055,10069,20069,30069,40069,10073,20073,30073,40073,10141,20141,30141,40141,10147,20147,30147,40147,10053,20053,30053,40053,10066,20066,30066,40066,10011,20011,30011,40011,10027,20027,30027,40027,10146,20146,30146,40146,10129,20129,30129,40129,10124,20124,30124,40124,10100,20100,30100,40100,10111,20111,30111,40111,10149,20149,30149,40149,10156,20156,30156,40156,10112,20112,30112,40112,10007,20007,30007,40007,10120,20120,40120,30120,10082,20082,30082,40082,10104,20104,30104,40104,10132,20132,30132,40132,10035,20035,30035,40035,10034,20034,30034,40034,10098,20098,30098,40098,10128,20128,30128,40128,10136,20136,30136,40136,10138,20138,30138,40138,10094,20094,30094,40094,10054,20054,30054,40054,10121,20121,30121,40121,10152,20152,30152,40152,10144,20144,30144,40144,10001,20001,30001,40001,10062,20062,30062,40062,10105,20105,30105,40105,10043,20043,30043,40043,10026,20026,30026,40026,10051,20051,30051,40051,10117,20117,30117,40117,10022,20022,30022,40022,10058,20058,30058,40058,10081,20081,30081,40081};
static const uint32_t G2[] = {10199,20199,30199,40199,10166,20166,30166,40166,10207,20207,30207,40207,10182,20182,30182,40182,10218,20218,30218,40218,10204,20204,30204,40204,10196,20196,30196,40196,10238,20238,30238,40237,10210,20210,30210,40210,10179,20179,30179,40179,10160,20160,30160,40160,10229,20229,30229,40228,10174,20174,30174,40174,10169,20169,30169,40169,10214,20214,30214,40214,10209,20209,30209,40209,10168,20168,30168,40168,10176,20176,30176,40176,10177,20177,30177,40177,10239,20239,30239,40238,10164,20164,30164,40164,10237,20237,30237,40236,10170,20170,30170,40170,10231,20231,30231,40230,10230,20230,30230,40229,10184,20184,30184,40184,10186,20186,30186,40186,10192,20192,30192,40192,10220,20220,30220,40220,10226,20226,30226,40225,10189,20189,30189,40189,10187,20187,30187,40187,10162,20162,30162,40162,10213,20213,30213,40213,10161,20161,30161,40161,10194,20194,30194,40194,10215,20215,30215,40215,10217,20217,30217,40217,10195,20195,30195,40195,10208,20208,30208,40208,10216,20216,30216,40216,10190,20190,30190,40190,10206,20206,30206,40206,10221,20221,30221,40221,10171,20171,30171,40171,10200,20200,30200,40200,10178,20178,30178,40178,10225,20225,30225,40224,10233,20233,30233,40232,10203,20203,30203,40203,10180,20180,30180,40180,10201,20201,30201,40201,10173,20173,30173,40173,10185,20185,30185,40185,10163,20163,30163,40163,10227,20227,30227,40226,10219,20219,30219,40219,10235,20235,30235,40234,10193,20193,30193,40193,10234,20234,30234,40233};
static const uint32_t G3[] = {110028,120028,130028,131028,110009,120009,130009,131009,110011,120011,130011,131011,110035,120035,130035,131035,110063,120063,130063,131060,110030,120030,130030,131030,110045,120045,130045,131045,110053,120053,130053,131053,110036,120036,130036,131036,110021,120021,130021,131021,110050,120050,130050,131050,110002,120002,130002,131002,110041,120041,130041,131041,110056,120056,130056,131056,110000,120000,130000,131000,110018,120018,130018,131018,110006,120006,130006,131006,110029,120029,130029,131029,110065,120065,130065,131062,110008,120008,130008,131008,110001,120001,130001,131001,110010,120010,130010,131010,110033,120033,130033,131033,110064,120064,130064,131061,110055,120055,130055,131055,110066,120066,130066,131063,110023,120023,130023,131023,110016,120016,130016,131016,110034,120034,130034,131034,110015,120015,130015,131015,110026,120026,130026,131026,110017,120017,130017,131017,110013,120013,130013,131013,110042,120042,130042,131042,110032,120032,130032,131032,110047,120047,130047,131047,110020,120020,130020,131020,110044,120044,130044,131044,110004,120004,130004,131004,110040,120040,130040,131040,110043,120043,130043,131043,110038,120038,130038,131038,110061,120061,130061,131058,110007,120007,130007,131007,110024,120024,130024,131024,110062,120062,130062,131059,110012,120012,130012,131012,110005,120005,130005,131005,110052,120052,130052,131052,110014,120014,130014,131014,110003,120003,130003,131003,110019,120019,130019,131019,110027,120027,130027,131027,110049,120049,130049,131049};
static const uint32_t G4[] = {160009,160006,160026,160003,160028,160007,160010,160015,160004,160018,160025,160019,160013,160000,160001,160017,160002,160016,160005,160027,160023,160014,160024,160008};
static const uint32_t G5[] = {181009,180009,181011,181026,181014,181019,200001,181002,180002,181013,181007,180007,181023,181020,181015,181010,181004,180004,200002,200004,181001,180001,181012,200000,181000,181016,181018,181008,180008,200007,200006,200008,181025,181006,180006,180011,180026,180014,180019,180013,180023,180020,180015,180010,180012,180016,180018,180022,180021,180024,180017,180025,181022,181021,181003,180003,200003,180000,200010,200009,200011,181024,181005,180005,181017};
static const uint32_t G6[] = {80004,80014,80022,80030,80002,80012,80020,80028,80003,80013,80021,80029,80001,80011,80019,80027,80000,80010,80018,80026,80005,80015,80023,80031};
static const uint32_t G7[] = {140001,100004,60000,60010,60007,60001,60012,19999,140002,100001,100002,100003,140000,100000,100005,60005,60006,60009,170000,60008};
static const uint32_t G8[] = {150018,150331,150631,150831,150006,150309,150609,150809,150088,150418,150718,150918,150060,150383,150683,150883,150107,150444,150744,150944,150051,150371,150671,150871,150166,150103,150438,150738,150938,150029,150343,150643,150843,150015,150325,150625,150825,150164,150464,150764,150964,150154,150454,150754,150954,150077,150402,150702,150902,150102,150437,150737,150937,150065,150390,150690,150890,150008,150311,150611,150811,150155,150455,150755,150955,150096,150430,150730,150930,150019,150332,150632,150832,150091,150424,150724,150924,150039,150357,150657,150857,150161,150461,150761,150961,150031,150345,150645,150845,150109,150448,150748,150948,150055,150375,150675,150875,150084,150413,150713,150913,150083,150412,150712,150912,150314,150116,150614,150814,150163,150463,150763,150963,150011,150319,150619,150819,150030,150344,150644,150844,150071,150396,150696,150896,150047,150366,150666,150866,150093,150427,150727,150927,150014,150322,150622,150822,150146,150435,150735,150935,150094,150428,150728,150928,150080,150405,150705,150905,150159,150459,150759,150959,150017,150327,150627,150827,150046,150365,150665,150865,150032,150346,150646,150846,150041,150359,150659,150859,150013,150321,150621,150821,150068,150393,150693,150893,150100,150434,150734,150934,150134,150382,150682,150882,150148,150442,150742,150942,150027,150340,150640,150840,150165,150465,150765,150965,150081,150408,150708,150908,150069,150394,150694,150894,150072,150397,150697,150897,150115,150313,150613,150813,150151,150451,150751,150951,150062,150387,150687,150887,150106,150443,150743,150943,150090,150422,150722,150922,150028,150342,150642,150842,150038,150356,150656,150856,150005,150305,150605,150805,150010,150315,150615,150815,150044,150363,150663,150863,150048,150368,150668,150868,150087,150417,150717,150917,150036,150353,150653,150853,150053,150373,150673,150873,150156,150456,150756,150956,150153,150453,150753,150953,150078,150403,150703,150903,150089,150419,150719,150919,150061,150384,150684,150884,150108,150445,150745,150945,150052,150372,150672,150872,150152,150452,150752,150952,150142,150420,150720,150920,150135,150385,150685,150885,150149,150446,150746,150946,150037,150354,150654,150854,150054,150374,150674,150874,150033,150349,150649,150849,150007,150310,150610,150810,150076,150401,150701,150901,150063,150388,150688,150888,150138,150407,150707,150907,150079,150404,150704,150904,150070,150395,150695,150895,150067,150392,150692,150892,150145,150425,150725,150925,150125,150341,150641,150841,150057,150378,150678,150878,150003,150303,150603,150803,150085,150414,150714,150914,150058,150379,150679,150879,150104,150439,150739,150939,150130,150362,150662,150862,150049,150369,150669,150869,150144,150423,150723,150923,150132,150376,150676,150876,150162,150462,150762,150962,150157,150457,150757,150957,150035,150351,150651,150851,150082,150410,150710,150910,150095,150429,150729,150929,150016,150326,150626,150826,150004,150304,150604,150804,150066,150391,150691,150891,150012,150320,150620,150820,150073,150398,150698,150898,150074,150399,150699,150899,150002,150137,150406,150706,150906,150009,150312,150612,150812,150034,150350,150650,150850,150101,150436,150736,150936,150126,150347,150647,150847,150141,150416,150716,150916,150133,150381,150681,150881,150147,150441,150741,150941,150128,150352,150652,150852,150075,150400,150700,150900,150064,150389,150689,150889};
static const uint32_t* const GROUP_ITEMS[9] = {G0,G1,G2,G3,G4,G5,G6,G7,G8};
static const int GROUP_N[9] = {62,480,240,216,24,65,24,20,482};
#define ITEM_MENU_ENTRIES \
    "Collapse_Item Pilihan", \
    "CollapseAdd_90_SeekBar_Jumlah Item_1_99", \
    "CollapseAdd_91_Spinner_Bibit_Pilih item,Angel Lantern Seeds,Asparagus Seeds,Baby Carrot Seeds,Beet Seeds,Blue Rose Seeds,Broccoli Seeds,Broccolini Seeds,Cabbage Seeds,Carrot Seeds,Celery Seeds,Chamomile Seeds,Champagne Flute Seeds,Chickpea Seeds,Corn Seeds,Cotton Rosemallow Seeds,Daisy Seeds,Dome Cabbage Seeds,Dragon Pineapple Seeds,Edelweiss Seeds,Eggplant Seeds,Fodder Corn Seeds,Goldband Lily Seeds,Grass Seeds,Green Bell Pepper Seeds,Hibiscus Seeds,Leek Seeds,Lettuce Seeds,Lightbulb Eggplant Seeds,Lotus Seeds,Marguerite Seeds,Mellow Yellow Seeds,Moulin Rouge Seeds,Onion Seeds,Orchid Cactus Seeds,Phalanx Seeds,Pineapple Seeds,Pink Daisy Seeds,Potato Seeds,Pumpkin Seeds,Queen of the Night Seeds,Red Rose Seeds,Rice Seeds,Romaine Lettuce Seeds,Royal Herb Seeds,San Marzano Seeds,Soybean Seeds,Spinach Seeds,Strawberry Pansy Seeds,Strawberry Seeds,Sunflower Seeds,Sweet Potato Seeds,Tomato Seeds,Tulip Seeds,Turnip Seeds (jual 5),Turnip Seeds (jual 5) #2,Viola Seeds,Water Lotus Seeds,Watercress Seeds,Watermelon Seeds,Wheat Seeds,Yellow Bell Pepper Seeds,Zucchini Seeds", \
    "CollapseAdd_92_Spinner_Hasil Panen_Pilih item,Alraune (jual 83),Alraune (jual 124),Alraune (jual 149),Alraune (jual 207),Angel Lantern (jual 190),Angel Lantern (jual 285),Angel Lantern (jual 342),Angel Lantern (jual 475),Aqua Carrot (jual 70),Aqua Carrot (jual 104),Aqua Carrot (jual 125),Aqua Carrot (jual 152),Aqua Strawberry (jual 176),Aqua Strawberry (jual 264),Aqua Strawberry (jual 317),Aqua Strawberry (jual 440),Asparagus (jual 110),Asparagus (jual 165),Asparagus (jual 198),Asparagus (jual 225),Baby Carrot (jual 75),Baby Carrot (jual 113),Baby Carrot (jual 135),Baby Carrot (jual 188),Barley (jual 75),Barley (jual 113),Barley (jual 135),Barley (jual 165),Beet (jual 115),Beet (jual 173),Beet (jual 207),Beet (jual 288),Black Carrot (jual 70),Black Carrot (jual 104),Black Carrot (jual 125),Black Carrot (jual 152),Black Potato (jual 77),Black Potato (jual 116),Black Potato (jual 139),Black Potato (jual 275),Blue Broccoli (jual 132),Blue Broccoli (jual 198),Blue Broccoli (jual 238),Blue Broccoli (jual 330),Blue Herb (jual 143),Blue Herb (jual 215),Blue Herb (jual 258),Blue Herb (jual 275),Blue Lettuce (jual 176),Blue Lettuce (jual 264),Blue Lettuce (jual 317),Blue Lettuce (jual 440),Blue Melon (jual 220),Blue Melon (jual 330),Blue Melon (jual 396),Blue Melon (jual 550),Broccoli (jual 84),Broccoli (jual 126),Broccoli (jual 152),Broccoli (jual 200),Broccolini (jual 84),Broccolini (jual 126),Broccolini (jual 152),Broccolini (jual 300),Cabbage (jual 50),Cabbage (jual 75),Cabbage (jual 90),Cabbage (jual 125),Cannonball (jual 264),Cannonball (jual 396),Cannonball (jual 476),Cannonball (jual 660),Carrot (jual 63),Carrot (jual 95),Carrot (jual 114),Carrot (jual 138),Cauliflower (jual 93),Cauliflower (jual 139),Cauliflower (jual 167),Cauliflower (jual 220),Celery (jual 130),Celery (jual 195),Celery (jual 234),Celery (jual 250),Chickpea (jual 195),Chickpea (jual 293),Chickpea (jual 351),Chickpea (jual 463),Colorful Teardrop (jual 105),Colorful Teardrop (jual 157),Colorful Teardrop (jual 189),Colorful Teardrop (jual 262),Corn (jual 140),Corn (jual 210),Corn (jual 252),Corn (jual 275),Cream Pumpkin (jual 187),Cream Pumpkin (jual 281),Cream Pumpkin (jual 337),Cream Pumpkin (jual 523),Crimson (jual 98),Crimson (jual 147),Crimson (jual 177),Crimson (jual 234),Crystal Corn (jual 198),Crystal Corn (jual 297),Crystal Corn (jual 357),Crystal Corn (jual 495),Crystal Pineapple (jual 143),Crystal Pineapple (jual 215),Crystal Pineapple (jual 258),Crystal Pineapple (jual 358),Diamond Pea (jual 215),Diamond Pea (jual 322),Diamond Pea (jual 387),Diamond Pea (jual 509),Dome Cabbage (jual 120),Dome Cabbage (jual 180),Dome Cabbage (jual 216),Dome Cabbage (jual 300),Dragon Pineapple (jual 190),Dragon Pineapple (jual 285),Dragon Pineapple (jual 342),Dragon Pineapple (jual 475),Eggplant (jual 81),Eggplant (jual 122),Eggplant (jual 146),Eggplant (jual 188),Endive (jual 165),Endive (jual 248),Endive (jual 297),Endive (jual 358),Evergreen (jual 93),Evergreen (jual 139),Evergreen (jual 167),Evergreen (jual 220),Fodder Corn (jual 180),Fodder Corn (jual 270),Fodder Corn (jual 324),Fodder Corn (jual 450),Garlic (jual 105),Garlic (jual 157),Garlic (jual 189),Garlic (jual 262),Giant Squash (jual 187),Giant Squash (jual 281),Giant Squash (jual 337),Giant Squash (jual 523),Giant's Heart (jual 127),Giant's Heart (jual 190),Giant's Heart (jual 228),Giant's Heart (jual 317),Glass Gem Corn (jual 198),Glass Gem Corn (jual 297),Glass Gem Corn (jual 357),Glass Gem Corn (jual 495),Glitterball (jual 220),Glitterball (jual 330),Glitterball (jual 396),Glitterball (jual 550),Gold Barrel Pineapple (jual 143),Gold Barrel Pineapple (jual 215),Gold Barrel Pineapple (jual 258),Gold Barrel Pineapple (jual 358),Golden Bell Pepper (jual 149),Golden Bell Pepper (jual 223),Golden Bell Pepper (jual 268),Golden Bell Pepper (jual 289),Green Bell Pepper (jual 135),Green Bell Pepper (jual 203),Green Bell Pepper (jual 243),Green Bell Pepper (jual 263),Green Pea (jual 182),Green Pea (jual 273),Green Pea (jual 327),Green Pea (jual 344),Heart of the Earth (jual 35),Heart of the Earth (jual 52),Heart of the Earth (jual 62),Heart of the Earth (jual 97),Ice Lettuce (jual 176),Ice Lettuce (jual 264),Ice Lettuce (jual 317),Ice Lettuce (jual 440),Ice Potato (jual 77),Ice Potato (jual 116),Ice Potato (jual 139),Ice Potato (jual 275),Ice Tomato (jual 98),Ice Tomato (jual 147),Ice Tomato (jual 177),Ice Tomato (jual 234),Icy Celery (jual 220),Icy Celery (jual 330),Icy Celery (jual 396),Icy Celery (jual 550),Jack-o'-Lantern (jual 187),Jack-o'-Lantern (jual 281),Jack-o'-Lantern (jual 337),Jack-o'-Lantern (jual 523),Jewel Onion (jual 50),Jewel Onion (jual 75),Jewel Onion (jual 90),Jewel Onion (jual 124),Kale (jual 123),Kale (jual 184),Kale (jual 207),Kale (jual 220),Killer Tomato (jual 248),Killer Tomato (jual 372),Killer Tomato (jual 446),Killer Tomato (jual 619),Lava Rice (jual 209),Lava Rice (jual 314),Lava Rice (jual 377),Lava Rice (jual 413),Leek (jual 95),Leek (jual 143),Leek (jual 171),Leek (jual 238),Lentil (jual 215),Lentil (jual 322),Lentil (jual 387),Lentil (jual 509),Lettuce (jual 150),Lettuce (jual 225),Lettuce (jual 270),Lettuce (jual 325),Lightbulb Eggplant (jual 145),Lightbulb Eggplant (jual 218),Lightbulb Eggplant (jual 261),Lightbulb Eggplant (jual 363),Lilac Spinach (jual 123),Lilac Spinach (jual 184),Lilac Spinach (jual 207),Lilac Spinach (jual 220),Long Sweet Potato (jual 231),Long Sweet Potato (jual 347),Long Sweet Potato (jual 416),Long Sweet Potato (jual 578),Macedonia (jual 220),Macedonia (jual 330),Macedonia (jual 396),Macedonia (jual 550),Magic Bell Pepper (jual 204),Magic Bell Pepper (jual 306),Magic Bell Pepper (jual 367),Magic Bell Pepper (jual 427),Mellow Yellow (jual 240),Mellow Yellow (jual 360),Mellow Yellow (jual 432),Mellow Yellow (jual 600),Mystic Herb (jual 220),Mystic Herb (jual 330),Mystic Herb (jual 396),Mystic Herb (jual 550),Onion (jual 45),Onion (jual 68),Onion (jual 81),Onion (jual 113),Peach Pineapple (jual 143),Peach Pineapple (jual 215),Peach Pineapple (jual 258),Peach Pineapple (jual 358),Pepper Cabbage (jual 55),Pepper Cabbage (jual 83),Pepper Cabbage (jual 99),Pepper Cabbage (jual 138),Phalanx (jual 200),Phalanx (jual 300),Phalanx (jual 360),Phalanx (jual 500),Pineapple (jual 130),Pineapple (jual 195),Pineapple (jual 234),Pineapple (jual 325),Pink Spinach (jual 171),Pink Spinach (jual 256),Pink Spinach (jual 307),Pink Spinach (jual 427),Pointy Cabbage (jual 55),Pointy Cabbage (jual 83),Pointy Cabbage (jual 99),Pointy Cabbage (jual 138),Potato (jual 70),Potato (jual 105),Potato (jual 126),Potato (jual 250),Princess's Eye (jual 209),Princess's Eye (jual 314),Princess's Eye (jual 377),Princess's Eye (jual 523),Pumpkin (jual 170),Pumpkin (jual 255),Pumpkin (jual 306),Pumpkin (jual 475),Purple Asparagus (jual 121),Purple Asparagus (jual 182),Purple Asparagus (jual 218),Purple Asparagus (jual 248),Red Bell Pepper (jual 149),Red Bell Pepper (jual 223),Red Bell Pepper (jual 268),Red Bell Pepper (jual 289),Red Cabbage (jual 55),Red Cabbage (jual 83),Red Cabbage (jual 99),Red Cabbage (jual 138),Red Eggplant (jual 160),Red Eggplant (jual 240),Red Eggplant (jual 288),Red Eggplant (jual 399),Red Leaf Lettuce (jual 165),Red Leaf Lettuce (jual 248),Red Leaf Lettuce (jual 297),Red Leaf Lettuce (jual 358),Red Onion (jual 50),Red Onion (jual 75),Red Onion (jual 90),Red Onion (jual 124),Red Turnip (jual 35),Red Turnip (jual 52),Red Turnip (jual 62),Red Turnip (jual 97),Rice (jual 190),Rice (jual 285),Rice (jual 342),Rice (jual 375),Rock Eggplant (jual 160),Rock Eggplant (jual 240),Rock Eggplant (jual 288),Rock Eggplant (jual 399),Romaine Lettuce (jual 160),Romaine Lettuce (jual 240),Romaine Lettuce (jual 288),Romaine Lettuce (jual 400),Romanesco (jual 132),Romanesco (jual 198),Romanesco (jual 238),Romanesco (jual 330),Royal Herb (jual 200),Royal Herb (jual 300),Royal Herb (jual 360),Royal Herb (jual 500),Rye (jual 75),Rye (jual 113),Rye (jual 135),Rye (jual 165),San Marzano (jual 225),San Marzano (jual 338),San Marzano (jual 405),San Marzano (jual 563),Sand Kale (jual 171),Sand Kale (jual 256),Sand Kale (jual 307),Sand Kale (jual 427),Sieglinde (jual 231),Sieglinde (jual 347),Sieglinde (jual 416),Sieglinde (jual 578),Snow Lantern (jual 209),Snow Lantern (jual 314),Snow Lantern (jual 377),Snow Lantern (jual 523),Snow Wheat (jual 75),Snow Wheat (jual 113),Snow Wheat (jual 135),Snow Wheat (jual 165),Snowman (jual 264),Snowman (jual 396),Snowman (jual 476),Snowman (jual 660),Soybean (jual 165),Soybean (jual 248),Soybean (jual 297),Soybean (jual 313),Sparta (jual 220),Sparta (jual 330),Sparta (jual 396),Sparta (jual 550),Spinach (jual 111),Spinach (jual 167),Spinach (jual 188),Spinach (jual 200),Squash (jual 242),Squash (jual 363),Squash (jual 436),Squash (jual 633),Strawberry (jual 160),Strawberry (jual 240),Strawberry (jual 288),Strawberry (jual 400),Sun Tomato (jual 248),Sun Tomato (jual 372),Sun Tomato (jual 446),Sun Tomato (jual 619),Sunset Corn (jual 154),Sunset Corn (jual 231),Sunset Corn (jual 278),Sunset Corn (jual 303),Sweet Corn (jual 154),Sweet Corn (jual 231),Sweet Corn (jual 278),Sweet Corn (jual 303),Sweet Potato (jual 210),Sweet Potato (jual 315),Sweet Potato (jual 378),Sweet Potato (jual 525),Tomato (jual 89),Tomato (jual 134),Tomato (jual 161),Tomato (jual 213),Turnip (jual 31),Turnip (jual 47),Turnip (jual 56),Turnip (jual 88),Water Beet (jual 127),Water Beet (jual 190),Water Beet (jual 228),Water Beet (jual 317),Water Dragon Pineapple (jual 209),Water Dragon Pineapple (jual 314),Water Dragon Pineapple (jual 377),Water Dragon Pineapple (jual 523),Water Eggplant (jual 160),Water Eggplant (jual 240),Water Eggplant (jual 288),Water Eggplant (jual 399),Watercress (jual 155),Watercress (jual 233),Watercress (jual 279),Watercress (jual 388),Watermelon (jual 200),Watermelon (jual 300),Watermelon (jual 360),Watermelon (jual 500),Wheat (jual 68),Wheat (jual 102),Wheat (jual 123),Wheat (jual 150),White Asparagus (jual 121),White Asparagus (jual 182),White Asparagus (jual 218),White Asparagus (jual 248),White Bell Pepper (jual 204),White Bell Pepper (jual 306),White Bell Pepper (jual 367),White Bell Pepper (jual 427),White Berry (jual 176),White Berry (jual 264),White Berry (jual 317),White Berry (jual 440),White Carrot (jual 70),White Carrot (jual 104),White Carrot (jual 125),White Carrot (jual 152),White Celery (jual 143),White Celery (jual 215),White Celery (jual 258),White Celery (jual 275),White Eggplant (jual 90),White Eggplant (jual 134),White Eggplant (jual 161),White Eggplant (jual 207),White Soybean (jual 182),White Soybean (jual 273),White Soybean (jual 327),White Soybean (jual 344),Whitesphere Cabbage (jual 132),Whitesphere Cabbage (jual 198),Whitesphere Cabbage (jual 238),Whitesphere Cabbage (jual 330),Yellow Bell Pepper (jual 185),Yellow Bell Pepper (jual 278),Yellow Bell Pepper (jual 333),Yellow Bell Pepper (jual 388),Zucchini (jual 220),Zucchini (jual 330),Zucchini (jual 396),Zucchini (jual 575)", \
    "CollapseAdd_93_Spinner_Bunga_Pilih item,Aqua Chamomile (jual 152),Aqua Chamomile (jual 273),Aqua Chamomile (jual 363),Aqua Chamomile (jual 605),Aqua Daisy (jual 173),Aqua Daisy (jual 312),Aqua Daisy (jual 415),Aqua Daisy (jual 692),Aquature (jual 238),Aquature (jual 428),Aquature (jual 571),Aquature (jual 951),Blue Hibiscus (jual 119),Blue Hibiscus (jual 214),Blue Hibiscus (jual 286),Blue Hibiscus (jual 476),Blue Rose (jual 180),Blue Rose (jual 324),Blue Rose (jual 432),Blue Rose (jual 720),Blue Valentine (jual 238),Blue Valentine (jual 428),Blue Valentine (jual 571),Blue Valentine (jual 951),Chamomile (jual 126),Chamomile (jual 227),Chamomile (jual 303),Chamomile (jual 504),Champagne Flute (jual 152),Champagne Flute (jual 273),Champagne Flute (jual 363),Champagne Flute (jual 605),Clear Water (jual 195),Clear Water (jual 350),Clear Water (jual 467),Clear Water (jual 778),Cotton Rosemallow (jual 90),Cotton Rosemallow (jual 162),Cotton Rosemallow (jual 216),Cotton Rosemallow (jual 360),Daisy (jual 36),Daisy (jual 65),Daisy (jual 87),Daisy (jual 144),Eclipse (jual 108),Eclipse (jual 195),Eclipse (jual 260),Eclipse (jual 432),Edelblau (jual 260),Edelblau (jual 467),Edelblau (jual 623),Edelblau (jual 1037),Edelweiss (jual 216),Edelweiss (jual 389),Edelweiss (jual 519),Edelweiss (jual 864),Epiphyllum (jual 173),Epiphyllum (jual 312),Epiphyllum (jual 415),Epiphyllum (jual 692),Flowing Clouds (jual 195),Flowing Clouds (jual 350),Flowing Clouds (jual 467),Flowing Clouds (jual 778),Goldband Lily (jual 108),Goldband Lily (jual 195),Goldband Lily (jual 260),Goldband Lily (jual 432),Hibiscus (jual 99),Hibiscus (jual 179),Hibiscus (jual 238),Hibiscus (jual 396),Hollyhock (jual 119),Hollyhock (jual 214),Hollyhock (jual 286),Hollyhock (jual 476),Holy Grail (jual 152),Holy Grail (jual 273),Holy Grail (jual 363),Holy Grail (jual 605),Ice Daisy (jual 44),Ice Daisy (jual 78),Ice Daisy (jual 104),Ice Daisy (jual 173),Ice Trumpet (jual 126),Ice Trumpet (jual 227),Ice Trumpet (jual 303),Ice Trumpet (jual 504),Incan Lily (jual 260),Incan Lily (jual 467),Incan Lily (jual 623),Incan Lily (jual 1037),Incredible Sunflower (jual 195),Incredible Sunflower (jual 350),Incredible Sunflower (jual 467),Incredible Sunflower (jual 778),Lemon Aura (jual 195),Lemon Aura (jual 350),Lemon Aura (jual 467),Lemon Aura (jual 778),Lotus (jual 72),Lotus (jual 130),Lotus (jual 173),Lotus (jual 288),Lullaby (jual 130),Lullaby (jual 234),Lullaby (jual 312),Lullaby (jual 519),Marguerite (jual 45),Marguerite (jual 81),Marguerite (jual 108),Marguerite (jual 180),Moon River (jual 216),Moon River (jual 389),Moon River (jual 519),Moon River (jual 864),Moulin Rouge (jual 162),Moulin Rouge (jual 292),Moulin Rouge (jual 389),Moulin Rouge (jual 648),Mountain Lotus (jual 87),Mountain Lotus (jual 156),Mountain Lotus (jual 208),Mountain Lotus (jual 346),Nirvana (jual 87),Nirvana (jual 156),Nirvana (jual 208),Nirvana (jual 346),Orange Daisy (jual 44),Orange Daisy (jual 78),Orange Daisy (jual 104),Orange Daisy (jual 173),Orchid Cactus (jual 144),Orchid Cactus (jual 260),Orchid Cactus (jual 346),Orchid Cactus (jual 576),Pink Daisy (jual 144),Pink Daisy (jual 260),Pink Daisy (jual 346),Pink Daisy (jual 576),Pink Marguerite (jual 54),Pink Marguerite (jual 98),Pink Marguerite (jual 130),Pink Marguerite (jual 216),Pink Orchid Cactus (jual 173),Pink Orchid Cactus (jual 312),Pink Orchid Cactus (jual 415),Pink Orchid Cactus (jual 692),Pink Rose (jual 98),Pink Rose (jual 175),Pink Rose (jual 234),Pink Rose (jual 389),Purple Marguerite (jual 54),Purple Marguerite (jual 98),Purple Marguerite (jual 130),Purple Marguerite (jual 216),Queen of the Night (jual 162),Queen of the Night (jual 292),Queen of the Night (jual 389),Queen of the Night (jual 648),Red Rose (jual 81),Red Rose (jual 146),Red Rose (jual 195),Red Rose (jual 324),Snow Queen (jual 130),Snow Queen (jual 234),Snow Queen (jual 312),Snow Queen (jual 519),Snow White Pansy (jual 65),Snow White Pansy (jual 117),Snow White Pansy (jual 156),Snow White Pansy (jual 260),Solar Ray (jual 216),Solar Ray (jual 389),Solar Ray (jual 519),Solar Ray (jual 864),Stargazer (jual 130),Stargazer (jual 234),Stargazer (jual 312),Stargazer (jual 519),Strawberry Pansy (jual 54),Strawberry Pansy (jual 98),Strawberry Pansy (jual 130),Strawberry Pansy (jual 216),Striped Hibiscus (jual 108),Striped Hibiscus (jual 195),Striped Hibiscus (jual 260),Striped Hibiscus (jual 432),Sunflower (jual 90),Sunflower (jual 162),Sunflower (jual 216),Sunflower (jual 360),Tulip (jual 63),Tulip (jual 114),Tulip (jual 152),Tulip (jual 252),Viola (jual 198),Viola (jual 357),Viola (jual 476),Viola (jual 792),Violet Hibiscus (jual 108),Violet Hibiscus (jual 195),Violet Hibiscus (jual 260),Violet Hibiscus (jual 432),Violet Pansy (jual 65),Violet Pansy (jual 117),Violet Pansy (jual 156),Violet Pansy (jual 260),Water Lily (jual 130),Water Lily (jual 234),Water Lily (jual 312),Water Lily (jual 519),Water Lotus (jual 108),Water Lotus (jual 195),Water Lotus (jual 260),Water Lotus (jual 432),White Daisy (jual 173),White Daisy (jual 312),White Daisy (jual 415),White Daisy (jual 692),White Night (jual 108),White Night (jual 195),White Night (jual 260),White Night (jual 432),White Rose (jual 98),White Rose (jual 175),White Rose (jual 234),White Rose (jual 389),White Tulip (jual 76),White Tulip (jual 137),White Tulip (jual 182),White Tulip (jual 303),Yellow Marguerite (jual 54),Yellow Marguerite (jual 98),Yellow Marguerite (jual 130),Yellow Marguerite (jual 216),Yellow Tulip (jual 76),Yellow Tulip (jual 137),Yellow Tulip (jual 182),Yellow Tulip (jual 303)", \
    "CollapseAdd_94_Spinner_Ikan_Pilih item,Akame (jual 200),Akame (jual 300),Akame (jual 400),Akame (jual 600),Arowana (jual 60),Arowana (jual 90),Arowana (jual 120),Arowana (jual 180),Banjar Arowana (jual 100),Banjar Arowana (jual 150),Banjar Arowana (jual 200),Banjar Arowana (jual 300),Barracuda (jual 100),Barracuda (jual 150),Barracuda (jual 200),Barracuda (jual 300),Black Carp (jual 60),Black Carp (jual 90),Black Carp (jual 120),Black Carp (jual 180),Black Goby (jual 20),Black Goby (jual 30),Black Goby (jual 40),Black Goby (jual 60),Black Porgy (jual 60),Black Porgy (jual 90),Black Porgy (jual 120),Black Porgy (jual 180),Black Tiger Prawn (jual 40),Black Tiger Prawn (jual 60),Black Tiger Prawn (jual 80),Black Tiger Prawn (jual 120),Blue Barracuda (jual 60),Blue Barracuda (jual 90),Blue Barracuda (jual 120),Blue Barracuda (jual 180),Blue Crayfish (jual 50),Blue Crayfish (jual 75),Blue Crayfish (jual 100),Blue Crayfish (jual 150),Bluefin Tuna (jual 320),Bluefin Tuna (jual 480),Bluefin Tuna (jual 640),Bluefin Tuna (jual 960),Calico Goldfish (jual 100),Calico Goldfish (jual 150),Calico Goldfish (jual 200),Calico Goldfish (jual 300),Cod (jual 80),Cod (jual 120),Cod (jual 160),Cod (jual 240),Continental Lobster (jual 100),Continental Lobster (jual 150),Continental Lobster (jual 200),Continental Lobster (jual 300),Crucian Carp (jual 30),Crucian Carp (jual 45),Crucian Carp (jual 60),Crucian Carp (jual 90),Diamond Piranha (jual 50),Diamond Piranha (jual 75),Diamond Piranha (jual 100),Diamond Piranha (jual 150),Dorado (jual 100),Dorado (jual 150),Dorado (jual 200),Dorado (jual 300),Goby (jual 20),Goby (jual 30),Goby (jual 40),Goby (jual 60),Golden Carp (jual 400),Golden Carp (jual 600),Golden Carp (jual 800),Golden Carp (jual 1200),Golden Dorado (jual 200),Golden Dorado (jual 300),Golden Dorado (jual 400),Golden Dorado (jual 600),Goldfish (jual 100),Goldfish (jual 150),Goldfish (jual 200),Goldfish (jual 300),Green Arowana (jual 80),Green Arowana (jual 120),Green Arowana (jual 160),Green Arowana (jual 240),Huchen (jual 110),Huchen (jual 165),Huchen (jual 220),Huchen (jual 330),Koi (jual 110),Koi (jual 165),Koi (jual 220),Koi (jual 330),Lobster (jual 60),Lobster (jual 90),Lobster (jual 120),Lobster (jual 180),Mahi-Mahi (jual 110),Mahi-Mahi (jual 165),Mahi-Mahi (jual 220),Mahi-Mahi (jual 330),Mosquitofish (jual 20),Mosquitofish (jual 30),Mosquitofish (jual 40),Mosquitofish (jual 60),Mozambique Tilapia (jual 80),Mozambique Tilapia (jual 120),Mozambique Tilapia (jual 160),Mozambique Tilapia (jual 240),Nidhogg (jual 560),Nidhogg (jual 840),Nidhogg (jual 1120),Nidhogg (jual 1680),Nile Tilapia (jual 60),Nile Tilapia (jual 90),Nile Tilapia (jual 120),Nile Tilapia (jual 180),Perch (jual 40),Perch (jual 60),Perch (jual 80),Perch (jual 120),Piranha (jual 40),Piranha (jual 60),Piranha (jual 80),Piranha (jual 120),Pirarucu (jual 200),Pirarucu (jual 300),Pirarucu (jual 400),Pirarucu (jual 600),Pollock (jual 110),Pollock (jual 165),Pollock (jual 220),Pollock (jual 330),Rainbow Trout (jual 30),Rainbow Trout (jual 45),Rainbow Trout (jual 60),Rainbow Trout (jual 90),Red Bream (jual 110),Red Bream (jual 165),Red Bream (jual 220),Red Bream (jual 330),Red Crayfish (jual 20),Red Crayfish (jual 30),Red Crayfish (jual 40),Red Crayfish (jual 60),Red Seabream (jual 80),Red Seabream (jual 120),Red Seabream (jual 160),Red Seabream (jual 240),Redeye (jual 100),Redeye (jual 150),Redeye (jual 200),Redeye (jual 300),Round Herring (jual 50),Round Herring (jual 75),Round Herring (jual 100),Round Herring (jual 150),Saffron Cod (jual 140),Saffron Cod (jual 210),Saffron Cod (jual 280),Saffron Cod (jual 420),Sardine (jual 30),Sardine (jual 45),Sardine (jual 60),Sardine (jual 90),Sea Bass (jual 110),Sea Bass (jual 165),Sea Bass (jual 220),Sea Bass (jual 330),Silver Dorado (jual 140),Silver Dorado (jual 210),Silver Dorado (jual 280),Silver Dorado (jual 420),Smelt (jual 30),Smelt (jual 45),Smelt (jual 60),Smelt (jual 90),Snakehead (jual 100),Snakehead (jual 150),Snakehead (jual 200),Snakehead (jual 300),Super Red Arowana (jual 110),Super Red Arowana (jual 165),Super Red Arowana (jual 220),Super Red Arowana (jual 330),Tiger Eye (jual 140),Tiger Eye (jual 210),Tiger Eye (jual 280),Tiger Eye (jual 420),Tiger Prawn (jual 100),Tiger Prawn (jual 150),Tiger Prawn (jual 200),Tiger Prawn (jual 300),Tilapia (jual 40),Tilapia (jual 60),Tilapia (jual 80),Tilapia (jual 120),Walleye (jual 60),Walleye (jual 90),Walleye (jual 120),Walleye (jual 180),White Piranha (jual 80),White Piranha (jual 120),White Piranha (jual 160),White Piranha (jual 240),Yellow Perch (jual 80),Yellow Perch (jual 120),Yellow Perch (jual 160),Yellow Perch (jual 240),Yellowfin Tuna (jual 240),Yellowfin Tuna (jual 360),Yellowfin Tuna (jual 480),Yellowfin Tuna (jual 720)", \
    "CollapseAdd_95_Spinner_Kerang dan Hasil Laut_Pilih item,Akoya Pearl Oyster,Ark Clam,Bamboo Shoot,Clam,Curved Bamboo Shoot,Egg Cockle,Geoduck,Jumbo Mushroom,Large Clam,Matsutake Mushroom,Miracle Truffle,Morel Mushroom,Mushroom,Mussel,Oyster,Porcini Mushroom,Razor Clam,Shiitake Mushroom,Surf Clam,Timber Bamboo Shoot,Truffle,White Mushroom,White Truffle,Yesso Scallop", \
    "CollapseAdd_96_Spinner_Bahan Tambang dan Kayu_Pilih item,Adamantite,Adamantite Ore,Agate,Alexandrite,Amethyst,Aquamarine,Board Lumber,Bronze,Bronze Ore,Crystal,Damascus Steel,Damascus Steel Ore,Diamond,Emerald,Garnet,Glass,Gold,Gold Ore,High-Quality Board Lumber,High-Quality Square Lumber,Iron,Iron Ore,Jade,Lumber,Material Stone,Moonstone,Opal,Orichalcum,Orichalcum Ore,Palm Board Lumber,Palm Lumber,Palm Square Lumber,Paraiba Tourmaline,Platinum,Platinum Ore,Raw Agate Gemstone,Raw Alexandrite Gemstone,Raw Amethyst Gemstone,Raw Aquamarine Gemstone,Raw Crystal Gemstone,Raw Diamond Gemstone,Raw Emerald Gemstone,Raw Garnet Gemstone,Raw Glass Gemstone,Raw Jade Gemstone,Raw Moonstone Gemstone,Raw Opal Gemstone,Raw Ruby Gemstone,Raw Sapphire Gemstone,Raw Tanzanite Gemstone,Raw Topaz Gemstone,Raw Tourmaline Gemstone,Ruby,Sapphire,Silver,Silver Ore,Square Lumber,Stone,Sturdy Board Lumber,Sturdy Lumber,Sturdy Square Lumber,Tanzanite,Titanium,Titanium Ore,Topaz", \
    "CollapseAdd_97_Spinner_Susu dan Telur_Pilih item,Araucana Egg (jual 100),Araucana Egg (jual 150),Araucana Egg (jual 200),Araucana Egg (jual 300),Ayrshire Milk (jual 240),Ayrshire Milk (jual 360),Ayrshire Milk (jual 480),Ayrshire Milk (jual 720),Egg (jual 50),Egg (jual 75),Egg (jual 100),Egg (jual 150),Jersey Milk (jual 160),Jersey Milk (jual 240),Jersey Milk (jual 320),Jersey Milk (jual 480),Milk (jual 80),Milk (jual 120),Milk (jual 160),Milk (jual 240),Silkie Egg (jual 150),Silkie Egg (jual 225),Silkie Egg (jual 300),Silkie Egg (jual 450)", \
    "CollapseAdd_98_Spinner_Pupuk Umpan dan Obat_Pilih item,Animal Medicine,Big Fish Bait,Compost,Desert Fertilizer,Fall Fertilizer,Fertilizer,Flatlands Fertilizer,Grass,Herbicide,Lv. 1 Regular Bait,Lv. 2 Regular Bait,Lv. 3 Regular Bait,Miracle Potion,No Bait,Rare Fish Bait,Spring Fertilizer,Summer Fertilizer,Volcanic Fertilizer,Weed,Winter Fertilizer", \
    "CollapseAdd_99_Spinner_Masakan_Pilih item,Ahi Poke (jual 250),Ahi Poke (jual 275),Ahi Poke (jual 300),Ahi Poke (jual 325),Alba-style Pot-au-feu (jual 100),Alba-style Pot-au-feu (jual 125),Alba-style Pot-au-feu (jual 150),Alba-style Pot-au-feu (jual 175),Apple Gelato (jual 50),Apple Gelato (jual 75),Apple Gelato (jual 100),Apple Gelato (jual 125),Apple Jam (jual 75),Apple Jam (jual 100),Apple Jam (jual 125),Apple Jam (jual 150),Apple Jam Baguette (jual 150),Apple Jam Baguette (jual 175),Apple Jam Baguette (jual 200),Apple Jam Baguette (jual 225),Apple Pie (jual 100),Apple Pie (jual 125),Apple Pie (jual 150),Apple Pie (jual 175),Baby Formula,Baguette (jual 75),Baguette (jual 100),Baguette (jual 125),Baguette (jual 150),Bean Chowder (jual 125),Bean Chowder (jual 150),Bean Chowder (jual 175),Bean Chowder (jual 200),Bean Salad (jual 125),Bean Salad (jual 150),Bean Salad (jual 175),Bean Salad (jual 200),Bean Tofu (jual 75),Bean Tofu (jual 100),Bean Tofu (jual 125),Bean Tofu (jual 150),Bismarck (jual 200),Bismarck (jual 225),Bismarck (jual 250),Bismarck (jual 275),Blue Carpaccio (jual 150),Blue Carpaccio (jual 175),Blue Carpaccio (jual 200),Blue Carpaccio (jual 225),Blue Cheese (jual 100),Blue Cheese (jual 125),Blue Cheese (jual 150),Blue Cheese (jual 175),Blue Fish Sushi (jual 150),Blue Fish Sushi (jual 175),Blue Fish Sushi (jual 200),Blue Fish Sushi (jual 225),Borscht (jual 125),Borscht (jual 150),Borscht (jual 175),Borscht (jual 200),Boscaiola (jual 175),Boscaiola (jual 200),Boscaiola (jual 225),Boscaiola (jual 250),Butter (jual 100),Butter (jual 125),Butter (jual 150),Butter (jual 175),Cheese Fondue (jual 250),Cheese Fondue (jual 275),Cheese Fondue (jual 300),Cheese Fondue (jual 325),Coconut Gelato (jual 50),Coconut Gelato (jual 75),Coconut Gelato (jual 100),Coconut Gelato (jual 125),Coconut Milk (jual 50),Coconut Milk (jual 75),Coconut Milk (jual 100),Coconut Milk (jual 125),Coconut Pizza (jual 75),Coconut Pizza (jual 100),Coconut Pizza (jual 125),Coconut Pizza (jual 150),Corn Potage Soup (jual 100),Corn Potage Soup (jual 125),Corn Potage Soup (jual 150),Corn Potage Soup (jual 175),Cream Baguette (jual 225),Cream Baguette (jual 250),Cream Baguette (jual 275),Cream Baguette (jual 300),Cream Pie (jual 100),Cream Pie (jual 125),Cream Pie (jual 150),Cream Pie (jual 175),Creamed Shrimp Pasta (jual 275),Creamed Shrimp Pasta (jual 300),Creamed Shrimp Pasta (jual 325),Creamed Shrimp Pasta (jual 350),Creamed Spinach Pasta (jual 150),Creamed Spinach Pasta (jual 175),Creamed Spinach Pasta (jual 200),Creamed Spinach Pasta (jual 225),Doria (jual 150),Doria (jual 175),Doria (jual 175) #2,Doria (jual 200),Eggy Tofu (jual 75),Eggy Tofu (jual 100),Eggy Tofu (jual 125),Eggy Tofu (jual 150),Fish au Gratin (jual 325),Fish au Gratin (jual 350),Fish au Gratin (jual 375),Fish au Gratin (jual 400),Fish Chowder (jual 250),Fish Chowder (jual 275),Fish Chowder (jual 300),Fish Chowder (jual 325),Fish Meunière (jual 250),Fish Meunière (jual 275),Fish Meunière (jual 300),Fish Meunière (jual 325),Flower Cake (jual 100),Flower Cake (jual 125),Flower Cake (jual 150),Flower Cake (jual 175),Flower Gelato (jual 100),Flower Gelato (jual 125),Flower Gelato (jual 150),Flower Gelato (jual 175),Flower Salad (jual 125),Flower Salad (jual 150),Flower Salad (jual 175),Flower Salad (jual 200),Fresh Cheese (jual 100),Fresh Cheese (jual 125),Fresh Cheese (jual 150),Fresh Cheese (jual 175),Fried Egg (jual 50),Fried Egg (jual 75),Fried Egg (jual 100),Fried Egg (jual 125),Fried Rice (jual 125),Fried Rice (jual 150),Fried Rice (jual 175),Fried Rice (jual 200),Fruit Pizza (jual 150),Fruit Pizza (jual 175),Fruit Pizza (jual 200),Fruit Pizza (jual 225),Fruit Punch (jual 100),Fruit Punch (jual 125),Fruit Punch (jual 150),Fruit Punch (jual 175),Fruitcake (jual 100),Fruitcake (jual 125),Fruitcake (jual 150),Fruitcake (jual 175),Gazpacho (jual 175),Gazpacho (jual 200),Gazpacho (jual 225),Gazpacho (jual 250),Green Juice (jual 75),Green Juice (jual 100),Green Juice (jual 125),Green Juice (jual 150),Green Salad (jual 100),Green Salad (jual 125),Green Salad (jual 150),Green Salad (jual 175),Grilled Fish (jual 125),Grilled Fish (jual 150),Grilled Fish (jual 175),Grilled Fish (jual 200),Hard Cheese (jual 100),Hard Cheese (jual 125),Hard Cheese (jual 150),Hard Cheese (jual 175),Hibiscus Jam (jual 75),Hibiscus Jam (jual 100),Hibiscus Jam (jual 125),Hibiscus Jam (jual 150),Hibiscus Jam Baguette (jual 150),Hibiscus Jam Baguette (jual 175),Hibiscus Jam Baguette (jual 200),Hibiscus Jam Baguette (jual 225),Hot Milk (jual 75),Hot Milk (jual 100),Hot Milk (jual 125),Hot Milk (jual 150),Jade Tofu (jual 175),Jade Tofu (jual 200),Jade Tofu (jual 225),Jade Tofu (jual 250),Jambalaya (jual 125),Jambalaya (jual 150),Jambalaya (jual 175),Jambalaya (jual 200),Jumbo Grilled Fish (jual 200),Jumbo Grilled Fish (jual 225),Jumbo Grilled Fish (jual 250),Jumbo Grilled Fish (jual 275),Jumbo Meunière (jual 325),Jumbo Meunière (jual 350),Jumbo Meunière (jual 375),Jumbo Meunière (jual 400),Macaroni au Gratin (jual 125),Macaroni au Gratin (jual 225),Macaroni au Gratin (jual 250),Macaroni au Gratin (jual 275),Margherita (jual 200),Margherita (jual 225),Margherita (jual 250),Margherita (jual 275),Marmalade (jual 75),Marmalade (jual 100),Marmalade (jual 125),Marmalade (jual 150),Marmalade Baguette (jual 150),Marmalade Baguette (jual 175),Marmalade Baguette (jual 200),Marmalade Baguette (jual 225),Milk Gelato (jual 75),Milk Gelato (jual 100),Milk Gelato (jual 125),Milk Gelato (jual 150),Minestrone (jual 125),Minestrone (jual 150),Minestrone (jual 175),Minestrone (jual 200),Mixed Juice (jual 100),Mixed Juice (jual 125),Mixed Juice (jual 150),Mixed Juice (jual 175),Mushroom Stew (jual 100),Mushroom Stew (jual 125),Mushroom Stew (jual 150),Mushroom Stew (jual 175),Mushrooms au Gratin (jual 200),Mushrooms au Gratin (jual 200) #2,Mushrooms au Gratin (jual 225),Mushrooms au Gratin (jual 250),No-Bake Cheesecake (jual 175),No-Bake Cheesecake (jual 200),No-Bake Cheesecake (jual 225),No-Bake Cheesecake (jual 250),Nut Cake (jual 100),Nut Cake (jual 125),Nut Cake (jual 150),Nut Cake (jual 175),Orange Gelato (jual 50),Orange Gelato (jual 75),Orange Gelato (jual 100),Orange Gelato (jual 125),Orange Juice (jual 50),Orange Juice (jual 75),Orange Juice (jual 100),Orange Juice (jual 125),Orange Pie (jual 100),Orange Pie (jual 125),Orange Pie (jual 150),Orange Pie (jual 175),Ortrana Bianca (jual 200),Ortrana Bianca (jual 225),Ortrana Bianca (jual 250),Ortrana Bianca (jual 275),Ortrana Rossa (jual 125),Ortrana Rossa (jual 150),Ortrana Rossa (jual 175),Ortrana Rossa (jual 200),Paella (jual 125),Paella (jual 150),Paella (jual 175),Paella (jual 200),Peach Gelato (jual 50),Peach Gelato (jual 75),Peach Gelato (jual 100),Peach Gelato (jual 125),Peach Jam (jual 75),Peach Jam (jual 100),Peach Jam (jual 125),Peach Jam (jual 150),Peach Jam Baguette (jual 150),Peach Jam Baguette (jual 175),Peach Jam Baguette (jual 200),Peach Jam Baguette (jual 225),Peach Pie (jual 100),Peach Pie (jual 125),Peach Pie (jual 150),Peach Pie (jual 175),Pescatore (jual 175),Pescatore (jual 200),Pescatore (jual 225),Pescatore (jual 250),Pineapple Gelato (jual 75),Pineapple Gelato (jual 100),Pineapple Gelato (jual 125),Pineapple Gelato (jual 150),Pineapple Jam (jual 75),Pineapple Jam (jual 100),Pineapple Jam (jual 125),Pineapple Jam (jual 150),Pineapple Jam Baguette (jual 150),Pineapple Jam Baguette (jual 175),Pineapple Jam Baguette (jual 200),Pineapple Jam Baguette (jual 225),Pineapple Juice (jual 50),Pineapple Juice (jual 75),Pineapple Juice (jual 100),Pineapple Juice (jual 125),Pumpkin Pie (jual 125),Pumpkin Pie (jual 150),Pumpkin Pie (jual 175),Pumpkin Pie (jual 200),Pumpkin Potage Soup (jual 100),Pumpkin Potage Soup (jual 125),Pumpkin Potage Soup (jual 150),Pumpkin Potage Soup (jual 175),Ratatouille (jual 125),Ratatouille (jual 150),Ratatouille (jual 175),Ratatouille (jual 200),Red Carpaccio (jual 225),Red Carpaccio (jual 250),Red Carpaccio (jual 275),Red Carpaccio (jual 300),Red Fish Sushi (jual 225),Red Fish Sushi (jual 250),Red Fish Sushi (jual 275),Red Fish Sushi (jual 300),Seafood Fried Rice (jual 425),Seafood Fried Rice (jual 450),Seafood Fried Rice (jual 475),Seafood Fried Rice (jual 500),Shrimp Pilaf (jual 275),Shrimp Pilaf (jual 300),Shrimp Pilaf (jual 325),Shrimp Pilaf (jual 350),Small Fish Meunière (jual 250),Small Fish Meunière (jual 275),Small Fish Meunière (jual 300),Small Fish Meunière (jual 325),Small Grilled Fish (jual 125),Small Grilled Fish (jual 150),Small Grilled Fish (jual 175),Small Grilled Fish (jual 200),Soy Gelato (jual 75),Soy Gelato (jual 100),Soy Gelato (jual 125),Soy Gelato (jual 150),Soy Milk (jual 75),Soy Milk (jual 100),Soy Milk (jual 125),Soy Milk (jual 150),Stargazy Pie (jual 400),Stargazy Pie (jual 425),Stargazy Pie (jual 450),Stargazy Pie (jual 475),Stew (jual 150),Stew (jual 175),Stew (jual 200),Stew (jual 225),Strawberry Gelato (jual 75),Strawberry Gelato (jual 100),Strawberry Gelato (jual 125),Strawberry Gelato (jual 150),Strawberry Jam (jual 75),Strawberry Jam (jual 100),Strawberry Jam (jual 125),Strawberry Jam (jual 150),Strawberry Jam Baguette (jual 150),Strawberry Jam Baguette (jual 175),Strawberry Jam Baguette (jual 200),Strawberry Jam Baguette (jual 225),Strawberry Milk (jual 75),Strawberry Milk (jual 100),Strawberry Milk (jual 125),Strawberry Milk (jual 150),Strawberry Pie (jual 125),Strawberry Pie (jual 150),Strawberry Pie (jual 175),Strawberry Pie (jual 200),Sweet Potato Gelato (jual 75),Sweet Potato Gelato (jual 100),Sweet Potato Gelato (jual 125),Sweet Potato Gelato (jual 150),Sweet Potato Pie (jual 125),Sweet Potato Pie (jual 150),Sweet Potato Pie (jual 175),Sweet Potato Pie (jual 200),Tofu (jual 175),Tofu (jual 200),Tofu (jual 225),Tofu (jual 250),Tofu Pizza (jual 350),Tofu Pizza (jual 375),Tofu Pizza (jual 400),Tofu Pizza (jual 425),Tomato Juice (jual 50),Tomato Juice (jual 75),Tomato Juice (jual 100),Tomato Juice (jual 125),Tomato Pasta (jual 125),Tomato Pasta (jual 150),Tomato Pasta (jual 175),Tomato Pasta (jual 200),Truffle Fried Egg (jual 100),Truffle Fried Egg (jual 125),Truffle Fried Egg (jual 150),Truffle Fried Egg (jual 175),Truffle Salad (jual 150),Truffle Salad (jual 175),Truffle Salad (jual 200),Truffle Salad (jual 225),Truffle Soup (jual 200),Truffle Soup (jual 225),Truffle Soup (jual 250),Truffle Soup (jual 275),Truffle Sushi (jual 200),Truffle Sushi (jual 225),Truffle Sushi (jual 250),Truffle Sushi (jual 275),Truffles au Gratin (jual 175),Truffles au Gratin (jual 200),Truffles au Gratin (jual 225),Truffles au Gratin (jual 250),Truffles Meunière (jual 300),Truffles Meunière (jual 325),Truffles Meunière (jual 350),Truffles Meunière (jual 375),Truffles à la Grande Meunière (jual 375),Truffles à la Grande Meunière (jual 400),Truffles à la Grande Meunière (jual 425),Truffles à la Grande Meunière (jual 450),Turnip Stew,Vegetable Fried Rice (jual 150),Vegetable Fried Rice (jual 175),Vegetable Fried Rice (jual 200),Vegetable Fried Rice (jual 225),Veggies au Gratin (jual 125),Veggies au Gratin (jual 150),Veggies au Gratin (jual 175),Veggies au Gratin (jual 200),Vichyssoise (jual 100),Vichyssoise (jual 125),Vichyssoise (jual 150),Vichyssoise (jual 175),Washed-Rind Cheese (jual 100),Washed-Rind Cheese (jual 125),Washed-Rind Cheese (jual 150),Washed-Rind Cheese (jual 175),Watermelon Gazpacho (jual 200),Watermelon Gazpacho (jual 225),Watermelon Gazpacho (jual 250),Watermelon Gazpacho (jual 275),Watermelon Gelato (jual 75),Watermelon Gelato (jual 100),Watermelon Gelato (jual 125),Watermelon Gelato (jual 150),Watermelon Jam (jual 75),Watermelon Jam (jual 100),Watermelon Jam (jual 125),Watermelon Jam (jual 150),Watermelon Jam Baguette (jual 150),Watermelon Jam Baguette (jual 175),Watermelon Jam Baguette (jual 200),Watermelon Jam Baguette (jual 225),Watermelon Juice (jual 50),Watermelon Juice (jual 75),Watermelon Juice (jual 100),Watermelon Juice (jual 125),White Carpaccio (jual 150),White Carpaccio (jual 175),White Carpaccio (jual 200),White Carpaccio (jual 225),White Fish Sushi (jual 150),White Fish Sushi (jual 175),White Fish Sushi (jual 200),White Fish Sushi (jual 225)", \
    "CollapseAdd_100_Button_Beri Item Terpilih",

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

static volatile int g_giveCount = 1;
static volatile int g_lastGroup = -1;
static volatile int g_giveReq = 0;
static int g_selPos[9];
static bool g_noMul = false;
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
typedef void* (*t_MsgGroup)(void*, int, void*);
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
static t_MsgGroup o_MsgGroup = nullptr;

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

// ------------------------------------------------------------- item list ---
static std::string utf16ToUtf8(void* s) {
    std::string r;
    if (!s) return r;
    int len = *(int*)((char*)s + 0x10);
    if (len < 0 || len > 300) return r;
    const uint16_t* c = (const uint16_t*)((char*)s + 0x14);
    for (int i = 0; i < len; i++) {
        uint32_t ch = c[i];
        if (ch == '\t' || ch == '\n' || ch == '\r') ch = ' ';
        if (ch < 0x80) r += (char)ch;
        else if (ch < 0x800) { r += (char)(0xC0 | (ch >> 6)); r += (char)(0x80 | (ch & 0x3F)); }
        else { r += (char)(0xE0 | (ch >> 12)); r += (char)(0x80 | ((ch >> 6) & 0x3F)); r += (char)(0x80 | (ch & 0x3F)); }
    }
    return r;
}

static void* g_msgMgr = nullptr;
static bool g_itemListDone = false;
static bool g_itemNamesDone = false;
static std::vector<uint32_t> g_itemUids;
static std::vector<uint32_t> g_itemNameIds;

static void writeItemList() {
    typedef void* (*t_getAcc)(void*);
    typedef uint32_t (*t_u32)(void*, uint32_t, void*);
    typedef int (*t_i32)(void*, uint32_t, void*);
    void* acc = ((t_getAcc)(g_base + OFF_GETITEMACC))(nullptr);
    if (!acc) return;
    char* dict = *(char**)((char*)acc + 0xB0);
    if (!dict) return;
    char* entries = *(char**)(dict + 0x18);
    int count = *(int*)(dict + 0x20);
    if (!entries || count <= 0 || count > 20000) return;
    std::vector<uint32_t> uids;
    for (int i = 0; i < count; i++) {
        char* e = entries + 0x20 + i * 16;
        int next = *(int*)(e + 4);
        uint32_t key = *(uint32_t*)(e + 8);
        if (next < -1 || key == 0) continue;
        uids.push_back(key);
    }
    if (uids.empty()) return;
    std::string path = outDir() + "/item_list.txt";
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;
    g_itemUids.clear();
    g_itemNameIds.clear();
    for (size_t i = 0; i < uids.size(); i++) {
        uint32_t u = uids[i];
        uint32_t nameId = ((t_u32)(g_base + OFF_ITEMNAME))(acc, u, nullptr);
        int cat = ((t_i32)(g_base + OFF_ITEMCAT))(acc, u, nullptr);
        int sub = ((t_i32)(g_base + OFF_ITEMSUB))(acc, u, nullptr);
        int kind = ((t_i32)(g_base + OFF_ITEMKIND))(acc, u, nullptr);
        int sell = ((t_i32)(g_base + OFF_ITEMSELL))(acc, u, nullptr);
        fprintf(f, "%u\t%u\t%d\t%d\t%d\t%d\n", u, nameId, cat, sub, kind, sell);
        g_itemUids.push_back(u);
        g_itemNameIds.push_back(nameId);
    }
    fclose(f);
    g_itemListDone = true;
}

static void writeItemNames() {
    typedef void* (*t_getMsg)(void*, int, void*);
    std::string dir = outDir();
    std::string mark = dir + "/item_names.started";
    FILE* m = fopen(mark.c_str(), "r");
    if (m) { fclose(m); g_itemNamesDone = true; return; }   // pernah crash di tahap ini, lewati
    m = fopen(mark.c_str(), "w");
    if (m) fclose(m);
    std::string path = dir + "/item_names.txt";
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;
    for (size_t i = 0; i < g_itemUids.size(); i++) {
        void* s = ((t_getMsg)(g_base + OFF_GETMSG))(g_msgMgr, (int)g_itemNameIds[i], nullptr);
        std::string nm = utf16ToUtf8(s);
        fprintf(f, "%u\t%s\n", g_itemUids[i], nm.c_str());
    }
    fclose(f);
    remove(mark.c_str());
    g_itemNamesDone = true;
}

// ------------------------------------------------------------ give item ----
static void giveItem() {
    typedef bool (*t_add)(void*, uint32_t, int, int*, int, bool, void*);
    int g = g_lastGroup;
    if (g < 0 || g > 8 || !g_itemInfo) return;
    int pos = g_selPos[g];
    if (pos < 1 || pos > GROUP_N[g]) return;
    uint32_t uid = GROUP_ITEMS[g][pos - 1];
    int cnt = g_giveCount;
    if (cnt < 1) cnt = 1;
    if (cnt > 99) cnt = 99;
    int ovf = 0;
    g_noMul = true;
    ((t_add)(g_base + OFF_ADDITEM))(g_itemInfo, uid, cnt, &ovf, INV_ITEMBAG, true, nullptr);
    g_noMul = false;
}

static void statusLog(const char* line) {
    std::string path = outDir() + "/hook_status.txt";
    FILE* f = fopen(path.c_str(), "a");
    if (!f) return;
    fprintf(f, "%s\n", line);
    fclose(f);
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
    if (g_giveReq) { g_giveReq = 0; giveItem(); }
    if (!g_itemListDone && g_frame > 900 && (g_frame % 300) == 0) writeItemList();
    if (g_itemListDone && !g_itemNamesDone && g_msgMgr && g_frame > 1800 && (g_frame % 300) == 0) writeItemNames();
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
    if (g_f[4] && num > 0 && !g_noMul) num = 99;
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

static void* h_MsgGroup(void* self, int id, void* mi) {
    g_msgMgr = self;
    return o_MsgGroup(self, id, mi);
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
    loadPosFile();
    struct HookJob { const HookDef* def; void* repl; void** orig; bool ok; };
    HookJob JOBS[] = {
    {&H_PMUPDATE, (void*)h_PMUpdate, (void**)&o_PMUpdate, false},
    {&H_ADDITEM, (void*)h_AddItem, (void**)&o_AddItem, false},
    {&H_SETHP, (void*)h_SetHp, (void**)&o_SetHp, false},
    {&H_FIGHT, (void*)h_Fight, (void**)&o_Fight, false},
    {&H_ANIMDAY, (void*)h_AnimDay, (void**)&o_AnimDay, false},
    {&H_TIMELATE, (void*)h_TimeLate, (void**)&o_TimeLate, false},
    {&H_APPLYHIT, (void*)h_ApplyHit, (void**)&o_ApplyHit, false},
    {&H_SPEED, (void*)h_Speed, (void**)&o_Speed, false},
    {&H_PLUPDATE, (void*)h_PlUpdate, (void**)&o_PlUpdate, false},
    {&H_GETHAVE, (void*)h_GetHave, (void**)&o_GetHave, false},
    {&H_ENVLATE, (void*)h_EnvLate, (void**)&o_EnvLate, false},
    {&H_FIINIT, (void*)h_FIInit, (void**)&o_FIInit, false},
    {&H_FIDESTROY, (void*)h_FIDestroy, (void**)&o_FIDestroy, false},
    {&H_MINEUPD, (void*)h_MineUpd, (void**)&o_MineUpd, false},
    {&H_CAP, (void*)h_Cap, (void**)&o_Cap, false},
    {&H_CROPUPD, (void*)h_CropUpd, (void**)&o_CropUpd, false},
    {&H_REMOVE, (void*)h_Remove, (void**)&o_Remove, false},
    {&H_DOWSE, (void*)h_Dowse, (void**)&o_Dowse, false},
    {&H_FISHCS, (void*)h_FishCS, (void**)&o_FishCS, false},
    {&H_AHUPD, (void*)h_AHUpd, (void**)&o_AHUpd, false},
    {&H_ADDBAG, (void*)h_AddBag, (void**)&o_AddBag, false},
    {&H_MSGGROUP, (void*)h_MsgGroup, (void**)&o_MsgGroup, false},
    };
    const int NJ = (int)(sizeof(JOBS) / sizeof(JOBS[0]));
    int attempts = 0;
    for (; attempts < 8; attempts++) {
        bool all = true;
        for (int i = 0; i < NJ; i++) {
            if (!JOBS[i].ok) JOBS[i].ok = installHook(*JOBS[i].def, JOBS[i].repl, JOBS[i].orig);
            if (!JOBS[i].ok) all = false;
        }
        if (all) break;
        sleep(3);
    }
    {
        char head[120];
        snprintf(head, sizeof(head), "--- hook status, percobaan: %d ---", attempts + 1);
        statusLog(head);
        for (int i = 0; i < NJ; i++) {
            char ln[200];
            snprintf(ln, sizeof(ln), "%s\t%s", JOBS[i].def->name, JOBS[i].ok ? "OK" : "GAGAL");
            statusLog(ln);
        }
    }
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
    "CollapseAdd_15_Toggle_Bibit, Pupuk, Pakan dan Umpan Tidak Habis",
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
    ITEM_MENU_ENTRIES
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
    } else if (featNum == 90) {
        int v = value;
        if (v < 1) v = 1;
        if (v > 99) v = 99;
        g_giveCount = v;
    } else if (featNum >= 91 && featNum <= 99) {
        int g = featNum - 91;
        g_selPos[g] = value;
        g_lastGroup = g;
    } else if (featNum == 100) {
        int g = g_lastGroup;
        if (g < 0 || g > 8 || g_selPos[g] < 1) { toast(env, ctx, "Pilih item dulu dari salah satu dropdown"); return; }
        if (!g_itemInfo) { toast(env, ctx, "Buka tas atau ambil satu item dulu, lalu coba lagi"); return; }
        g_giveReq = 1;
        toast(env, ctx, "Item diberikan");
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
