// Board.cpp
#include "Board.hpp"
#include <iostream>

Board::Board() {
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			grid[i][j] = PieceType::Empty;
		}
	}

	// std::cout << "Chess board logic initialized." << std::endl;
	// setup pawns
	for (int i = 0; i < 8; ++i) {
		grid[1][i] = PieceType::Pawn; // white pawns
		grid[6][i] = PieceType::Pawn; // black pawns

	}

	// setup rooks, knights, bishops
	grid[0][0] = grid[0][7] = PieceType::Rook;
	grid[7][0] = grid[7][7] = PieceType::Rook;

	grid[0][1] = grid[0][6] = PieceType::Knight;
	grid[7][1] = grid[7][6] = PieceType::Knight;

	grid[0][2] = grid[0][5] = PieceType::Bishop;
	grid[7][2] = grid[7][5] = PieceType::Bishop;

	grid[0][3] = PieceType::Queen; grid[0][4] = PieceType::King;
	grid[7][3] = PieceType::Queen; grid[7][4] = PieceType::King;

	std::cout << "Chess board logic initialized (starting position)." << std::endl;

}

void Board::printStatus() {
	std::cout << "Board is ready for piecec." << std::endl;
}

void Board::draw(sf::RenderWindow& window) {
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
			square.setPosition(j * tileSize + offset, i * tileSize + offset);
			
			if ((i + j) % 2 == 0) {
				 square.setFillColor(sf::Color(240, 217, 181)); // light color (brownish)
				 // square.setFillColor(sf::Color(238, 238, 210)); // light color (yellowish)
			} else {
				square.setFillColor(sf::Color(181, 136, 99)); // dark color (brownish)	
				// square.setFillColor(sf::Color(118, 150, 86)); // dark color (yellowish)
			}
			window.draw(square);
		}
	}
}