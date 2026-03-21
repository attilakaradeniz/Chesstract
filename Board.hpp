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
    sf::Vector2i lastPawnDoubleMove = sf::Vector2i(-1, -1); // Stores the destination of the last 2-square pawn move
    bool whiteKingMoved = false;
    bool whiteRook0Moved = false; // white left rook (a1)
    bool whiteRook7Moved = false; // black right rook (h1)

    bool blackKingMoved = false;
    bool blackRook0Moved = false; // black left rook (a8)
    bool blackRook7Moved = false; // black right rook (h8)


	// SFML visual assets
    sf::Texture piecesTexture;
    sf::Sprite pieceSprite;
	sf::Vector2i selectedSquare = sf::Vector2i(-1, -1); // No square selected

    void loadAssets(); // to load asset

    bool isWhite(PieceType type); // Add this helper function here
};