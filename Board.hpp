#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

enum class PieceType { Empty, Pawn, Rook, Knight, Bishop, Queen, King };

class Board {
public:
    Board();
    void printStatus();

    // New function: draws the 8x8 grid to the window
    void draw(sf::RenderWindow& window);

private:
    PieceType grid[8][8];
    const float tileSize = 100.f; // Each square is 100x100 pixels
	float offset = 50.f; // Offset for centering the board in the window
    const float setPosition = 100.f;
};