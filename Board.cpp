#define _CRT_SECURE_NO_WARNINGS
#include "Board.hpp"
#include <iostream>
#include <cmath> // For std::abs

Board::Board() :
    whiteTurn(true),
    gameOver(false),
    isFlowFlipped(false),
    showCoordinates(true),
    selectedSquare(-1, -1),
    lastPawnDoubleMove(-1, -1),
    whiteKingMoved(false),
    blackKingMoved(false),
    whiteRook0Moved(false),
    whiteRook7Moved(false),
    blackRook0Moved(false),
    blackRook7Moved(false),
    tileSize(100.0f),
    offset(50.0f),
    scrollOffset(0.0f), // initial there is no scroll
    isDragging(false),
	draggedPieceSource(-1, -1),
	mousePos(0.f, 0.f)



{
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
    // 1. Load Piece Texture
    if (!piecesTexture.loadFromFile("assets/pieces.png")) {
        std::cerr << "Error: Could not load assets/pieces.png!" << std::endl;
    }
    piecesTexture.setSmooth(true);
    pieceSprite.setTexture(piecesTexture);

    // 2. Load UI Font
    if (!font.loadFromFile("assets/arial.ttf")) {
        std::cerr << "Error: Could not load assets/arial.ttf!" << std::endl;
    }

    // --- 3. Load Audio Buffers ---
    if (!moveBuffer.loadFromFile("assets/move.wav")) {
        std::cerr << "Error: Could not load assets/move.wav!" << std::endl;
    }
    moveSound.setBuffer(moveBuffer);

    if (!captureBuffer.loadFromFile("assets/capture.wav")) {
        std::cerr << "Error: Could not load assets/capture.wav!" << std::endl;
    }
    captureSound.setBuffer(captureBuffer);

    // Set default volume levels
    moveSound.setVolume(75.f);
    captureSound.setVolume(85.f);

    // 4. Finalize Setup
    setupPieceSources();
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

bool Board::isSquareAttacked(int targetRow, int targetCol, bool attackerIsWhite) {
    int rowDirs[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
    int colDirs[] = { 0, 0, -1, 1, -1, 1, -1, 1 };

    // Sliding Pieces
    for (int i = 0; i < 8; ++i) {
        int r = targetRow + rowDirs[i];
        int c = targetCol + colDirs[i];
        while (r >= 0 && r < 8 && c >= 0 && c < 8) {
            PieceType piece = grid[r][c];
            if (piece != PieceType::Empty) {
                if (isWhite(piece) == attackerIsWhite) {
                    if (i < 4) { // Rook/Queen
                        if (piece == PieceType::W_Rook || piece == PieceType::B_Rook ||
                            piece == PieceType::W_Queen || piece == PieceType::B_Queen) return true;
                    }
                    else { // Bishop/Queen
                        if (piece == PieceType::W_Bishop || piece == PieceType::B_Bishop ||
                            piece == PieceType::W_Queen || piece == PieceType::B_Queen) return true;
                    }
                }
                break;
            }
            r += rowDirs[i]; c += colDirs[i];
        }
    }

    // Knight attacks
    int kRow[] = { -2, -2, -1, -1, 1, 1, 2, 2 };
    int kCol[] = { -1, 1, -2, 2, -2, 2, -1, 1 };
    for (int i = 0; i < 8; ++i) {
        int r = targetRow + kRow[i], c = targetCol + kCol[i];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            PieceType piece = grid[r][c];
            if (isWhite(piece) == attackerIsWhite && (piece == PieceType::W_Knight || piece == PieceType::B_Knight)) return true;
        }
    }

    // Pawn attacks
    int pDir = (attackerIsWhite) ? 1 : -1;
    int pCols[] = { targetCol - 1, targetCol + 1 };
    for (int i = 0; i < 2; ++i) {
        int r = targetRow + pDir, c = pCols[i];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            PieceType piece = grid[r][c];
            PieceType enemyPawn = attackerIsWhite ? PieceType::W_Pawn : PieceType::B_Pawn;
            if (piece == enemyPawn) return true;
        }
    }

    // King attacks
    for (int i = 0; i < 8; ++i) {
        int r = targetRow + rowDirs[i], c = targetCol + colDirs[i];
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
    return sf::Vector2i(-1, -1);
}

bool Board::isMoveValid(int startRow, int startCol, int endRow, int endCol) {
    PieceType movingPiece = grid[startRow][startCol];
    if (movingPiece == PieceType::Empty) return false;
    if (startRow == endRow && startCol == endCol) return false;

    PieceType targetPiece = grid[endRow][endCol];
    if (targetPiece != PieceType::Empty && isWhite(movingPiece) == isWhite(targetPiece)) return false;

    int rowDiff = std::abs(startRow - endRow);
    int colDiff = std::abs(startCol - endCol);
    bool isWhitePiece = isWhite(movingPiece);
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
        if (rowDiff <= 1 && colDiff <= 1) basicMoveOk = true;
        else if (rowDiff == 0 && colDiff == 2) {
            bool kingMoved = isWhitePiece ? whiteKingMoved : blackKingMoved;
            if (!kingMoved && !isSquareAttacked(startRow, startCol, !isWhitePiece)) basicMoveOk = true;
        }
        break;
    case PieceType::W_Pawn: case PieceType::B_Pawn: {
        int dir = isWhitePiece ? -1 : 1;
        int startPawnRow = isWhitePiece ? 6 : 1;
        if (colDiff == 0 && (endRow - startRow) == dir && grid[endRow][endCol] == PieceType::Empty) basicMoveOk = true;
        else if (colDiff == 0 && startRow == startPawnRow && (endRow - startRow) == 2 * dir &&
            grid[startRow + dir][startCol] == PieceType::Empty && grid[endRow][endCol] == PieceType::Empty) basicMoveOk = true;
        else if (colDiff == 1 && (endRow - startRow) == dir) {
            if (grid[endRow][endCol] != PieceType::Empty) basicMoveOk = true;
            else if (lastPawnDoubleMove.x != -1 && lastPawnDoubleMove == sf::Vector2i(endCol, startRow)) basicMoveOk = true;
        }
        break;
    }
    }

    if (!basicMoveOk) return false;

    // Simulation check
    PieceType originalStart = grid[startRow][startCol], originalEnd = grid[endRow][endCol];
    grid[endRow][endCol] = originalStart; grid[startRow][startCol] = PieceType::Empty;
    bool isEnPassant = (originalStart == PieceType::W_Pawn || originalStart == PieceType::B_Pawn) && colDiff == 1 && originalEnd == PieceType::Empty;
    PieceType epPawn = PieceType::Empty; if (isEnPassant) { epPawn = grid[startRow][endCol]; grid[startRow][endCol] = PieceType::Empty; }
    sf::Vector2i kingPos = findKing(isWhitePiece);
    bool kingInDanger = isSquareAttacked(kingPos.y, kingPos.x, !isWhitePiece);
    grid[startRow][startCol] = originalStart; grid[endRow][endCol] = originalEnd;
    if (isEnPassant) grid[startRow][endCol] = epPawn;

    return !kingInDanger;
}

