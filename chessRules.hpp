#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

enum class PieceType {
    Empty,
    W_Pawn, W_Rook, W_Knight, W_Bishop, W_Queen, W_King,
    B_Pawn, B_Rook, B_Knight, B_Bishop, B_Queen, B_King
};

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

    // pawn promotion state
    PieceType promotedTo = PieceType::Empty;
};

class ChessRules {
public:
    ChessRules();

    // Logic members moved from Board
    PieceType grid[8][8];
    bool whiteTurn;
    bool gameOver;
    std::string resultText;
    sf::Vector2i lastPawnDoubleMove;
    bool whiteKingMoved, whiteRook0Moved, whiteRook7Moved;
    bool blackKingMoved, blackRook0Moved, blackRook7Moved;
    std::vector<MoveRecord> moveHistory;
    int currentMoveIndex;
    MoveRecord pendingPromotionMove;

    // Logic functions
    bool isWhite(PieceType type);
    bool isMoveValid(int startRow, int startCol, int endRow, int endCol);
    bool isSquareAttacked(int row, int col, bool attackerIsWhite);
    bool needsDisambiguation(int startRow, int startCol, int endRow, int endCol, PieceType type);
    sf::Vector2i findKing(bool white);
    bool hasLegalMoves(bool white);
    void checkGameEnd();
    void resetBoardToStart();
};