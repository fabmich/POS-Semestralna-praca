#ifndef HRA_H
#define HRA_H


#define POCET_RIADKOV 6
#define POCET_STLPCOV 7


typedef struct hra {
    char hraciaPlocha[POCET_RIADKOV][POCET_STLPCOV];
    bool hraSkoncila;
    bool jeKlientNaRade;
    char znakClientHraca;
    int vysledokHry;
    int akoVyhral;
    char znakServerHraca;


} HRA;

void hra_init(struct hra *hra);
void hra_destroy(struct hra *hra);
void vykresliHraciuPlochu(struct hra *hra);

#endif
