#include "roll.h"
#include "gui.h"
#include "stb_ds.h"
#include "melody.h"

void roll(Painter* p, int y) {
//    Point pos = getPos();
    Size windowSize = guiGetSize();
    int height = windowSize.height-y;

    guiDrawTextZT(p, "440", STRU(Point, 10, y + height/2), 0xffffffff);
    // SDL_Log("Window %d resized to %dx%d",
    // event->window.windowID, ,
    // );
    guiSetForeground(p, 0xffff00ff);
//    int start = 0;
    FOR_STB_ARRAY(anote, piece) {
        guiFillRectangle(p,
                         anote->start*20, y + height/2-(anote->freq-400), anote->length*20, 10);
//        start += r.w;
    }
    if(event.type == SDL_MOUSEBUTTONUP) {
        SDL_MouseButtonEvent e =
                event.button;
        e.y -= y;
        Note n = {-(e.y-height/2)+400,
                 e.x / 20.0,
                 0.5};
        insert(n);
        guiRedraw();
    }
}
