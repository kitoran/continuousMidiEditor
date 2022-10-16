#include <gui.h>
#include <gridlayout.h>
#include <stdbool.h>
#include "stb_ds.h"
//const char* appName = "FractionalMidi";
//int main() {
//    guiStartDrawing();
//}
///*
#include <SDL2/SDL_ttf.h>
//#include <SDL2/SDL2.h>
typedef unsigned int u32;
typedef short int i16;
enum { SCREEN_WIDTH = 800,
       SCREEN_HEIGHT = 600};
int windowWidth = SCREEN_WIDTH;
int windowHeight = SCREEN_HEIGHT;
//const double tau = 6.283185307179586;
TTF_Font* Sans;
typedef struct textures {
    char* key;
    SDL_Texture*value;
} textures;
textures *textTextures = NULL;
void drawText(char* text, SDL_Renderer* renderer,  int x, int y) {
    fprintf(stderr,"drawText {%s}" ,text);
    //  var zigs = std.mem.span(text);
    //  var value = textTextures.get(zigs);
    //  if (value == NULL) {
    SDL_Color White= {.r = 255, .g = 255, .b = 255, .a = 255};
    SDL_Surface *surfaceMessage =
            TTF_RenderUTF8_Solid(Sans, text, White);
    SDL_Texture*textext = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    //     textTextures->put(zigs, textext);
    SDL_Texture*value = textext;
    SDL_FreeSurface(surfaceMessage);
    //  }
    SDL_Texture* texture = value;
    int w ;  int h  ;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect  Message_rect= {.x = x,  //controls the rect's x coordinate
                             .y = y, // controls the rect's y coordinte
                             .w = w, // controls the width of the rect
                             .h = h}; // controls the height of the rect
    // (0,0) is on the top left of the window/screen,
    // think a rect as the text's box,
    // that way it would be very simple to understand

    // Now since it's a texture, you have to put RenderCopy
    // in your game loop area, the area where the whole code executes

    // you put the renderer's name first, the Message,
    // the crop size (you can ignore this if you don't want
    // to dabble with cropping), and the rect which is the size
    // and coordinate of your texture
    SDL_RenderCopy(renderer, texture, 0, &Message_rect);

}


double  freq1 = 110.0;
double  freq2 = 110.0*1.09050773267 ;
double  freq3 = 110.0*1.09050773267 *1.09050773267 ;//*1.61803398875;
SDL_AudioSpec  have ;
void MyAudioCallback(   void*  userdata,
                        Uint8* stream8,
                        int len)
{


    //  fprintf(stderr,"callback {}" ,len);
    i16* stream =   stream8;
    //      userdata;
    static int phase = 0;
    u32  i = 0;
    while(i <  len/ 2) {
        stream[i] = (sin(phase * freq1 * tau / (have.freq))*1000) +(sin(phase * freq2 * tau / (have.freq))*1000)
                +(sin(phase * freq3 * tau / (have.freq))*1000);
        //        stream[i] = (sin(phase * freq1 * tau / (have.freq))*1000) +(sin(phase * freq2 * tau / (have.freq))*1000)
        //                +(sin(phase * freq3 * tau / (have.freq))*1000);
        phase = phase+1;
        i+=1;
    }

}

Uint32 timerCallback (Uint32 interval, void *param) {
    freq1 = freq1*1.17398499671;
    freq2 = freq2*1.17398499671;
    freq3 = freq3*1.17398499671;

    return 1000;
    /*static int pure = 0;
    switch(pure%4) {
    case 0: {
        freq2 = freq1/pow(2,3.0/12);
        freq3 = freq1/pow(2,7.0/12);
        break;
    }
    case 1: {
        freq2 = freq1/6.0*5;//  /4*5;
        freq3 = freq1/6.0*4;//  /4*5;
        break;
    }
    case 2: {
        freq2 = freq1/5*4;//  /4*5;
        freq3 = freq1/6.0*4;//  /4*5;
        break;
    }
    case 3: {
        freq2 = freq1/pow(2,4.0/12);
        freq3 = freq1/pow(2,7.0/12);
        break;
    }
    }
    pure = (pure+1)%4;
    fprintf(stderr, "%lf %lf %d\n", freq1/freq2,freq1/freq3, pure);
//    pure = !pure;
    return 2000;
    */
    (void)param;
    //        char notes[] = {5,4,4,5,4,4,5,4,4,5,4,4,2,1,0,2};
    char notes[] = {9,8,8,9,8,8,9,8,8,9,8,8,4,3,0,4};
    static int index = 0;
    double in = pow(2, 1.0/16);

    freq1 = 220*pow(in, notes[index]);
    index = (index+1)%sizeof(notes);
    fprintf(stderr, "freq=%lf in=%lf\n", freq1,in);
    return 250;
}

typedef struct note {
    double freq;
    double start;
    double length;
} note;

