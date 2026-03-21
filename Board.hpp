#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>

struct PieceSource {
    int col;
    int row;
};

// Pieces types and colors
enum class PieceType {
    Empty,
    W_Pawn, W_Rook, W_Knight, W_Bishop, W_Queen, W_King,
    B_Pawn, B_Rook, B_Knight, B_Bishop, B_Queen, B_King
};

class Board {
public:
    Board();
    void printStatus();
    void draw(sf::RenderWindow& window);

    void handleMouseClick(const sf::Vector2i mousePos);
    bool isMoveValid(int startRow, int startCol, int endRow, int endCol);

private:
    bool whiteTurn = true; // True for White's turn, False for Black's
    PieceType grid[8][8];
    const float tileSize = 100.f;
    float offset = 50.f;
    std::map<PieceType, PieceSource> pieceSourceMap;
    void setupPieceSources();

	// SFML visual assets
    sf::Texture piecesTexture;
    sf::Sprite pieceSprite;
	sf::Vector2i selectedSquare = sf::Vector2i(-1, -1); // No square selected

    void loadAssets(); // to load asset

    bool isWhite(PieceType type); // Add this helper function here
};