char Board::getPieceChar(PieceType type) {
    switch (type) {
    case PieceType::W_King:   case PieceType::B_King:   return 'K';
    case PieceType::W_Queen:  case PieceType::B_Queen:  return 'Q';
    case PieceType::W_Rook:   case PieceType::B_Rook:   return 'R';
    case PieceType::W_Bishop: case PieceType::B_Bishop: return 'B';
    case PieceType::W_Knight: case PieceType::B_Knight: return 'N';
    default: return ' ';
    }
}



void Board::checkGameEnd() {
    if (!hasLegalMoves(whiteTurn)) {
        gameOver = true;
        sf::Vector2i kingPos = findKing(whiteTurn);
        if (isSquareAttacked(kingPos.y, kingPos.x, !whiteTurn)) resultText = whiteTurn ? "BLACK WINS BY CHECKMATE" : "WHITE WINS BY CHECKMATE";
        else resultText = "DRAW BY STALEMATE";
        std::cout << "!!! " << resultText << " !!!" << std::endl;
    }
}

void Board::draw(sf::RenderWindow& window) {
    const int sourceSize = 45;
    const float scale = tileSize / (float)sourceSize;
    pieceSprite.setScale(scale, scale);

    // draw fonksiyonunun en başında:
    static sf::Clock animationClock;
    float time = animationClock.getElapsedTime().asSeconds();
    // 0.8 ile 1.2 arasında gidip gelen bir çarpan (yavaşça büyüyüp küçülür)
    float pulseScale = 1.0f + 0.10f * std::sin(time * 3.0f);

    // --- Satranç Tahtası ve Taşların Çizimi ---
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int renderRow = isFlowFlipped ? (7 - i) : i;
            int renderCol = isFlowFlipped ? (7 - j) : j;
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(235, 235, 210) : sf::Color(180, 50, 50));
            window.draw(square);

            if (selectedSquare == sf::Vector2i(j, i)) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                highlight.setFillColor(sf::Color(255, 255, 0, 80));
                window.draw(highlight);
            }

            PieceType type = grid[i][j];
            if (type != PieceType::Empty) {
                // Check if this specific piece is currently being dragged
                // If it is being dragged, we skip drawing it here so it doesn't appear twice
                if (!(isDragging && draggedPieceSource == sf::Vector2i(j, i))) {
                    PieceSource src = pieceSourceMap[type];
                    pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));
                    pieceSprite.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                    window.draw(pieceSprite);
                }
            }

            // Inside the nested loop for drawing squares:
            // for last move and turn indicator
            if (sf::Vector2i(j, i) == lastMoveStart || sf::Vector2i(j, i) == lastMoveEnd) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                // Light yellow with some transparency
                highlight.setFillColor(sf::Color(255, 255, 0, 35));
                window.draw(highlight);
            }
        }
    }

    // --- DRAW DRAGGED PIECE ON TOP ---
    if (isDragging) {
        // Get the type of the piece that was picked up
        PieceType type = grid[draggedPieceSource.y][draggedPieceSource.x];
        PieceSource src = pieceSourceMap[type];

        // Set the texture area for the piece
        pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));

        // Set origin to the center of the sprite so it's held by the middle
        pieceSprite.setOrigin(sourceSize / 2.0f, sourceSize / 2.0f);

        // Follow the mouse cursor position
        pieceSprite.setPosition(mousePos);

        window.draw(pieceSprite);

        // Crucial: Reset origin so it doesn't mess up the next frame's regular piece drawing
        pieceSprite.setOrigin(0, 0);
    }

    // Draw Dots and Rings
    for (const auto& move : validMoves) {
        int rRow = isFlowFlipped ? (7 - move.y) : move.y;
        int rCol = isFlowFlipped ? (7 - move.x) : move.x;
        float centerX = rCol * tileSize + offset + tileSize / 2.0f;
        float centerY = rRow * tileSize + offset + tileSize / 2.0f;

        if (grid[move.y][move.x] == PieceType::Empty) {
            sf::CircleShape dot(12.f); dot.setOrigin(12.f, 12.f);
            dot.setFillColor(sf::Color(0, 0, 0, 45)); dot.setPosition(centerX, centerY);
            window.draw(dot);
        }
        else {
            float ringRadius = tileSize / 2.0f - 5.0f;
            sf::CircleShape ring(ringRadius); ring.setOrigin(ringRadius, ringRadius);
            ring.setFillColor(sf::Color::Transparent); ring.setOutlineThickness(6.f);
            ring.setOutlineColor(sf::Color(0, 0, 0, 45)); ring.setPosition(centerX, centerY);
            window.draw(ring);
        }
    }

    // Side Panel Notation Background
    sf::RectangleShape sidePanel(sf::Vector2f(300, 900));
    sidePanel.setPosition(900, 0); sidePanel.setFillColor(sf::Color(45, 45, 45));
    window.draw(sidePanel);

    // --- MOVE HISTORY WITH SCROLLING ---
    sf::Text title("Move History", font, 24);
    title.setPosition(920, 20);
    window.draw(title);

    // Otomatik Scroll Hesaplama
    int activeRow = (currentMoveIndex / 2);
    float targetY = 70 + (activeRow * 25);
    if (targetY > 800) {
        scrollOffset = targetY - 800;
    }
    else {
        scrollOffset = 0;
    }

    sf::Text moveText("", font, 18);
    for (size_t i = 0; i < moveHistory.size(); ++i) {
        int turnNumber = (i / 2) + 1;
        float xPos = (i % 2 == 0) ? 930 : 1060;
        float yPos = 70 + ((turnNumber - 1) * 25) - scrollOffset;

        // Kırpma (Clipping): Sadece panel içindeyse çiz
        if (yPos > 60 && yPos < 860) {
            std::string moveStr = (i % 2 == 0) ? (std::to_string(turnNumber) + ". " + moveHistory[i].notation) : moveHistory[i].notation;
            moveText.setString(moveStr);
            moveText.setPosition(xPos, yPos);

            if ((int)i == currentMoveIndex) {
                moveText.setFillColor(sf::Color::Yellow);
                moveText.setStyle(sf::Text::Bold | sf::Text::Underlined);
            }
            else {
                moveText.setFillColor(sf::Color::White);
                moveText.setStyle(sf::Text::Regular);
            }
            window.draw(moveText);
        }
    }

    // --- 3D PULSING TURN INDICATOR ---
    float baseRadius = 15.f;
    float currentRadius = baseRadius * pulseScale;

    sf::CircleShape top(currentRadius);
    top.setOrigin(currentRadius, currentRadius);
    float indicatorX = 8 * tileSize + offset + 25;
    float indicatorY = 4 * tileSize + offset;
    top.setPosition(indicatorX, indicatorY);

    sf::Color mainColor = whiteTurn ? sf::Color(220, 220, 220) : sf::Color(30, 30, 30);
    sf::Color highlightColor = whiteTurn ? sf::Color::White : sf::Color(70, 70, 70);

    top.setFillColor(mainColor);
    top.setOutlineThickness(2.f);
    top.setOutlineColor(sf::Color(30, 30, 30, 150));
    window.draw(top);

    sf::CircleShape lightSpot(currentRadius * 0.4f);
    lightSpot.setOrigin(lightSpot.getRadius(), lightSpot.getRadius());
    lightSpot.setPosition(indicatorX - currentRadius * 0.3f, indicatorY - currentRadius * 0.3f);
    lightSpot.setFillColor(highlightColor);
    window.draw(lightSpot);

    if (gameOver) {
        sf::RectangleShape overlay(sf::Vector2f(tileSize * 8, tileSize * 8));
        overlay.setPosition(offset, offset); overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);
        sf::Text endMsg(resultText, font, 50); endMsg.setStyle(sf::Text::Bold);
        sf::FloatRect tr = endMsg.getLocalBounds();
        endMsg.setOrigin(tr.left + tr.width / 2.0f, tr.top + tr.height / 2.0f);
        endMsg.setPosition((tileSize * 8) / 2.0f + offset, (tileSize * 8) / 2.0f + offset);
        window.draw(endMsg);
    }

    if (showCoordinates) {
        for (int i = 0; i < 8; ++i) {
            sf::Text cT("", font, 14);
            int logR = isFlowFlipped ? (7 - i) : i;
            cT.setString(std::to_string(8 - logR)); cT.setFillColor((i % 2 == 0) ? sf::Color(180, 50, 50) : sf::Color(235, 235, 210));
            cT.setPosition(offset + 2, i * tileSize + offset + 2); window.draw(cT);
            int logC = isFlowFlipped ? (7 - i) : i; std::string l = ""; l += (char)('a' + logC);
            cT.setString(l); cT.setFillColor(((7 + i) % 2 == 0) ? sf::Color(180, 50, 50) : sf::Color(235, 235, 210));
            cT.setPosition(i * tileSize + offset + tileSize - 15, 7 * tileSize + offset + tileSize - 20); window.draw(cT);
        }
    }
}