note* piece = NULL;
SDL_Renderer* renderer;
void draw() {

    drawText("440", renderer, 10, windowHeight/2);
    // SDL_Log("Window %d resized to %dx%d",
    // event->window.windowID, ,
    // );
    SDL_SetRenderDrawColor(renderer, 255,0,255,255);
    FOR_STB_ARRAY(anote, piece) {
        SDL_Rect r = {anote->start, anote->freq, anote->length, 10};
        SDL_RenderFillRect(renderer, &r);
    }
//    SDL_RenderPresent(renderer);

}
void audio() {
    SDL_AudioSpec  want;
    memset(&want,0,sizeof(want));//= std.mem.zeroes(SDL_AudioSpec );
    // SDL_AudioDeviceID  dev;

    // inline for (want) |i| {
    // i = 0;
    // }
    //  @memset(&want, 0, sizeof(want)); /* || SDL_zero(want) * /
    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 4096;
    want.callback = MyAudioCallback;  // you wrote this function elsewhere.
    //   SDL_AudioDeviceID   dev= SDL_OpenAudioDevice(NULL, 0,  SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    SDL_AudioDeviceID ret =  SDL_OpenAudioDevice(
                NULL,
                SDL_FALSE,
                &want, &have,
                0);
    fprintf(stderr,"%s, returend %d %d %d %d %d %d \n",
            SDL_GetError(), want.format, want.channels, want.samples,have.format, have.channels, have.samples);

    SDL_PauseAudioDevice(ret, 0);
}

void main() {
    // Note that info level log messages are by default printed only in Debug
    // and ReleaseSafe build modes.
    note n = {
        440, 0, 50
    };
    arrpush(piece, n);
    //    guiStartDrawing();
    fprintf(stderr,"All your codebase are belong to us." );
    //    if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO  ) < 0) {
    //        fprintf(stderr,"SDL could not initialize! SDL_Error: {s} \n", SDL_GetError());
    //        abort();
    //    }
    SDL_Init(SDL_INIT_AUDIO);
    audio();
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    //    SDL_Window *wind = SDL_CreateWindow(
    //        "SDL2 Demo",
    //        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    //        SCREEN_WIDTH, SCREEN_HEIGHT,
    //        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    //    );         SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );

    guiStartDrawing();
    //    guiShowWindow(rootWindow);
    renderer = SDL_CreateRenderer(rootWindow, -1,  0);
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_Rect f = {0,0,windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &f);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    SDL_SetRenderDrawColor(renderer, 255,255,255, 255);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    SDL_RenderDrawLine(renderer,
                       10.5, 10.5, 105.5, 30.5);
    //        SDL_RenderClear(renderer);
    //        SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    Painter p = {renderer, SDL_GetWindowSurface(rootWindow), rootWindow};
    TTF_Init();

    //this opens a font style and sets a size

    //        const rw = SDL_RWFromConstMem(ComicSans, ComicSans.len);
    Sans = TTF_OpenFont("/home/n/.fonts/comici.ttf", 24);
    //        const Sans = TTF_OpenFont("C:\\Windows\\Fonts\\comic.ttf", 24);//C:\\Windows\\WinSxS\\amd64_microsoft-windows-font-truetype-corbel_31bf3856ad364e35_10.0.19041.1_none_889a9b699e1310d9\\corbell.ttf", 24);
    fprintf(stderr, "{*}", Sans);
//    draw();
//    SDL_Event  e;

    SDL_AddTimer(250,
                 timerCallback,
                 NULL);
    getPos = gridGetPos; feedbackSize = gridFeedbackSize;
    Grid grid = allocateGrid(100, 100, 5);
    grid.gridStart.x = grid.gridStart.y = 0;
    pushGrid(&grid);
    draw();
//    static int er = 30;
//    SDL_RenderDrawLine(renderer,
//                       10.5, 10.5, 50.5, er++);

    while(1) {
//        SDL_RenderDrawLine(renderer,
//                           10.5, 10.5, 105.5, er++);

        guiNextEvent();
//        fprintf(stderr, "got event {x} res {}", event.type, pollres);
        guiSetForeground(&p,0);
        if(event.type != SDL_MOUSEMOTION)
            guiFillRectangle(&p,0,0,windowWidth,windowHeight);
        setCurrentGridPos(0,0);
        if(guiButton(&p, "play", 4)) {
//            play();
        }
        draw();
        SDL_RenderPresent(renderer);

        if(event.type == SDL_WINDOWEVENT) {
            if(event.window.event == SDL_WINDOWEVENT_RESIZED  || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                fprintf(stderr,"resizeevent" );
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
//                SDL_SetRenderDrawColor(renderer, 0,0,0,255);
//                SDL_Rect f = {0,0,windowWidth, windowHeight};
//                SDL_RenderFillRect(renderer, &f);
                //                drawToolButton
//                draw();
            }
            if(event.window.event == SDL_WINDOWEVENT_CLOSE) {
                return;
            }
        }

        if(event.type == SDL_MOUSEMOTION) {
            //            freq = event.motion.y + 140.0;
            fprintf(stderr,"motion {} {}", event.motion.y, event.motion.y + 140.0);
        }
        //          fprintf(stderr,"{}", @intToEnum(SDL_EventType, event.type));
        // fprintf(stderr, "{}", SDL_POLLSENTINEL          );
        if(event.type == SDL_QUIT) {
            return;
        }
    }

}
//*/
