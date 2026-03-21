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

    // adjust for flip
    if (isFlowFlipped) {
        col = 7 - col;
		row = 7 - row;      
    }

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

            // Promotion handling
            if (movingPiece == PieceType::W_Pawn && row == 0) { grid[row][col] = PieceType::W_Queen; moveNotation += "=Q"; }
            if (movingPiece == PieceType::B_Pawn && row == 7) { grid[row][col] = PieceType::B_Queen; moveNotation += "=Q"; }

            // --- Check for "+" or "#" (Notation update) ---
            // After the move, check if the opponent's king is under attack
            sf::Vector2i opponentKing = findKing(!isWhite(movingPiece));
            bool isCheck = isSquareAttacked(opponentKing.y, opponentKing.x, isWhite(movingPiece));

            // Check if it's also a checkmate
            if (isCheck) {
                if (!hasLegalMoves(!isWhite(movingPiece))) {
                    moveNotation += "#"; // Checkmate
                }
                else {
                    moveNotation += "+"; // Check
                }
            }

            // --- Record Move for History (Now with correct notation) ---
            MoveRecord record;
            record.start = selectedSquare;
            record.end = sf::Vector2i(col, row);
            record.movedPiece = movingPiece;
            record.capturedPiece = targetPiece;
            record.notation = moveNotation; // Now contains + or #
            record.isWhiteMove = isWhite(movingPiece);

            // Save state for undo
            record.prevLastPawnDoubleMove = lastPawnDoubleMove;
            record.prevWhiteKingMoved = whiteKingMoved;
            record.prevBlackKingMoved = blackKingMoved;
            record.prevWhiteRook0Moved = whiteRook0Moved;
            record.prevWhiteRook7Moved = whiteRook7Moved;
            record.prevBlackRook0Moved = blackRook0Moved;
            record.prevBlackRook7Moved = blackRook7Moved;

            moveHistory.push_back(record);

            // Print to console and swap turn
            std::cout << (isWhite(movingPiece) ? "White moved: " : "Black moved: ") << moveNotation << std::endl;
            whiteTurn = !whiteTurn;

            // Trigger game end check
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

    // --- 1. Draw Chess Board and Pieces ---
    for (int i = 0; i < 8; ++i) { // i = Logical Row (0-7)
        for (int j = 0; j < 8; ++j) { // j = Logical Column (0-7)

            // Calculate visual position based on flip status
            int renderRow = isFlowFlipped ? (7 - i) : i;
            int renderCol = isFlowFlipped ? (7 - j) : j;

            // Draw individual square
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);

            // Standard chess pattern logic (i+j)
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(235, 235, 210) : sf::Color(180, 50, 50));
            window.draw(square);

            // Draw selection highlight if this square is selected
            if (selectedSquare == sf::Vector2i(j, i)) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                highlight.setFillColor(sf::Color(255, 255, 0, 80));
                window.draw(highlight);
            }

            // Draw the piece occupying this square
            PieceType type = grid[i][j];
            if (type != PieceType::Empty) {
                PieceSource src = pieceSourceMap[type];
                pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));
                pieceSprite.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                window.draw(pieceSprite);
            }
        }
    }

    // --- 2. Draw Side Panel (Notation UI) ---
    sf::RectangleShape sidePanel(sf::Vector2f(300, 900));
    sidePanel.setPosition(900, 0);
    sidePanel.setFillColor(sf::Color(45, 45, 45)); // Charcoal Gray
    window.draw(sidePanel);

    // Header for the side panel
    sf::Text title;
    title.setFont(font);
    title.setString("Move History");
    title.setCharacterSize(24);
    title.setFillColor(sf::Color(200, 200, 200));
    title.setPosition(920, 20);
    window.draw(title);

    // Render individual moves from history
    sf::Text moveText;
    moveText.setFont(font);
    moveText.setCharacterSize(18);
    moveText.setFillColor(sf::Color::White);

    float startX = 930;
    float startY = 70;
    float lineHeight = 25;

    for (size_t i = 0; i < moveHistory.size(); ++i) {
        int turnNumber = (i / 2) + 1;
        bool isWhiteMove = (i % 2 == 0);

        float xPos = isWhiteMove ? startX : startX + 130;
        float yPos = startY + ((turnNumber - 1) * lineHeight);

        std::string moveStr = isWhiteMove ? (std::to_string(turnNumber) + ". " + moveHistory[i].notation)
            : moveHistory[i].notation;

        moveText.setString(moveStr);
        moveText.setPosition(xPos, yPos);

        // Stop rendering if moves go beyond window height
        if (yPos < 860) {
            window.draw(moveText);
        }
    }

    // --- 3. Draw Game Over Overlay ---
    if (gameOver) {
        sf::RectangleShape overlay(sf::Vector2f(tileSize * 8, tileSize * 8));
        overlay.setPosition(offset, offset);
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text text;
        text.setFont(font);
        text.setString(resultText);
        text.setCharacterSize(50);
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);

        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        text.setPosition((tileSize * 8) / 2.0f + offset, (tileSize * 8) / 2.0f + offset);

        window.draw(text);
    }

    // --- 4. Draw Board Coordinates (a-h, 1-8) ---
    if (showCoordinates) {
        for (int i = 0; i < 8; ++i) {
            sf::Text coordText;
            coordText.setFont(font);
            coordText.setCharacterSize(14); // Slightly smaller for professional look

            // 1. Numbers (1-8) - Vertical (Placed on the left edge of the first column)
            int logicRow = isFlowFlipped ? (7 - i) : i;
            coordText.setString(std::to_string(8 - logicRow));

            // Dynamic color: Light grey on dark squares, dark grey on light squares
            // Since it's the first column (j=0), color depends on i % 2
            coordText.setFillColor((i % 2 == 0) ? sf::Color(180, 50, 50) : sf::Color(235, 235, 210));
            coordText.setPosition(offset + 2, i * tileSize + offset + 2);
            window.draw(coordText);

            // 2. Letters (a-h) - Horizontal (Placed on the bottom edge of the last row)
            int logicCol = isFlowFlipped ? (7 - i) : i;
            std::string colLabel = "";
            colLabel += (char)('a' + logicCol);
            coordText.setString(colLabel);

            // Dynamic color for letters: Based on (7 + i) % 2 since they are on the 7th row
            coordText.setFillColor(((7 + i) % 2 == 0) ? sf::Color(180, 50, 50) : sf::Color(235, 235, 210));
            coordText.setPosition(i * tileSize + offset + tileSize - 15, 7 * tileSize + offset + tileSize - 20);
            window.draw(coordText);
        }
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
void Board::exportPGN() {
    if (moveHistory.empty()) {
        std::cout << "No moves to export yet." << std::endl;
        return;
    }

    std::cout << "\n--- PGN EXPORT ---" << std::endl;
    std::string pgn = "";
    int moveNumber = 1;

    for (size_t i = 0; i < moveHistory.size(); ++i) {
        if (moveHistory[i].isWhiteMove) {
            pgn += std::to_string(moveNumber) + ". " + moveHistory[i].notation + " ";
        }
        else {
            pgn += moveHistory[i].notation + " ";
            moveNumber++;
        }
    }

    std::cout << pgn << std::endl;
    if (gameOver) {
        std::cout << (resultText.find("DRAW") != std::string::npos ? "1/2-1/2" : (whiteTurn ? "0-1" : "1-0")) << std::endl;
    }
    std::cout << "------------------\n" << std::endl;
}

void Board::undoMove() {
    if (moveHistory.empty()) {
        std::cout << "No moves to undo." << std::endl;
        return;
    }

    // 1. Get the last move and remove it from history
    MoveRecord last = moveHistory.back();
    moveHistory.pop_back();

    // 2. Restore the piece to its original position
    grid[last.start.y][last.start.x] = last.movedPiece;
    grid[last.end.y][last.end.x] = last.capturedPiece;

    // 3. Special Case: Undo En Passant
    // If a pawn moved diagonally to an empty square, it was an en passant capture
    if ((last.movedPiece == PieceType::W_Pawn || last.movedPiece == PieceType::B_Pawn) &&
        last.start.x != last.end.x && last.capturedPiece == PieceType::Empty) {

        // Put the captured pawn back (White captured Black or vice versa)
        grid[last.start.y][last.end.x] = last.isWhiteMove ? PieceType::B_Pawn : PieceType::W_Pawn;
    }

    // 4. Special Case: Undo Castling
    // If the king moved 2 squares, we must move the rook back as well
    if ((last.movedPiece == PieceType::W_King || last.movedPiece == PieceType::B_King) &&
        std::abs(last.end.x - last.start.x) == 2) {

        if (last.end.x == 6) { // Kingside castling
            grid[last.end.y][7] = grid[last.end.y][5];
            grid[last.end.y][5] = PieceType::Empty;
        }
        else if (last.end.x == 2) { // Queenside castling
            grid[last.end.y][0] = grid[last.end.y][3];
            grid[last.end.y][3] = PieceType::Empty;
        }
    }

    // 5. Restore game state flags (Castling rights and En Passant square)
    lastPawnDoubleMove = last.prevLastPawnDoubleMove;
    whiteKingMoved = last.prevWhiteKingMoved;
    blackKingMoved = last.prevBlackKingMoved;
    whiteRook0Moved = last.prevWhiteRook0Moved;
    whiteRook7Moved = last.prevWhiteRook7Moved;
    blackRook0Moved = last.prevBlackRook0Moved;
    blackRook7Moved = last.prevBlackRook7Moved;

    // 6. Reset turn and clear game over status
    whiteTurn = last.isWhiteMove;
    gameOver = false;

    std::cout << "Undo successful. Turn is now " << (whiteTurn ? "White" : "Black") << "." << std::endl;
}

void Board::flipBoard() {
	isFlowFlipped = !isFlowFlipped;
	std::cout << "Board flipped. " << (isFlowFlipped ? "Black at bottom." : "White at bottom.") << std::endl;
}

void Board::toggleCoordinates() {
    showCoordinates = !showCoordinates;
    std::cout << "Coordinates " << (showCoordinates ? "enabled." : "disabled.") << std::endl;
}

