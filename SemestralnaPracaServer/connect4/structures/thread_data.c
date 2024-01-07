
#include "thread_data.h"


void thread_data_init(struct thread_data *data, short p_port, ACTIVE_SOCKET *p_socket, HRA *p_hra) {
    data->hra = *p_hra;
    pthread_mutex_init(&data->mutex, NULL);

    data->druhyHracConnected = false;
    data->port = p_port;
    data->server_socket = p_socket;
    data->hra.prveKolo = 1;
    pthread_cond_init(&data->hra.serverHracJeNaRade, NULL);
    pthread_cond_init(&data->hra.serverHracSpravilTah, NULL);
    char_buffer_init(&data->recievedDataFromClient);
};

void thread_data_destroy(struct thread_data *data) {
    hra_destroy(&data->hra);
    data->port = 0;
    data->server_socket = NULL;
    pthread_mutex_destroy(&data->mutex);

    char_buffer_destroy(&data->recievedDataFromClient);
}
