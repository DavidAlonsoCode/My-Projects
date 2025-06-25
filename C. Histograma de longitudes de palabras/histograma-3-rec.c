#include "histograma.h"
#include <stdio.h>
#include <ctype.h>

int c;
unsigned wl = 0; // mide largo de palabra

unsigned* contador(struct dataset* datos) {
    if ((c = getchar()) != EOF) {
        if (isspace(c)) {
            if (wl > 13) {
                datos->longitudes[13]++;
            } else {
                datos->longitudes[wl - 1]++;
            }
            wl = 0;
        } else {
            ++wl;
        }
        return contador(datos);
    }

    if (wl > 0) {
        if (wl > 13) {
            datos->longitudes[13]++;
        } else {
            datos->longitudes[wl - 1]++;
        }
    }

    return datos->longitudes;
}
