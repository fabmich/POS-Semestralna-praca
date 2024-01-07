#pragma once

#ifndef CONNECT4_H
#define CONNECT4_H

#include <stdbool.h>
#include "../pos_sockets/active_socket.h"
#include "structures/hra.h"
#include "structures/thread_data.h"


_Bool try_read_data_from_client(struct thread_data *data);
void *produce(void *thread_data);
void *consume(void *thread_data);
void *spracuj_client_data(void *thread_data);

#endif