bool Board::hasLegalMoves(bool white) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (grid[r][c] != PieceType::Empty && isWhite(grid[r][c]) == white) {
                for (int tr = 0; tr < 8; ++tr) {
                    for (int tc = 0; tc < 8; ++tc) {
                        if (isMoveValid(r, c, tr, tc)) return true;
                    }
                }
            }
        }
    }
    return false;
}

void Board::undoMove() {
    if (moveHistory.empty()) return;
    MoveRecord last = moveHistory.back(); moveHistory.pop_back();
    grid[last.start.y][last.start.x] = last.movedPiece;
    grid[last.end.y][last.end.x] = last.capturedPiece;
    if ((last.movedPiece == PieceType::W_Pawn || last.movedPiece == PieceType::B_Pawn) && last.start.x != last.end.x && last.capturedPiece == PieceType::Empty)
        grid[last.start.y][last.end.x] = last.isWhiteMove ? PieceType::B_Pawn : PieceType::W_Pawn;
    if ((last.movedPiece == PieceType::W_King || last.movedPiece == PieceType::B_King) && std::abs(last.end.x - last.start.x) == 2) {
        if (last.end.x == 6) { grid[last.end.y][7] = grid[last.end.y][5]; grid[last.end.y][5] = PieceType::Empty; }
        else { grid[last.end.y][0] = grid[last.end.y][3]; grid[last.end.y][3] = PieceType::Empty; }
    }
    lastPawnDoubleMove = last.prevLastPawnDoubleMove;
    whiteKingMoved = last.prevWhiteKingMoved; blackKingMoved = last.prevBlackKingMoved;
    whiteRook0Moved = last.prevWhiteRook0Moved; whiteRook7Moved = last.prevWhiteRook7Moved;
    blackRook0Moved = last.prevBlackRook0Moved; blackRook7Moved = last.prevBlackRook7Moved;
    whiteTurn = last.isWhiteMove; gameOver = false;
    moveSound.play();
}

