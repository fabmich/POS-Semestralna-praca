#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <sstream>
#include "my_socket.h"


#define POCET_RIADKOV 6
#define POCET_STLPCOV 7


typedef struct thread_data {
    char hraciaPlocha[POCET_RIADKOV][POCET_STLPCOV];
    bool hraSkoncila;
    bool jeKlientNaRade;
    char znakServerHraca;

    std::string dataNaOdoslanie;
    std::string prijateData;

    MySocket *mySocket;

    std::mutex mutex;
    std::condition_variable jeClientHracNaRade;

} THREAD_DATA;

void thread_data_init(struct thread_data *data, MySocket *mySocket) {
    for (int riadok = 0; riadok < POCET_RIADKOV; ++riadok) {
        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {
            data->hraciaPlocha[riadok][stlpec] = '.';
        }
    }
    data->znakServerHraca = 'O';
    data->hraSkoncila = false;
    data->jeKlientNaRade = false;
    data->mySocket = mySocket;
}


void vykresliHraciuPlochu(struct thread_data *data) {

    char abeceda[] = {'A', 'B', 'C', 'D', 'E', 'F', ' '};


    for (int riadok = 0; riadok < POCET_RIADKOV + 1; ++riadok) {
        printf("%c |", abeceda[riadok]);

        for (int stlpec = 0; stlpec < POCET_STLPCOV; ++stlpec) {

            if (riadok < POCET_RIADKOV) {
                printf(" %c |", data->hraciaPlocha[riadok][stlpec]);
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

// v podstate to iste ako na servri
void *produce(void *thread_data) {
    THREAD_DATA *data = (THREAD_DATA *) thread_data;

    while (!data->hraSkoncila) {

        std::unique_lock<std::mutex> lock(data->mutex);
        while (!data->jeKlientNaRade) {
            data->jeClientHracNaRade.wait(lock);

        }

        int riadokIndex = -1;
        int stlpecIndex = -1;

        std::cout << "\n Zadaj poziciu (napr. 1 2): \n";
        std::cin >> riadokIndex >> stlpecIndex;

        data->hraciaPlocha[riadokIndex][stlpecIndex] = data->znakServerHraca;

        data->mySocket->sendData(std::to_string(riadokIndex) + " " + std::to_string(stlpecIndex));

        vykresliHraciuPlochu(data);
        data->jeKlientNaRade = false;

    }

    return nullptr;
}


void *consume(void *thread_data) {
    auto *data = (THREAD_DATA *) thread_data;

    std::string dataLastRecieved;// plocha sa renderne 2 krat pri ziskani dat, toto bude porovnavat ci prisli rovnake data aby sa nerenderovala plocha 2 a viac krat

    while (!data->hraSkoncila) {
        std::string dataFromServer = data->mySocket->receiveData();//ziskanie dat od servra

        if (!dataFromServer.empty() && dataLastRecieved != dataFromServer) {
            std::cout << dataFromServer << std::endl;//????

            //ziskanie dat zo stringu
            std::istringstream iss(dataFromServer);
            int riadok, stlpec;

            //zapisanie dat do hracej plochy u klientovej hracej plochy
            if (iss >> riadok >> stlpec) {
                data->hraciaPlocha[riadok][stlpec] = 'X';
            }
            vykresliHraciuPlochu(data);

            data->jeClientHracNaRade.notify_all();

            data->jeKlientNaRade = true;

            dataLastRecieved = dataFromServer;
        }


    }

    return nullptr;

}

int main() {//todo argument

    std::string ipAdresa = "frios2.fri.uniza.sk";
//std::string ipAdresa = "91.127.42.240";

    THREAD_DATA data;

    MySocket *mySocket = MySocket::createConnection(ipAdresa, 15069);

    thread_data_init(&data, mySocket);


    std::thread thConsume(consume, &data);
    std::thread thProduce(produce, &data);


    thConsume.join();
    thProduce.join();


    delete mySocket;
    mySocket = nullptr;

    return 0;
}
