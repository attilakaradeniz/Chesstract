#define _CRT_SECURE_NO_WARNINGS
#include "Board.hpp"
#include <iostream>
#include <cmath> 
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#endif // _WIN32


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
    mousePos(0.f, 0.f),
    isPromoting(false),
    promotionSquare(-1, -1)
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
    //if (!piecesTexture.loadFromFile("assets/pieces_pixelled.png")) {
    //if (!piecesTexture.loadFromFile("assets/pieces_redgreen.png")) {
    //if (!piecesTexture.loadFromFile("assets/pieces_fenset.png")) {
    //if (!piecesTexture.loadFromFile("assets/pieces_fenset_60x60.png")) {
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

    // finalize Setup
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

// for fixing notation ambiguities (e.g. two knights can move to the same square, so we need to specify which one)
bool Board::needsDisambiguation(int startRow, int startCol, int endRow, int endCol, PieceType type) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            // Eğer aynı tipte, aynı renkte ama FARKLI bir taştan bahsediyorsak
            if (grid[r][c] == type && (r != startRow || c != startCol)) {
                // Bu diğer taş da aynı yere gidebiliyor mu?
                if (isMoveValid(r, c, endRow, endCol)) {
                    return true; // Evet, bir belirsizlik var!
                }
            }
        }
    }
    return false;
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
    //const int sourceSize = 60;

    const float scale = tileSize / (float)sourceSize;
    pieceSprite.setScale(scale, scale);

    static sf::Clock animationClock;
    float time = animationClock.getElapsedTime().asSeconds();
    float pulseScale = 1.0f + 0.10f * std::sin(time * 3.0f);

    // --- Check Status Calculation for Rendering ---
    sf::Vector2i wKingPos = findKing(true);
    sf::Vector2i bKingPos = findKing(false);

    bool isWCheck = isSquareAttacked(wKingPos.y, wKingPos.x, false); // White king attacked by Black
    bool isBCheck = isSquareAttacked(bKingPos.y, bKingPos.x, true);  // Black king attacked by White

    // --- chess board & pieces draw ---
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int renderRow = isFlowFlipped ? (7 - i) : i;
            int renderCol = isFlowFlipped ? (7 - j) : j;
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(235, 235, 210) : sf::Color(180, 50, 50));
            //square.setFillColor(((i + j) % 2 == 0) ? sf::Color(35, 35, 10) : sf::Color(80, 10, 10));
            //square.setFillColor(((i + j) % 2 == 0) ? sf::Color(110,110, 110) : sf::Color(50, 10, 10));
            window.draw(square);

            // stylish check glow
            float centerX = renderCol * tileSize + offset + tileSize / 2.0f;
            float centerY = renderRow * tileSize + offset + tileSize / 2.0f;

            // if White King is in check and this is his square
            if (isWCheck && sf::Vector2i(j, i) == wKingPos) {
                // draw 4 concentric circles to create a soft, degrading glow effect
                for (int step = 0; step < 4; ++step) {
                    float radius = (tileSize / 2.5f) + (step * 10.0f); // expands outward beyond the square
                    sf::CircleShape glow(radius);
                    glow.setOrigin(radius, radius);
                    glow.setPosition(centerX, centerY);

                    // decrease opacity as radius increases
                    glow.setFillColor(sf::Color(255, 0, 0, 150 - (step * 40)));
                    window.draw(glow);
                }
            }

            // If Black King is in check and this is his square
            if (isBCheck && sf::Vector2i(j, i) == bKingPos) {
                for (int step = 0; step < 4; ++step) {
                    float radius = (tileSize / 2.5f) + (step * 10.0f);
                    sf::CircleShape glow(radius);
                    glow.setOrigin(radius, radius);
                    glow.setPosition(centerX, centerY);

                    glow.setFillColor(sf::Color(255, 0, 0, 150 - (step * 40)));
                    window.draw(glow);
                }
            }

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

            // Inside the nested loop for drawing squares
            // for last move and turn indicator
            if (sf::Vector2i(j, i) == lastMoveStart || sf::Vector2i(j, i) == lastMoveEnd) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                // light yellow with some transparency
                highlight.setFillColor(sf::Color(255, 255, 0, 35));
                window.draw(highlight);
            }
        }
    }

    // draw dragged piece on top
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

    // draw dots and rings
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

    // side panel notation background
    sf::RectangleShape sidePanel(sf::Vector2f(300, 900));
    sidePanel.setPosition(900, 0); sidePanel.setFillColor(sf::Color(45, 45, 45));
    window.draw(sidePanel);

    // move history with scrolling
    sf::Text title("Move History", font, 24);
    title.setPosition(920, 20);
    window.draw(title);

    // Scroll 
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

        // Clipping, side panel for moves
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

    // pulsing turn indicator (like pingpong ball)
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

    // draw pawn promotion menu
    if (isPromoting) {
        // determine the color explicitly using the frozen turn state
        bool isWhitePromo = whiteTurn;
        // define the 4 promotion options in order (Queen, Rook, Bishop, Knight)
        PieceType options[4] = {
            isWhitePromo ? PieceType::W_Queen : PieceType::B_Queen,
            isWhitePromo ? PieceType::W_Rook : PieceType::B_Rook,
            isWhitePromo ? PieceType::W_Bishop : PieceType::B_Bishop,
            isWhitePromo ? PieceType::W_Knight : PieceType::B_Knight
        };

        // calculate rendering coordinates based on the promotion square
        int rRow = isFlowFlipped ? (7 - promotionSquare.y) : promotionSquare.y;
        int rCol = isFlowFlipped ? (7 - promotionSquare.x) : promotionSquare.x;

        float startX = rCol * tileSize + offset;
        float startY = rRow * tileSize + offset;

        // determine menu draw direction
        int drawDirection = (rRow < 4) ? 1 : -1;

        // draw the menu background
        sf::RectangleShape menuBg(sf::Vector2f(tileSize, tileSize * 4));
        if (drawDirection == -1) {
            menuBg.setPosition(startX, startY - (tileSize * 3));
        }
        else {
            menuBg.setPosition(startX, startY);
        }
        menuBg.setFillColor(sf::Color(240, 240, 240, 240));
        menuBg.setOutlineThickness(3.f);
        menuBg.setOutlineColor(sf::Color(50, 50, 50));
        window.draw(menuBg);

        // draw the 4 piece sprites inside the menu
        for (int i = 0; i < 4; ++i) {
            PieceSource src = pieceSourceMap[options[i]];
            pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));

            float pieceY = startY + (i * tileSize * drawDirection);
            pieceSprite.setPosition(startX, pieceY);
            window.draw(pieceSprite);
        }
    }

	//shortcuts legend at the bottom (UX)
        // text bar at the bottom
    std::string shortcuts = "Shortcuts: [Left/Right] Navigate History | [U] Undo | [S] Save PGN | [F] Flip Board | [C] Toggle Coordinates";
    sf::Text shortcutsText(shortcuts, font, 14);

    // grey color
    shortcutsText.setFillColor(sf::Color(150, 150, 150));

    // position it at the bottom left, just under the board
    shortcutsText.setPosition(offset, 8 * tileSize + offset + 15);

    window.draw(shortcutsText);

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

    // paradox fix
        // if we travelled back in time, delete the alternate future before undoing
    if (currentMoveIndex < (int)moveHistory.size() - 1) {
        moveHistory.erase(moveHistory.begin() + (currentMoveIndex + 1), moveHistory.end());
    }

    // safety check in case we erased everything back to the start
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
    currentMoveIndex--; // FIX: ensure index stays synced when manually undoing
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

