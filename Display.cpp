#include <iostream>
#include "Display.hpp"

Display::Display(int w, int h, std::string title, World *world, std::condition_variable &gameStartWaiter)
        : gameStartWaiter(gameStartWaiter) {
    window = new sf::RenderWindow(sf::VideoMode(w, h), title);
    window->clear(sf::Color::Black);
    window->display();
    this->world = world;
}

Display::~Display() {
    delete window;
}

void Display::Update() {
    pollEvents();

    {
        std::chrono::steady_clock clock;
        static auto timeOfPreviousUpdate(clock.now());
        std::chrono::duration<double> delta = clock.now() - timeOfPreviousUpdate;
        if (delta.count() > 0.5) {
            window->setTitle(
                    std::string("Render layer: " + std::to_string(renderLayer) + "\t" +
                                //" BlockID: " + std::to_string(currentId) +
                                " Mouse pos" + std::to_string(mousePos.x) + "  " + std::to_string(mousePos.y) +
                                " FPS: " + std::to_string(1 / frameTime)));
            timeOfPreviousUpdate = clock.now();
        }
    }

    window->clear(sf::Color::Black);
    if (isGameStarted)
        renderWorld();
    window->display();
}

void Display::renderWorld() {
    //currentId = 0;
    for (auto sectionIt = world->m_sections.begin(); sectionIt != world->m_sections.end(); ++sectionIt) {
        if (sectionIt->first.GetY() != renderLayer / 16)
            continue;
        Section &section = sectionIt->second;
        sf::Texture &texture = GetSectionTexture(sectionIt->first);
        sf::Sprite sprite(texture);
        sprite.setPosition(sectionIt->first.GetX() * 16, sectionIt->first.GetZ() * 16);
        window->draw(sprite);
        //                                                                                                                                              sf::Texture &texture = GetSectionTexture(sectionIt->first);
        /*for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                int y = renderLayer - sectionIt->first.GetY() * 16;
                int absoluteX = sectionIt->first.GetX() * 16 + x;
                int absoluteZ = sectionIt->first.GetZ() * 16 + z;


                Block &block = section.GetBlock(PositionI(x, z, y));
                sf::RectangleShape shape(sf::Vector2f(1, 1));

                shape.setPosition(absoluteX, absoluteZ);
                shape.setFillColor(sf::Color::Magenta);
                if (mousePos.x > shape.getPosition().x && mousePos.y > shape.getPosition().y) {
                    if (mousePos.x < shape.getPosition().x + 1 && mousePos.y < shape.getPosition().y + 1) {
                        currentId = block.id;
                        if (isClicked) {
                            std::cout << "Clicked it " << absoluteX << " " << absoluteZ << std::endl;
                            isClicked = false;
                        }
                    }
                }
                switch (block.id) {
                    case 0:
                        shape.setFillColor(sf::Color::Transparent);
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
                        shape.setFillColor(sf::Color(139, 69, 69));
                        break;
                    case 13:
                        shape.setFillColor(sf::Color(220, 220, 220));
                        break;
                    case 9:
                        shape.setFillColor(sf::Color::Blue);
                        break;
                    case 8:
                        shape.setFillColor(sf::Color::Blue);
                        break;
                    case 2:
                        shape.setFillColor(sf::Color::Green);
                        break;
                    default:
                        //std::cout << "Unknown id is " << sectionIt.second.GetId() << std::endl;
                        break;
                }
                sf::Color darkness(0, 0, 0, ((15 - block.light) / 15.0f) * 255);
                shape.setFillColor(shape.getFillColor() + darkness);
                window->draw(shape);
            }
        }
        sf::Vector2f p1 = sf::Vector2f(sectionIt->first.GetX() * 16, sectionIt->first.GetZ() * 16);
        sf::Vector2f p2 = sf::Vector2f(sectionIt->first.GetX() * 16 + 16, sectionIt->first.GetZ() * 16);
        sf::Vector2f p3 = sf::Vector2f(sectionIt->first.GetX() * 16 + 16, sectionIt->first.GetZ() * 16 + 16);
        sf::Vector2f p4 = sf::Vector2f(sectionIt->first.GetX() * 16, sectionIt->first.GetZ() * 16 + 16);
        sf::Vertex line1[] = {
                sf::Vertex(p1),
                sf::Vertex(p2),
        };
        sf::Vertex line2[] = {
                sf::Vertex(p2),
                sf::Vertex(p3),
        };
        sf::Vertex line3[] = {
                sf::Vertex(p3),
                sf::Vertex(p4),
        };
        sf::Vertex line4[] = {
                sf::Vertex(p4),
                sf::Vertex(p1),
        };
        window->draw(line1, 2, sf::Lines);
        window->draw(line2, 2, sf::Lines);
        window->draw(line3, 2, sf::Lines);
        window->draw(line4, 2, sf::Lines);*/
    }
}

