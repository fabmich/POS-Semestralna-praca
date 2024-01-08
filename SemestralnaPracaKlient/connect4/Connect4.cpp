#include "Connect4.h"

void *produce(void *thread_data) {
    auto *data = (THREAD_DATA *) thread_data;

    vykresliHraciuPlochu(&data->hra);

    while (!data->hra.hraSkoncila) {

        std::unique_lock<std::mutex> lock(data->mutex);
        while (!data->hra.jeKlientNaRade) {
            data->jeClientHracNaRade.wait(lock);

        }

        if (data->hra.hraSkoncila) {
            lock.unlock();
            break;
        }

        int riadokIndex = -1;
        int stlpecIndex = -1;

        std::cout << "\n Zadaj poziciu stlpca do ktoreho chcete vkladat: \n";
        std::cin >> stlpecIndex;
        stlpecIndex = stlpecIndex - 1;
        if (stlpecIndex < 0 || stlpecIndex >= POCET_STLPCOV || data->hra.hraciaPlocha[0][stlpecIndex] != '.') {
            printf("\nDo zvoleneho stlpca (%d) nemozno vlozit symbol: ", stlpecIndex);
            if (stlpecIndex < 0 || stlpecIndex >= POCET_STLPCOV) {
                printf("Index je mimo hracej plochy\n");
            } else if (data->hra.hraciaPlocha[0][stlpecIndex] != '.') {
                printf("Stlpec je plny\n");
            }
        } else {
            for (int i = 0; i < POCET_RIADKOV; ++i) {
                if (data->hra.hraciaPlocha[i + 1][stlpecIndex] != '.' && i < POCET_RIADKOV - 1) {
                    riadokIndex = i;
                    data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakClientHraca;
                    break;

                } else if (data->hra.hraciaPlocha[i][stlpecIndex] == '.' && i == POCET_RIADKOV - 1) {
                    riadokIndex = i;
                    data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakClientHraca;
                    break;
                }

            }

            data->hra.hraciaPlocha[riadokIndex][stlpecIndex] = data->hra.znakClientHraca;

            data->mySocket->sendData(std::to_string(riadokIndex) + " " + std::to_string(stlpecIndex));

            vykresliHraciuPlochu(&data->hra);
            data->hra.jeKlientNaRade = false;
            lock.unlock();
        }
    }

    return nullptr;
}


void *consume(void *thread_data) {
    auto *data = (THREAD_DATA *) thread_data;

    char abeceda[] = {'A', 'B', 'C', 'D', 'E', 'F', ' '};

    std::string dataLastRecieved;// plocha sa renderne 2 krat pri ziskani dat, toto bude porovnavat ci prisli rovnake structures aby sa nerenderovala plocha 2 a viac krat

    while (!data->hra.hraSkoncila) {
        std::string dataFromServer = data->mySocket->receiveData();//ziskanie dat od servra

        std::unique_lock<std::mutex> lock(data->mutex);

        if (!dataFromServer.empty() && dataLastRecieved != dataFromServer) {

            //ziskanie dat zo stringu
            std::istringstream iss(dataFromServer);
            int riadok, stlpec, vysledokHry, zacinaClient, akoVyhral;
            char znakServra, znakClienta;

            //zapisanie dat do hracej plochy u klientovej hracej plochy
            if (iss >> riadok >> stlpec >> vysledokHry >> znakServra >> znakClienta >> zacinaClient >> akoVyhral) {
                if (zacinaClient == 0) {
                    if (vysledokHry != data->hra.vysledokHry) {
                        data->mySocket->sendEndMessage();

                        data->hra.vysledokHry = vysledokHry;
                        data->hra.akoVyhral = akoVyhral;
                        data->hra.hraSkoncila = true;
                        data->hra.jeKlientNaRade = true;
                        data->jeClientHracNaRade.notify_all();

                        data->hra.hraciaPlocha[riadok][stlpec] = znakServra;

                        lock.unlock();
                        break;
                    }

                    data->hra.hraciaPlocha[riadok][stlpec] = znakServra;
                    data->hra.znakClientHraca = znakClienta;
                    data->hra.znakServerHraca = znakServra;

                    vykresliHraciuPlochu(&data->hra);
                    std::cout << "Hrac na serveri spravil tah: " << abeceda[riadok] << stlpec + 1<< std::endl;

                    data->jeClientHracNaRade.notify_all();

                    data->hra.jeKlientNaRade = true;

                    dataLastRecieved = dataFromServer;

                } else{
                    data->hra.znakClientHraca = znakClienta;
                    data->hra.jeKlientNaRade = true;
                    data->jeClientHracNaRade.notify_all();
                }


            } else {
                data->mySocket->sendEndMessage();

                data->hra.hraSkoncila = true;
                data->hra.jeKlientNaRade = true;
                data->jeClientHracNaRade.notify_all();

                lock.unlock();
                break;
            }
        }
    }

    vykresliHraciuPlochu(&data->hra);

    if (data->hra.vysledokHry == 1) {

        if (data->hra.akoVyhral == 1) {
            std::cout << "Vidim 4 znaky (" <<  data->hra.znakServerHraca << ") horizontalne" << std::endl;
        }
        else if(data->hra.akoVyhral == 2) {
            std::cout << "Vidim 4 znaky (" <<  data->hra.znakServerHraca<<") vertikalne" << std::endl;

        }
        else if(data->hra.akoVyhral == 3) {
            std::cout << "Vidim 4 znaky (" <<  data->hra.znakServerHraca<<") diagonale" << std::endl;

        }
        std::cout << "Vyhral hrac na servri, gratulujeme!" << std::endl;

    } else if (data->hra.vysledokHry == 2) {

        if (data->hra.akoVyhral == 1) {
            std::cout << "Vidim 4 znaky (" <<  data->hra.znakClientHraca<<") horizontalne" << std::endl;
        }
        else if(data->hra.akoVyhral == 2) {
            std::cout << "Vidim 4 znaky (" <<  data->hra.znakClientHraca<<") vertikalne" << std::endl;
        }

        else if(data->hra.akoVyhral == 3) {
            std::cout << "Vidim 4 znaky (" <<  data->hra.znakClientHraca<<") diagonale" << std::endl;
        }

        std::cout << "Vyhrali ste, gratulujeme!" << std::endl;

    } else {
        std::cout << "Nikto nevyhral, hracia plocha je plna" << std::endl;

    }

    return nullptr;

}