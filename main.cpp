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

    Board myBoard;

    myBoard.printStatus();


    // Wait for user input before closing the console
    // std::cin.get();

    // SFML window object 
    sf::RenderWindow window(sf::VideoMode(800, 600), "Chesstract - visual test");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color::Blue);
        // Here comes chess board and pieces
        window.display();
	}

    return 0;
}