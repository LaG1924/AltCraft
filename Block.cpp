#include "Block.hpp"


Block::Block(int id, unsigned char state, unsigned char light, PositionI position) : m_id(id), m_position(position),
                                                                                     m_light(light), m_state(state) {

}

Block::~Block() {

}

int Block::GetId() {
    return m_id;
}

int Block::GetState() {
    return m_state;
}

int Block::GetLight() {
    return m_light;
}

Block::Block(unsigned short idAndState, unsigned char light) {
    m_id = idAndState >> 4;
    m_state = idAndState & 0b00001111;
    m_light = light;
}
