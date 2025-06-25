#include "histograma.h"
#include <stdio.h>
#include <ctype.h>

unsigned wl = 0;
void (*current_state)(int, struct dataset*);

void state_out(int c, struct dataset* datos);
void state_in(int c, struct dataset* datos);

unsigned* contador(struct dataset* datos) {
    int c;
    current_state = state_out;

    while ((c = getchar()) != EOF) {
        current_state(c, datos);
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

void state_out(int c, struct dataset* datos) {
    if (!isspace(c)) {
        wl = 1;
        current_state = state_in;
    }
}

void state_in(int c, struct dataset* datos) {
    if (isspace(c)) {
        if (wl > 13) {
            datos->longitudes[13]++;
        } else {
            datos->longitudes[wl - 1]++;
        }
        wl = 0;
        current_state = state_out;
    } else {
        wl++;
    }
}
