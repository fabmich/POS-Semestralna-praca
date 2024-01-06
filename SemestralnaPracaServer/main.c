#include <stdio.h>
#include <pthread.h>
#include "pos_sockets/char_buffer.h"
#include "pos_sockets/active_socket.h"
#include "pos_sockets/passive_socket.h"
#include <string.h>

#define POCET_RIADKOV 6
#define POCET_STLPCOV 7

//**********************************************************
//                  STRUCTURES
//**********************************************************
typedef struct coords {
    int riadok;
    int stlpec;
} COORDS;

typedef struct hra {
    char hraciaPlocha[POCET_RIADKOV][POCET_STLPCOV];
    bool jeServerHracNaRade;
    bool hraSkoncila;
    int ktoVyhral; //0 zatial nikto, 1 - server, 2 - klient

    char znakServerHraca;
    char znakClientHraca;

    CHAR_BUFFER dataSendToClient;
    COORDS suradnicePoslednehoTahuServra;
    COORDS suradnicePoslednehoTahuClienta;

    pthread_cond_t serverHracJeNaRade;
    pthread_cond_t serverHracSpravilTah;

} HRA;

typedef struct thread_data {
    HRA hra;
    bool druhyHracConnected;

    short port;
    ACTIVE_SOCKET *server_socket;

    CHAR_BUFFER recievedDataFromClient;

    pthread_mutex_t mutex;

} THREAD_DATA;

//**********************************************************
//                  INITIALIZATION
//**********************************************************
void thread_data_init(struct thread_data *data, short p_port, ACTIVE_SOCKET *p_socket, HRA *p_hra) {
    data->hra = *p_hra;
    pthread_mutex_init(&data->mutex, NULL);

    data->druhyHracConnected = false;
    data->port = p_port;
    data->server_socket = p_socket;
    pthread_cond_init(&data->hra.serverHracJeNaRade, NULL);
    pthread_cond_init(&data->hra.serverHracSpravilTah, NULL);
    char_buffer_init(&data->recievedDataFromClient);
};

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

}

//**********************************************************
//                  DESTRUCTION
//**********************************************************
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
}

void thread_data_destroy(struct thread_data *data) {
    hra_destroy(&data->hra);
    data->port = 0;
    data->server_socket = NULL;
    pthread_mutex_destroy(&data->mutex);

    char_buffer_destroy(&data->recievedDataFromClient);
}


