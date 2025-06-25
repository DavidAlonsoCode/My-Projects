#include "histograma.h"
#include <stdio.h>
#include <ctype.h>

unsigned* contador(struct dataset* datos) {
    unsigned wl = 0;
    int c;

    goto Out;
Out:
    c = getchar();
    if (c == EOF)
        goto End;
    if (isspace(c))
        goto Out;
    wl = 1;
    goto In;

In:
    c = getchar();
    if (c == EOF)
        goto End;
    if (isspace(c)) {
        if (wl > 13) {
            datos->longitudes[13]++;
        } else {
            datos->longitudes[wl - 1]++;
        }
        wl = 0;
        goto Out;
    } else {
        wl++;
        goto In;
    }

End:
    if (wl > 0) {
        if (wl > 13) {
            datos->longitudes[13]++;
        } else {
            datos->longitudes[wl - 1]++;
        }
    }
    return datos->longitudes;
}