void Board::exportPGN() {
    if (moveHistory.empty()) {
        std::cout << "No moves to export yet." << std::endl;
        return;
    }

    std::string pgn = "";
    int moveNumber = 1;

    // Standard PGN Header Tags
    pgn += "[Event \"Casual Game\"]\n";
    pgn += "[Site \"chesstracted\"]\n";

    // Get current date
    time_t now = time(0);
    tm* ltm = localtime(&now);
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
        }
        else {
            pgn += moveHistory[i].notation + " ";
            moveNumber++;
        }
    }

    pgn += result; // Append final result at the end of moves

    std::cout << "\n--- PGN OUTPUT ---\n" << pgn << "\n------------------\n" << std::endl;
}

void Board::savePGNToFile() {
    if (moveHistory.empty()) {
        std::cout << "No moves to export." << std::endl;
        return;
    }

    std::string filename = "";

#ifdef _WIN32
	// windoes save dialog
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };

    // Suggest a default name with the current date
    std::time_t t = std::time(nullptr);
    std::tm* ltm = std::localtime(&t);
    std::strftime(szFile, sizeof(szFile), "game_%Y%m%d_%H%M%S.pgn", ltm);

    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "PGN Files (*.pgn)\0*.pgn\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "pgn"; // Default extension

    // Display the Save As dialog box
    if (GetSaveFileNameA(&ofn) == TRUE) {
        filename = ofn.lpstrFile;
    }
    else {
        std::cout << "Save dialog canceled by user." << std::endl;
        return; // Exit if user closed the window
    }
