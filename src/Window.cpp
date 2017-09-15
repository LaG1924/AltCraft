#include "Window.hpp"

void Window::MakeClick(short ClickedSlot, bool Lmb) {
    PacketClickWindow packet(WindowId, ClickedSlot, Lmb? 0 : 1, actions++, 0, slots[ClickedSlot]);
    this->pendingTransactions.push(packet);
}