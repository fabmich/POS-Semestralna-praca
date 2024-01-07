
#pragma once
#include "structures/Hra.h"
#include "structures/ThreadData.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <sstream>
#include "../my_socket.h"


void *produce(void *thread_data);
void *consume(void *thread_data);