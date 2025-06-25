#ifndef HISTOGRAMA_H
#define HISTOGRAMA_H

extern const unsigned Max;

struct dataset {
    unsigned cantDeCategorias;
    unsigned longitudes[14];
};

//#define Max 14 //Si hago esto ya funciona
extern struct dataset datos;

unsigned* contador(struct dataset*);

#endif // HISTOGRAMA_H