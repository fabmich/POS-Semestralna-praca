//
// Created by Miso on 06/01/2024.
//

#include "ThreadData.h"


void thread_data_init(struct hra *hra,struct thread_data *data, MySocket *mySocket) {

    data->hra = *hra;

    data->mySocket = mySocket;
}


