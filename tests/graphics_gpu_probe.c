/* Synthetic data only. Hidden GL context; readback is a test assertion, never
 * part of the game's presentation path. */
#include "gl_core_3_1.h"
#include "desktop/sdl_compat.h"
#include "desktop/graphics_shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned shader(unsigned type,const char *text){
  unsigned s=glCreateShader(type);int ok;char log[2048];
  glShaderSource(s,1,&text,0);glCompileShader(s);glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
  if(!ok){glGetShaderInfoLog(s,sizeof(log),0,log);puts(log);exit(2);}return s;
}
int main(void){
  if(!SDL_Init(SDL_INIT_VIDEO)){puts(SDL_GetError());return 2;}
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_Window *w=SDL_CreateWindow("Synthetic HD renderer verification",64,64,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);
  SDL_GLContext c=SDL_GL_CreateContext(w);if(!c){puts(SDL_GetError());return 2;}ogl_LoadFunctions();
  printf("GPU: %s / %s\n",glGetString(GL_RENDERER),glGetString(GL_VERSION));
  const char *vs="#version 330 core\nout vec2 TexCoord;void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);TexCoord=p;gl_Position=vec4(p*2.0-1.0,0,1);}";
  unsigned p=glCreateProgram();glAttachShader(p,shader(GL_VERTEX_SHADER,vs));glAttachShader(p,shader(GL_FRAGMENT_SHADER,kGraphicsFragmentShader));glLinkProgram(p);
  int ok;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok)return 3;glUseProgram(p);
  unsigned vao,tex,fbo,out;glGenVertexArrays(1,&vao);glBindVertexArray(vao);
  unsigned char source[256*224*4];for(int y=0;y<224;y++)for(int x=0;x<256;x++){int i=(y*256+x)*4;source[i]=(x*13+y*3)&255;source[i+1]=(x^y)&255;source[i+2]=(x*5+y*7)&255;source[i+3]=255;}
  glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,256,224,0,GL_RGBA,GL_UNSIGNED_BYTE,source);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glGenTextures(1,&out);
  int sizes[][2]={{512,448},{1920,1080},{3840,2160}};int failures=0;
  for(int sz=0;sz<3;sz++){
    int width=sizes[sz][0],height=sizes[sz][1];size_t bytes=(size_t)width*height*4;
    unsigned char *baseline=malloc(bytes),*pixels=malloc(bytes);
    glBindTexture(GL_TEXTURE_2D,out);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,out,0);if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)return 4;
    glBindTexture(GL_TEXTURE_2D,tex);glViewport(0,0,width,height);glUniform2f(glGetUniformLocation(p,"OutputSize"),(float)width,(float)height);
    for(int mode=0;mode<3;mode++)for(int filter=0;filter<3;filter++){
      glUniform1i(glGetUniformLocation(p,"GraphicsMode"),mode);glUniform1i(glGetUniformLocation(p,"GraphicsFilter"),filter);
      glFinish();unsigned long long start=SDL_GetPerformanceCounter();
      for(int n=0;n<30;n++)glDrawArrays(GL_TRIANGLES,0,3);glFinish();
      double ms=1000.0*(SDL_GetPerformanceCounter()-start)/SDL_GetPerformanceFrequency()/30;
      glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
      if(mode==0&&filter==0){memcpy(baseline,pixels,bytes);
        /* Compare every output pixel against independent nearest sampling. */
        for(int y=0;y<height;y++)for(int x=0;x<width;x++){
          double fx=((double)x+0.5)*256/width,fy=((double)y+0.5)*224/height; int sx=(int)fx,sy=(int)fy; /* Exact texel-boundary ties vary with interpolation precision. */ if(fx-sx<0.00001||fy-sy<0.00001)continue;
          if(memcmp(pixels+((size_t)y*width+x)*4,source+(sy*256+sx)*4,4)){failures++;goto checked;}
        }checked:;
      }else if(mode==0|| (mode==1&&filter==2)){if(memcmp(baseline,pixels,bytes))failures++;}
      if(mode==2)for(int y=0;y<height;y++)if(memcmp(baseline+(size_t)y*width*4,pixels+(size_t)y*width*4,(width/2-2)*4)){failures++;break;}
      printf("%dx%d mode=%d filter=%d GPU+submit=%.3f ms\n",width,height,mode,filter,ms);
      if(glGetError()!=GL_NO_ERROR)failures++;
    }free(baseline);free(pixels);
  }
  printf("failures=%d; source_texture=229376 bytes; output is display/FBO dependent\n",failures);
  SDL_GL_DestroyContext(c);SDL_DestroyWindow(w);SDL_Quit();return failures?1:0;
}


