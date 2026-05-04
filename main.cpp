//
// Created by andr on 04.05.2026.
//
#include "harvester.h"

int main(){
    harvester *h = new harvester;
    h->initialize();
    h->step();
    h->terminate();
    return 0;
}