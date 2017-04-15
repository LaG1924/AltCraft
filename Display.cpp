#include <iostream>
#include "Display.hpp"

Display::Display(int w, int h, std::string title) {
    window = new sf::RenderWindow(sf::VideoMode(w, h), title);
}

Display::~Display() {
    delete window;
}

void Display::SetWorld(World *wrd) {
    world = wrd;
}

void Display::Update() {
    sf::Event e;
    static sf::Vector2f mousePos;
    static int renderLayer = 0, currentId = 0;
    int coeff = 10;
    while (window->pollEvent(e)) {
        switch (e.type) {
            case sf::Event::Closed:
                window->close();
                break;
            case sf::Event::MouseMoved:
                mousePos = window->mapPixelToCoords(sf::Vector2i(e.mouseMove.x, e.mouseMove.y));
                break;
            case sf::Event::KeyPressed:
                if (e.key.code == sf::Keyboard::Z) {
                    renderLayer--;
                } else if (e.key.code == sf::Keyboard::X) {
                    renderLayer++;
                } else if (e.key.code == sf::Keyboard::Up) {
                    sf::View view = window->getView();
                    view.move(0, -coeff);
                    window->setView(view);
                } else if (e.key.code == sf::Keyboard::Down) {
                    sf::View view = window->getView();
                    view.move(0, coeff);
                    window->setView(view);
                } else if (e.key.code == sf::Keyboard::Right) {
                    sf::View view = window->getView();
                    view.move(coeff, 0);
                    window->setView(view);
                } else if (e.key.code == sf::Keyboard::Left) {
                    sf::View view = window->getView();
                    view.move(-coeff, 0);
                    window->setView(view);
                } else if (e.key.code == sf::Keyboard::A) {
                    sf::View view = window->getView();
                    //view.setSize(view.getSize().x + coeff2, view.getSize().y + coeff2);
                    view.zoom(1.1);
                    window->setView(view);
                } else if (e.key.code == sf::Keyboard::S) {
                    sf::View view = window->getView();
                    view.zoom(0.9);
                    //view.setSize(view.getSize().x - coeff2, view.getSize().y - coeff2);
                    window->setView(view);
                }
        }
    }
    window->setTitle(
            std::string("Render layer: " +
                        std::to_string(renderLayer) + "\t" + " BlockID: " + std::to_string(currentId) +
                        std::string("\tMouse pos: ") + std::to_string(mousePos.x) + "  " + std::to_string(mousePos.y)));
    currentId = 0;
    window->clear(sf::Color::Green);

    if (!world->m_blocks.empty())
        for (auto it:world->m_blocks) {
            if (it.first.GetY() != renderLayer)
                continue;
            //std::cout<<it.first.GetY()<<std::endl;
            sf::RectangleShape shape(sf::Vector2f(1, 1));
            shape.setPosition(it.first.GetX(), it.first.GetZ());
            shape.setFillColor(sf::Color::Blue);
            if (mousePos.x > shape.getPosition().x && mousePos.y > shape.getPosition().y) {
                if (mousePos.x < shape.getPosition().x + 1 && mousePos.y < shape.getPosition().y + 1) {
                    currentId = it.second.GetId();
                }
            }
            switch (it.second.GetId()) {
                case 0:
                    shape.setFillColor(sf::Color::Black);
                    break;
                case 7:
                    shape.setFillColor(sf::Color::Yellow);
                    break;
                case 1:
                    shape.setFillColor(sf::Color::White);
                    break;
                case 11:
                    shape.setFillColor(sf::Color::Red);
                    break;
                case 10:
                    shape.setFillColor(sf::Color::Red);
                    break;
                case 3:
                    shape.setFillColor(sf::Color(165, 42, 42));
                    break;
                case 13:
                    shape.setFillColor(sf::Color(220, 220, 220));
                    break;
                default:
                    //std::cout << "Unknown id is " << it.second.GetId() << std::endl;
                    break;
            }
            window->draw(shape);
        }
    window->display();

}


bool Display::IsClosed() {
    return !window->isOpen();
}

void Display::SetPlayerPos(float x, float z) {
    float div = 1;
    float X = window->getSize().x / div, Z = window->getSize().y / div;
    sf::View view(sf::Vector2f(x, z), sf::Vector2f(X, Z));
    window->setView(view);
}