#else
    // for mac/linux
    // if not on Windows, just save to the current working directory
    std::time_t t = std::time(nullptr);
    std::tm* ltm = std::localtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "game_%Y%m%d_%H%M%S.pgn", ltm);
    filename = buf;
#endif

	// write to selected file
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
    //reset all flags
    whiteTurn = true;
    gameOver = false;

    // FIX: Clear promotion states so undoing a promotion menu doesn't break the game
    isPromoting = false;
    promotionSquare = sf::Vector2i(-1, -1);

    whiteKingMoved = false;
    blackKingMoved = false;
    whiteRook0Moved = false;
    whiteRook7Moved = false;
    blackRook0Moved = false;
    blackRook7Moved = false;
    lastPawnDoubleMove = sf::Vector2i(-1, -1);
    // REMOVED moveHistory.clear(); from here! It caused the crash.

    // clear the board (Empty)
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            grid[i][j] = PieceType::Empty;
        }
    }

    // pieces openning plaeces
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

    // standard piece move or Promotion placement
    if (record.promotedTo != PieceType::Empty) {
        // Place the PROMOTED piece (Queen, Rook, etc.) on the final square
        grid[record.end.y][record.end.x] = record.promotedTo;
    }
    else {
        // standard piece move
        grid[record.end.y][record.end.x] = record.movedPiece;
    }

    grid[record.start.y][record.start.x] = PieceType::Empty;

    // en passant
    if ((record.movedPiece == PieceType::W_Pawn || record.movedPiece == PieceType::B_Pawn) &&
        record.start.x != record.end.x && record.capturedPiece == PieceType::Empty) {
        grid[record.start.y][record.end.x] = PieceType::Empty;
    }

    // castle 
    if ((record.movedPiece == PieceType::W_King || record.movedPiece == PieceType::B_King) &&
        std::abs(record.end.x - record.start.x) == 2) {
        if (record.end.x == 6) { // Short Castle (Kingside)
            grid[record.end.y][5] = grid[record.end.y][7];
            grid[record.end.y][7] = PieceType::Empty;
        }
        else { // Long Castle (Queenside)
            grid[record.end.y][3] = grid[record.end.y][0];
            grid[record.end.y][0] = PieceType::Empty;
        }
    }
}

void Board::goToMove(int targetIndex) {
    if (targetIndex < -1 || targetIndex >= (int)moveHistory.size()) return;

    resetBoardToStart();

    for (int i = 0; i <= targetIndex; ++i) {
        applyMoveIndependently(moveHistory[i]);
    }

    currentMoveIndex = targetIndex;

    if (currentMoveIndex == -1) {
        whiteTurn = true;
    }
    else {
        whiteTurn = !moveHistory[currentMoveIndex].isWhiteMove;
    }

    selectedSquare = sf::Vector2i(-1, -1);
    validMoves.clear();
}

