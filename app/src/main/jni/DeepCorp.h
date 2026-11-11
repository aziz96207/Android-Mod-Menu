// DeepCorp.h - port dari deepcorp_menu.lua (Game Guardian) ke C++ untuk mod menu.
// Tanpa dependensi KittyMemory: tulis memori sendiri lewat mprotect.
// Semua tambalan hanya di memori. Alamat dicek dulu; kalau tidak cocok, tidak ada yang diubah.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>

namespace dc {

static const uint32_t RET = 0xD65F03C0;
static const char *LIB = "libil2cpp.so";
#define DC_LOG(...) __android_log_print(ANDROID_LOG_INFO, "DeepCorp", __VA_ARGS__)

// ---------- Pembantu instruksi ----------
static uint32_t imm8(int n) {
    switch (n) {
        case 2:  return 0x00;
        case 3:  return 0x08;
        case 4:  return 0x10;
        case 5:  return 0x14;
        case 10: return 0x24;
        case 20: return 0x34;
    }
    return 0x00;
}
static uint32_t fmovS1(int n) { return 0x1E201001u | (imm8(n) << 13); }
static uint32_t fmovS2(int n) { return 0x1E201002u | (imm8(n) << 13); }
static uint32_t movz(int c)   { return 0x52800000u | ((uint32_t) c << 5); } // MOVZ W0,#c
static uint32_t pf(int n) {
    switch (n) {
        case 2:  return 0x1E201000;
        case 5:  return 0x1E229000;
        case 10: return 0x1E249000;
        case 20: return 0x1E269000;
    }
    return 0;
}

static const uint32_t PATCH_WH = 0x321107E0; // ORR W0,WZR,#0x18000 (98.304)

// ---------- Tabel tambalan ----------
struct Entry   { uint32_t rva; uint32_t (*fn)(int); };
struct Feature { const char *name; std::vector<Entry> list; std::vector<int> opts; };

static std::vector<Feature> &features() {
    static std::vector<Feature> F = {
        // 0. Kapasitas gudang
        {"Kapasitas gudang", {
            {0x30EAA60, [](int) -> uint32_t { return PATCH_WH; }},
            {0x30CAB9C, [](int) -> uint32_t { return PATCH_WH; }},
            {0x30CABA0, [](int) -> uint32_t { return RET; }}
        }, {98304}},

        // 1. Bahan pabrik hemat (75 = hemat 75%, 69 = hemat ~69%)
        {"Bahan pabrik hemat", {
            {0x3016890, [](int n) -> uint32_t { return n == 75 ? 0x53027E81u : 0x0B140A81u; }},
            {0x3016894, [](int n) -> uint32_t { return n == 75 ? 0xAA1F03E2u : 0x53047C21u; }}
        }, {75, 69}},

        // 2. Produksi pabrik
        {"Produksi pabrik", {
            {0x308A8D0, [](int n) -> uint32_t { return n == 50 ? 0x52A84900u : pf(n); }},
            {0x308A8D4, [](int n) -> uint32_t { return n == 50 ? 0x1E270000u : RET; }},
            {0x308A8D8, [](int n) -> uint32_t { return n == 50 ? RET : 0xA9055FFEu; }}
        }, {2, 5, 10, 20, 50}},

        // 3. Penambang menggali
        {"Penambang menggali", {
            {0x30854A4, [](int)   -> uint32_t { return 0xBD405909; }},
            {0x30854AC, [](int n) -> uint32_t { return fmovS1(n); }},
            {0x30854B4, [](int)   -> uint32_t { return 0x1E210929; }},
            {0x30854B8, [](int)   -> uint32_t { return 0xD503201F; }}
        }, {2, 3, 5, 10}},

        // 4. Gerobak tambang
        {"Gerobak tambang", {
            {0x3069A20, [](int)   -> uint32_t { return 0xF9401108; }},
            {0x3069A24, [](int)   -> uint32_t { return 0xF9402908; }},
            {0x3069A2C, [](int)   -> uint32_t { return 0xBD401C08; }},
            {0x3069A30, [](int n) -> uint32_t { return fmovS1(n); }},
            {0x3069A34, [](int)   -> uint32_t { return 0x1E210908; }},
            {0x3069A64, [](int)   -> uint32_t { return 0xF9401108; }},
            {0x3069A68, [](int)   -> uint32_t { return 0xF9402908; }},
            {0x3069A70, [](int)   -> uint32_t { return 0xBD401808; }},
            {0x3069A74, [](int n) -> uint32_t { return fmovS1(n); }},
            {0x3069A78, [](int)   -> uint32_t { return 0x1E210908; }}
        }, {2, 3, 4, 5}},

        // 5. Kargo: kecepatan
        {"Kargo: kecepatan", {
            {0x30793DC, [](int)   -> uint32_t { return 0xF9400900; }},
            {0x30793E0, [](int)   -> uint32_t { return 0xB40001A0; }},
            {0x30793E4, [](int)   -> uint32_t { return 0xF001CCC8; }},
            {0x30793E8, [](int)   -> uint32_t { return 0xF940B908; }},
            {0x30793EC, [](int)   -> uint32_t { return 0xB9403281; }},
            {0x30793F0, [](int)   -> uint32_t { return 0xF9400102; }},
            {0x30793F4, [](int)   -> uint32_t { return 0x942B2145; }},
            {0x30793F8, [](int)   -> uint32_t { return 0xBD401261; }},
            {0x30793FC, [](int n) -> uint32_t { return fmovS2(n); }},
            {0x3079400, [](int)   -> uint32_t { return 0x1E220821; }}
        }, {2, 3, 4, 5}},

        // 6. Kargo: kapasitas
        {"Kargo: kapasitas", {
            {0x2FFC698, [](int c) -> uint32_t { return movz(c); }},
            {0x30B1190, [](int c) -> uint32_t { return movz(c); }},
            {0x30B1194, [](int)   -> uint32_t { return RET; }},
            {0x30C01B8, [](int c) -> uint32_t { return movz(c); }},
            {0x30C01BC, [](int)   -> uint32_t { return RET; }}
        }, {1000, 5000, 10000}}
    };
    return F;
}

// ---------- Cek versi game (isi yang tidak diubah tambalan) ----------
struct Chk { uint32_t rva, val; };
static const Chk CHECKS[] = {
    {0x30E3958, 0xB9403000},  // MapEnvironment.get_MoneyBalance
    {0x30E395C, RET},
    {0x3079408, 0x1E210800},
    {0x2FFC69C, RET},
    {0x30854D4, 0x1E280921},
    {0x30854D8, 0x1E200820},
    {0x3069A48, 0x1E200900},
    {0x3069A8C, 0x1E200900},
    {0x308A2D4, 0x1E280800},
    {0x301688C, 0xAA1603E0},
    {0x30EAA64, RET}
};

// ---------- Memori ----------
static uintptr_t g_base = 0;
static std::string g_err;
static std::map<uint32_t, uint32_t> g_bak; // isi asli, diambil sebelum tulis pertama

static uintptr_t findBase() {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof line, f)) {
        if (strstr(line, LIB)) { base = (uintptr_t) strtoull(line, nullptr, 16); break; }
    }
    fclose(f);
    return base;
}

