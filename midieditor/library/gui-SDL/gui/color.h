#ifndef COLOR_GUI_H
#define COLOR_GUI_H
#include <stdlib.h>
//typedef struct {
//    double r;       // a fraction between 0 and 1
//    double g;       // a fraction between 0 and 1
//    double b;       // a fraction between 0 and 1
//} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

//static hsv   rgb2hsv(rgb in);
//static rgb   /hsv2rgb(hsv in);

#pragma GCC push_options
#pragma GCC optimize ("O0")
static int hsvd2bgr(double h, double s, double v)
{
    hsv in = {h, s, v};
    double      hh, p, q, t, ff;
    long        i;
//    rgb         out;
    int out = 0;
    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
//        out |= ((int)(in.v*255)) << 8;
//        out |= ((int)(in.v*255)) << 8;
//        out |= ((int)(in.v*255)) << 16;
        abort();
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out |= ((int)(in.v*255)) << 0;
        out |= ((int)(t*255)) << 8;
        out |= ((int)(p*255)) << 16;
        break;
    case 1:
        out |= ((int)(q*255)) << 0;
        out |= ((int)(in.v*255)) << 8;
        out |= ((int)(p*255)) << 16;
        break;
    case 2:
        out |= ((int)(p*255)) << 0;
        out |= ((int)(in.v*255)) << 8;
        out |= ((int)(t*255)) << 16;
        break;
    case 3:
        out |= ((int)(p*255)) << 0;
        out |= ((int)(q*255)) << 8;
        out |= ((int)(in.v*255)) << 16;
        break;
    case 4:
        out |= ((int)(t*255)) << 0;
        out |= ((int)(p*255)) << 8;
        out |= ((int)(in.v*255)) << 16;
        break;
    case 5:
    default:
        out |= ((int)(in.v*255)) << 0;
        out |= ((int)(p*255)) << 8;
        out |= ((int)(q*255)) << 16;
        break;
    }
//    out |= 255 << 16;
//    out |= 255 << 8;
//    out |= 255 << 0;
//    out |= 255 ;


    return out;
}
#pragma GCC pop_options
#endif
