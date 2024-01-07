#pragma once

#include <stdio.h>
#include <pthread.h>
#include "../../pos_sockets/char_buffer.h"
#include "../../pos_sockets/active_socket.h"
#include "../../pos_sockets/passive_socket.h"
#include <string.h>
#include "hra.h"


typedef struct thread_data {
    HRA hra;
    bool druhyHracConnected;

    short port;
    ACTIVE_SOCKET *server_socket;

    CHAR_BUFFER recievedDataFromClient;

    pthread_mutex_t mutex;

} THREAD_DATA;

void thread_data_init(struct thread_data *data, short p_port, ACTIVE_SOCKET *p_socket, HRA *p_hra);
void thread_data_destroy(struct thread_data *data) ;