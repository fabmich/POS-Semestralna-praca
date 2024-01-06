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
    char znakClientHraca;
    int vysledokHry;

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
//    data->znakClientHraca = 'O';
    data->hraSkoncila = false;
    data->jeKlientNaRade = false;
    data->mySocket = mySocket;
    data->vysledokHry = 0;
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

        if (data->hraSkoncila) {
            break;
        }

        int riadokIndex = -1;
        int stlpecIndex = -1;

        std::cout << "\n Zadaj poziciu stlpca do ktoreho chcete vkladat: \n";
        std::cin >> stlpecIndex;

        if (stlpecIndex < 0 || stlpecIndex >= POCET_STLPCOV || data->hraciaPlocha[0][stlpecIndex] != '.') {
            printf("\nDo zvoleneho stlpca (%d) nemozno vlozit symbol: ", stlpecIndex);
            if (stlpecIndex < 0 || stlpecIndex >= POCET_STLPCOV) {
                printf("Index je mimo hracej plochy\n");
            } else if (data->hraciaPlocha[0][stlpecIndex] != '.') {
                printf("Stlpec je plny\n");
            }
        }
        else {
            for (int i = 0; i < POCET_RIADKOV; ++i) {
                if (data->hraciaPlocha[i + 1][stlpecIndex] != '.' && i < POCET_RIADKOV - 1) {
                    riadokIndex = i;
                    data->hraciaPlocha[riadokIndex][stlpecIndex] = data->znakClientHraca;
                    break;

                } else if (data->hraciaPlocha[i][stlpecIndex] == '.' && i == POCET_RIADKOV - 1) {
                    riadokIndex = i;
                    data->hraciaPlocha[riadokIndex][stlpecIndex] = data->znakClientHraca;
                    break;
                }

            }



        data->hraciaPlocha[riadokIndex][stlpecIndex] = data->znakClientHraca;

        data->mySocket->sendData(std::to_string(riadokIndex) + " " + std::to_string(stlpecIndex));

        vykresliHraciuPlochu(data);
        data->jeKlientNaRade = false;

        }
    }

    return nullptr;
}


void *consume(void *thread_data) {
    auto *data = (THREAD_DATA *) thread_data;

    std::string dataLastRecieved;// plocha sa renderne 2 krat pri ziskani dat, toto bude porovnavat ci prisli rovnake data aby sa nerenderovala plocha 2 a viac krat

    while (!data->hraSkoncila) {
        std::string dataFromServer = data->mySocket->receiveData();//ziskanie dat od servra

        if (!dataFromServer.empty() && dataLastRecieved != dataFromServer) {
//            std::cout << dataFromServer << std::endl;//????

            //ziskanie dat zo stringu
            std::istringstream iss(dataFromServer);
            int riadok, stlpec, vysledokHry;
            char znakServra, znakClienta;

            //zapisanie dat do hracej plochy u klientovej hracej plochy
            if (iss >> riadok >> stlpec >> vysledokHry >>  znakServra >>znakClienta ) {

                if (vysledokHry != data->vysledokHry) {
                    data->mySocket->sendEndMessage();

                    data->vysledokHry = vysledokHry;

                    data->hraSkoncila = true;
                    data->jeKlientNaRade = true;
                    data->jeClientHracNaRade.notify_all();

                    break;
                }

                data->hraciaPlocha[riadok][stlpec] = znakServra;
                data->znakClientHraca = znakClienta;

                vykresliHraciuPlochu(data);

                data->jeClientHracNaRade.notify_all();

                data->jeKlientNaRade = true;

                dataLastRecieved = dataFromServer;

            } else {
                data->mySocket->sendEndMessage();

                data->hraSkoncila = true;
                data->jeKlientNaRade = true;
                data->jeClientHracNaRade.notify_all();
                break;
            }

        }


    }

    if (data->vysledokHry == 1) {
        std::cout << "Vyhral hrac na servi, gratulujeme!" << std::endl;
    } else if (data->vysledokHry == 2) {
        std::cout << "Vyhrali ste, gratulujeme!" << std::endl;
    }


    return nullptr;

}

int main(int args, char **argv) {//poradie: server, port

    std::string server;
    short port;

    if (args > 2) {
        server = argv[1];
        port = atoi(argv[2]);
    }
    else {
        server = "frios2.fri.uniza.sk";
        port = 15071;
    }

//std::string ipAdresa = "91.127.42.240";

    THREAD_DATA data;

    MySocket *mySocket = MySocket::createConnection(server, port);
    thread_data_init(&data, mySocket);


    std::thread thConsume(consume, &data);
    std::thread thProduce(produce, &data);


    thConsume.join();
    thProduce.join();


    delete mySocket;
    mySocket = nullptr;

    return 0;
}
