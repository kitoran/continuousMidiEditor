#include "roll.h"
#include "gui.h"
#include "stb_ds.h"
#include "melody.h"

void Roll(Painter* p) {
    Point pos = getPos();
    Size windowSize = guiGetSize();
    int height = windowSize.height-pos.y;

    guiDrawTextZT(p, "440", STRU(Point, 10, pos.y + height/2), 0xffffffff);
    // SDL_Log("Window %d resized to %dx%d",
    // event->window.windowID, ,
    // );
    guiSetForeground(p, 0xffff00ff);
//    int start = 0;
    FOR_STB_ARRAY(anote, piece) {
        SDL_Rect r = {anote->start*20, pos.y + height/2-(anote->freq-400), anote->length*20, 10};
        SDL_RenderFillRect(renderer, &r);
//        start += r.w;
    }
    if(event.type == SDL_MOUSEBUTTONUP) {
        SDL_MouseButtonEvent e =
                event.button;
        e.y -= pos.y;
        Note n = {-(e.y-height/2)+400,
                 e.x / 20.0,
                 0.5};
        insert(n);
        guiRedraw();
    }
}
