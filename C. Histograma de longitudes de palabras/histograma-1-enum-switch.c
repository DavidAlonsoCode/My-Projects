#include "histograma.h"
#include <stdio.h>
#include <ctype.h>

typedef enum { Out, In } State;
State state = Out;

unsigned* contador(struct dataset* datos) {
    unsigned wl = 0; // mide largo de palabra
    int c;

    while ((c = getchar()) != EOF) {
        switch (state) {
            case Out:
                if (!isspace(c)) {
                    state = In;
                    wl = 1;
                }
                break;
            case In:
                if (isspace(c)) {
                    if (wl > 13) {
                        datos->longitudes[13]++;
                    } else {
                        datos->longitudes[wl - 1]++;
                    }
                    wl = 0;
                    state = Out;
                } else {
                    wl++;
                }
                break;
        }
    }

    if (state == In) {
        if (wl > 13) {
            datos->longitudes[13]++;
        } else {
            datos->longitudes[wl - 1]++;
        }
    }

    return datos->longitudes;
}
