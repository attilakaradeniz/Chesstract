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

// Helper to determine team color
bool Board::isWhite(PieceType type) {
    if (type == PieceType::Empty) return false;
    return (int)type >= (int)PieceType::W_Pawn && (int)type <= (int)PieceType::W_King;
}

void Board::loadAssets() {
    // Load texture
    if (!piecesTexture.loadFromFile("assets/pieces.png")) {
        std::cerr << "Error: Could not load assets/pieces.png!" << std::endl;
    }
    piecesTexture.setSmooth(true);
    pieceSprite.setTexture(piecesTexture);

    // CRITICAL: Load the font! Ensure the path is correct
    if (!font.loadFromFile("assets/arial.ttf")) {
        std::cerr << "Error: Could not load assets/arial.ttf! Game Over text will not show." << std::endl;
    }

    setupPieceSources();
}

void Board::setupPieceSources() {
    // Mapping piece types to sprite sheet coordinates (column, row)
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

bool Board::isSquareAttacked(int targetRow, int targetCol, bool attackerIsWhite) {
    // Directions for Rooks, Bishops, and Queens
    int rowDirs[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
    int colDirs[] = { 0, 0, -1, 1, -1, 1, -1, 1 };

    // 1. Sliding Pieces (Rook, Bishop, Queen)
    for (int i = 0; i < 8; ++i) {
        int r = targetRow + rowDirs[i];
        int c = targetCol + colDirs[i];

        while (r >= 0 && r < 8 && c >= 0 && c < 8) {
            PieceType piece = grid[r][c];
            if (piece != PieceType::Empty) {
                if (isWhite(piece) == attackerIsWhite) {
                    if (i < 4) { // Orthogonal (Rook, Queen)
                        if (piece == PieceType::W_Rook || piece == PieceType::B_Rook ||
                            piece == PieceType::W_Queen || piece == PieceType::B_Queen) return true;
                    }
                    else { // Diagonal (Bishop, Queen)
                        if (piece == PieceType::W_Bishop || piece == PieceType::B_Bishop ||
                            piece == PieceType::W_Queen || piece == PieceType::B_Queen) return true;
                    }
                }
                break; // Path blocked by any piece
            }
            r += rowDirs[i];
            c += colDirs[i];
        }
    }

    // 2. Knight attacks
    int kRow[] = { -2, -2, -1, -1, 1, 1, 2, 2 };
    int kCol[] = { -1, 1, -2, 2, -2, 2, -1, 1 };
    for (int i = 0; i < 8; ++i) {
        int r = targetRow + kRow[i];
        int c = targetCol + kCol[i];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            PieceType piece = grid[r][c];
            if (isWhite(piece) == attackerIsWhite && (piece == PieceType::W_Knight || piece == PieceType::B_Knight)) return true;
        }
    }

    // 3. Pawn attacks (FIXED DIRECTION)
    // If attacker is White, they are coming from a higher row index (e.g., target is 4, white pawn is at 5)
    // If attacker is Black, they are coming from a lower row index (e.g., target is 4, black pawn is at 3)
    int pDir = (attackerIsWhite) ? 1 : -1;
    int pCols[] = { targetCol - 1, targetCol + 1 };

    for (int i = 0; i < 2; ++i) {
        int r = targetRow + pDir;
        int c = pCols[i];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            PieceType piece = grid[r][c];
            PieceType enemyPawn = attackerIsWhite ? PieceType::W_Pawn : PieceType::B_Pawn;
            if (piece == enemyPawn) return true;
        }
    }

    // 4. King attacks (To prevent Kings from touching)
    for (int i = 0; i < 8; ++i) {
        int r = targetRow + rowDirs[i];
        int c = targetCol + colDirs[i];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            PieceType piece = grid[r][c];
            if (isWhite(piece) == attackerIsWhite && (piece == PieceType::W_King || piece == PieceType::B_King)) return true;
        }
    }

    return false;
}

sf::Vector2i Board::findKing(bool white) {
    PieceType targetKing = white ? PieceType::W_King : PieceType::B_King;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (grid[r][c] == targetKing) return sf::Vector2i(c, r);
        }
    }
    return sf::Vector2i(-1, -1); // Should never happen
}

