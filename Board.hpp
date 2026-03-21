#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <vector>

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

// struct for record 
struct MoveRecord {
    sf::Vector2i start;
    sf::Vector2i end;
    PieceType movedPiece;
    PieceType capturedPiece;
    std::string notation;
    bool isWhiteMove;

	// states for undo functionality
    sf::Vector2i prevLastPawnDoubleMove;
    bool prevWhiteKingMoved;
    bool prevBlackKingMoved;
    bool prevWhiteRook0Moved;
    bool prevWhiteRook7Moved;
    bool prevBlackRook0Moved;
    bool prevBlackRook7Moved;
};

class Board {
public:
    Board();
    void printStatus();
    void draw(sf::RenderWindow& window);

    void handleMouseClick(const sf::Vector2i mousePos);
    bool isMoveValid(int startRow, int startCol, int endRow, int endCol);
    void undoMove(); 
    void exportPGN();
    void flipBoard(); 


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
    // Checks if a specific square (row, col) is under attack by a specific color
    bool isSquareAttacked(int row, int col, bool attackerIsWhite);
    // for the algebric notation
    char getPieceChar(PieceType type);
    // to find  if it is check
    sf::Vector2i findKing(bool white);

	// SFML visual assets
    sf::Texture piecesTexture;
    sf::Sprite pieceSprite;
	sf::Vector2i selectedSquare = sf::Vector2i(-1, -1); // No square selected

    void loadAssets(); // to load asset

    bool isWhite(PieceType type); // Add this helper function here

    bool hasLegalMoves(bool white);

    sf::Font font;
    bool gameOver = false;
    std::string resultText = "";
    void checkGameEnd();

	bool isFlowFlipped = false; // Default is white at bottom

	std::vector<MoveRecord> moveHistory; // Store move history for PGN export
    // fınction to export PGN
    
};