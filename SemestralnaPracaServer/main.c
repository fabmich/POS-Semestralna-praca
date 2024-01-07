
#include <stdio.h>
#include <pthread.h>
#include "pos_sockets/char_buffer.h"
#include "pos_sockets/active_socket.h"
#include "pos_sockets/passive_socket.h"
#include <string.h>
#include "connect4/connect4.h"
#include "connect4/structures/hra.h"
#include "connect4/structures/thread_data.h"


void printIntro() {
    char hviezda = '*';
    for (int i = 0; i < 31; ++i) {
        printf("%c", hviezda);
    }
    printf("\n*          CONNECT4          *\n");

    for (int i = 0; i < 31; ++i) {
        printf("%c", hviezda);
    }
    printf("\n");
}



//**********************************************************
//                  MAIN
//**********************************************************
int main(int args, char **argv) {//poradie argumentov: port znakServerHraca znakClientHraca

    short port;
    char znakServerHraca ;
    char znakClientHraca;
    bool zacinaServer;

    if (args > 4) {
        port = atoi(argv[1]);
        znakServerHraca = *argv[2];
        znakClientHraca = *argv[3];
        if ( atoi(argv[4]) == 1) {
            zacinaServer = true;
        } else {
            zacinaServer = false;
        }
    }
    else {
        port = 15071;
        znakServerHraca = 'S';
        znakClientHraca = 'C';
        zacinaServer = true;

    }

    printIntro();


// ***init
    pthread_t th_produce; // hracov input
    pthread_t th_retrieveClient; //pripojenie clienta
    pthread_t th_consume; //logika a ziskavanie dat od klienta

    struct hra hra;
    struct thread_data data;
    struct active_socket server_socket;

    active_socket_init(&server_socket);

    hra_init(&hra, znakServerHraca, znakClientHraca,
             zacinaServer);
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
