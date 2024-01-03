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
    int x;
    int y;
} COORDS;

typedef struct hra {
    char hraciaPlocha[POCET_RIADKOV][POCET_STLPCOV];
    bool jeServerHracNaRade;
    bool hraNeskoncila;

    char znakServerHraca;
    char znakClientHraca;

    CHAR_BUFFER dataSendToClient;
    COORDS suradnicePoslednehoTahuServra;

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
void thread_data_init(struct thread_data* data, short p_port, ACTIVE_SOCKET* p_socket, HRA* p_hra) {
    data->hra = *p_hra;
    pthread_mutex_init(&data->mutex, NULL);

    data->druhyHracConnected = false;
    data->port = p_port;
    data->server_socket= p_socket;
    pthread_cond_init(&data->hra.serverHracJeNaRade, NULL);
    pthread_cond_init(&data->hra.serverHracSpravilTah, NULL);
    char_buffer_init(&data->recievedDataFromClient);
};

void hra_init(struct hra* hra, char p_znakServerHraca,char p_znakClientHraca, bool p_jeServerHracNaRade) {

    for (int riadok = 0; riadok < POCET_RIADKOV; ++riadok) {
        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {
                hra->hraciaPlocha[riadok][stlpec] = '.';
        }
    }

    hra->znakServerHraca = p_znakServerHraca;
    hra->znakClientHraca = p_znakClientHraca;
    hra->jeServerHracNaRade = p_jeServerHracNaRade;
    hra->hraNeskoncila = false;
    char_buffer_init(&hra->dataSendToClient);

    hra->suradnicePoslednehoTahuServra.x = -1;
    hra->suradnicePoslednehoTahuServra.y = -1;

}

//**********************************************************
//                  DESTRUCTION
//**********************************************************
void hra_destroy (struct hra* hra) {
    hra->hraNeskoncila = false;
    hra->jeServerHracNaRade = false;
    hra->znakClientHraca = ' ';
    hra->znakServerHraca = ' ';
    for (int riadok = 0; riadok < POCET_RIADKOV; ++riadok) {
        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {
            hra->hraciaPlocha[riadok][stlpec] = ' ';
        }
    }
}

void thread_data_destroy(struct thread_data* data) {
    hra_destroy(&data->hra);
    data->port = 0;
    data->server_socket = NULL;
    pthread_mutex_destroy(&data->mutex);

    //TODO destroy conditions?
}


//**********************************************************
//                  RENDER PLOCHY
//**********************************************************
void vykresliHraciuPlochu(struct hra* hra) {

    char abeceda[] = {'A', 'B', 'C', 'D','E','F', ' '};

    printf("\n");

    for (int riadok = 0; riadok < POCET_RIADKOV + 1; ++riadok) {
        printf("%c |",abeceda[riadok]);

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
bool validujHru(struct hra* hra) {

}


//ziskanie dat od clienta, skopirovane z cvika
_Bool try_read_data_from_client(struct thread_data* data) {
    _Bool result = false;

    CHAR_BUFFER r_buf;
    char_buffer_init(&r_buf);

    if(active_socket_try_get_read_data(data->server_socket, &r_buf)) {
        if(r_buf.size > 0) {
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

    pthread_mutex_lock(&data->mutex);
    while (!data->hra.hraNeskoncila ) {
        pthread_mutex_unlock(&data->mutex);

        pthread_mutex_lock(&data->mutex);
        while (!data->hra.jeServerHracNaRade) {
            pthread_cond_wait(&data->hra.serverHracJeNaRade, &data->mutex);
        }
        pthread_mutex_unlock(&data->mutex);


        int riadokIndex = -1;
        int stlpecIndex = -1;

        printf("Zadaj poziciu( zatial ako napr 1 2, FIX THIS SHIT na iba 1,2,3 a pod staci len Y poziciu zadavat): \n");
        scanf("%d %d", &riadokIndex, &stlpecIndex);

        data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakServerHraca;

        pthread_mutex_lock(&data->mutex);
        data->hra.jeServerHracNaRade = false;
        data->hra.suradnicePoslednehoTahuServra.x = riadokIndex;
        data->hra.suradnicePoslednehoTahuServra.y = stlpecIndex;
        pthread_mutex_unlock(&data->mutex);


        pthread_cond_signal(&data->hra.serverHracSpravilTah);

        pthread_mutex_lock(&data->mutex);
        vykresliHraciuPlochu(&data->hra);
        pthread_mutex_unlock(&data->mutex);

    }
    pthread_mutex_unlock(&data->mutex);

    return NULL;
}




// samotna hra, bude prijmat inputy od hraca na serveri a clientovi
void *consume(void *thread_data) {
    THREAD_DATA *data = (THREAD_DATA *)thread_data;

    bool boloposlaneraz = false;

    while (!data->hra.hraNeskoncila ) {

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
                sprintf(buffer, "%d %d", data->hra.suradnicePoslednehoTahuServra.x, data->hra.suradnicePoslednehoTahuServra.y);
                strcpy(data->hra.dataSendToClient.data, buffer);
                pthread_mutex_unlock(&data->mutex);


                //odoslanie dat klientovy
                if (!boloposlaneraz) {
                    active_socket_write_data_aspon_toto_funguje(data->server_socket, data->hra.dataSendToClient.data);
                    boloposlaneraz = true;
                }


                //prazdny loop pre pockanie na data od clienta
                while ( !try_read_data_from_client(data)) {}
                //citanie a formatovanie dat od klienta
                int riadokIndex, stlpecIndex;
                sscanf(data->recievedDataFromClient.data, "%d %d", &riadokIndex, &stlpecIndex);

                //vykreslenie plochy po tahu klienta
                pthread_mutex_lock(&data->mutex);
                data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakClientHraca;

                vykresliHraciuPlochu(&data->hra);
                data->hra.jeServerHracNaRade = true;
                pthread_mutex_unlock(&data->mutex);

            }
        }

    }
    return NULL;

}

void* spracuj_client_data(void* thread_data) {
    THREAD_DATA *data = (THREAD_DATA *)thread_data;
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
int main(int argc, char *argv[]) {

// ***init
    pthread_t th_produce; // hracov input
    pthread_t th_retrieveClient; //pripojenie clienta
    pthread_t th_consume; //logika a ziskavanie dat od klienta

    struct hra hra;
    struct thread_data data;
    struct active_socket server_socket;

    active_socket_init(&server_socket);

    hra_init(&hra, 'X', 'O', false);//TODO FIX ze sa bude dat nastavit kto ide prvy, zatial sa hra zapina z passive socketu
    thread_data_init(&data, 15069, &server_socket, &hra);


    pthread_create(&th_retrieveClient, NULL, spracuj_client_data , &data);
    pthread_create(&th_produce, NULL, produce, &data);
    pthread_create(&th_consume, NULL,  consume, &data);

// ***use

    pthread_join(th_retrieveClient, NULL);
    pthread_join(th_produce, NULL);
    pthread_join(th_consume, NULL);

// ***destroy
    thread_data_destroy(&data);
    active_socket_destroy(&server_socket);

    return 0;
}