void Board::calculateValidMoves(int startRow, int startCol) {
    validMoves.clear();
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (isMoveValid(startRow, startCol, r, c)) validMoves.push_back(sf::Vector2i(c, r));
        }
    }
}

void Board::flipBoard() { isFlowFlipped = !isFlowFlipped; }
void Board::toggleCoordinates() { showCoordinates = !showCoordinates; }
void Board::printStatus() { std::cout << "Current turn: " << (whiteTurn ? "White" : "Black") << std::endl; }
//void Board::exportPGN() { /* PGN Export implementation... */ }
void Board::exportPGN() {
    if (moveHistory.empty()) {
        std::cout << "No moves to export yet." << std::endl;
        return;
    }

    std::string pgn = "";
    int moveNumber = 1;

    // Standard PGN Header Tags (Optional but professional)
    pgn += "[Event \"Casual Game\"]\n";
//    pgn += "[Site \"Gemini Chess Engine\"]\n";
    pgn += "[Site \"chesstracted\"]\n";

    // Get current date
    time_t now = time(0);
    tm *ltm = localtime(&now);
    pgn += "[Date \"" + std::to_string(1900 + ltm->tm_year) + "." + 
           std::to_string(1 + ltm->tm_mon) + "." + std::to_string(ltm->tm_mday) + "\"]\n";
    
    pgn += "[White \"Player 1\"]\n";
    pgn += "[Black \"Player 2\"]\n";
    
    // Result determination
    std::string result = "*";
    if (gameOver) {
        if (resultText.find("WHITE WINS") != std::string::npos) result = "1-0";
        else if (resultText.find("BLACK WINS") != std::string::npos) result = "0-1";
        else if (resultText.find("DRAW") != std::string::npos) result = "1/2-1/2";
    }
    pgn += "[Result \"" + result + "\"]\n\n";

    // Build the move list
    for (size_t i = 0; i < moveHistory.size(); ++i) {
        if (moveHistory[i].isWhiteMove) {
            pgn += std::to_string(moveNumber) + ". " + moveHistory[i].notation + " ";
        } else {
            pgn += moveHistory[i].notation + " ";
            moveNumber++;
        }
    }
    
    pgn += result; // Append final result at the end of moves

    std::cout << "\n--- PGN OUTPUT ---\n" << pgn << "\n------------------\n" << std::endl;
}

