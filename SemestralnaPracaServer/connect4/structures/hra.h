#pragma once

#ifndef HRA_H
#define HRA_H

#include <pthread.h>
#include "../../pos_sockets/char_buffer.h"
#include "../../pos_sockets/active_socket.h"
#include "../../pos_sockets/passive_socket.h"
#include <stdbool.h>

#define POCET_RIADKOV 6
#define POCET_STLPCOV 7
#define P {'a','b'}

typedef struct coords {
    int riadok;
    int stlpec;
    int a;
} COORDS;

typedef struct hra {
    bool jeServerHracNaRade;
    bool hraSkoncila;
    int ktoVyhral; // 0 zatial nikto, 1 - server, 2 - klient, 3 - plna plocha
    int akoVyhral;// 0 ziadno, 1 horizontalne, 2 vertik., 3 diagonala
    int prveKolo;

    char znakServerHraca;
    char znakClientHraca;

    CHAR_BUFFER dataSendToClient;

    char hraciaPlocha[POCET_RIADKOV][POCET_STLPCOV];
    COORDS suradnicePoslednehoTahuServra;
    COORDS suradnicePoslednehoTahuClienta;

    pthread_mutex_t hraciaPlochaMutex;

    pthread_cond_t serverHracJeNaRade;
    pthread_cond_t serverHracSpravilTah;

} HRA;

void hra_init(struct hra *hra, char p_znakServerHraca, char p_znakClientHraca, bool p_jeServerHracNaRade);
void hra_destroy(struct hra *hra);
void vykresliHraciuPlochu(struct hra *hra);
bool validujHru(struct hra *hra, char znak);
void setVyhru(int ktoVyhral,int akoVyhral , int ktoVyhral_src, int akoVyhral_src);
#endif
