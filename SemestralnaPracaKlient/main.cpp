#include "MemoryLeak.h"
#include "connect4/structures/Hra.h"
#include "connect4/structures/ThreadData.h"
#include "connect4/Connect4.h"
#include <thread>
#include <queue>
#include <string>
#include "my_socket.h"

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
    printf("\n");
}



int main(int args, char **argv) {//poradie: server, port
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    printIntro();

    std::string server;
    short port;

    if (args > 2) {
        server = argv[1];
        port = atoi(argv[2]);
    } else {
        server = "frios2.fri.uniza.sk";
        port = 15071;
    }
//std::string ipAdresa = "91.127.42.240";

    THREAD_DATA data;
    HRA hra;
    hra_init(&hra);
    MySocket *mySocket = MySocket::createConnection(server, port);
    thread_data_init(&hra, &data, mySocket);


    std::thread thConsume(consume, &data);
    std::thread thProduce(produce, &data);


    thConsume.join();
    thProduce.join();


    delete mySocket;
    mySocket = nullptr;

    _CrtDumpMemoryLeaks();
    return 0;
}
