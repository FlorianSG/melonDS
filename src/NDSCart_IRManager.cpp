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

#include <stdio.h>
#include <string.h>
#include "NDSCart_IRManager.h"

namespace NDSCart_IRManager
{

bool txDone = false;

void printbuff(u8* buffer, u8 len)
{
    for (int i = 0; i < len; ++i) {
        printf("0x%02x ", buffer[i]);
    }
}

void IRRxBuffer(u8* buffer, u8* len)
{
    u8 buffer_[] = {0x56};
    u8 bufferSize = 1;

    if (txDone)
        bufferSize = 0;

    printf("NDSCart_IRManager::IRRecv( ");
    printbuff(buffer_, bufferSize);
    printf(")\n");
    fflush(stdout);

    memcpy(buffer, buffer_, bufferSize);
    *len = bufferSize;
}

void IRTxBuffer(u8* buffer, u8 len)
{
    printf("NDSCart_IRManager::IRSend( ");
    printbuff(buffer, len);
    printf(")\n");
    fflush(stdout);
    txDone = true;
}

}