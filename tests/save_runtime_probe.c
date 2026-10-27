/* Private-ROM integration probe. Build against the playable runtime; never
 * commit its output (states/pictures contain game-derived data).
 * Usage: probe ROM write|read checkpoint.state expected.txt [players=1|2]
 * A fresh `read` process must produce the same 180 future-frame digests. */
#include "game_rtl.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "cpu_state.h"
#include "beam_frame_driver.h"
#include "snes/snes.h"
#include "snes/ppu.h"
#include "snes/apu.h"
#include "snes/spc.h"
#include "snes/dsp.h"
#include "sha256.h"
#include "crc32.h"
#include "snapshot_guard.h"
#include "desktop/config.h"
#include "snes_savestate_menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t pixels[1024*256];
static void picture(const char *path) {
    FILE *f=fopen(path,"wb"); if(!f) exit(20);
    fprintf(f,"P6\n256 224\n255\n");
    for(unsigned y=0;y<224;++y) for(unsigned x=0;x<256;++x) {
        uint32_t p=pixels[y*1024+x];
        unsigned char rgb[3]={(unsigned char)(p>>16),(unsigned char)(p>>8),(unsigned char)p};
        fwrite(rgb,1,3,f);
    }
    fclose(f);
}
static void frame(unsigned input) {
    RtlRunFrame(input);
    PpuBeginDrawing(g_ppu, (uint8_t *)pixels, 1024*4, kPpuRenderFlags_NewRenderer | kPpuRenderFlags_NoSpriteLimits);
    GameDrawPpuFrame();
}
static void frames(unsigned n, unsigned input) { while(n--) frame(input); }
static void digest(char out[65]) {
    /* Whole guest stream with the host PCM delivery queue blanked. This
     * includes all CPU/PPU/APU/DMA/WRAM, joypads and native execution residue. */
    DspOutputRing live;
    dsp_output_ring_save(g_snes->apu->dsp, &live);
    DspOutputRing blank = {0};
    dsp_output_ring_restore(g_snes->apu->dsp, &blank);
    size_t n = RtlSaveSnapshotToMemory(NULL, 0);
    uint8_t *blob = malloc(n+256*224*4); if (!blob) exit(10);
    if (RtlSaveSnapshotToMemory(blob,n) != n) exit(11);
    for(unsigned y=0;y<224;++y) memcpy(blob+n+y*256*4,pixels+y*1024,256*4);
    uint8_t hash[32]; sha256_compute(blob,n+256*224*4,hash); free(blob);
    dsp_output_ring_restore(g_snes->apu->dsp, &live);
    for(unsigned i=0;i<32;++i) sprintf(out+2*i,"%02x",hash[i]);
}
int main(int argc, char **argv) {
    if(argc==2 && !strcmp(argv[1],"keys")) {
        ConfigUsePersistentSaveDefaults();
        FILE *config=fopen("save-key-test.ini","w");if(!config) return 30;
        fputs("[KeyMap]\nLoad=F1,F2,F3,F4,F5,F6,F7,F8,F9,F10\nSave=Shift+F1,Shift+F2,Shift+F3,Shift+F4,Shift+F5,Shift+F6\n",config);fclose(config);
        ParseConfigFile("save-key-test.ini");
        if(FindCmdForSdlKey(SDLK_F5,0)!=kKeys_Save+5 ||
           FindCmdForSdlKey(SDLK_F9,0)!=kKeys_Load+5 ||
           FindCmdForSdlKey(SDLK_F11,0)!=kKeys_SaveStateMenu ||
           FindCmdForSdlKey(SDLK_F1,0)!=0) return 31;
        config=fopen("save-key-test.ini","w");if(!config) return 32;
        fputs("[KeyMap]\nQuickSave=F6\nQuickLoad=F10\nSaveStateMenu=F7\n",config);fclose(config);
        ConfigReloadKeyMap("save-key-test.ini");
        if(FindCmdForSdlKey(SDLK_F6,0)!=kKeys_Save+5 ||
           FindCmdForSdlKey(SDLK_F10,0)!=kKeys_Load+5 ||
           FindCmdForSdlKey(SDLK_F7,0)!=kKeys_SaveStateMenu ||
           FindCmdForSdlKey(SDLK_F5,0)!=0 || FindCmdForSdlKey(SDLK_F9,0)!=0) return 33;
        remove("save-key-test.ini");
        RtlSetSaveRoot("C:/long-user-profile-directory-for-testing-paths-with-more-than-ninety-six-characters/TMNTIVRecomp/saves/");
        for(int i=0;i<6;++i) {
            char path[1200],expected[1200];RtlSaveSlotPath(i,path,sizeof(path));
            if(i==5) sprintf(expected,"%s/quicksave.state",RtlSaveRoot());
            else sprintf(expected,"%s/slot_%02d.state",RtlSaveRoot(),i+1);
            if(strcmp(path,expected)) return 34;
        }
        puts("PASS: F5/F9, legacy conflicts, configurable keys, five manual paths and separate quick path");
        return 0;
    }
    if(argc<5) return 2;
    FILE *f = fopen(argv[1],"rb"); if(!f) return 3;
    uint8_t *rom=malloc(1048576); if(!rom) return 4;
    if(fread(rom,1,1048576,f)!=1048576) return 5; fclose(f);
    uint8_t hash[32]; char hex[65]; sha256_compute(rom,1048576,hash);
    for(unsigned i=0;i<32;++i) sprintf(hex+2*i,"%02x",hash[i]);
    if(strcmp(hex,"5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4")) return 6;
    printf("ROM verified; SRAM header byte=%u\n",rom[0x7fd8]);
    RtlRegisterGame(&kGameInfo);
    if(!SnesInit(rom,1048576)) return 7;
    free(rom); RtlEnableExtendedFrameTiming();
    int writing=!strcmp(argv[2],"write");
    unsigned two=argc>5 && atoi(argv[5])==2;
    if(writing) {
        frames(2000,0); frames(2,8); frames(120,0);
        frames(2,8 | (two ? (8<<12) : 0)); frames(300,0);
        frames(2,8);
        bool nmi=argc>6 && !strcmp(argv[6],"nmi");
        if(nmi) {
            unsigned i;
            for(i=0;i<2500;++i) {
                frame(0);
                unsigned pc=snes_beam_frame_driver_resume_pc();
                if(pc>=0x8174 && pc<0x8400 && g_cpu.S<0x1dff) break;
            }
            if(i==2500) return 35;
            printf("Suspended NMI PC=%06x S=%04x $003C=%02x\n",
                   snes_beam_frame_driver_resume_pc(),g_cpu.S,g_ram[0x3c]);
        } else frames(2300,0);
        if(two) { frames(2,8<<12); frames(60,0); frames(2,8<<12); frames(120,0); }
        unsigned action=nmi?0:argc>6?(unsigned)atoi(argv[6]):180;
        frames(action,0x82 | (two ? (0x82<<12) : 0));
        if(!RtlSaveSnapshot(argv[3])) return 8;
    } else if(!RtlLoadSnapshot(argv[3])) return 9;
    /* Reject corrupt and wrong-ROM/schema envelopes before touching gameplay. */
    size_t bytes=RtlSaveSnapshotToMemory(NULL,0);
    uint8_t *invalid=malloc(bytes); if(!invalid) return 24;
    for(unsigned test=0;test<5;++test) {
        if(RtlSaveSnapshotToMemory(invalid,bytes)!=bytes) return 25;
        char before[65],after[65];digest(before);
        size_t length=bytes;
        if(test==0) invalid[bytes-1]^=1;
        else if(test==1) --length;
        else {
            const char *needle=test!=3?"5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4":"tmnt4-beam-save-v1";
            size_t pos=0;
            while(pos+strlen(needle)<=bytes && memcmp(invalid+pos,needle,strlen(needle))) ++pos;
            if(pos+strlen(needle)>bytes) return 26;
            if(test==4) pos+=65; /* runtime schema follows ROM hash */
            invalid[pos]^=1;
            uint32_t crc=crc32_compute(invalid,bytes-4);memcpy(invalid+bytes-4,&crc,4);
        }
        if(RtlLoadSnapshotFromMemory(invalid,length)) return 27;
        digest(after);if(strcmp(before,after)) return 28;
    }
    free(invalid);puts("PASS: corruption, truncation, wrong ROM/schema/identity preserve live state");
    /* Pure overlay contract: opening, navigating, drawing and cancelling
     * must not advance or modify any serialized guest state or visible pixel.
     * This does not exercise the SDL audio/device event pump. */
    char menu_before[65],menu_after[65];digest(menu_before);
    snes_savestate_menu_poll_open(0);
    if(!snes_savestate_menu_poll_open((1u<<2)|(1u<<11)) ||
       !snes_savestate_menu_is_open()) return 39;
    snes_savestate_menu_poll_nav(0,1);
    snes_savestate_menu_poll_nav(1u<<5,2);
    const uint32_t *panel;int pw,ph;
    if(!snes_savestate_menu_overlay_image(&panel,&pw,&ph) || !panel || pw<=0 || ph<=0) return 40;
    snes_savestate_menu_close();digest(menu_after);
    if(strcmp(menu_before,menu_after)) return 41;
    puts("PASS: open/navigate/render/cancel overlay preserves complete machine and visible frame");
    unsigned checkpoint_pc=snes_beam_frame_driver_resume_pc();
    bool suspended=checkpoint_pc>=0x8174 && checkpoint_pc<0x8400 && g_cpu.S<0x1dff;
    if(writing) picture("checkpoint.ppm");
    f=fopen(argv[4],writing?"w":"r"); if(!f) return 12;
    for(unsigned i=0;i<180;++i) {
        unsigned input=i<100?0x82:0;
        frame(input | (two ? (input<<12) : 0));
        char actual[65],expected[65]; digest(actual);
        if(writing) fprintf(f,"%s\n",actual);
        else if(fscanf(f,"%64s",expected)!=1 || strcmp(actual,expected)) {
            fprintf(stderr,"DIVERGENCE frame %u PC=%06x S=%04x\n",i,
                    snes_beam_frame_driver_resume_pc(),g_cpu.S);
            return 13;
        }
    }
    fclose(f);
    picture(writing?"baseline-after.ppm":"restored-after.ppm");
    if(suspended) {
        for(unsigned replay=0;replay<8;++replay) {
            if(!RtlLoadSnapshot(argv[3])) return 36;
            FILE *reference=fopen(argv[4],"r");if(!reference) return 37;
            for(unsigned i=0;i<180;++i) {
                unsigned input=i<100?0x82:0;
                frame(input);char actual[65],expected[65];digest(actual);
                if(fscanf(reference,"%64s",expected)!=1 || strcmp(actual,expected)) return 38;
            }
            fclose(reference);
        }
        puts("PASS: 8 repeated file restores of the suspended NMI; 180 frames each");
    }
    /* Repeat every boundary, including the old-NMI wait path when reached. */
    unsigned nested=0;
    for(unsigned i=0;i<120;++i) {
        size_t n=RtlSaveSnapshotToMemory(NULL,0);
        void *state=malloc(n); if(!state || RtlSaveSnapshotToMemory(state,n)!=n) return 21;
        unsigned pc=snes_beam_frame_driver_resume_pc();
        if(pc>=0x8174 && pc<0x8400) ++nested;
        frame(0x82); char expected[65],actual[65];digest(expected);
        if(!RtlLoadSnapshotFromMemory(state,n)) return 22;
        frame(0x82);digest(actual);free(state);
        if(strcmp(expected,actual)) {fprintf(stderr,"repeat divergence pc=%06x\n",pc);return 23;}
    }
    printf("PASS: 120 boundary restore/replays; %u checkpoints inside NMI\n",nested);
    printf("PASS %s: 180 complete-machine + visible-frame digests, players=%u PC=%06x S=%04x\n",
           writing?"baseline":"fresh-process restore",two?2:1,
           snes_beam_frame_driver_resume_pc(),g_cpu.S);
    return 0;
}
