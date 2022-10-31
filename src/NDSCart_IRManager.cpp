/*
    Copyright 2016-2021 FlorianSG

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include "NDSCart_IRManager.h"

#ifdef IR_DEBUG
#include <stdio.h>

void printbuf(u8* buffer, u8* length) {
    printf("[");
    for (unsigned int i = 0; i < *length; ++i) {
        printf("%i", buffer[i]);
        if (i < *length - 1) printf(", ");
    }
    printf("]");
}
#endif

namespace NDSCart_IRManager
{

N3T1R::IRCommunicationHandler irch;

bool Init()
{
    #ifdef IR_DEBUG
    printf("NDSCart_IRManager::Init()\n");
    fflush(stdout);
    #endif

    return true;
}

void DeInit()
{
    #ifdef IR_DEBUG
    printf("NDSCart_IRManager::DeInit()\n");
    fflush(stdout);
    #endif
}

void Reset()
{
    #ifdef IR_DEBUG
    printf("NDSCart_IRManager::Reset()\n");
    fflush(stdout);
    #endif

    printf("Listing N3T1R rooms:\n");
    std::vector<std::string> rooms = N3T1R::IRCommunicationHandler::get_available_rooms();
    for (std::vector<std::string>::iterator it = rooms.begin(); it != rooms.end(); ++it) {
        printf("\t -> %s\n", it->c_str());
    }
    fflush(stdout);

    printf("Listing N3T1R serial ports:\n");
    std::map<std::string, std::string> serial_ports = N3T1R::IRCommunicationHandler::get_available_serial_ports();
    for (std::map<std::string, std::string>::iterator it = serial_ports.begin(); it != serial_ports.end(); ++it) {
        printf("\t -> %s (%s)\n", it->first.c_str(), it->second.c_str());
    }
    fflush(stdout);

    irch.reset();

    // TO-DO: proper backend selection
    irch.select_serial_backend("COM7");
    
    try {
        irch.enable();
    }
    catch (std::runtime_error& e) {
        printf("\n\n<<<< irch.enable() error: %s >>>>\n\n\n", e.what());
    }
}

void TxBuffer(u8* buffer, u8* length) {
    #ifdef IR_DEBUG
    printf("NDSCart_IRManager::TxBuffer(");
    printbuf(buffer, length);
    printf("\n");
    fflush(stdout);
    #endif

    try {
        irch.send(buffer, (size_t) *length);
    }
    catch (std::runtime_error& e) {
        printf("\n\n<<<< irch.send() error: %s >>>>\n\n\n", e.what());
    }
}

void RxBuffer(u8* buffer, u8* length) {
    try {
        *length = irch.receive(buffer, MaximumBufferLength);
    }
    catch (std::runtime_error& e) {
        printf("\n\n<<<< irch.receive() error: %s >>>>\n\n\n", e.what());
    }

    #ifdef IR_DEBUG
    if (*length) {
        printf("NDSCart_IRManager::RxBuffer() -> ");
        printbuf(buffer, length);
        printf("\n");
        fflush(stdout);
    }
    #endif
}

}