void Board::handleKeyPress(sf::Keyboard::Key key) {
    //  block keyboard while promoting 
    if (isPromoting) return;

    // navigation arrows 
    if (key == sf::Keyboard::Left) {
        goToMove(currentMoveIndex - 1);
    }
    else if (key == sf::Keyboard::Right) {
        goToMove(currentMoveIndex + 1);
    }
    else if (key == sf::Keyboard::Up) {
		goToMove(-1); // starting position
    }
    else if (key == sf::Keyboard::Down) {
		goToMove((int)moveHistory.size() - 1); // last state (most recent move)
    }

    // shortcuts for other functions
    else if (key == sf::Keyboard::U) {
        undoMove();
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
    // block dragging while promoting
    if (isPromoting) return;

    int col = static_cast<int>((mPos.x - offset) / tileSize);
    int row = static_cast<int>((mPos.y - offset) / tileSize);

    int logCol = isFlowFlipped ? (7 - col) : col;
    int logRow = isFlowFlipped ? (7 - row) : row;

    if (logCol >= 0 && logCol < 8 && logRow >= 0 && logRow < 8) {
        PieceType clickedPiece = grid[logRow][logCol];
        if (clickedPiece != PieceType::Empty && isWhite(clickedPiece) == whiteTurn) {
            isDragging = true;
            draggedPieceSource = sf::Vector2i(logCol, logRow);

            selectedSquare = draggedPieceSource;
            calculateValidMoves(logRow, logCol);
        }
    }
}

void Board::handleMouseClick(const sf::Vector2i mousePos) {
    if (gameOver) return;

	// handle promotion menu clicks first before any other board interactions
    // If the game is waiting for a promotion choice, intercept the click for the menu
    if (isPromoting) {
        // reconstruct the menu bounds to check if the user clicked an option
        int rRow = isFlowFlipped ? (7 - promotionSquare.y) : promotionSquare.y;
        int rCol = isFlowFlipped ? (7 - promotionSquare.x) : promotionSquare.x;

        float startX = rCol * tileSize + offset;
        float startY = rRow * tileSize + offset;
        int drawDirection = (rRow < 4) ? 1 : -1;

        // check if the mouse X coordinate is within the menu column
        if (mousePos.x >= startX && mousePos.x <= startX + tileSize) {

            // check which of the 4 pieces was clicked based on Y coordinate
            for (int i = 0; i < 4; ++i) {
                float pieceY = startY + (i * tileSize * drawDirection);

                // if the click is inside this specific piece's bounding box
                if (mousePos.y >= pieceY && mousePos.y <= pieceY + tileSize) {

                    // FIX: Use the frozen turn state to determine promotion color reliably
                    bool isWhitePromo = whiteTurn;
                    PieceType selectedPiece;
                    char promoChar;

                    // Map the clicked index to the chosen piece (0: Queen, 1: Rook, 2: Bishop, 3: Knight)
                    if (i == 0) { selectedPiece = isWhitePromo ? PieceType::W_Queen : PieceType::B_Queen; promoChar = 'Q'; }
                    else if (i == 1) { selectedPiece = isWhitePromo ? PieceType::W_Rook : PieceType::B_Rook; promoChar = 'R'; }
                    else if (i == 2) { selectedPiece = isWhitePromo ? PieceType::W_Bishop : PieceType::B_Bishop; promoChar = 'B'; }
                    else { selectedPiece = isWhitePromo ? PieceType::W_Knight : PieceType::B_Knight; promoChar = 'N'; }

                    // promotion
                    // transform the pawn into the chosen piece on the grid
                    grid[promotionSquare.y][promotionSquare.x] = selectedPiece;

                    // append the promotion choice to the algebraic notation (e.g., "e8=R")
                    pendingPromotionMove.notation += "=";
                    pendingPromotionMove.notation += promoChar;

                    // save the chosen piece to our history record so undo/redo works! ---
                    pendingPromotionMove.promotedTo = selectedPiece;

                    // re-evaluate Check/Mate now that the NEW piece is on the board
                    sf::Vector2i opponentKing = findKing(!isWhitePromo);
                    bool isCheck = isSquareAttacked(opponentKing.y, opponentKing.x, isWhitePromo);
                    if (isCheck) pendingPromotionMove.notation += (!hasLegalMoves(!isWhitePromo)) ? "#" : "+";

                    // save the finalized move to history
                    if (currentMoveIndex < (int)moveHistory.size() - 1) {
                        moveHistory.erase(moveHistory.begin() + (currentMoveIndex + 1), moveHistory.end());
                    }
                    moveHistory.push_back(pendingPromotionMove);
                    currentMoveIndex = (int)moveHistory.size() - 1;

                    // play sound based on the final move state
                    if (isCheck || pendingPromotionMove.capturedPiece != PieceType::Empty) captureSound.play();
                    else moveSound.play();

                    std::cout << (isWhitePromo ? "White promoted: " : "Black promoted: ") << pendingPromotionMove.notation << std::endl;

                    // finalize the turn and close the menu
                    whiteTurn = !whiteTurn;
                    checkGameEnd();

                    isPromoting = false;
                    promotionSquare = sf::Vector2i(-1, -1);
                    return; // exit successfully, move is complete!
                }
            }
        }

        // if they clicked outside the menu squares, just return and force them to choose
        return;
    }

    // convert screen coordinates to grid indices
    int col = (mousePos.x - (int)offset) / (int)tileSize;
    int row = (mousePos.y - (int)offset) / (int)tileSize;

    // Adjust for flipped board view
    if (isFlowFlipped) { col = 7 - col; row = 7 - row; }

    // out of bounds check
    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    // if a piece is already selected, try to move it
    if (selectedSquare != sf::Vector2i(-1, -1)) {
        PieceType movingPiece = grid[selectedSquare.y][selectedSquare.x];
        PieceType targetPiece = grid[row][col];

        // If the user clicks the SAME square, just keep it selected
        if (selectedSquare == sf::Vector2i(col, row)) {
            return;
        }

        // Check if the clicked target is a valid move
        if (isMoveValid(selectedSquare.y, selectedSquare.x, row, col)) {

            // notation generation
            bool isCapture = (targetPiece != PieceType::Empty);
            char pieceChar = getPieceChar(movingPiece);
            std::string moveNotation = "";

            // En Passant Logic
            if ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && col != selectedSquare.x && targetPiece == PieceType::Empty) {
                isCapture = true;
                grid[selectedSquare.y][col] = PieceType::Empty;
            }

            // Build basic notation string
            //if (pieceChar != ' ') moveNotation += pieceChar;
            //else if (isCapture) moveNotation += (char)('a' + selectedSquare.x);

            //if (isCapture) moveNotation += "x";

            //moveNotation += (char)('a' + col);
            //moveNotation += std::to_string(8 - row);

            if (pieceChar != ' ') {
                moveNotation += pieceChar;

                // Check for ambiguity if the moving piece is a Knight or a Rook
                if (movingPiece == PieceType::W_Knight || movingPiece == PieceType::B_Knight ||
                    movingPiece == PieceType::W_Rook || movingPiece == PieceType::B_Rook)
                {
                    // Verify if another piece of the same type can reach the target square
                    if (needsDisambiguation(selectedSquare.y, selectedSquare.x, row, col, movingPiece)) {

                        bool sameFile = false;
                        // Iterate through the board to find the conflicting piece
                        for (int r = 0; r < 8; ++r) {
                            for (int c = 0; c < 8; ++c) {
                                // Look for another piece of the same type and color
                                if (grid[r][c] == movingPiece && (r != selectedSquare.y || c != selectedSquare.x)) {
                                    // If this other piece can also move to the target square
                                    if (isMoveValid(r, c, row, col)) {
                                        // Check if it's on the same file (column)
                                        if (c == selectedSquare.x) {
                                            sameFile = true;
                                        }
                                    }
                                }
                            }
                        }

                        if (sameFile) {
                            // If on the same file, use the rank (row) for disambiguation (e.g., R3h2)
                            moveNotation += std::to_string(8 - selectedSquare.y);
                        }
                        else {
                            // If on different files, use the file (column) letter (e.g., Ngf3)
                            moveNotation += (char)('a' + selectedSquare.x);
                        }
                    }
                }
            }
            else if (isCapture) {
                // For pawn captures, the starting file is always required (e.g., exd5)
                moveNotation += (char)('a' + selectedSquare.x);
            }

            if (isCapture) moveNotation += "x";
            moveNotation += (char)('a' + col);
            moveNotation += std::to_string(8 - row);
            // ---------------------------------------------------------


            // Record History for PGN and Undo (FIXED: Placed BEFORE early return to capture all data properly)
            MoveRecord record;
            record.promotedTo = PieceType::Empty; // Initialize empty, will change if promotion occurs
            record.start = selectedSquare;
            record.end = sf::Vector2i(col, row);
            record.movedPiece = movingPiece;
            record.capturedPiece = targetPiece;
            record.isWhiteMove = isWhite(movingPiece);
            record.prevLastPawnDoubleMove = lastPawnDoubleMove;
            record.prevWhiteKingMoved = whiteKingMoved;
            record.prevBlackKingMoved = blackKingMoved;
            record.prevWhiteRook0Moved = whiteRook0Moved;
            record.prevWhiteRook7Moved = whiteRook7Moved;
            record.prevBlackRook0Moved = blackRook0Moved;
            record.prevBlackRook7Moved = blackRook7Moved;

            // promotion     
            bool isPromotion = (movingPiece == PieceType::W_Pawn && row == 0) ||
                (movingPiece == PieceType::B_Pawn && row == 7);

            if (isPromotion) {
                // 1. Temporarily move the pawn to the final square
                grid[row][col] = movingPiece;
                grid[selectedSquare.y][selectedSquare.x] = PieceType::Empty;

                // save the base move notation
                record.notation = moveNotation;

                // store all the details of this pending move into our new variable
                pendingPromotionMove = record;

                // 4activate the Promotion State
                isPromoting = true;
                promotionSquare = sf::Vector2i(col, row);

                // clean up visuals
                selectedSquare = sf::Vector2i(-1, -1);
                validMoves.clear();
                lastMoveStart = pendingPromotionMove.start;
                lastMoveEnd = sf::Vector2i(col, row);

                // FREEZE THE GAME: Exit the function early!
                std::cout << "Game Paused: Waiting for promotion selection..." << std::endl;
                return;
            }

            // Castling Logic: King moves 2 squares
            if ((movingPiece == PieceType::W_King || movingPiece == PieceType::B_King) && std::abs(col - selectedSquare.x) == 2) {
                if (col == 6) {
                    grid[row][5] = grid[row][7];
                    grid[row][7] = PieceType::Empty;
                    moveNotation = "O-O";
                }
                else {
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

            // Check / Checkmate notation evaluation
            sf::Vector2i opponentKing = findKing(!isWhite(movingPiece));
            bool isCheck = isSquareAttacked(opponentKing.y, opponentKing.x, isWhite(movingPiece));
            if (isCheck) moveNotation += (!hasLegalMoves(!isWhite(movingPiece))) ? "#" : "+";

            record.notation = moveNotation;

            // Delete any alternate future if we travelled back in time
            if (currentMoveIndex < (int)moveHistory.size() - 1) {
                moveHistory.erase(moveHistory.begin() + (currentMoveIndex + 1), moveHistory.end());
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
                selectedSquare = sf::Vector2i(col, row);
                calculateValidMoves(row, col);
            }
            else {
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
    isDragging = false;
}