void Board::savePGNToFile() {
    if (moveHistory.empty()) return;

    // 1. Get current system time
    std::time_t t = std::time(nullptr);

    // 2. Convert to local time structure (Standard C++)
    std::tm* ltm = std::localtime(&t);

    // 3. Format the filename: game_YYYYMMDD_HHMMSS.pgn
    char buf[32];
    std::strftime(buf, sizeof(buf), "game_%Y%m%d_%H%M%S.pgn", ltm);
    std::string filename = buf;

    // 4. Open file and write
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        int moveNumber = 1;
        for (size_t i = 0; i < moveHistory.size(); ++i) {
            if (moveHistory[i].isWhiteMove) {
                outFile << moveNumber << ". " << moveHistory[i].notation << " ";
            }
            else {
                outFile << moveHistory[i].notation << " ";
                moveNumber++;
            }
        }

        // Add result at the end
        std::string result = "*";
        if (gameOver) {
            if (resultText.find("WHITE WINS") != std::string::npos) result = "1-0";
            else if (resultText.find("BLACK WINS") != std::string::npos) result = "0-1";
            else if (resultText.find("DRAW") != std::string::npos) result = "1/2-1/2";
        }
        outFile << result;

        outFile.close();
        std::cout << "Successfully saved to: " << filename << std::endl;
    }
    else {
        std::cerr << "Error: Could not create file " << filename << std::endl;
    }
}

