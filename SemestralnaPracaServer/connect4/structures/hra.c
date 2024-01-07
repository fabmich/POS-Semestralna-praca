#include <stdio.h>
#include <pthread.h>
#include "../../pos_sockets/char_buffer.h"
#include "../../pos_sockets/active_socket.h"
#include "../../pos_sockets/passive_socket.h"
#include "hra.h"

#define POCET_RIADKOV 6
#define POCET_STLPCOV 7



void hra_init(struct hra *hra, char p_znakServerHraca, char p_znakClientHraca, bool p_jeServerHracNaRade) {

    for (int riadok = 0; riadok < POCET_RIADKOV; ++riadok) {
        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {
            hra->hraciaPlocha[riadok][stlpec] = '.';
        }
    }
//    hra->hraciaPlocha[0][0] = '.';

    hra->znakServerHraca = p_znakServerHraca;
    hra->znakClientHraca = p_znakClientHraca;
    hra->jeServerHracNaRade = p_jeServerHracNaRade;
    hra->hraSkoncila = false;
    char_buffer_init(&hra->dataSendToClient);

    hra->suradnicePoslednehoTahuServra.riadok = -1;
    hra->suradnicePoslednehoTahuServra.stlpec = -1;

    hra->suradnicePoslednehoTahuClienta.riadok = -1;
    hra->suradnicePoslednehoTahuClienta.stlpec = -1;

    hra->ktoVyhral = 0;
    hra->akoVyhral = 0;

    pthread_mutex_init(&hra->hraciaPlochaMutex, NULL);

}

void hra_destroy(struct hra *hra) {
    hra->hraSkoncila = false;
    hra->jeServerHracNaRade = false;
    hra->znakClientHraca = ' ';
    hra->znakServerHraca = ' ';
    for (int riadok = 0; riadok < POCET_RIADKOV; ++riadok) {
        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {
            hra->hraciaPlocha[riadok][stlpec] = ' ';
        }
    }

    char_buffer_destroy(&hra->dataSendToClient);
    pthread_cond_destroy(&hra->serverHracSpravilTah);
    pthread_cond_destroy(&hra->serverHracJeNaRade);
    pthread_mutex_destroy(&hra->hraciaPlochaMutex);
}

void vykresliHraciuPlochu(struct hra *hra) {

    char abeceda[] = {'A', 'B', 'C', 'D', 'E', 'F', ' '};

    printf("\n");

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
    printf("\n");

}


//FALSE - nevyhral, TRUE - VYHRA
bool validujHru(struct hra *hra, char znak) {
    int riadok, stlpec, cisloHraca;
    if (znak == hra->znakServerHraca) {
        riadok = hra->suradnicePoslednehoTahuServra.riadok;
        stlpec = hra->suradnicePoslednehoTahuServra.stlpec;
        cisloHraca = 1;
    } else {
        riadok = hra->suradnicePoslednehoTahuClienta.riadok;
        stlpec = hra->suradnicePoslednehoTahuClienta.stlpec;
        cisloHraca = 2;
    }

    int znakCounter = 0;

//horizontalna validacia
    for (int i = 0; i < POCET_STLPCOV - 1; ++i) {
        if (hra->hraciaPlocha[riadok][i] == znak) {
            znakCounter++;
            if (znakCounter == 4) {
//                printf("DEBUG: Niekto vyhral horizontalne");
                printf("Vidím 4 znaky (%c)horizontálne\n", znak);
                hra->ktoVyhral = cisloHraca;
                hra->akoVyhral = 1;
                return true;
            }
        } else {
            znakCounter = 0;
        }
    }

    //vertikalna validacia
    znakCounter = 0;
    for (int i = 0; i < POCET_RIADKOV; ++i) {
        if (hra->hraciaPlocha[i][stlpec] == znak) {
            znakCounter++;
            if (znakCounter == 4) {
//                printf("DEBUG: Niekto vyhral vertikalne");
                printf("Vidím 4 znaky (%c) vertikálne\n", znak);
                hra->ktoVyhral = cisloHraca;
                hra->akoVyhral = 2;
                return true;
            }
        } else {
            znakCounter = 0;
        }
    }

    znakCounter = 0;
    //diagonala zlava hora do prava

    int riadokDiag, stlpecDiag;

    if (riadok < stlpec) {
        riadokDiag = 0;
        stlpecDiag = stlpec - riadok;
    } else if (riadok > stlpec) {
        riadokDiag = riadok - stlpec;
        stlpecDiag = 0;
    } else {
        riadokDiag = 0;
        stlpecDiag = 0;
    }

    for (int i = 0; i < POCET_RIADKOV; ++i) {//diagonala je max do 5
        if (hra->hraciaPlocha[riadokDiag][stlpecDiag] == znak) {
            znakCounter++;
            if (znakCounter == 4) {
//                printf("DEBUG: Niekto vyhral po diagonale z lava hora do prava");
                printf("Vidím 4 znaky (%c) po diagonále\n", znak);
                hra->ktoVyhral = cisloHraca;
                hra->akoVyhral = 3;

                return true;
            }
        } else {
            znakCounter = 0;
        }
        riadokDiag++;
        stlpecDiag++;
    }
    znakCounter = 0;

    //validacia po diagonale z lava dola hore doprava

    riadokDiag = riadok;
    stlpecDiag = stlpec;
    for (int i = 1; i < 4; ++i) {
        riadokDiag -= 1;
        stlpecDiag += 1;
        if (riadokDiag >= 0 && stlpecDiag < POCET_STLPCOV) {
            if (hra->hraciaPlocha[riadokDiag][stlpecDiag] == znak) {
                znakCounter++;
//                printf("DEBUG: pocet znakov smerom hore %c je %d \n", znak, znakCounter);
                if (znakCounter == 3) {
//                    printf("DEBUG: Niekto vyhral po diagonale z lava dola do prava");
                    printf("Vidím 4 znaky (%c) po diagonále\n", znak);
                    hra->ktoVyhral = cisloHraca;
                    hra->akoVyhral = 3;
                    return true;
                }
            } else { break; }
        }
    }
    //validacia po diagonale z lava dola hore doprava

    riadokDiag = riadok;
    stlpecDiag = stlpec;
    for (int i = 1; i < 4; ++i) {
        riadokDiag += 1;
        stlpecDiag -= 1;
        if (riadokDiag < POCET_RIADKOV && stlpecDiag >= 0) {
            if (hra->hraciaPlocha[riadokDiag][stlpecDiag] == znak) {
                znakCounter++;
//                printf("DEBUG: pocet znakov smerom dole %c je %d \n", znak, znakCounter);

                if (znakCounter == 3) {
//                    printf("DEBUG: Niekto vyhral po diagonale z lava dola do prava");
                    printf("Vidím 4 znaky (%c) po diagonále\n", znak);
                    hra->ktoVyhral = cisloHraca;
                    hra->akoVyhral = 3;
                    return true;
                }
            } else { break; }
        }
    }


    znakCounter = 0;
    //validacia plny prvy riadok
    for (int i = 0; i < POCET_STLPCOV; ++i) {
        if (hra->hraciaPlocha[0][i] != '.') {
            znakCounter++;
            if (znakCounter == POCET_STLPCOV) {
//                printf("DEBUG: HRACIA PLOCHA JE PLNA");
                hra->ktoVyhral = 3;
                hra->akoVyhral = 4;
                return true;
            }
        }
    }

    return false;
}
