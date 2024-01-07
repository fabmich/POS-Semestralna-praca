#include <cstdio>
#include "Hra.h"

void hra_init(struct hra *hra) {

    for (int riadok = 0; riadok < POCET_RIADKOV; ++riadok) {
        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {
            hra->hraciaPlocha[riadok][stlpec] = '.';
        }
    }

    hra->hraSkoncila = false;
    hra->jeKlientNaRade = false;
    hra->vysledokHry = 0;
    hra->akoVyhral = 0;

}

void vykresliHraciuPlochu(struct hra *hra) {
    char abeceda[] = {'A', 'B', 'C', 'D', 'E', 'F', ' '};

    for (int riadok = 0; riadok < POCET_RIADKOV + 1; ++riadok) {
        printf("%c |", abeceda[riadok]);

        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {

            if (riadok < POCET_RIADKOV) {
                printf(" %c |", hra->hraciaPlocha[riadok][stlpec]);
            } else {
                printf(" %d |", stlpec + 1);
            }
        }

        // horizontalne ciary
        if (riadok < POCET_RIADKOV) {
            printf("\n");
            for (int lanes = 0; lanes < 31; ++lanes) {
                printf("-");
            }
        }
        printf("\n");
    }
}