#include "stb_image.h"
#include "loadImage.h"
#include "newFile.h"
#include "stb_image_write.h"
#ifdef SDL
#include <SDL2/SDL.h>
#else
#include <X11/Xlib.h>
#endif
#include <linux/limits.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#ifdef SDL
struct SDL_Surface;
#define IMAGE struct SDL_Surface
#else
struct _XImage;
#define IMAGE struct _XImage
#endif
#pragma GCC push_options
#pragma GCC optimize ("Ofast")

#ifndef SDL
extern Display * xdisplay;
#endif
//extern Display * xdisplay;
IMAGE *loadImageZT(char* startOfPath, const char *path) {

    char imagePath[PATH_MAX] = {};
//    int r;
//    r+=5;
//#define stringify2(a) #a
//#define stringify(a) stringify2(a)
    strncat(imagePath, startOfPath, PATH_MAX-1);
    strncat(imagePath, "/", PATH_MAX);
    strncat(imagePath, path, PATH_MAX-1);
    int x,y, n;
    unsigned char *data = stbi_load(imagePath, &x, &y, &n, 4);
    if(!data) abort();
    for(int i = 0; i < x*y; i++) {
        char temp;
//        memcpy(temp, rcdata+i*4, 4);
        temp = data[i*4+2];
        data[i*4+2] = data[i*4];
        data[i*4] = temp;
    }
#ifdef SDL
    SDL_Surface *res = SDL_CreateRGBSurfaceWithFormatFrom(data, x, y, 24, x*4, SDL_PIXELFORMAT_ARGB32);
#else
    XImage *res = XCreateImage(xdisplay, DefaultVisual(xdisplay, DefaultScreen(xdisplay)), 24,
                     ZPixmap, 0, data, x, y, 32,
                             x*4);
#endif
    //    SDL_Texture * texture = SDL_CreateTextureFromSurface(a->rend, surface);
    //IMG_LoadTexture_RW../?
    return res;
}
#pragma GCC pop_options

FILE* saveImage = 0;

#ifndef SDL
static void stbicallback(void *context, void *data, int size) {
    (void)context;
    fwrite(data, size, 1, saveImage);
}
void saveImageSomewhereNewWrongChannelsZT(XImage *image, char *name) {
    for(int i = 0; i < image->width*image->height; i++) {
        ((char*)(image->data))[i*4+3] = 0xff;
    }
    char* path = newFile(name, "bmp");
    saveImage = fopen(path, "w");
    stbi_write_bmp_to_func(stbicallback, NULL, image->width,
                           image->height, 4, image->data);
    fclose(saveImage);
    saveImage=0;
}
#endif

IMAGE *loadLocalImageZT(const char *path) {
    return loadImageZT(MY_PATH, path);
}