void Board::resetBoardToStart() {
    // 1. reset all flags
    whiteTurn = true;
    gameOver = false;
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteRook0Moved = false;
    whiteRook7Moved = false;
    blackRook0Moved = false;
    blackRook7Moved = false;
    lastPawnDoubleMove = sf::Vector2i(-1, -1);

    // 2. clear the oard (Empty)
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            grid[i][j] = PieceType::Empty;
        }
    }

    // 3. pieces openning plaeces(same in the Constructor)
    for (int i = 0; i < 8; ++i) {
        grid[1][i] = PieceType::B_Pawn;
        grid[6][i] = PieceType::W_Pawn;
    }
    grid[0][0] = grid[0][7] = PieceType::B_Rook;
    grid[7][0] = grid[7][7] = PieceType::W_Rook;
    grid[0][1] = grid[0][6] = PieceType::B_Knight;
    grid[7][1] = grid[7][6] = PieceType::W_Knight;
    grid[0][2] = grid[0][5] = PieceType::B_Bishop;
    grid[7][2] = grid[7][5] = PieceType::W_Bishop;
    grid[0][3] = PieceType::B_Queen; grid[0][4] = PieceType::B_King;
    grid[7][3] = PieceType::W_Queen; grid[7][4] = PieceType::W_King;
}

void Board::applyMoveIndependently(const MoveRecord& record) {
    // 1. standard piece move
    grid[record.end.y][record.end.x] = record.movedPiece;
    grid[record.start.y][record.start.x] = PieceType::Empty;

    // 2. En Passant
    if ((record.movedPiece == PieceType::W_Pawn || record.movedPiece == PieceType::B_Pawn) &&
        record.start.x != record.end.x && record.capturedPiece == PieceType::Empty) {
        grid[record.start.y][record.end.x] = PieceType::Empty;
    }

    // 3. castle 
    if ((record.movedPiece == PieceType::W_King || record.movedPiece == PieceType::B_King) &&
        std::abs(record.end.x - record.start.x) == 2) {
        if (record.end.x == 6) { // Kısa Rok
            grid[record.end.y][5] = grid[record.end.y][7];
            grid[record.end.y][7] = PieceType::Empty;
        }
        else { // caastle long
            grid[record.end.y][3] = grid[record.end.y][0];
            grid[record.end.y][0] = PieceType::Empty;
        }
    }

    // ATTENTION: Promotion is for now auto queen (in record.movedPiece) 
    
}

void Board::goToMove(int targetIndex) {
    // 1. Sınırları aşma (History dışına çıkma)
    if (targetIndex < -1 || targetIndex >= (int)moveHistory.size()) return;

    // 2. Tahtayı ve flagleri sıfırla
    resetBoardToStart();

    // 3. Tarihi baştan yaz (Sessizce)
    for (int i = 0; i <= targetIndex; ++i) {
        applyMoveIndependently(moveHistory[i]);
    }

    // 4. İmleci güncelle
    currentMoveIndex = targetIndex;

    // 5. Sırayı belirle
    // Eğer currentMoveIndex -1 ise (başlangıç) sıra Beyazda (true).
    // Eğer currentMoveIndex 0 ise (beyaz oynadı) sıra Siyahda (false).
    // Kısacası: Çift index ise sıra siyahta, tek (veya -1) ise beyazda.
    if (currentMoveIndex == -1) {
        whiteTurn = true;
    }
    else {
        whiteTurn = !moveHistory[currentMoveIndex].isWhiteMove;
    }

    // 6. Görsel netlik için seçimleri ve valid hareketleri temizle
    selectedSquare = sf::Vector2i(-1, -1);
    validMoves.clear();
}