void Display::pollEvents() {
    sf::Event e;
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
                    if (renderLayer > 0)
                        renderLayer--;
                } else if (e.key.code == sf::Keyboard::X) {
                    if (renderLayer < 256)
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
                break;
            case sf::Event::MouseButtonPressed:
                isClicked = true;
                break;
        }
    }
}


bool Display::IsClosed() {
    return !window->isOpen();
}

void Display::SetPlayerPos(float x, float z) {
    x = -55;
    z = 196;
    isGameStarted = true;
    float div = 5;
    float X = window->getSize().x / div, Z = window->getSize().y / div;
    sf::View view(sf::Vector2f(x, z), sf::Vector2f(X, Z));
    window->setView(view);
}

void Display::MainLoop() {
    /*std::unique_lock<std::mutex> gameStartLocker(gameStartMutex);
    gameStartWaiter.wait(gameStartLocker);
    while (!isGameStarted) {
        std::cout << "Catch spirious wakeup" << std::endl;
        gameStartWaiter.wait(gameStartLocker);
    }
    std::cout << "Graphics subsystem initialized" << std::endl;*/
    while (!IsClosed()) {
        Update();
        std::chrono::steady_clock clock;
        static auto timeOfPreviousUpdate(clock.now());
        std::chrono::duration<double> delta = clock.now() - timeOfPreviousUpdate;
        timeOfPreviousUpdate = clock.now();
        frameTime = delta.count();
    }
}

sf::Texture &Display::GetSectionTexture(PositionI pos) {
    if (sectionTextures.find(pos) != sectionTextures.end() &&
        sectionTextures[pos][renderLayer - pos.GetY() * 16].getSize() != sf::Vector2u(0, 0))
        return sectionTextures[pos][renderLayer - pos.GetY() * 16];

    auto sectionIt = world->m_sections.find(pos);
    Section &section = sectionIt->second;
    sf::Image image;
    image.create(16, 16);
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int y = renderLayer - sectionIt->first.GetY() * 16;
            sf::Color color = sf::Color::Magenta;
            switch (section.GetBlock(PositionI(x, z, y)).id) {
                case 0:
                    color = sf::Color::Transparent;
                    break;
                case 7:
                    color = sf::Color::Yellow;
                    break;
                case 1:
                    color = sf::Color::White;
                    break;
                case 11:
                    color = sf::Color::Red;
                    break;
                case 10:
                    color = sf::Color::Red;
                    break;
                case 3:
                    color = sf::Color(139, 69, 69);
                    break;
                case 13:
                    color = sf::Color(220, 220, 220);
                    break;
                case 9:
                    color = sf::Color::Blue;
                    break;
                case 8:
                    color = sf::Color::Blue;
                    break;
                case 2:
                    color = sf::Color::Green;
                    break;
                default:
                    break;
            }
            image.setPixel(x, z, color);
        }
    }
    /*for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            std::cout << std::hex << (int)pixels[i * 256 + j * 16] << (int)pixels[i * 256 + j * 16 + 1]
                      << (int)pixels[i * 256 + j * 16 + 2] <<(int) pixels[i * 256 + j * 16 + 3] << " ";
        }
        std::cout<<std::endl;
    }*/
    sf::Texture texture;
    texture.create(16, 16);
    texture.update(image);
    sectionTextures[pos][renderLayer - pos.GetY() * 16] = texture;
    return sectionTextures[pos][renderLayer - pos.GetY() * 16];
}