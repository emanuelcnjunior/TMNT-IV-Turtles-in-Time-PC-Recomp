#include "hd_assets.h"
#include "desktop/hd_pack.h"
#include "desktop/graphics_video.h"
#include "desktop/config.h"
#include "snes/ppu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);return 1;}checks++;}while(0)
static uint32_t pixels[256*224];
static unsigned checks;
static unsigned resolve_any(const HdTile *t,void *unused){(void)unused;return t->layer==0?1:2;}
static int copy_file(const char *from,const char *to){
  FILE *a=fopen(from,"rb"),*b=fopen(to,"wb");if(!a||!b){if(a)fclose(a);if(b)fclose(b);return 0;}
  char buf[4096];size_t n;int ok=1;while((n=fread(buf,1,sizeof(buf),a))!=0)if(fwrite(buf,1,n,b)!=n){ok=0;break;}
  if(ferror(a))ok=0;fclose(a);fclose(b);return ok;
}
static void base(Ppu *p){
  ppu_reset(p);p->inidisp=15;p->bgmode=1;p->screenEnabled[0]=1;
  p->bgXsc[0]=4; /* map at word 0x400, patterns at 0 */
  for(int i=0;i<8;i++)p->vram[16+i]=255; /* tile 1, colour 1 */
  for(int i=0;i<1024;i++)p->vram[0x400+i]=1;
  p->cgram[1]=31;
  for(int i=0;i<128;i++)p->oam[i*2]=0xe000; /* offscreen */
  PpuBeginDrawing(p,(uint8_t*)pixels,256*4,kPpuRenderFlags_NewRenderer);
}
int main(int argc,char **argv){
  if(argc!=3){puts("probe sample-dir invalid-dir");return 2;}
  HdTile t={0};t.bpp=4;t.layer=0;t.oam=-1;for(int i=0;i<8;i++)t.planar[i]=255;t.palette[1]=31;
  char id[65],other[65];HdTileId(&t,id);printf("synthetic ID %s\n",id);
  t.x=70;t.y=80;t.oam=23;HdTileId(&t,other);CHECK(!strcmp(id,other));
  t.palette[1]=32;HdTileId(&t,other);CHECK(strcmp(id,other));t.palette[1]=31;
  t.planar[0]=254;HdTileId(&t,other);CHECK(strcmp(id,other));t.planar[0]=255;
  t.layer=1;HdTileId(&t,other);CHECK(strcmp(id,other));t.layer=0;
  ConfigUseGraphicsDefaults();g_config.hdpack_enabled=1;
  CHECK(HdPack_Load(argv[1]));unsigned revision;const unsigned char *atlas=HdPack_Atlas(&revision);CHECK(atlas&&revision);
  char path[1024],source[1024];snprintf(path,sizeof(path),"%s/manifest.json",argv[2]);
  FILE *bad=fopen(path,"wb");CHECK(bad);fputs("{\"version\":999}",bad);fclose(bad);
  unsigned original_revision=revision;CHECK(!HdPack_Load(argv[2]));CHECK(HdPack_Atlas(&revision)==atlas&&revision==original_revision);
  snprintf(path,sizeof(path),"%s/manifest.json",argv[2]);
  bad=fopen(path,"wb");CHECK(bad);fprintf(bad,"{\"version\":1,\"author\":\"test\",\"license\":\"test\",\"entries\":[{\"id\":\"%s\",\"file\":\"../author-emblem.png\"}]}",id);fclose(bad);
  CHECK(!HdPack_Load(argv[2]));CHECK(HdPack_Atlas(&revision)==atlas&&revision==original_revision);
  snprintf(path,sizeof(path),"%s/context.png",argv[2]);snprintf(source,sizeof(source),"%s/author-emblem.png",argv[1]);CHECK(copy_file(source,path));
  snprintf(path,sizeof(path),"%s/manifest.json",argv[2]);bad=fopen(path,"wb");CHECK(bad);
  fprintf(bad,"{\"version\":1,\"author\":\"test\",\"license\":\"test\",\"entries\":[{\"id\":\"%s\",\"file\":\"context.png\",\"oam\":23,\"rect\":[64,64,80,88]}]}",id);fclose(bad);
  CHECK(HdPack_Load(argv[2]));snprintf(g_config.hdpack_dir,sizeof(g_config.hdpack_dir),"%s",argv[2]);HdPack_Update();CHECK(hd_resolve);
  t.x=70;t.y=80;t.oam=23;CHECK(hd_resolve(&t,NULL)==1);t.oam=22;CHECK(hd_resolve(&t,NULL)==0);t.oam=23;t.x=90;CHECK(hd_resolve(&t,NULL)==0);
  snprintf(g_config.hdpack_dir,sizeof(g_config.hdpack_dir),"%s",argv[1]);HdPack_Update();CHECK(HdPack_Load(argv[1]));CHECK(hd_resolve);
  CHECK(hd_resolve(&t,NULL)==1);
  Ppu *p=ppu_init();base(p);ppu_runLine(p,1);CHECK((hd_pixel_tags[0]&255)==1);CHECK(pixels[0]==0xff0000);
  unsigned saved[256];memcpy(saved,pixels,sizeof(saved));hd_resolve=NULL;ppu_runLine(p,1);CHECK(!memcmp(saved,pixels,sizeof(saved)));
  hd_resolve=resolve_any;ppu_runLine(p,1);CHECK((hd_pixel_tags[0]&255)==1);
  /* Same VRAM address overwritten must get the original fallback. */
  HdPack_Update();t.planar[1]=254;CHECK(hd_resolve(&t,NULL)==0);t.planar[1]=255;p->vram[17]=254;ppu_runLine(p,1);CHECK((hd_pixel_tags[0]&255)==0);p->vram[17]=255;
  /* BG3 HUD wins over BG1 regardless of replacement colour. */
  hd_resolve=resolve_any;p->bgmode=9;p->bgXsc[2]=8;p->bgTileAdr=0x100;
  for(int i=0;i<8;i++)p->vram[0x1000+8+i]=255;
  for(int i=0;i<1024;i++)p->vram[0x800+i]=0x2001;
  p->screenEnabled[0]=5;ppu_runLine(p,1);CHECK((hd_pixel_tags[0]&255)==2);
  p->bgmode=1;p->screenEnabled[0]=1;
  /* Colour math and unsupported effects fall back without changing native output. */
  p->cgadsub=1;ppu_runLine(p,1);CHECK(hd_pixel_tags[0]==0);p->cgadsub=0;
  p->mosaic=0x11;ppu_runLine(p,1);CHECK(hd_pixel_tags[0]==0);p->mosaic=0;
  p->bgmode=7;ppu_runLine(p,1);CHECK(hd_pixel_tags[0]==0);
  base(p);hd_resolve=resolve_any;
  p->screenEnabled[0]=17;p->obsel=1; /* OBJ patterns word 0x2000 */
  for(int i=0;i<8;i++)p->vram[0x2000+16+i]=255;
  p->oam[0]=0;p->oam[1]=0x3001;p->cgram[129]=0x3e0;
  ppu_runLine(p,1);CHECK((hd_pixel_tags[0]&255)==2);CHECK(pixels[0]==0xff00);
  p->oam[1]=1;ppu_runLine(p,1);CHECK((hd_pixel_tags[0]&255)==1); /* BG occludes low OBJ */
  ppu_free(p);g_config.hdpack_enabled=0;HdPack_Update();CHECK(!hd_resolve);HdPack_Shutdown();
  /* Catalog roundtrips and exact aspect bounds, including small windows. */
  for(int choice=0;choice<7;choice++){int m,f;GraphicsFromChoice(choice,&m,&f);CHECK(GraphicsChoice(m,f)==choice);}
  for(int a=0;a<3;a++)for(int integer=0;integer<2;integer++){
    SnesDisplayViewport v;SnesDisplayAspect_ComputeViewport(256,224,3840,2160,(SnesDisplayAspect)a,0,integer,&v);
    CHECK(v.width>0&&v.height>0&&v.x>=0&&v.y>=0&&v.x+v.width<=3840&&v.y+v.height<=2160);
  }
  FILE *empty=fopen("graphics-probe.ini","wb");CHECK(empty);fclose(empty);
  ConfigUsePersistentSaveDefaults();ParseConfigFile("graphics-probe.ini");
  CHECK(FindCmdForSdlKey(SDLK_F5,0)==kKeys_Save+5);
  CHECK(FindCmdForSdlKey(SDLK_F9,0)==kKeys_Load+5);
  CHECK(FindCmdForSdlKey(SDLK_F11,0)==kKeys_SaveStateMenu);
  CHECK(FindCmdForSdlKey(SDLK_F6,0)==kKeys_GraphicsToggle);
  CHECK(FindCmdForSdlKey(SDLK_F7,0)==kKeys_GraphicsCompare);
  CHECK(FindCmdForSdlKey(SDLK_F10,0)==kKeys_GraphicsMenu);
  g_config.graphics_mode=2;g_config.graphics_filter=1;g_config.graphics_integer=false;
  g_config.window_width=3840;g_config.window_height=2160;g_config.hdpack_enabled=true;
  WriteConfigFile("graphics-probe.ini");g_config.graphics_mode=0;g_config.graphics_filter=0;
  g_config.window_width=g_config.window_height=0;g_config.hdpack_enabled=false;
  ParseConfigFile("graphics-probe.ini");CHECK(g_config.graphics_mode==2&&g_config.graphics_filter==1);
  CHECK(g_config.window_width==3840&&g_config.window_height==2160&&g_config.hdpack_enabled&&!g_config.graphics_integer);
  remove("graphics-probe.ini");
  printf("PASS %u checks\n",checks);return 0;
}





