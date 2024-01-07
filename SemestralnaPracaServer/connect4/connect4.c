#include <stdio.h>
#include <pthread.h>
#include "../pos_sockets/char_buffer.h"
#include "../pos_sockets/active_socket.h"
#include "../pos_sockets/passive_socket.h"
#include <string.h>
#include "structures/hra.h"
#include "structures/thread_data.h"


//ziskanie dat od clienta, skopirovane z cvika
_Bool try_read_data_from_client(struct thread_data *data) {
    _Bool result = false;

    CHAR_BUFFER r_buf;
    char_buffer_init(&r_buf);

    if (active_socket_try_get_read_data(data->server_socket, &r_buf)) {
        if (r_buf.size > 0) {
            char_buffer_copy(&data->recievedDataFromClient, &r_buf);
//            printf("%s", data->recievedDataFromClient.data);
            char_buffer_destroy(&r_buf);
            return true;
        }
    }
    char_buffer_destroy(&r_buf);

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

            printf("Zadaj poziciu stlpca do ktoreho chces vkladat: \n");
            scanf("%d", &stlpecIndex);
            stlpecIndex = stlpecIndex - 1;
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
                    pthread_mutex_lock(&data->hra.hraciaPlochaMutex);
                    if (data->hra.hraciaPlocha[i + 1][stlpecIndex] != '.' && i < POCET_RIADKOV - 1) {
                        riadokIndex = i;
                        data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakServerHraca;
                        pthread_mutex_unlock(&data->hra.hraciaPlochaMutex);
                        break;

                    } else if (data->hra.hraciaPlocha[i][stlpecIndex] == '.' && i == POCET_RIADKOV - 1) {
                        riadokIndex = i;
                        data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakServerHraca;
                        pthread_mutex_unlock(&data->hra.hraciaPlochaMutex);
                        break;
                    }
                    pthread_mutex_unlock(&data->hra.hraciaPlochaMutex);

                }


                pthread_mutex_lock(&data->hra.hraciaPlochaMutex);
                data->hra.suradnicePoslednehoTahuServra.riadok = riadokIndex;
                data->hra.suradnicePoslednehoTahuServra.stlpec = stlpecIndex;
                // pthread_mutex_unlock(&data->mutex);


//            pthread_mutex_lock(&data->mutex);
                vykresliHraciuPlochu(&data->hra);

                pthread_mutex_unlock(&data->hra.hraciaPlochaMutex);

                // pthread_mutex_lock(&data->mutex);
                if (validujHru(&data->hra, data->hra.znakServerHraca)) {
                    data->hra.hraSkoncila = true;
//                    data->hra.ktoVyhral = 1;
                }
                data->hra.jeServerHracNaRade = false;
                pthread_mutex_unlock(&data->mutex);

                pthread_cond_signal(&data->hra.serverHracSpravilTah);

            }
        }
    }
    pthread_mutex_unlock(&data->mutex);
//    printf("\nDEBUG: HRA SKONCILA (produce output)\n");

    return NULL;
}


// samotna hra, bude prijmat inputy od hraca na serveri a clientovi
void *consume(void *thread_data) {
    THREAD_DATA *data = (THREAD_DATA *) thread_data;

    char abeceda[] = {'A', 'B', 'C', 'D', 'E', 'F', ' '};

    bool boloposlaneraz = false;

    while (!data->hra.hraSkoncila) {

        if (data->druhyHracConnected) {


            if (data->hra.jeServerHracNaRade) {
                data->hra.prveKolo = 0;
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
                int zacinaServer = 0;
                if (data->hra.prveKolo == 1) {
                    zacinaServer = 1;
                    data->hra.prveKolo = 0;
                }

                char buffer[256];
                sprintf(buffer, "%d %d %d %c %c %d 0", data->hra.suradnicePoslednehoTahuServra.riadok,
                        data->hra.suradnicePoslednehoTahuServra.stlpec, 0, data->hra.znakServerHraca, data->hra.znakClientHraca
                        , zacinaServer);
                strcpy(data->hra.dataSendToClient.data, buffer);
                pthread_mutex_unlock(&data->mutex);


                //odoslanie dat klientovy
                if (!boloposlaneraz) {
//                    printf("%s",data->hra.dataSendToClient.data);

                    active_socket_write_data_aspon_toto_funguje(data->server_socket, data->hra.dataSendToClient.data);

//                    pthread_mutex_lock(&data->mutex);
                    boloposlaneraz = true;
//                    pthread_mutex_unlock(&data->mutex);

                }


                //prazdny loop pre pockanie na data od clienta
                //citanie a formatovanie dat od klienta
                pthread_mutex_lock(&data->mutex);
                while (!try_read_data_from_client(data)) {}
                pthread_mutex_unlock(&data->mutex);

                int riadokIndex, stlpecIndex;

                pthread_mutex_lock(&data->mutex);
                if (sscanf(data->recievedDataFromClient.data, "%d %d", &riadokIndex, &stlpecIndex) != 2) {
                    data->hra.hraSkoncila = true;
                    printf("ERROR: CLIENT POSLAL ZLY FORMAT DAT!!");
                    break;
                }
                pthread_mutex_unlock(&data->mutex);


                //vykreslenie plochy po tahu klienta
                pthread_mutex_lock(&data->hra.hraciaPlochaMutex);
                data->hra.suradnicePoslednehoTahuClienta.riadok = riadokIndex;
                data->hra.suradnicePoslednehoTahuClienta.stlpec = stlpecIndex;
                data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakClientHraca;
                vykresliHraciuPlochu(&data->hra);
                printf("Klient spravil ťah: %c%d \n", abeceda[riadokIndex], stlpecIndex + 1);
                pthread_mutex_unlock(&data->hra.hraciaPlochaMutex);
//
                pthread_mutex_lock(&data->hra.hraciaPlochaMutex);
                if (validujHru(&data->hra, data->hra.znakClientHraca)) {
                    data->hra.hraSkoncila = true;
//                    data->hra.ktoVyhral = 2;
                }

                data->hra.jeServerHracNaRade = true;
                pthread_mutex_unlock(&data->hra.hraciaPlochaMutex);
            }


        }

    }

    pthread_mutex_lock(&data->mutex);

//    printf("\nDEBUG: HRA SKONCILA (consume output)\n");
    char buffer[256];
    sprintf(buffer, "%d %d %d %c %c 0 %d",data->hra.suradnicePoslednehoTahuServra.riadok,data->hra.suradnicePoslednehoTahuServra.stlpec,
            data->hra.ktoVyhral, data->hra.znakServerHraca, data->hra.znakClientHraca
            , data->hra.akoVyhral);
    strcpy(data->hra.dataSendToClient.data, buffer);

    active_socket_write_data_aspon_toto_funguje(data->server_socket, data->hra.dataSendToClient.data);
    if (data->hra.ktoVyhral == 1) {
        printf("\nVyhral si, gratulujeme!\n");
    } else if (data->hra.ktoVyhral == 2) {
        printf("\nVyhral Klient, gratulujeme!\n");
    } else if (data->hra.ktoVyhral == 3) {
        printf("\nKoniec hry, hracia plocha je plná\n");

    }
    pthread_mutex_unlock(&data->mutex);

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


    printf("Druhý hráč sa pripojil\n");

    pthread_mutex_lock(&data->mutex);
    data->druhyHracConnected = true;
    vykresliHraciuPlochu(&data->hra);

    pthread_mutex_unlock(&data->mutex);

    active_socket_start_reading(data->server_socket);
    return NULL;
}