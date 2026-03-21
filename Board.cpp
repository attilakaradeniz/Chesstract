#include "Board.hpp"
#include <iostream>
#include <cmath> // For std::abs

Board::Board() {
    loadAssets();

    // Initialize the board with Empty squares
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            grid[i][j] = PieceType::Empty;
        }
    }

    // Setup pawns
    for (int i = 0; i < 8; ++i) {
        grid[1][i] = PieceType::B_Pawn; // Black pawns (Top)
        grid[6][i] = PieceType::W_Pawn; // White pawns (Bottom)
    }

    // Setup pieces (Rooks, Knights, Bishops)
    grid[0][0] = grid[0][7] = PieceType::B_Rook;
    grid[7][0] = grid[7][7] = PieceType::W_Rook;

    grid[0][1] = grid[0][6] = PieceType::B_Knight;
    grid[7][1] = grid[7][6] = PieceType::W_Knight;

    grid[0][2] = grid[0][5] = PieceType::B_Bishop;
    grid[7][2] = grid[7][5] = PieceType::W_Bishop;

    // Setup Kings and Queens
    grid[0][3] = PieceType::B_Queen; grid[0][4] = PieceType::B_King;
    grid[7][3] = PieceType::W_Queen; grid[7][4] = PieceType::W_King;

    std::cout << "Chess board logic initialized (starting position)." << std::endl;
}

void Board::loadAssets() {
    // Load texture from assets
    if (!piecesTexture.loadFromFile("assets/pieces.png")) {
        std::cerr << "Error: Could not load assets/pieces.png!" << std::endl;
    }
    piecesTexture.setSmooth(true);
    pieceSprite.setTexture(piecesTexture);
    setupPieceSources();
}

void Board::setupPieceSources() {
    // Mapping piece types to sprite sheet coordinates
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

bool Board::isMoveValid(int startRow, int startCol, int endRow, int endCol) {
    PieceType movingPiece = grid[startRow][startCol];
    if (startRow == endRow && startCol == endCol) return false;

    int rowDiff = std::abs(startRow - endRow);
    int colDiff = std::abs(startCol - endCol);

    switch (movingPiece) {
    case PieceType::W_Knight:
    case PieceType::B_Knight:
        // Knight moves in L-shape
        return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);

    case PieceType::W_Rook:
    case PieceType::B_Rook:
        if (startRow != endRow && startCol != endCol) return false;
        goto line_check;

    case PieceType::W_Bishop:
    case PieceType::B_Bishop:
        if (rowDiff != colDiff) return false;
        goto line_check;

    case PieceType::W_Queen:
    case PieceType::B_Queen:
        if (rowDiff != colDiff && (startRow != endRow && startCol != endCol)) return false;
        goto line_check;

    case PieceType::W_King:
    case PieceType::B_King:
        // Basic 1 square movement
        if (rowDiff <= 1 && colDiff <= 1) return true;

        // Castling logic
        if (rowDiff == 0 && colDiff == 2) {
            bool isWhitePiece = isWhite(movingPiece);
            if (isWhitePiece && whiteKingMoved) return false;
            if (!isWhitePiece && blackKingMoved) return false;

            if (endCol == 6) { // Kingside Castling
                bool rookMoved = isWhitePiece ? whiteRook7Moved : blackRook7Moved;
                if (rookMoved || grid[startRow][5] != PieceType::Empty || grid[startRow][6] != PieceType::Empty) return false;
                return true;
            }
            if (endCol == 2) { // Queenside Castling
                bool rookMoved = isWhitePiece ? whiteRook0Moved : blackRook0Moved;
                if (rookMoved || grid[startRow][1] != PieceType::Empty || grid[startRow][2] != PieceType::Empty || grid[startRow][3] != PieceType::Empty) return false;
                return true;
            }
        }
        return false;

    case PieceType::W_Pawn:
    case PieceType::B_Pawn: {
        int dir = (movingPiece == PieceType::W_Pawn) ? -1 : 1;
        int startPawnRow = (movingPiece == PieceType::W_Pawn) ? 6 : 1;

        // Move forward 1 square
        if (colDiff == 0 && (endRow - startRow) == dir && grid[endRow][endCol] == PieceType::Empty) return true;

        // Initial double move
        if (colDiff == 0 && startRow == startPawnRow && (endRow - startRow) == 2 * dir) {
            if (grid[startRow + dir][startCol] == PieceType::Empty && grid[endRow][endCol] == PieceType::Empty) return true;
        }

        // Standard capture
        if (colDiff == 1 && (endRow - startRow) == dir && grid[endRow][endCol] != PieceType::Empty) return true;

        // En Passant capture
        if (colDiff == 1 && (endRow - startRow) == dir && grid[endRow][endCol] == PieceType::Empty) {
            if (lastPawnDoubleMove == sf::Vector2i(endCol, startRow)) return true;
        }
        return false;
    }
    default: return false;
    }

line_check:
    // Sliding pieces path obstruction check
    int rStep = (endRow == startRow) ? 0 : (endRow > startRow ? 1 : -1);
    int cStep = (endCol == startCol) ? 0 : (endCol > startCol ? 1 : -1);
    int currR = startRow + rStep;
    int currC = startCol + cStep;
    while (currR != endRow || currC != endCol) {
        if (grid[currR][currC] != PieceType::Empty) return false;
        currR += rStep; currC += cStep;
    }
    return true;
}