bool Board::isMoveValid(int startRow, int startCol, int endRow, int endCol) {
    PieceType movingPiece = grid[startRow][startCol];
    if (movingPiece == PieceType::Empty) return false;
    if (startRow == endRow && startCol == endCol) return false;

    // A piece cannot capture a piece of its own color
    PieceType targetPiece = grid[endRow][endCol];
    if (targetPiece != PieceType::Empty && isWhite(movingPiece) == isWhite(targetPiece)) {
        return false;
    }

    int rowDiff = std::abs(startRow - endRow);
    int colDiff = std::abs(startCol - endCol);
    bool isWhitePiece = isWhite(movingPiece);

    // --- PHASE 1: Basic Piece Movement Rules ---
    bool basicMoveOk = false;
    switch (movingPiece) {
    case PieceType::W_Knight: case PieceType::B_Knight:
        basicMoveOk = (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
        break;
    case PieceType::W_Rook: case PieceType::B_Rook:
        if (startRow == endRow || startCol == endCol) {
            int rStep = (endRow == startRow) ? 0 : (endRow > startRow ? 1 : -1);
            int cStep = (endCol == startCol) ? 0 : (endCol > startCol ? 1 : -1);
            int currR = startRow + rStep, currC = startCol + cStep;
            basicMoveOk = true;
            while (currR != endRow || currC != endCol) {
                if (grid[currR][currC] != PieceType::Empty) { basicMoveOk = false; break; }
                currR += rStep; currC += cStep;
            }
        }
        break;
    case PieceType::W_Bishop: case PieceType::B_Bishop:
        if (rowDiff == colDiff) {
            int rStep = (endRow > startRow ? 1 : -1);
            int cStep = (endCol > startCol ? 1 : -1);
            int currR = startRow + rStep, currC = startCol + cStep;
            basicMoveOk = true;
            while (currR != endRow || currC != endCol) {
                if (grid[currR][currC] != PieceType::Empty) { basicMoveOk = false; break; }
                currR += rStep; currC += cStep;
            }
        }
        break;
    case PieceType::W_Queen: case PieceType::B_Queen:
        if (rowDiff == colDiff || startRow == endRow || startCol == endCol) {
            int rStep = (endRow == startRow) ? 0 : (endRow > startRow ? 1 : -1);
            int cStep = (endCol == startCol) ? 0 : (endCol > startCol ? 1 : -1);
            int currR = startRow + rStep, currC = startCol + cStep;
            basicMoveOk = true;
            while (currR != endRow || currC != endCol) {
                if (grid[currR][currC] != PieceType::Empty) { basicMoveOk = false; break; }
                currR += rStep; currC += cStep;
            }
        }
        break;
    case PieceType::W_King: case PieceType::B_King:
        if (rowDiff <= 1 && colDiff <= 1) {
            basicMoveOk = true;
        }
        // Castling logic: Prevent castling if the king has moved or is currently in check
        else if (rowDiff == 0 && colDiff == 2) {
            bool kingMoved = isWhitePiece ? whiteKingMoved : blackKingMoved;
            if (!kingMoved) {
                // In chess, you cannot castle out of check.
                // This prevents hasLegalMoves from thinking castling is a valid escape.
                if (!isSquareAttacked(startRow, startCol, !isWhitePiece)) {
                    basicMoveOk = true;
                }
            }
        }
        break;
    case PieceType::W_Pawn: case PieceType::B_Pawn: {
        int dir = isWhitePiece ? -1 : 1;
        int startPawnRow = isWhitePiece ? 6 : 1;
        // Forward move
        if (colDiff == 0 && (endRow - startRow) == dir && grid[endRow][endCol] == PieceType::Empty) {
            basicMoveOk = true;
        }
        // Double move from start
        else if (colDiff == 0 && startRow == startPawnRow && (endRow - startRow) == 2 * dir &&
            grid[startRow + dir][startCol] == PieceType::Empty && grid[endRow][endCol] == PieceType::Empty) {
            basicMoveOk = true;
        }
        // Capture (Standard or strict En Passant)
        else if (colDiff == 1 && (endRow - startRow) == dir) {
            if (grid[endRow][endCol] != PieceType::Empty) {
                basicMoveOk = true;
            }
            // Strict En Passant check: Ensure lastPawnDoubleMove is actually valid on the board
            else if (lastPawnDoubleMove.x != -1 && lastPawnDoubleMove == sf::Vector2i(endCol, startRow)) {
                basicMoveOk = true;
            }
        }
        break;
    }
    }

    // If it's not even a valid move for the piece, no need to simulate
    if (!basicMoveOk) return false;

    // --- PHASE 2: Simulation to check if King is safe ---
    PieceType originalStart = grid[startRow][startCol];
    PieceType originalEnd = grid[endRow][endCol];

    // Temporarily apply the move
    grid[endRow][endCol] = originalStart;
    grid[startRow][startCol] = PieceType::Empty;

    // Special case: if we captured en passant, we must remove the pawn for the simulation too
    bool isEnPassant = (originalStart == PieceType::W_Pawn || originalStart == PieceType::B_Pawn) &&
        colDiff == 1 && originalEnd == PieceType::Empty;
    PieceType enPassantPawn = PieceType::Empty;
    if (isEnPassant) {
        enPassantPawn = grid[startRow][endCol];
        grid[startRow][endCol] = PieceType::Empty;
    }

    // Find the King's position (might have moved)
    sf::Vector2i kingPos = findKing(isWhitePiece);

    // Check if the move leaves the King under attack
    // The attacker's color is !isWhitePiece
    bool kingInDanger = isSquareAttacked(kingPos.y, kingPos.x, !isWhitePiece);

    // UNDO Simulation
    grid[startRow][startCol] = originalStart;
    grid[endRow][endCol] = originalEnd;
    if (isEnPassant) grid[startRow][endCol] = enPassantPawn;

    return !kingInDanger;
}

char Board::getPieceChar(PieceType type) {
    switch (type) {
    case PieceType::W_King:   case PieceType::B_King:   return 'K';
    case PieceType::W_Queen:  case PieceType::B_Queen:  return 'Q';
    case PieceType::W_Rook:   case PieceType::B_Rook:   return 'R';
    case PieceType::W_Bishop: case PieceType::B_Bishop: return 'B';
    case PieceType::W_Knight: case PieceType::B_Knight: return 'N';
    default: return ' '; // Pawns don't have a letter prefix in algebraic notation
    }
}

void Board::handleMouseClick(const sf::Vector2i mousePos) {
    // 1. If game is over, don't process any more clicks
    if (gameOver) return;

    int col = (mousePos.x - (int)offset) / (int)tileSize;
    int row = (mousePos.y - (int)offset) / (int)tileSize;

    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    if (selectedSquare != sf::Vector2i(-1, -1)) {
        PieceType movingPiece = grid[selectedSquare.y][selectedSquare.x];
        PieceType targetPiece = grid[row][col];

        if (isMoveValid(selectedSquare.y, selectedSquare.x, row, col)) {
            // Friendly fire check
            if (targetPiece != PieceType::Empty && isWhite(movingPiece) == isWhite(targetPiece)) {
                selectedSquare = sf::Vector2i(col, row);
                return;
            }

            // --- Capture Logic & Notation ---
            bool isCapture = (targetPiece != PieceType::Empty);
            char pieceChar = getPieceChar(movingPiece);
            std::string moveNotation = "";

            if ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) &&
                col != selectedSquare.x && targetPiece == PieceType::Empty) {
                isCapture = true;
                grid[selectedSquare.y][col] = PieceType::Empty;
            }

            if (pieceChar != ' ') moveNotation += pieceChar;
            else if (isCapture) moveNotation += (char)('a' + selectedSquare.x);

            if (isCapture) moveNotation += "x";
            moveNotation += (char)('a' + col);
            moveNotation += std::to_string(8 - row);

            // --- Castling & Flags ---
            if ((movingPiece == PieceType::W_King || movingPiece == PieceType::B_King) && std::abs(col - selectedSquare.x) == 2) {
                if (col == 6) {
                    grid[row][5] = grid[row][7]; grid[row][7] = PieceType::Empty;
                    moveNotation = "O-O";
                }
                else {
                    grid[row][3] = grid[row][0]; grid[row][0] = PieceType::Empty;
                    moveNotation = "O-O-O";
                }
            }

            if (movingPiece == PieceType::W_King) whiteKingMoved = true;
            if (movingPiece == PieceType::B_King) blackKingMoved = true;
            if (selectedSquare == sf::Vector2i(0, 0)) blackRook0Moved = true;
            if (selectedSquare == sf::Vector2i(7, 0)) blackRook7Moved = true;
            if (selectedSquare == sf::Vector2i(0, 7)) whiteRook0Moved = true;
            if (selectedSquare == sf::Vector2i(7, 7)) whiteRook7Moved = true;

            if ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && std::abs(row - selectedSquare.y) == 2)
                lastPawnDoubleMove = sf::Vector2i(col, row);
            else
                lastPawnDoubleMove = sf::Vector2i(-1, -1);

            // --- Movement ---
            grid[row][col] = movingPiece;
            grid[selectedSquare.y][selectedSquare.x] = PieceType::Empty;

            // Promotion
            if (movingPiece == PieceType::W_Pawn && row == 0) { grid[row][col] = PieceType::W_Queen; moveNotation += "=Q"; }
            if (movingPiece == PieceType::B_Pawn && row == 7) { grid[row][col] = PieceType::B_Queen; moveNotation += "=Q"; }

            // Turn Swap
            whiteTurn = !whiteTurn;

            // Notation Check (+)
            sf::Vector2i opponentKing = findKing(whiteTurn);
            if (isSquareAttacked(opponentKing.y, opponentKing.x, !whiteTurn)) {
                moveNotation += "+";
            }

            std::cout << (!whiteTurn ? "White moved: " : "Black moved: ") << moveNotation << std::endl;

            // Trigger game end check AFTER turn swap
            checkGameEnd();
        }
        selectedSquare = sf::Vector2i(-1, -1);
    }
    else {
        PieceType clickedPiece = grid[row][col];
        if (clickedPiece != PieceType::Empty && isWhite(clickedPiece) == whiteTurn) {
            selectedSquare = sf::Vector2i(col, row);
        }
    }
}

