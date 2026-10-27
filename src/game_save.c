/* The framework owns the machine serializer. This chunk supplies the beam
 * continuation and its existing CPU/interpreter/timing extension. No ROM bytes. */
#include "game_save.h"
#include "beam_frame_driver.h"
#include "common_rtl.h"
#include "cpu_state.h"
#include "snes/saveload.h"
#include "snes/ppu.h"
#include "snesrecomp_rom_identity.h"
#include "save_schema.h"
#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t magic, version, pc, wai;
    char rom[65], schema[65];
    uint8_t reserved[2];
} GameSaveHeader;
static GameSaveHeader staged;
static bool staged_valid;

const char *GameSaveIdentity(void) { return "tmnt4-beam-save-v1"; }
bool GameSaveAllowed(void) {
    return cpu_pc24_resumable(snes_beam_frame_driver_resume_pc());
}

void GameSaveExtra(SaveLoadInfo *sli) {
    if (!GameSaveAllowed()) { RtlStateStreamFail(sli); return; }
    GameSaveHeader h = {0};
    h.magic = 0x34544d54; h.version = 1;
    h.pc = snes_beam_frame_driver_resume_pc();
    h.wai = snes_beam_frame_driver_wai_halted();
    memcpy(h.rom, SNESRECOMP_ROM_EXPECTED_SHA256, 65);
    memcpy(h.schema, TMNT_SAVE_SCHEMA, 65);
    sli->func(sli, &h, sizeof(h));
    RtlSaveExecutionState(sli);
    ppu_frame_state_save(g_ppu, sli);
}

static bool valid_header(const GameSaveHeader *h) {
    return h->magic == 0x34544d54 && h->version == 1 &&
        h->pc <= 0xffffff && cpu_pc24_resumable(h->pc) && h->wai <= 1 &&
        !memcmp(h->rom, SNESRECOMP_ROM_EXPECTED_SHA256, 65) &&
        !memcmp(h->schema, TMNT_SAVE_SCHEMA, 65) &&
        !h->reserved[0] && !h->reserved[1];
}

typedef struct {
    SaveLoadInfo base;
    const uint8_t *data;
    size_t size, pos;
    bool failed;
} Probe;
static void probe_read(SaveLoadInfo *sli, void *out, size_t n) {
    Probe *p = (Probe *)sli;
    if (p->failed || n > p->size - p->pos) {
        p->failed = true; memset(out, 0, n); return;
    }
    memcpy(out, p->data + p->pos, n); p->pos += n;
}
bool GameSavePreflight(const void *data, size_t size) {
    uint32_t version;
    size_t guest = RtlSnapshotGuestSize(&version);
    uint32_t hdr[2];
    if (size < sizeof(hdr)) return false;
    memcpy(hdr, data, sizeof(hdr));
    if (hdr[1] != version || !guest || guest > size) return false;
    Probe p = {{probe_read, NULL}, data, size, guest, false};
    GameSaveHeader h;
    probe_read(&p.base, &h, sizeof(h));
    if (p.failed || !valid_header(&h)) return false;
    /* Only the framework's staging area changes. The live machine is untouched. */
    return RtlLoadExecutionState(&p.base) &&
        ppu_frame_state_load(g_ppu, &p.base, false) && !p.failed && p.pos == size;
}
void GameLoadExtra(SaveLoadInfo *sli, uint32_t version) {
    (void)version;
    sli->func(sli, &staged, sizeof(staged));
    staged_valid = valid_header(&staged) && RtlLoadExecutionState(sli) &&
        ppu_frame_state_load(g_ppu, sli, true);
    if (!staged_valid) RtlStateStreamFail(sli);
}
void GameStateLoaded(uint32_t version) {
    (void)version;
    if (!staged_valid) return;
    RtlApuLock();
    RtlApplyExecutionState();
    RtlApuUnlock();
    snes_beam_frame_driver_restore(staged.pc, staged.wai);
    staged_valid = false;
}