void Board::handleMouseClick(sf::Vector2i mousePos) {
    int col = (mousePos.x - (int)offset) / (int)tileSize;
    int row = (mousePos.y - (int)offset) / (int)tileSize;

    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    if (selectedSquare != sf::Vector2i(-1, -1)) {
        PieceType movingPiece = grid[selectedSquare.y][selectedSquare.x];
        PieceType targetPiece = grid[row][col];

        if (isMoveValid(selectedSquare.y, selectedSquare.x, row, col)) {
            // Friendly fire check: switch selection instead of moving
            if (targetPiece != PieceType::Empty && isWhite(movingPiece) == isWhite(targetPiece)) {
                selectedSquare = sf::Vector2i(col, row);
                return;
            }

            // En Passant: remove the captured pawn
            if ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && col != selectedSquare.x && targetPiece == PieceType::Empty) {
                grid[selectedSquare.y][col] = PieceType::Empty;
            }

            // Castling: jump the rook over the king
            if ((movingPiece == PieceType::W_King || movingPiece == PieceType::B_King) && std::abs(col - selectedSquare.x) == 2) {
                if (col == 6) { // Kingside
                    grid[row][5] = grid[row][7]; grid[row][7] = PieceType::Empty;
                }
                else { // Queenside
                    grid[row][3] = grid[row][0]; grid[row][0] = PieceType::Empty;
                }
            }

            // Update movement flags for Castling
            if (movingPiece == PieceType::W_King) whiteKingMoved = true;
            if (movingPiece == PieceType::B_King) blackKingMoved = true;
            if (selectedSquare == sf::Vector2i(0, 0)) blackRook0Moved = true;
            if (selectedSquare == sf::Vector2i(7, 0)) blackRook7Moved = true;
            if (selectedSquare == sf::Vector2i(0, 7)) whiteRook0Moved = true;
            if (selectedSquare == sf::Vector2i(7, 7)) whiteRook7Moved = true;

            // Update En Passant memory for the next turn
            if ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && std::abs(row - selectedSquare.y) == 2)
                lastPawnDoubleMove = sf::Vector2i(col, row);
            else
                lastPawnDoubleMove = sf::Vector2i(-1, -1);

            // Execute move and switch turns
            grid[row][col] = movingPiece;
            grid[selectedSquare.y][selectedSquare.x] = PieceType::Empty;
            whiteTurn = !whiteTurn;
        }
        selectedSquare = sf::Vector2i(-1, -1);
    }
    else {
        // Selection phase
        PieceType clickedPiece = grid[row][col];
        if (clickedPiece != PieceType::Empty && isWhite(clickedPiece) == whiteTurn) {
            selectedSquare = sf::Vector2i(col, row);
        }
    }
}

void Board::draw(sf::RenderWindow& window) {
    const int sourceSize = 45;
    const float scale = tileSize / (float)sourceSize;
    pieceSprite.setScale(scale, scale);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            // Draw square
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(j * tileSize + offset, i * tileSize + offset);
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(240, 217, 181) : sf::Color(181, 136, 99));
            window.draw(square);

            // Draw selection highlight
            if (selectedSquare == sf::Vector2i(j, i)) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(j * tileSize + offset, i * tileSize + offset);
                highlight.setFillColor(sf::Color(255, 255, 0, 80));
                window.draw(highlight);
            }

            // Draw piece
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

bool Board::isWhite(PieceType type) {
    if (type == PieceType::Empty) return false;
    // Helper to determine team color
    return (int)type >= (int)PieceType::W_Pawn && (int)type <= (int)PieceType::W_King;
}