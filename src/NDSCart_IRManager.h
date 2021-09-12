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

#ifndef NDSCART_IRMANAGER_H
#define NDSCART_IRMANAGER_H

#include "types.h"

namespace NDSCart_IRManager
{
    const u8 MaximumBufferLength = 0xB8;
    const s64 InterPacketTimeoutMs = 20;

    bool Init();
    void DeInit();
    void Setup();

    void TxBuffer(u8* buffer, u8* length);
    void RxBuffer(u8* buffer, u8* length);

    void IOThread_Main();
    void IOThread_SendBuffer();
    void IOThread_RecvBuffer();
}

#endif // NDSCART_IRMANAGER_H