// helper: to clear the board
//void Board::resetBoardToStart() {
//    // same in the Constructor (grid codes here)
//    // place the pieces and set the flags (kingmoved etc)
//}



void Board::handleKeyPress(sf::Keyboard::Key key) {
    // --- Navigasyon Okları ---
    if (key == sf::Keyboard::Left) {
        goToMove(currentMoveIndex - 1);
    }
    else if (key == sf::Keyboard::Right) {
        goToMove(currentMoveIndex + 1);
    }
    else if (key == sf::Keyboard::Up) {
        goToMove(-1); // En başa dön
    }
    else if (key == sf::Keyboard::Down) {
        goToMove((int)moveHistory.size() - 1); // En sona git
    }

    // --- Diğer Fonksiyonlar (Eskiden main'de olanlar) ---
    else if (key == sf::Keyboard::U) {
        undoMove();
        // Not: Undo hamleyi SİLER, Sol Ok ise sadece GERİ GİDER (silmez).
    }
    else if (key == sf::Keyboard::P) {
        exportPGN();
    }
    else if (key == sf::Keyboard::S) {
        savePGNToFile();
    }
    else if (key == sf::Keyboard::F) {
        flipBoard();
    }
    else if (key == sf::Keyboard::C) {
        toggleCoordinates();
    }
}

void Board::handleMouseDown(sf::Vector2f mPos) {
    // Convert screen coordinates to board grid indices
    int col = static_cast<int>((mPos.x - offset) / tileSize);
    int row = static_cast<int>((mPos.y - offset) / tileSize);

    // Adjust for flipped board view
    int logCol = isFlowFlipped ? (7 - col) : col;
    int logRow = isFlowFlipped ? (7 - row) : row;

    // Check bounds and ensure we are clicking on a piece of the current player's color
    if (logCol >= 0 && logCol < 8 && logRow >= 0 && logRow < 8) {
        PieceType clickedPiece = grid[logRow][logCol];
        if (clickedPiece != PieceType::Empty && isWhite(clickedPiece) == whiteTurn) {
            isDragging = true;
            draggedPieceSource = sf::Vector2i(logCol, logRow);

            // Set selection and calculate legal moves
            selectedSquare = draggedPieceSource;
            calculateValidMoves(logRow, logCol); // FIXED: Name matches your existing function
        }
    }
}

