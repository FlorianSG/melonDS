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
#include <chrono>
#include <atomic>
#include "NDSCart_IRManager.h"
#include "Platform.h"
#include "serialib/serialib.h"

namespace NDSCart_IRManager
{

u8 PendingTxBuffer[MaximumBufferLength];
u8 PendingRxBuffer[MaximumBufferLength];

u8 PendingTxBufferLength;
u8 PendingRxBufferLength;

Platform::Mutex* PendingTxBufferLock;
Platform::Mutex* PendingRxBufferLock;

Platform::Thread* IOThread;
std::atomic_bool IOThreadRunning;

u8 IOThread_TxBuffer[MaximumBufferLength];
u8 IOThread_RxBuffer[MaximumBufferLength];

u8 IOThread_TxBufferLength;
u8 IOThread_RxBufferLength;

s64 IOThread_LastRxTimestamp;

serialib SerialConnection;
bool SerialConnectionEstablished;

bool Init()
{
    printf("NDSCart_IRManager::Init()\n");
    fflush(stdout);

    PendingTxBufferLength = 0;
    PendingRxBufferLength = 0;

    PendingTxBufferLock = Platform::Mutex_Create();
    PendingRxBufferLock = Platform::Mutex_Create();

    int res = SerialConnection.openDevice("COM4", 115200);
    if (res == 1)
    {
        SerialConnectionEstablished = true;
    }
    else
    {
        printf("Error while openning serial device 'COM4': ");
        switch (res)
        {
            case -1:
                printf("device not found !\n");
                break;
            case -2:
                printf("error while opening the device !\n");
                break;
            case -3:
                printf("error while getting port parameters !\n");
                break;
            case -4:
                printf("speed (bauds) not recognized !\n");
                break;
            case -5:
                printf("error while writing port parameters !\n");
                break;
            case -6:
                printf("error while writing timeout parameters !\n");
                break;
        }
        fflush(stdout);

        SerialConnectionEstablished = false;
    }

    return true;
}

void DeInit()
{
    printf("NDSCart_IRManager::DeInit()\n");
    fflush(stdout);

    if (IOThreadRunning)
    {
        IOThreadRunning = false;
        Platform::Thread_Wait(IOThread);
        Platform::Thread_Free(IOThread);
    }

    if (SerialConnectionEstablished)
    {
        printf("Closing serial connection...\n");
        SerialConnection.closeDevice();
    }

    Platform::Mutex_Free(PendingTxBufferLock);
    Platform::Mutex_Free(PendingRxBufferLock);
}

void Setup()
{
    printf("NDSCart_IRManager::Setup()\n");
    fflush(stdout);

    IOThreadRunning = true;
    IOThread = Platform::Thread_Create(IOThread_Main);
}

void SwapBuffer(u8* src_buffer, u8* src_length, u8* dst_buffer, u8* dst_length)
{
    if (*src_length)
    {
        memcpy(dst_buffer, src_buffer, *src_length);
        *dst_length = *src_length;
        *src_length = 0;
    }
}

void TxBuffer(u8* buffer, u8* length) {
    Platform::Mutex_Lock(PendingTxBufferLock);
    SwapBuffer(buffer, length, PendingTxBuffer, &PendingTxBufferLength);
    Platform::Mutex_Unlock(PendingTxBufferLock);
}

void RxBuffer(u8* buffer, u8* length) {
    Platform::Mutex_Lock(PendingRxBufferLock);
    SwapBuffer(PendingRxBuffer, &PendingRxBufferLength, buffer, length);
    Platform::Mutex_Unlock(PendingRxBufferLock);
}

void IOThread_Main()
{
    for (;;)
    {
        if (!IOThreadRunning)
            return;
        
        IOThread_SendBuffer();
        IOThread_RecvBuffer();

        Platform::Sleep(250); // 250 µs
    }
}

// For devel/debug only
void printbuff(u8* buffer, u8 len)
{
    for (int i = 0; i < len; ++i) {
        printf("0x%02x ", buffer[i]);
    }
}

void IOThread_SendBuffer()
{

    Platform::Mutex_Lock(PendingTxBufferLock);
    SwapBuffer(PendingTxBuffer, &PendingTxBufferLength, IOThread_TxBuffer, &IOThread_TxBufferLength);
    Platform::Mutex_Unlock(PendingTxBufferLock);

    if (IOThread_TxBufferLength)
    {
        printf("NDSCart_IRManager::IOThread_SendBuffer( ");
        printbuff(IOThread_TxBuffer, IOThread_TxBufferLength);
        printf(")\n\n");
        fflush(stdout);

        if (SerialConnectionEstablished)
        {
            SerialConnection.writeBytes(IOThread_TxBuffer, IOThread_TxBufferLength);
        }

        IOThread_TxBufferLength = 0;
    }
}

s64 IOThread_getMsTimestamp()
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

void IOThread_RecvBuffer()
{
    u8 buffer[MaximumBufferLength];
    u8 buffer_length = 0;

    if (SerialConnectionEstablished)
    {
        if (SerialConnection.available())
        {
            IOThread_LastRxTimestamp = IOThread_getMsTimestamp();
            buffer_length = (u8) SerialConnection.readBytes(buffer, MaximumBufferLength, 1);

            if (IOThread_RxBufferLength + buffer_length <= MaximumBufferLength)
            {
                memcpy((IOThread_RxBuffer + IOThread_RxBufferLength), buffer, buffer_length);
                IOThread_RxBufferLength += buffer_length;
            }
            else
            {
                printf("NDSCart_IRManager::IOThread_RecvBuffer() => Oooops, too much data: %i bytes !\n", IOThread_RxBufferLength + buffer_length);
            }
        }
        else if (IOThread_getMsTimestamp() - IOThread_LastRxTimestamp > InterPacketTimeoutMs && IOThread_RxBufferLength)
        {
            printf("NDSCart_IRManager::IOThread_RecvBuffer( ");
            printbuff(IOThread_RxBuffer, IOThread_RxBufferLength);
            printf(")\n\n");
            fflush(stdout);

            Platform::Mutex_Lock(PendingRxBufferLock);
            SwapBuffer(IOThread_RxBuffer, &IOThread_RxBufferLength, PendingRxBuffer, &PendingRxBufferLength);
            Platform::Mutex_Unlock(PendingRxBufferLock);
        }
    }
}

}