void Board::checkGameEnd() {
    // Debug: This should print after every move
    std::cout << "Checking for game end..." << std::endl;

    if (!hasLegalMoves(whiteTurn)) {
        gameOver = true;
        sf::Vector2i kingPos = findKing(whiteTurn);

        // Final determination: Checkmate vs Stalemate
        if (isSquareAttacked(kingPos.y, kingPos.x, !whiteTurn)) {
            resultText = whiteTurn ? "BLACK WINS BY CHECKMATE" : "WHITE WINS BY CHECKMATE";
        }
        else {
            resultText = "DRAW BY STALEMATE";
        }
        std::cout << "!!! " << resultText << " !!!" << std::endl;
    }
}

void Board::draw(sf::RenderWindow& window) {
    const int sourceSize = 45;
    const float scale = tileSize / (float)sourceSize;
    pieceSprite.setScale(scale, scale);

    // 1. Draw the board and the pieces first
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

            // Draw pieces
            PieceType type = grid[i][j];
            if (type != PieceType::Empty) {
                PieceSource src = pieceSourceMap[type];
                pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));
                pieceSprite.setPosition(j * tileSize + offset, i * tileSize + offset);
                window.draw(pieceSprite);
            }
        }
    }

    // 2. Overlay Layer: Show Game Over screen if the game has ended
    // This must be drawn last to appear on top of everything
    if (gameOver) {
        // Semi-transparent black rectangle to dim the board
        sf::RectangleShape overlay(sf::Vector2f(tileSize * 8, tileSize * 8));
        overlay.setPosition(offset, offset);
        overlay.setFillColor(sf::Color(0, 0, 0, 180)); // 180 is the alpha (transparency)
        window.draw(overlay);

        // Result Text configuration
        sf::Text text;
        text.setFont(font);
        text.setString(resultText);
        text.setCharacterSize(50);
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);

        // Center the text based on its local bounds
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

        // Position the text at the center of the 8x8 board
        float centerX = (tileSize * 8) / 2.0f + offset;
        float centerY = (tileSize * 8) / 2.0f + offset;
        text.setPosition(centerX, centerY);

        window.draw(text);
    }
}

void Board::printStatus() {
    std::cout << "Current turn: " << (whiteTurn ? "White" : "Black") << std::endl;
}

bool Board::hasLegalMoves(bool white) {
    // Iterate through every square on the board
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            PieceType piece = grid[r][c];

            // If the piece belongs to the current player
            if (piece != PieceType::Empty && isWhite(piece) == white) {
                // Check every possible destination for this piece
                for (int targetR = 0; targetR < 8; ++targetR) {
                    for (int targetC = 0; targetC < 8; ++targetC) {
                        // If even ONE move is valid, the game is not over
                        if (isMoveValid(r, c, targetR, targetC)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    // No legal moves found for any piece
    return false;
}