void Board::handleMouseClick(const sf::Vector2i mousePos) {
    if (gameOver) return;

    // Convert screen coordinates to grid indices
    int col = (mousePos.x - (int)offset) / (int)tileSize;
    int row = (mousePos.y - (int)offset) / (int)tileSize;

    // Adjust for flipped board view
    if (isFlowFlipped) { col = 7 - col; row = 7 - row; }

    // Out of bounds check
    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    // If a piece is already selected, try to move it
    if (selectedSquare != sf::Vector2i(-1, -1)) {
        PieceType movingPiece = grid[selectedSquare.y][selectedSquare.x];
        PieceType targetPiece = grid[row][col];

        // If the user clicks the SAME square, just keep it selected (do not deselect)
        // This is crucial for the Drag & Drop flow to work smoothly with clicks
        if (selectedSquare == sf::Vector2i(col, row)) {
            return;
        }

        // Check if the clicked target is a valid move
        if (isMoveValid(selectedSquare.y, selectedSquare.x, row, col)) {

            // --- ACTUAL CHESS LOGIC AND NOTATION GENERATION ---
            bool isCapture = (targetPiece != PieceType::Empty);
            char pieceChar = getPieceChar(movingPiece);
            std::string moveNotation = "";

            // En Passant Logic: Pawn moves diagonally but target is empty
            if ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && col != selectedSquare.x && targetPiece == PieceType::Empty) {
                isCapture = true;
                grid[selectedSquare.y][col] = PieceType::Empty; // Remove the captured pawn
            }

            // Build basic notation string
            if (pieceChar != ' ') moveNotation += pieceChar;
            else if (isCapture) moveNotation += (char)('a' + selectedSquare.x); // Pawn capture includes starting file

            if (isCapture) moveNotation += "x";

            moveNotation += (char)('a' + col);
            moveNotation += std::to_string(8 - row);

            // Castling Logic: King moves 2 squares
            if ((movingPiece == PieceType::W_King || movingPiece == PieceType::B_King) && std::abs(col - selectedSquare.x) == 2) {
                if (col == 6) {
                    // Kingside (Short) Castle
                    grid[row][5] = grid[row][7];
                    grid[row][7] = PieceType::Empty;
                    moveNotation = "O-O";
                }
                else {
                    // Queenside (Long) Castle
                    grid[row][3] = grid[row][0];
                    grid[row][0] = PieceType::Empty;
                    moveNotation = "O-O-O";
                }
            }

            // Update State Flags for castling rights and En Passant
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

            // Execute the primary piece move on the grid
            grid[row][col] = movingPiece;
            grid[selectedSquare.y][selectedSquare.x] = PieceType::Empty;

            // Update last move indicators for highlights
            lastMoveStart = selectedSquare;
            lastMoveEnd = sf::Vector2i(col, row);

            // Promotion Logic (Auto-Queen for now)
            if (movingPiece == PieceType::W_Pawn && row == 0) { grid[row][col] = PieceType::W_Queen; moveNotation += "=Q"; }
            if (movingPiece == PieceType::B_Pawn && row == 7) { grid[row][col] = PieceType::B_Queen; moveNotation += "=Q"; }

            // Check / Checkmate notation evaluation
            sf::Vector2i opponentKing = findKing(!isWhite(movingPiece));
            bool isCheck = isSquareAttacked(opponentKing.y, opponentKing.x, isWhite(movingPiece));
            if (isCheck) moveNotation += (!hasLegalMoves(!isWhite(movingPiece))) ? "#" : "+";

            // Record History for PGN and Undo
            MoveRecord record;
            record.start = selectedSquare;
            record.end = sf::Vector2i(col, row);
            record.movedPiece = movingPiece;
            record.capturedPiece = targetPiece;
            record.notation = moveNotation;
            record.isWhiteMove = isWhite(movingPiece);
            record.prevLastPawnDoubleMove = lastPawnDoubleMove;
            record.prevWhiteKingMoved = whiteKingMoved;
            record.prevBlackKingMoved = blackKingMoved;
            record.prevWhiteRook0Moved = whiteRook0Moved;
            record.prevWhiteRook7Moved = whiteRook7Moved;
            record.prevBlackRook0Moved = blackRook0Moved;
            record.prevBlackRook7Moved = blackRook7Moved;

            // Delete any alternate future if we travelled back in time
            if (currentMoveIndex < (int)moveHistory.size() - 1) {
                moveHistory.erase(moveHistory.begin() + currentMoveIndex + 1, moveHistory.end());
            }

            moveHistory.push_back(record);
            currentMoveIndex = (int)moveHistory.size() - 1;

            // Play appropriate sound effect
            if (isCheck || isCapture) captureSound.play();
            else moveSound.play();

            std::cout << (isWhite(movingPiece) ? "White moved: " : "Black moved: ") << moveNotation << std::endl;

            // Switch turn and check for game over
            whiteTurn = !whiteTurn;
            checkGameEnd();

            // Clear selection and valid moves after a successful turn
            validMoves.clear();
            selectedSquare = sf::Vector2i(-1, -1);
        }
        else {
            // If the user clicked an invalid square, check if they clicked another of their own pieces
            PieceType clickedPiece = grid[row][col];
            if (clickedPiece != PieceType::Empty && isWhite(clickedPiece) == whiteTurn) {
                // Change selection to the newly clicked friendly piece
                selectedSquare = sf::Vector2i(col, row);
                calculateValidMoves(row, col);
            }
            else {
                // Clicked an empty square or enemy piece invalidly: Deselect everything
                selectedSquare = sf::Vector2i(-1, -1);
                validMoves.clear();
            }
        }
    }
    else {
        // No piece is currently selected, try to select one
        PieceType clickedPiece = grid[row][col];
        if (clickedPiece != PieceType::Empty && isWhite(clickedPiece) == whiteTurn) {
            selectedSquare = sf::Vector2i(col, row);
            calculateValidMoves(row, col);
        }
    }
}

void Board::handleMouseUp(sf::Vector2f mPos) {
    // Only used to stop the visual dragging effect.
    // We do NOT reset selectedSquare here so the Click event can use it.
    isDragging = false;
    // draggedPieceSource = sf::Vector2i(-1, -1); // Keep this to know what we were dragging
}