//**********************************************************
//                  RENDER PLOCHY
//**********************************************************
void vykresliHraciuPlochu(struct hra *hra) {

    char abeceda[] = {'A', 'B', 'C', 'D', 'E', 'F', ' '};

    printf("\n");

    for (int riadok = 0; riadok < POCET_RIADKOV + 1; ++riadok) {
        printf("%c |", abeceda[riadok]);

        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {

            if (riadok < POCET_RIADKOV) {
                printf(" %c |", hra->hraciaPlocha[riadok][stlpec]);
            } else {
                printf(" %d |", stlpec);
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

//**********************************************************
//                  VALIDATION
//**********************************************************
//FALSE - nevyhral, TRUE - VYHRA
bool validujHru(struct hra *hra, char znak) {
    int riadok, stlpec;
    if (znak == hra->znakServerHraca) {
        riadok = hra->suradnicePoslednehoTahuServra.riadok;//riadok
        stlpec = hra->suradnicePoslednehoTahuServra.stlpec;//stlpec
    } else {
        riadok = hra->suradnicePoslednehoTahuClienta.riadok;//riadok
        stlpec = hra->suradnicePoslednehoTahuClienta.stlpec;//stlpec
    }

    int znakCounter = 0;


//horizontalna validacia
    for (int i = 0; i < POCET_STLPCOV - 1; ++i) {
        if (hra->hraciaPlocha[riadok][i] == znak) {
            znakCounter++;
            if (znakCounter == 4) {
                printf("DEBUG: Niekto vyhral horizontalne");
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
                printf("DEBUG: Niekto vyhral vertikalne");
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
                printf("DEBUG: Niekto vyhral po diagonale z lava hora do prava");
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
                printf("DEBUG: pocet znakov smerom hore %c je %d \n", znak, znakCounter);
                if (znakCounter == 3) {
                    printf("DEBUG: Niekto vyhral po diagonale z lava dola do prava");
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
                printf("DEBUG: pocet znakov smerom dole %c je %d \n", znak, znakCounter);

                if (znakCounter == 3) {
                    printf("DEBUG: Niekto vyhral po diagonale z lava dola do prava");
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
                //TODO PREROBIT VALIDACIU ABY NEVRACALA BOOL?
                printf("DEBUG: HRACIA PLOCHA JE PLNA");
            }
        }
    }

    return false;
}


//ziskanie dat od clienta, skopirovane z cvika
_Bool try_read_data_from_client(struct thread_data *data) {
    _Bool result = false;

    CHAR_BUFFER r_buf;
    char_buffer_init(&r_buf);

    if (active_socket_try_get_read_data(data->server_socket, &r_buf)) {
        if (r_buf.size > 0) {
            data->recievedDataFromClient.data = r_buf.data;
//            printf("%s", data->recievedDataFromClient.data);
            return true;
        }
    }

    return result;

}


//**********************************************************
//                  THREADS
//**********************************************************

//hrac na serveri bude zadavat poziciu kde chce dat znak
void *produce(void *thread_data) {
    THREAD_DATA *data = (THREAD_DATA *) thread_data;

//    pthread_mutex_lock(&data->mutex);
    while (!data->hra.hraSkoncila) {
//        pthread_mutex_unlock(&data->mutex);

        if (data->druhyHracConnected && data->hra.jeServerHracNaRade) {

            pthread_mutex_lock(&data->mutex);
            while (!data->hra.jeServerHracNaRade) {
                pthread_cond_wait(&data->hra.serverHracJeNaRade, &data->mutex);
            }
            pthread_mutex_unlock(&data->mutex);


            int riadokIndex = -1;
            int stlpecIndex = -1;

            if (data->hra.hraSkoncila){
                break;
            }

            printf("Zadaj poziciu stlpca do ktoreho chcete vkladat: \n");
            scanf("%d", &stlpecIndex);

            if (stlpecIndex < 0 || stlpecIndex >= POCET_STLPCOV || data->hra.hraciaPlocha[0][stlpecIndex] != '.') {
                printf("\nDo zvoleneho stlpca (%d) nemozno vlozit symbol: ", stlpecIndex);
                if (stlpecIndex < 0 || stlpecIndex >= POCET_STLPCOV) {
                    printf("Index je mimo hracej plochy\n");
                } else if (data->hra.hraciaPlocha[0][stlpecIndex] != '.') {
                    printf("Stlpec je plny\n");
                }
            } else {


                //checkovanie znakov ci pod akt. poziciou je nejaky znak
                for (int i = 0; i < POCET_RIADKOV; ++i) {
                    pthread_mutex_lock(&data->mutex);
                    if (data->hra.hraciaPlocha[i + 1][stlpecIndex] != '.' && i < POCET_RIADKOV - 1) {
                        riadokIndex = i;
                        data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakServerHraca;
                        pthread_mutex_unlock(&data->mutex);
                        break;

                    } else if (data->hra.hraciaPlocha[i][stlpecIndex] == '.' && i == POCET_RIADKOV - 1) {
                        riadokIndex = i;
                        data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakServerHraca;
                        pthread_mutex_unlock(&data->mutex);
                        break;
                    }
                    pthread_mutex_unlock(&data->mutex);

                }


                pthread_mutex_lock(&data->mutex);
                data->hra.suradnicePoslednehoTahuServra.riadok = riadokIndex;
                data->hra.suradnicePoslednehoTahuServra.stlpec = stlpecIndex;
                // pthread_mutex_unlock(&data->mutex);


//            pthread_mutex_lock(&data->mutex);
                vykresliHraciuPlochu(&data->hra);

                pthread_mutex_unlock(&data->mutex);

                // pthread_mutex_lock(&data->mutex);
                if (validujHru(&data->hra, data->hra.znakServerHraca)) {
                    data->hra.hraSkoncila = true;
                    data->hra.ktoVyhral = 1;
                }
                data->hra.jeServerHracNaRade = false;
                pthread_mutex_unlock(&data->mutex);

                pthread_cond_signal(&data->hra.serverHracSpravilTah);

            }
        }
    }
    pthread_mutex_unlock(&data->mutex);
    printf("\nDEBUG: HRA SKONCILA (produce output)\n");

    return NULL;
}


// samotna hra, bude prijmat inputy od hraca na serveri a clientovi
void *consume(void *thread_data) {
    THREAD_DATA *data = (THREAD_DATA *) thread_data;

    bool boloposlaneraz = false;

    while (!data->hra.hraSkoncila) {

        if (data->druhyHracConnected) {


            if (data->hra.jeServerHracNaRade) {
                pthread_cond_signal(&data->hra.serverHracJeNaRade);
                boloposlaneraz = false;

            } else {

                pthread_mutex_lock(&data->mutex);
                while (data->hra.jeServerHracNaRade) {  //cakanie na serverHraca nech dokonci svoj tah
                    pthread_cond_wait(&data->hra.serverHracSpravilTah, &data->mutex);
                }
                pthread_mutex_unlock(&data->mutex);

                //formatovanie dat pre klienta, momentalne sa posiela suradnica X a Y posledneho tahu serverHraca
                pthread_mutex_lock(&data->mutex);
                char buffer[256];
                sprintf(buffer, "%d %d %d %c %c", data->hra.suradnicePoslednehoTahuServra.riadok,
                        data->hra.suradnicePoslednehoTahuServra.stlpec, 0, data->hra.znakServerHraca, data->hra.znakClientHraca);
                strcpy(data->hra.dataSendToClient.data, buffer);
                pthread_mutex_unlock(&data->mutex);


                //odoslanie dat klientovy
                if (!boloposlaneraz) {
//                    printf("%s",data->hra.dataSendToClient.data);

                    active_socket_write_data_aspon_toto_funguje(data->server_socket, data->hra.dataSendToClient.data);

                    pthread_mutex_lock(&data->mutex);
                    boloposlaneraz = true;
                    pthread_mutex_unlock(&data->mutex);

                }


                //prazdny loop pre pockanie na data od clienta

                while (!try_read_data_from_client(data)) {}

                //citanie a formatovanie dat od klienta
                int riadokIndex, stlpecIndex;

                pthread_mutex_lock(&data->mutex);
                if (sscanf(data->recievedDataFromClient.data, "%d %d", &riadokIndex, &stlpecIndex) != 2) {
                    data->hra.hraSkoncila = true;
                    printf("ERROR: CLIENT POSLAL ZLY FORMAT DAT!!");
                    break;
                }
                pthread_mutex_unlock(&data->mutex);


                //vykreslenie plochy po tahu klienta
                pthread_mutex_lock(&data->mutex);
                data->hra.suradnicePoslednehoTahuClienta.riadok = riadokIndex;
                data->hra.suradnicePoslednehoTahuClienta.stlpec = stlpecIndex;
                data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakClientHraca;
                vykresliHraciuPlochu(&data->hra);

//                pthread_mutex_unlock(&data->mutex);
//
//                pthread_mutex_lock(&data->mutex);
                if (validujHru(&data->hra, data->hra.znakClientHraca)) {
                    data->hra.hraSkoncila = true;
                    data->hra.ktoVyhral = 2;
                }

                data->hra.jeServerHracNaRade = true;
                pthread_mutex_unlock(&data->mutex);
            }


        }

    }
    printf("\nDEBUG: HRA SKONCILA (consume output)\n");
    char buffer[256];
    sprintf(buffer, "1 1 %d %c %c", data->hra.ktoVyhral, data->hra.znakServerHraca, data->hra.znakClientHraca);
    strcpy(data->hra.dataSendToClient.data, buffer);

    active_socket_write_data_aspon_toto_funguje(data->server_socket, data->hra.dataSendToClient.data);
    if (data->hra.ktoVyhral == 1) {
        printf("\nVyhral si, gratulujeme!\n");
    } else if (data->hra.ktoVyhral == 2) {
        printf("\nVyhral Klient, gratulujeme!\n");

    }

    return NULL;

}

void *spracuj_client_data(void *thread_data) {
    THREAD_DATA *data = (THREAD_DATA *) thread_data;
    PASSIVE_SOCKET p_socket;
    passive_socket_init(&p_socket);
    passive_socket_start_listening(&p_socket, data->port);
    passive_socket_wait_for_client(&p_socket, data->server_socket);
    passive_socket_stop_listening(&p_socket);
    passive_socket_destroy(&p_socket);

    pthread_mutex_lock(&data->mutex);
    data->hra.jeServerHracNaRade = true; //TODO FIX?
    data->druhyHracConnected = true;
    pthread_mutex_unlock(&data->mutex);


    printf("connected\n");
    active_socket_start_reading(data->server_socket);
    return NULL;
}


//**********************************************************
//                  MAIN
//**********************************************************
int main(int args, char **argv) {//poradie argumentov: port znakServerHraca znakClientHraca

    short port;
    char znakServerHraca ;
    char znakClientHraca;

    if (args > 3) {
        port = atoi(argv[1]);
        znakServerHraca = *argv[2];
        znakClientHraca = *argv[3];
    }
    else {
        port = 15071;
        znakServerHraca = 'S';
        znakClientHraca = 'C';
    }

// ***init
    pthread_t th_produce; // hracov input
    pthread_t th_retrieveClient; //pripojenie clienta
    pthread_t th_consume; //logika a ziskavanie dat od klienta

    struct hra hra;
    struct thread_data data;
    struct active_socket server_socket;

    active_socket_init(&server_socket);

    hra_init(&hra, znakServerHraca, znakClientHraca,
             false);//TODO FIX ze sa bude dat nastavit kto ide prvy, zatial sa hra zapina z passive socketu
    thread_data_init(&data, port, &server_socket, &hra);


    pthread_create(&th_retrieveClient, NULL, spracuj_client_data, &data);
    pthread_create(&th_produce, NULL, produce, &data);
    pthread_create(&th_consume, NULL, consume, &data);

// ***use

    pthread_join(th_retrieveClient, NULL);
    pthread_join(th_produce, NULL);
    pthread_join(th_consume, NULL);

// ***destroy
    thread_data_destroy(&data);
    active_socket_destroy(&server_socket);


    return 0;
}
