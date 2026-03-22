#include <iostream>
#include <SFML/Graphics.hpp>
#include "Board.hpp"

/**
 * Chesstract - A simple chess application with PGN export functionality.
 * Purpose: Learning C++ through game development and OOP principles.
 */

int main() {
    // Basic terminal output for project initialization
    std::cout << "================================" << std::endl;
    std::cout << "   Chesstract Project Started   " << std::endl;
    std::cout << "================================" << std::endl;

    // Check the current C++ standard version being used
    std::cout << "C++ Standard Version: " << __cplusplus << std::endl;
    std::cout << "\nInitializing chess board..." << std::endl;

    sf::RenderWindow window(sf::VideoMode(1200, 900), "Chesstract - board render");
    Board myBoard;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // 1. closing the  window
            if (event.type == sf::Event::Closed)
                window.close();

            // 2. Keyboard enter (arrows etc)
            if (event.type == sf::Event::KeyPressed) {
                myBoard.handleKeyPress(event.key.code);
            }

            // --- DRAG & DROP  ---

            // A. mouse movement (piece sticks to mouse pointer)
            if (event.type == sf::Event::MouseMoved) {
                // SFML 
                sf::Vector2f mPos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
                myBoard.updateMousePos(mPos);
            }

            // --- DRAG & DROP AND CLICK LOGIC ---

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    // This selects the piece and starts the drag
                    myBoard.handleMouseDown(mPos);
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                    // 1. First, tell the board the dragging has stopped visually
                    myBoard.handleMouseUp(mPos);

                    // 2. Second, process the click. 
                    // If the user dragged to a new square, handleMouseClick will see 
                    // that a square was already selected (from MouseDown) and move it there.
                    myBoard.handleMouseClick(sf::Vector2i(static_cast<int>(mPos.x), static_cast<int>(mPos.y)));
                }
            }
        }

        window.clear(sf::Color(60, 60, 60));
        myBoard.draw(window);
        window.display();
    }

    return 0;
}