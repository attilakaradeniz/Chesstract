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

    //Board myBoard;

    // myBoard.printStatus();


    // Wait for user input before closing the console
    // std::cin.get();

    // SFML window object 
    sf::RenderWindow window(sf::VideoMode(1200, 900), "Chesstract - board render");

	Board myBoard;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    myBoard.handleMouseClick(mousePos);
                }
            }

            // 2. Keyboard Input Handling (NEW)
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::P) {
                    myBoard.exportPGN(); // Trigger PGN export to console
                }
                if (event.key.code == sf::Keyboard::U || event.key.code == sf::Keyboard::Left) {
                    myBoard.undoMove(); // Revert the last move
                }
                if (event.key.code == sf::Keyboard::F) {
                    myBoard.flipBoard(); // Flip the board orientation
				}
                if (event.key.code == sf::Keyboard::C) {
                    myBoard.toggleCoordinates();
                }
                // Press 'S' to save PGN to a File
                if (event.key.code == sf::Keyboard::S) {
                    myBoard.savePGNToFile();
                }
            }

            if (event.type == sf::Event::Closed)
                window.close();
        }
		window.clear(sf::Color(60, 60, 60)); // Dark gray background
        //window.clear(sf::Color::Blue);
        //window.clear();
        // Here comes chess board and pieces
		myBoard.draw(window);
        
        window.display();
	}

    return 0;
}