// Board.cpp
#include "Board.hpp"
#include <iostream>

Board::Board() {
	loadAssets();
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			grid[i][j] = PieceType::Empty;
		}
	}

	// std::cout << "Chess board logic initialized." << std::endl;
	// setup pawns
	for (int i = 0; i < 8; ++i) {
		grid[1][i] = PieceType::W_Pawn; // white pawns
		grid[6][i] = PieceType::B_Pawn; // black pawns

	}

	// setup rooks, knights, bishops
	grid[0][0] = grid[0][7] = PieceType::W_Rook;
	grid[7][0] = grid[7][7] = PieceType::B_Rook;

	grid[0][1] = grid[0][6] = PieceType::W_Knight;
	grid[7][1] = grid[7][6] = PieceType::B_Knight;

	grid[0][2] = grid[0][5] = PieceType::W_Bishop;
	grid[7][2] = grid[7][5] = PieceType::B_Bishop;

	grid[0][3] = PieceType::W_Queen; grid[0][4] = PieceType::W_King;
	grid[7][3] = PieceType::B_Queen; grid[7][4] = PieceType::B_King;

	std::cout << "Chess board logic initialized (starting position)." << std::endl;

}

void Board::printStatus() {
	std::cout << "Board is ready for piecec." << std::endl;
}

void Board::draw(sf::RenderWindow& window) {
	const int sourceSize = 45; // original size of pieces in the texture
    const float scale = tileSize / (float)sourceSize;
    pieceSprite.setScale(scale, scale);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            // draw 1st square
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(j * tileSize + offset, i * tileSize + offset);
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(240, 217, 181) : sf::Color(181, 136, 99));
            window.draw(square);

			// highlight selected square
            if (selectedSquare != sf::Vector2i(-1, -1)) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(selectedSquare.x * tileSize + offset, selectedSquare.y * tileSize + offset);
				highlight.setFillColor(sf::Color(255, 255, 0, 05)); // semi opaque yellow
                window.draw(highlight);
            }


            // 2. piece drawing (via Lookup Table )
            PieceType type = grid[i][j];
            if (type != PieceType::Empty) {
                PieceSource src = pieceSourceMap[type];

                pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));
                pieceSprite.setPosition(j * tileSize + offset, i * tileSize + offset);

                window.draw(pieceSprite);
            }
        }
    }
}

void Board::setupPieceSources() {
    pieceSourceMap[PieceType::W_King] = { 0, 0 };
    pieceSourceMap[PieceType::W_Queen] = { 1, 0 };
    pieceSourceMap[PieceType::W_Bishop] = { 2, 0 };
    pieceSourceMap[PieceType::W_Knight] = { 3, 0 };
    pieceSourceMap[PieceType::W_Rook] = { 4, 0 };
    pieceSourceMap[PieceType::W_Pawn] = { 5, 0 };

    pieceSourceMap[PieceType::B_King] = { 0, 1 };
    pieceSourceMap[PieceType::B_Queen] = { 1, 1 };
    pieceSourceMap[PieceType::B_Bishop] = { 2, 1 };
    pieceSourceMap[PieceType::B_Knight] = { 3, 1 };
    pieceSourceMap[PieceType::B_Rook] = { 4, 1 };
    pieceSourceMap[PieceType::B_Pawn] = { 5, 1 };
}
void Board::loadAssets() {
	// loading pieces.png file
    if (!piecesTexture.loadFromFile("assets/pieces.png")) {
        std::cerr << "ERROR: assets/pieces.png couldnt load! File path needs to be checked." << std::endl;
    }
	setupPieceSources();
	// setting texture to sprite for later use
    pieceSprite.setTexture(piecesTexture);
}

void Board::handleMouseClick(sf::Vector2i mousePos) {
    // from offset to square size
    int col = (mousePos.x - (int)offset) / (int)tileSize;
    int row = (mousePos.y - (int)offset) / (int)tileSize;

    // Tıklanan yer tahta sınırları içinde mi?
    if (col >= 0 && col < 8 && row >= 0 && row < 8) {
        selectedSquare = sf::Vector2i(col, row);
        std::cout << "clicked square: " << row << ", " << col << std::endl;

        // if there is a piece print its type (test)
        if (grid[row][col] != PieceType::Empty) {
            std::cout << "There is a piece here!" << std::endl;
        }
    }
    else {
		selectedSquare = sf::Vector2i(-1, -1); // unselect if clicked outside
    }
}