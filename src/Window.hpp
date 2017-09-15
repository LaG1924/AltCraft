#pragma once

#include <string>
#include <queue>

#include "Packet.hpp"

struct Window {
    unsigned char WindowId = 0;
    std::string type;

    SlotData handSlot;
    std::vector<SlotData> slots;

    short actions = 1;

    void MakeClick(short ClickedSlot, bool Lmb);

    std::queue<PacketClickWindow> pendingTransactions;
};