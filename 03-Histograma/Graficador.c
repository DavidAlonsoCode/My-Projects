#include "Graficador.h"
#include <stdio.h>

void dibujar_histograma(unsigned *longitudes) {
    FILE *file = fopen("histograma.svg", "w");

    fprintf(file, "<svg width=\"600\" height=\"400\">\n");

    int bar_height = 20;
    int bar_gap = 5;
    int max_bar_width = 500; // Ancho máximo de las barras
    int y_offset = 20;

    for (unsigned i = 0; i < Max; i++) {
        int bar_width = (longitudes[i] * max_bar_width) / 13; // Escalar las barras
        int y_position = y_offset + i * (bar_height + bar_gap);

        fprintf(file, "<rect x=\"0\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"red\" />\n",
                y_position, bar_width, bar_height);

        if (i < 13) {
            fprintf(file, "<text x=\"0\" y=\"%d\" font-size=\"12\" fill=\"lightgray\">%d</text>\n",
                    y_position + bar_height / 2 + 4, i + 1);
        } else {
            fprintf(file, "<text x=\"0\" y=\"%d\" font-size=\"12\" fill=\"lightgray\">13+</text>\n",
                    y_position + bar_height / 2 + 4);
        }
    }

    fprintf(file, "</svg>\n");
    fclose(file);
}
