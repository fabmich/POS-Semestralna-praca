#pragma once
#include "Hra.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include "../../my_socket.h"


typedef struct thread_data {
    HRA hra;

    std::string dataNaOdoslanie;
    std::string prijateData;

    MySocket *mySocket;

    std::mutex mutex;

    std::condition_variable jeClientHracNaRade;

} THREAD_DATA;

void thread_data_init(struct hra *hra,struct thread_data *data, MySocket *mySocket);