static uint32_t rd(uintptr_t a) { return *(volatile uint32_t *) a; }

static bool wr(uintptr_t a, uint32_t v) {
    long ps = sysconf(_SC_PAGESIZE);
    uintptr_t page = a & ~((uintptr_t) ps - 1);
    if (mprotect((void *) page, ps, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) return false;
    *(volatile uint32_t *) a = v;
    __builtin___clear_cache((char *) a, (char *) a + 4);
    mprotect((void *) page, ps, PROT_READ | PROT_EXEC);
    return rd(a) == v;
}

static bool ensure() {
    if (g_base) return true;
    uintptr_t b = findBase();
    if (!b) { g_err = "libil2cpp.so belum dimuat"; return false; }
    for (const Chk &c : CHECKS) {
        if (rd(b + c.rva) != c.val) {
            g_err = "Versi game tidak cocok. Tidak ada yang diubah.";
            DC_LOG("cek gagal di RVA 0x%X", c.rva);
            return false;
        }
    }
    g_base = b;
    return true;
}

// ---------- API ----------
// feat: indeks fitur 0..6. opt: indeks pilihan, atau -1 untuk kembali normal.
static bool applyFeature(int feat, int opt) {
    auto &F = features();
    if (feat < 0 || feat >= (int) F.size()) return false;
    if (!ensure()) { DC_LOG("%s", g_err.c_str()); return false; }
    Feature &ft = F[feat];

    if (opt < 0) { // kembali normal
        for (Entry &e : ft.list) {
            auto it = g_bak.find(e.rva);
            if (it != g_bak.end()) wr(g_base + e.rva, it->second);
        }
        return true;
    }
    if (opt >= (int) ft.opts.size()) return false;
    int n = ft.opts[opt];
    for (Entry &e : ft.list)                       // simpan isi asli sekali saja
        if (!g_bak.count(e.rva)) g_bak[e.rva] = rd(g_base + e.rva);
    for (Entry &e : ft.list)
        if (!wr(g_base + e.rva, e.fn(n))) { DC_LOG("gagal tulis 0x%X", e.rva); return false; }
    return true;
}

static const char *lastError() { return g_err.c_str(); }

} // namespace dc
