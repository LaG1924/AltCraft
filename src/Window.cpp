#include "Window.hpp"

void Window::MakeClick(short ClickedSlot, bool Lmb, bool dropMode) {
    if (!dropMode) {
        PacketClickWindow packet(WindowId, ClickedSlot, Lmb ? 0 : 1, actions++, 0, slots[ClickedSlot]);
        this->pendingTransactions.push(packet);
        std::swap(slots[ClickedSlot], handSlot);
        transactions.emplace_back(std::make_pair(actions, std::make_pair(ClickedSlot, -1)));
    } else {
        PacketClickWindow packet(WindowId, ClickedSlot, Lmb ? 0 : 1, actions++, 0, SlotDataType());
        this->pendingTransactions.push(packet);
        transactions.emplace_back(std::make_pair(actions, std::make_pair(ClickedSlot, -1)));
    }
}

void Window::ConfirmTransaction(PacketConfirmTransactionCB packet) {
    if (!packet.Accepted) {
        throw std::logic_error("Transaction failed");
    }
    /*auto toDelete = transactions.begin();
    for (auto it = transactions.begin(); it != transactions.end(); ++it) {
        if (it->first == packet.ActionNumber) {
            toDelete = it;
            if (!packet.Accepted) {
                std::swap(slots[std::get<0>(it->second)], slots[std::get<1>(it->second)]);
            }
            break;
        }
    }
    if (toDelete->first == packet.ActionNumber)
        transactions.erase(toDelete);*/
}