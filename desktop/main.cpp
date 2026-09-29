#include "chess_engine.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <optional>

int main() {
    (void)chesslab::engine_version();

    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "ChessLab");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color(25, 25, 30));

        const float square_size = 70.0f;

        for (int rank = 0; rank < 8; ++rank) {
            for (int file = 0; file < 8; ++file) {
                sf::RectangleShape square(sf::Vector2f(square_size, square_size));
                square.setPosition({static_cast<float>(file) * square_size,
                                    static_cast<float>(rank) * square_size});
                square.setFillColor((file + rank) % 2 == 0 ? sf::Color(240, 217, 181)
                                                         : sf::Color(181, 136, 99));
                window.draw(square);
            }
        }

        window.display();
    }

    return 0;
}
