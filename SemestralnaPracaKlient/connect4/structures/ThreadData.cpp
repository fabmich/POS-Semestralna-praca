#include "ThreadData.h"


void thread_data_init(struct hra *hra,struct thread_data *data, MySocket *mySocket) {

    data->hra = *hra;

    data->mySocket = mySocket;


}


