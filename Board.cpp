#define _CRT_SECURE_NO_WARNINGS
#include "sqlite3.h"
#include "Board.hpp"
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "User32.lib") // to use clipboard functions
#endif
#include <string.h>

Board::Board() :
    tileSize(100.0f),
    offset(50.0f),
    db("chess_collection.db"),
    scrollOffset(0.0f),
    isDragging(false),
    draggedPieceSource(-1, -1),
    mousePos(0.f, 0.f),
    isPromoting(false),
    promotionSquare(-1, -1)
{
    loadAssets();
    setupPieceSources();
    std::cout << "SQLite Version: " << sqlite3_libversion() << std::endl;
}

void Board::loadAssets() {
    if (!piecesTexture.loadFromFile("assets/pieces.png")) {
        std::cerr << "Error: Could not load assets/pieces.png!" << std::endl;
    }
    piecesTexture.setSmooth(true);
    pieceSprite.setTexture(piecesTexture);

    if (!font.loadFromFile("assets/arial.ttf")) {
        std::cerr << "Error: Could not load assets/arial.ttf!" << std::endl;
    }

    if (!moveBuffer.loadFromFile("assets/move.wav")) {
        std::cerr << "Error: Could not load assets/move.wav!" << std::endl;
    }
    moveSound.setBuffer(moveBuffer);

    if (!captureBuffer.loadFromFile("assets/capture.wav")) {
        std::cerr << "Error: Could not load assets/capture.wav!" << std::endl;
    }
    captureSound.setBuffer(captureBuffer);

    moveSound.setVolume(75.f);
    captureSound.setVolume(85.f);
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

void Board::draw(sf::RenderWindow& window) {
	// auto playback logic
    if (isPlayingBack) {
        if (playbackIndex < playbackMoves.size()) {
            // 0.2f = Hamleler arası bekleme süresi (0.4 saniye)
            if (playbackClock.getElapsedTime().asSeconds() > 0.4f) {
                if (!playNotationMove(playbackMoves[playbackIndex])) {
                    std::cerr << "Playback Error at move: " << playbackMoves[playbackIndex] << std::endl;
                    isPlayingBack = false; // Stop on error
                }
                playbackIndex++;
                playbackClock.restart();
            }
        }
        else {
            isPlayingBack = false; // playback finished!
            std::cout << ">>> Playback complete!" << std::endl;
        }
    }
	// end auto playback logic
    const int sourceSize = 45;
    const float scale = tileSize / (float)sourceSize;
    pieceSprite.setScale(scale, scale);

    static sf::Clock animationClock;
    float time = animationClock.getElapsedTime().asSeconds();
    float pulseScale = 1.0f + 0.10f * std::sin(time * 3.0f);

    sf::Vector2i wKingPos = gameRules.findKing(true);
    sf::Vector2i bKingPos = gameRules.findKing(false);

    bool isWCheck = gameRules.isSquareAttacked(wKingPos.y, wKingPos.x, false);
    bool isBCheck = gameRules.isSquareAttacked(bKingPos.y, bKingPos.x, true);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int renderRow = isFlowFlipped ? (7 - i) : i;
            int renderCol = isFlowFlipped ? (7 - j) : j;
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(235, 235, 210) : sf::Color(180, 50, 50));
            window.draw(square);

            float centerX = renderCol * tileSize + offset + tileSize / 2.0f;
            float centerY = renderRow * tileSize + offset + tileSize / 2.0f;

            if (isWCheck && sf::Vector2i(j, i) == wKingPos) {
                for (int step = 0; step < 4; ++step) {
                    float radius = (tileSize / 2.5f) + (step * 10.0f);
                    sf::CircleShape glow(radius);
                    glow.setOrigin(radius, radius);
                    glow.setPosition(centerX, centerY);
                    glow.setFillColor(sf::Color(255, 0, 0, 150 - (step * 40)));
                    window.draw(glow);
                }
            }

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

            PieceType type = gameRules.grid[i][j];
            if (type != PieceType::Empty) {
                if (!(isDragging && draggedPieceSource == sf::Vector2i(j, i))) {
                    PieceSource src = pieceSourceMap[type];
                    pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));
                    pieceSprite.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                    window.draw(pieceSprite);
                }
            }

            if (sf::Vector2i(j, i) == lastMoveStart || sf::Vector2i(j, i) == lastMoveEnd) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(renderCol * tileSize + offset, renderRow * tileSize + offset);
                highlight.setFillColor(sf::Color(255, 255, 0, 35));
                window.draw(highlight);
            }
        }
    }

    if (isDragging) {
        PieceType type = gameRules.grid[draggedPieceSource.y][draggedPieceSource.x];
        PieceSource src = pieceSourceMap[type];
        pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));
        pieceSprite.setOrigin(sourceSize / 2.0f, sourceSize / 2.0f);
        pieceSprite.setPosition(mousePos);
        window.draw(pieceSprite);
        pieceSprite.setOrigin(0, 0);
    }

    for (const auto& move : validMoves) {
        int rRow = isFlowFlipped ? (7 - move.y) : move.y;
        int rCol = isFlowFlipped ? (7 - move.x) : move.x;
        float centerX = rCol * tileSize + offset + tileSize / 2.0f;
        float centerY = rRow * tileSize + offset + tileSize / 2.0f;

        if (gameRules.grid[move.y][move.x] == PieceType::Empty) {
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

    sf::RectangleShape sidePanel(sf::Vector2f(300, 900));
    sidePanel.setPosition(900, 0); sidePanel.setFillColor(sf::Color(45, 45, 45));
    window.draw(sidePanel);

    sf::Text title("Move History", font, 24);
    title.setPosition(920, 20);
    window.draw(title);

    int activeRow = (gameRules.currentMoveIndex / 2);
    float targetY = 70 + (activeRow * 25);
    scrollOffset = (targetY > 800) ? targetY - 800 : 0;

    sf::Text moveText("", font, 18);
    for (size_t i = 0; i < gameRules.moveHistory.size(); ++i) {
        int turnNumber = (i / 2) + 1;
        float xPos = (i % 2 == 0) ? 930 : 1060;
        float yPos = 70 + ((turnNumber - 1) * 25) - scrollOffset;

        if (yPos > 60 && yPos < 860) {
            std::string moveStr = (i % 2 == 0) ? (std::to_string(turnNumber) + ". " + gameRules.moveHistory[i].notation) : gameRules.moveHistory[i].notation;
            moveText.setString(moveStr);
            moveText.setPosition(xPos, yPos);
            moveText.setFillColor(((int)i == gameRules.currentMoveIndex) ? sf::Color::Yellow : sf::Color::White);
            window.draw(moveText);
        }
    }

    float currentRadius = 15.f * pulseScale;
    sf::CircleShape top(currentRadius);
    top.setOrigin(currentRadius, currentRadius);
    top.setPosition(8 * tileSize + offset + 25, 4 * tileSize + offset);
    top.setFillColor(gameRules.whiteTurn ? sf::Color(220, 220, 220) : sf::Color(30, 30, 30));
    window.draw(top);

    if (gameRules.gameOver) {
        sf::RectangleShape overlay(sf::Vector2f(tileSize * 8, tileSize * 8));
        overlay.setPosition(offset, offset); overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);
        sf::Text endMsg(gameRules.resultText, font, 50);
        sf::FloatRect tr = endMsg.getLocalBounds();
        endMsg.setOrigin(tr.left + tr.width / 2.0f, tr.top + tr.height / 2.0f);
        endMsg.setPosition((tileSize * 8) / 2.0f + offset, (tileSize * 8) / 2.0f + offset);
        window.draw(endMsg);
    }

    if (showCoordinates) {
        for (int i = 0; i < 8; ++i) {
            sf::Text cT("", font, 14);
            int logR = isFlowFlipped ? (7 - i) : i;
            cT.setString(std::to_string(8 - logR));
            cT.setPosition(offset + 2, i * tileSize + offset + 2); window.draw(cT);
            int logC = isFlowFlipped ? (7 - i) : i;
            std::string l = ""; l += (char)('a' + logC);
            cT.setString(l);
            cT.setPosition(i * tileSize + offset + tileSize - 15, 7 * tileSize + offset + tileSize - 20); window.draw(cT);
        }
    }

    if (isPromoting) {
        bool isWhitePromo = gameRules.whiteTurn;
        PieceType options[4] = {
            isWhitePromo ? PieceType::W_Queen : PieceType::B_Queen,
            isWhitePromo ? PieceType::W_Rook : PieceType::B_Rook,
            isWhitePromo ? PieceType::W_Bishop : PieceType::B_Bishop,
            isWhitePromo ? PieceType::W_Knight : PieceType::B_Knight
        };
        int rRow = isFlowFlipped ? (7 - promotionSquare.y) : promotionSquare.y;
        int rCol = isFlowFlipped ? (7 - promotionSquare.x) : promotionSquare.x;
        float startX = rCol * tileSize + offset;
        float startY = rRow * tileSize + offset;
        int drawDirection = (rRow < 4) ? 1 : -1;

        sf::RectangleShape menuBg(sf::Vector2f(tileSize, tileSize * 4));
        menuBg.setPosition(startX, (drawDirection == -1) ? startY - (tileSize * 3) : startY);
        menuBg.setFillColor(sf::Color(240, 240, 240, 240));
        window.draw(menuBg);

        for (int i = 0; i < 4; ++i) {
            PieceSource src = pieceSourceMap[options[i]];
            pieceSprite.setTextureRect(sf::IntRect(src.col * sourceSize, src.row * sourceSize, sourceSize, sourceSize));
            pieceSprite.setPosition(startX, startY + (i * tileSize * drawDirection));
            window.draw(pieceSprite);
        }
    }

    std::string shortcuts = "Shortcuts: [Left/Right] Navigate History | [U] Undo | [S] Save PGN | [F] Flip Board | [C] Toggle Coordinates";
    sf::Text shortcutsText(shortcuts, font, 14);
    shortcutsText.setFillColor(sf::Color(150, 150, 150));
    shortcutsText.setPosition(offset, 8 * tileSize + offset + 15);
    window.draw(shortcutsText);
}

bool Board::processPromotionClick(sf::Vector2i mousePos) {
    // promoiton menu handling 
    if (isPromoting) {
        int rRow = isFlowFlipped ? (7 - promotionSquare.y) : promotionSquare.y;
        int rCol = isFlowFlipped ? (7 - promotionSquare.x) : promotionSquare.x;
        float startX = rCol * tileSize + offset;
        float startY = rRow * tileSize + offset;
        int drawDirection = (rRow < 4) ? 1 : -1;
		//return true; // to prevent any other click processing while promotion menu is active

        if (mousePos.x >= startX && mousePos.x <= startX + tileSize) {
            for (int i = 0; i < 4; ++i) {
                float pieceY = startY + (i * tileSize * drawDirection);
                if (mousePos.y >= pieceY && mousePos.y <= pieceY + tileSize) {
                    bool isWhitePromo = gameRules.whiteTurn;
                    PieceType selectedPiece; char promoChar;
                    if (i == 0) { selectedPiece = isWhitePromo ? PieceType::W_Queen : PieceType::B_Queen; promoChar = 'Q'; }
                    else if (i == 1) { selectedPiece = isWhitePromo ? PieceType::W_Rook : PieceType::B_Rook; promoChar = 'R'; }
                    else if (i == 2) { selectedPiece = isWhitePromo ? PieceType::W_Bishop : PieceType::B_Bishop; promoChar = 'B'; }
                    else { selectedPiece = isWhitePromo ? PieceType::W_Knight : PieceType::B_Knight; promoChar = 'N'; }

                    gameRules.grid[promotionSquare.y][promotionSquare.x] = selectedPiece;
                    gameRules.pendingPromotionMove.notation += "=";
                    gameRules.pendingPromotionMove.notation += promoChar;
                    gameRules.pendingPromotionMove.promotedTo = selectedPiece;

                    sf::Vector2i opponentKing = gameRules.findKing(!isWhitePromo);
                    bool isCheck = gameRules.isSquareAttacked(opponentKing.y, opponentKing.x, isWhitePromo);
                    if (isCheck) gameRules.pendingPromotionMove.notation += (!gameRules.hasLegalMoves(!isWhitePromo)) ? "#" : "+";

                    if (gameRules.currentMoveIndex < (int)gameRules.moveHistory.size() - 1) {
                        gameRules.moveHistory.erase(gameRules.moveHistory.begin() + (gameRules.currentMoveIndex + 1), gameRules.moveHistory.end());
                    }
                    gameRules.moveHistory.push_back(gameRules.pendingPromotionMove);
                    gameRules.currentMoveIndex = (int)gameRules.moveHistory.size() - 1;

                    if (isCheck || gameRules.pendingPromotionMove.capturedPiece != PieceType::Empty) captureSound.play();
                    else moveSound.play();

                    std::cout << (isWhitePromo ? "White promoted: " : "Black promoted: ") << gameRules.pendingPromotionMove.notation << std::endl;

                    gameRules.whiteTurn = !gameRules.whiteTurn;
                    gameRules.checkGameEnd();
                    isPromoting = false;
                    promotionSquare = sf::Vector2i(-1, -1);
                    // for showing the latest PGN in console after promotion
                    exportPGN();

                    return true;
                }
            }
        }
        return true;
    }
	return false;
}

void Board::executeMove(int row, int col, PieceType movingPiece, PieceType targetPiece, std::string moveNotation, bool isCapture) {

    // Create Move Record
    MoveRecord record;
    record.start = selectedSquare; record.end = sf::Vector2i(col, row);
    record.movedPiece = movingPiece; record.capturedPiece = targetPiece;
    record.isWhiteMove = gameRules.isWhite(movingPiece);
    record.prevLastPawnDoubleMove = gameRules.lastPawnDoubleMove;
    record.prevWhiteKingMoved = gameRules.whiteKingMoved; record.prevBlackKingMoved = gameRules.blackKingMoved;
    record.prevWhiteRook0Moved = gameRules.whiteRook0Moved; record.prevWhiteRook7Moved = gameRules.whiteRook7Moved;
    record.prevBlackRook0Moved = gameRules.blackRook0Moved; record.prevBlackRook7Moved = gameRules.blackRook7Moved;

    // Check for Promotion
    bool isPromotion = (movingPiece == PieceType::W_Pawn && row == 0) || (movingPiece == PieceType::B_Pawn && row == 7);
    if (isPromotion) {
        gameRules.grid[row][col] = movingPiece;
        gameRules.grid[selectedSquare.y][selectedSquare.x] = PieceType::Empty;
        record.notation = moveNotation;
        gameRules.pendingPromotionMove = record;
        isPromoting = true; promotionSquare = sf::Vector2i(col, row);
        selectedSquare = sf::Vector2i(-1, -1); validMoves.clear();
        lastMoveStart = record.start; lastMoveEnd = record.end;
        std::cout << "Game Paused: Waiting for promotion selection..." << std::endl;
        return;
    }

    // Castling Logic
    if ((movingPiece == PieceType::W_King || movingPiece == PieceType::B_King) && std::abs(col - selectedSquare.x) == 2) {
        if (col == 6) { gameRules.grid[row][5] = gameRules.grid[row][7]; gameRules.grid[row][7] = PieceType::Empty; moveNotation = "O-O"; }
        else { gameRules.grid[row][3] = gameRules.grid[row][0]; gameRules.grid[row][0] = PieceType::Empty; moveNotation = "O-O-O"; }
    }

    // Update Castling/Pawn Flags
    if (movingPiece == PieceType::W_King) gameRules.whiteKingMoved = true;
    if (movingPiece == PieceType::B_King) gameRules.blackKingMoved = true;
    if (selectedSquare == sf::Vector2i(0, 0)) gameRules.blackRook0Moved = true;
    if (selectedSquare == sf::Vector2i(7, 0)) gameRules.blackRook7Moved = true;
    if (selectedSquare == sf::Vector2i(0, 7)) gameRules.whiteRook0Moved = true;
    if (selectedSquare == sf::Vector2i(7, 7)) gameRules.whiteRook7Moved = true;
    gameRules.lastPawnDoubleMove = ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && std::abs(row - selectedSquare.y) == 2) ? sf::Vector2i(col, row) : sf::Vector2i(-1, -1);

    // Execute Move
    gameRules.grid[row][col] = movingPiece;
    gameRules.grid[selectedSquare.y][selectedSquare.x] = PieceType::Empty;

    // Check for Check/Mate
    sf::Vector2i opponentKing = gameRules.findKing(!gameRules.isWhite(movingPiece));
    bool isCheck = gameRules.isSquareAttacked(opponentKing.y, opponentKing.x, gameRules.isWhite(movingPiece));
    if (isCheck) moveNotation += (!gameRules.hasLegalMoves(!gameRules.isWhite(movingPiece))) ? "#" : "+";
    record.notation = moveNotation;

    // Save to History
    if (gameRules.currentMoveIndex < (int)gameRules.moveHistory.size() - 1) {
        gameRules.moveHistory.erase(gameRules.moveHistory.begin() + (gameRules.currentMoveIndex + 1), gameRules.moveHistory.end());
    }
    gameRules.moveHistory.push_back(record);
    gameRules.currentMoveIndex = (int)gameRules.moveHistory.size() - 1;

    // UI feedback: Console, Sound, Highlights
    std::cout << (gameRules.isWhite(movingPiece) ? "White moved: " : "Black moved: ") << moveNotation << std::endl;
    if (isCheck || isCapture) captureSound.play();
    else moveSound.play();

    lastMoveStart = selectedSquare;
    lastMoveEnd = sf::Vector2i(col, row);

    // finalize (Switch side and clean up selection)
    gameRules.whiteTurn = !gameRules.whiteTurn;
    gameRules.checkGameEnd();
    validMoves.clear();
    selectedSquare = sf::Vector2i(-1, -1);
    // for showing the latest PGN in console after every move
    exportPGN();

}

std::string Board::buildNotation(sf::Vector2i start, sf::Vector2i end, PieceType movingPiece, PieceType targetPiece) {
    bool isCapture = (targetPiece != PieceType::Empty);
    char pieceChar = getPieceChar(movingPiece);
    std::string moveNotation = "";

    // En Passant Capture Logic (Notation context)
    if ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && end.x != start.x && targetPiece == PieceType::Empty) {
        isCapture = true;
    }

    if (pieceChar != ' ') {
        moveNotation += pieceChar;
        if (movingPiece == PieceType::W_Knight || movingPiece == PieceType::B_Knight ||
            movingPiece == PieceType::W_Rook || movingPiece == PieceType::B_Rook) {
            if (gameRules.needsDisambiguation(start.y, start.x, end.y, end.x, movingPiece)) {
                bool sameFile = false;
                for (int r = 0; r < 8; ++r) {
                    for (int c = 0; c < 8; ++c) {
                        if (gameRules.grid[r][c] == movingPiece && (r != start.y || c != start.x)) {
                            if (gameRules.isMoveValid(r, c, end.y, end.x) && c == start.x) sameFile = true;
                        }
                    }
                }
                if (sameFile) moveNotation += std::to_string(8 - start.y);
                else moveNotation += (char)('a' + start.x);
            }
        }
    }
    else if (isCapture) moveNotation += (char)('a' + start.x);

    if (isCapture) moveNotation += "x";
    moveNotation += (char)('a' + end.x);
    moveNotation += std::to_string(8 - end.y);

    return moveNotation;
}

void Board::handleMouseClick(const sf::Vector2i mousePos) {
    if (gameRules.gameOver) return;

    // Hande Promotion menu function here
	if (processPromotionClick(mousePos)) return;

    // regular move handling
    int col = (mousePos.x - (int)offset) / (int)tileSize;
    int row = (mousePos.y - (int)offset) / (int)tileSize;
    if (isFlowFlipped) { col = 7 - col; row = 7 - row; }
    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    if (selectedSquare != sf::Vector2i(-1, -1)) {
        PieceType movingPiece = gameRules.grid[selectedSquare.y][selectedSquare.x];
        PieceType targetPiece = gameRules.grid[row][col];
        if (selectedSquare == sf::Vector2i(col, row)) return;

        if (gameRules.isMoveValid(selectedSquare.y, selectedSquare.x, row, col)) {
            // build notation function call here 
			std::string moveNotation = buildNotation(selectedSquare, sf::Vector2i(col, row), movingPiece, targetPiece);
            bool isCapture = (targetPiece != PieceType::Empty) ||
                ((movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn) && col != selectedSquare.x);

            // if it's en-passant, clear the pawn from the grid manually before executeMove
            if (isCapture && targetPiece == PieceType::Empty && (movingPiece == PieceType::W_Pawn || movingPiece == PieceType::B_Pawn)) {
                gameRules.grid[selectedSquare.y][col] = PieceType::Empty;
            }

            // execute move func call here 
			executeMove(row, col, movingPiece, targetPiece, moveNotation, isCapture);

        }
        else {
            // reselect if another piece of the same color is clicked
            PieceType clickedPiece = gameRules.grid[row][col];
            if (clickedPiece != PieceType::Empty && gameRules.isWhite(clickedPiece) == gameRules.whiteTurn) {
                selectedSquare = sf::Vector2i(col, row); calculateValidMoves(row, col);
            }
            else {
                selectedSquare = sf::Vector2i(-1, -1); validMoves.clear();
            }
        }
    }
    else {
        // initial selection
        PieceType clickedPiece = gameRules.grid[row][col];
        if (clickedPiece != PieceType::Empty && gameRules.isWhite(clickedPiece) == gameRules.whiteTurn) {
            selectedSquare = sf::Vector2i(col, row); calculateValidMoves(row, col);
        }
    }
}



void Board::undoMove() {
    if (gameRules.moveHistory.empty()) return;
    if (gameRules.currentMoveIndex < (int)gameRules.moveHistory.size() - 1) {
        gameRules.moveHistory.erase(gameRules.moveHistory.begin() + (gameRules.currentMoveIndex + 1), gameRules.moveHistory.end());
    }
    if (gameRules.moveHistory.empty()) return;

    MoveRecord last = gameRules.moveHistory.back();
    gameRules.moveHistory.pop_back();

    gameRules.grid[last.start.y][last.start.x] = last.movedPiece;
    gameRules.grid[last.end.y][last.end.x] = last.capturedPiece;

    if ((last.movedPiece == PieceType::W_Pawn || last.movedPiece == PieceType::B_Pawn) && last.start.x != last.end.x && last.capturedPiece == PieceType::Empty)
        gameRules.grid[last.start.y][last.end.x] = last.isWhiteMove ? PieceType::B_Pawn : PieceType::W_Pawn;

    if ((last.movedPiece == PieceType::W_King || last.movedPiece == PieceType::B_King) && std::abs(last.end.x - last.start.x) == 2) {
        if (last.end.x == 6) { gameRules.grid[last.end.y][7] = gameRules.grid[last.end.y][5]; gameRules.grid[last.end.y][5] = PieceType::Empty; }
        else { gameRules.grid[last.end.y][0] = gameRules.grid[last.end.y][3]; gameRules.grid[last.end.y][3] = PieceType::Empty; }
    }

    gameRules.lastPawnDoubleMove = last.prevLastPawnDoubleMove;
    gameRules.whiteKingMoved = last.prevWhiteKingMoved; gameRules.blackKingMoved = last.prevBlackKingMoved;
    gameRules.whiteRook0Moved = last.prevWhiteRook0Moved; gameRules.whiteRook7Moved = last.prevWhiteRook7Moved;
    gameRules.blackRook0Moved = last.prevBlackRook0Moved; gameRules.blackRook7Moved = last.prevBlackRook7Moved;
    gameRules.whiteTurn = last.isWhiteMove; gameRules.gameOver = false;
    gameRules.currentMoveIndex--;
    moveSound.play();
}

void Board::applyMoveIndependently(const MoveRecord& record) {
    if (record.promotedTo != PieceType::Empty) gameRules.grid[record.end.y][record.end.x] = record.promotedTo;
    else gameRules.grid[record.end.y][record.end.x] = record.movedPiece;
    gameRules.grid[record.start.y][record.start.x] = PieceType::Empty;
    if ((record.movedPiece == PieceType::W_Pawn || record.movedPiece == PieceType::B_Pawn) && record.start.x != record.end.x && record.capturedPiece == PieceType::Empty) gameRules.grid[record.start.y][record.end.x] = PieceType::Empty;
    if ((record.movedPiece == PieceType::W_King || record.movedPiece == PieceType::B_King) && std::abs(record.end.x - record.start.x) == 2) {
        if (record.end.x == 6) { gameRules.grid[record.end.y][5] = gameRules.grid[record.end.y][7]; gameRules.grid[record.end.y][7] = PieceType::Empty; }
        else { gameRules.grid[record.end.y][3] = gameRules.grid[record.end.y][0]; gameRules.grid[record.end.y][0] = PieceType::Empty; }
    }
}

void Board::goToMove(int targetIndex) {
    if (targetIndex < -1 || targetIndex >= (int)gameRules.moveHistory.size()) return;
    gameRules.resetBoardToStart();
    for (int i = 0; i <= targetIndex; ++i) applyMoveIndependently(gameRules.moveHistory[i]);
    gameRules.currentMoveIndex = targetIndex;
    gameRules.whiteTurn = (gameRules.currentMoveIndex == -1) ? true : !gameRules.moveHistory[gameRules.currentMoveIndex].isWhiteMove;
    selectedSquare = sf::Vector2i(-1, -1); validMoves.clear();
	//moveSound.play();
}

void Board::calculateValidMoves(int startRow, int startCol) {
    validMoves.clear();
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (gameRules.isMoveValid(startRow, startCol, r, c)) validMoves.push_back(sf::Vector2i(c, r));
        }
    }
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

void Board::flipBoard() { isFlowFlipped = !isFlowFlipped; }
void Board::toggleCoordinates() { showCoordinates = !showCoordinates; }
void Board::printStatus() { std::cout << "Current turn: " << (gameRules.whiteTurn ? "White" : "Black") << std::endl; }

void Board::handleKeyPress(sf::Keyboard::Key key) {
    if (isPromoting) return;
    if (key == sf::Keyboard::Left) goToMove(gameRules.currentMoveIndex - 1);
    else if (key == sf::Keyboard::Right) goToMove(gameRules.currentMoveIndex + 1);
    else if (key == sf::Keyboard::Up) goToMove(-1);
    else if (key == sf::Keyboard::Down) goToMove((int)gameRules.moveHistory.size() - 1);
    else if (key == sf::Keyboard::U) undoMove();
    else if (key == sf::Keyboard::P) exportPGN();
    else if (key == sf::Keyboard::S) {

        // new full formatted pgn form string
        std::string fullPgn = getFullPGNString();

        savePGNToFile();
        //      std::string white = "Player 1";
              //std::string black = "Player 2";
        std::string white = "White Player";
        std::string black = "Black Player";
        //        db.saveGame(white, black, "Result Pending", gameRules.moveHistory, "Full PGN String placeholder");
        db.saveGame(white, black, "Result Pending", gameRules.moveHistory, fullPgn);
    }
    else if (key == sf::Keyboard::K) {
        // extract moves in a format suitable for Lichess/Chess.com import
        std::string rawMoves = "";
        int moveNum = 1;
        for (size_t i = 0; i < gameRules.moveHistory.size(); ++i) {
            if (gameRules.moveHistory[i].isWhiteMove) {
                rawMoves += std::to_string(moveNum) + ". " + gameRules.moveHistory[i].notation + " ";
            }
            else {
                rawMoves += gameRules.moveHistory[i].notation + " ";
                moveNum++;
            }
        }
        // add the result at the end
        std::string res = "*";
        if (gameRules.gameOver) {
            if (gameRules.resultText.find("WHITE WINS") != std::string::npos) res = "1-0";
            else if (gameRules.resultText.find("BLACK WINS") != std::string::npos) res = "0-1";
            else if (gameRules.resultText.find("DRAW") != std::string::npos) res = "1/2-1/2";
        }
        rawMoves += res;

        copyToClipboard(rawMoves);
    }
    else if (key == sf::Keyboard::L) {
        // search the database for games starting with "e4"
        std::string searchParam = "e4";
        std::vector<GameEntry> foundGames = db.searchByOpening(searchParam);

        if (!foundGames.empty()) {
            // get the ID of the LATEST game added 
            int latestGameId = foundGames.back().id;

            GameEntry gameToLoad = db.getGameById(latestGameId);

            if (gameToLoad.id != -1) {
                std::cout << "\n>>> Loading Game ID: " << gameToLoad.id << " from database..." << std::endl;
                std::cout << "White: " << gameToLoad.whitePlayer << " | Black: " << gameToLoad.blackPlayer << std::endl;

                // 1. prepare the board for a new game
                gameRules.resetBoardToStart();
                validMoves.clear();
                selectedSquare = sf::Vector2i(-1, -1);
                lastMoveStart = sf::Vector2i(-1, -1);
                lastMoveEnd = sf::Vector2i(-1, -1);

                // start the cinematic playback!
                playbackMoves = extractMovesFromPGN(gameToLoad.fullPgn);
                playbackIndex = 0;
                isPlayingBack = true;
                playbackClock.restart(); // Start the timer

                std::cout << "Starting playback of " << playbackMoves.size() << " moves..." << std::endl;

                std::cout << ">>> Game loading complete!" << std::endl;
            }
        }
        else {
            std::cout << "No games found starting with e4. Please save a game first (S key)." << std::endl;
        }
    }
    else if (key == sf::Keyboard::F) flipBoard();
    else if (key == sf::Keyboard::C) toggleCoordinates();
}

void Board::handleMouseDown(sf::Vector2f mPos) {
    if (isPromoting) return;
    int col = static_cast<int>((mPos.x - offset) / tileSize);
    int row = static_cast<int>((mPos.y - offset) / tileSize);
    int logCol = isFlowFlipped ? (7 - col) : col;
    int logRow = isFlowFlipped ? (7 - row) : row;
    if (logCol >= 0 && logCol < 8 && logRow >= 0 && logRow < 8) {
        PieceType clickedPiece = gameRules.grid[logRow][logCol];
        if (clickedPiece != PieceType::Empty && gameRules.isWhite(clickedPiece) == gameRules.whiteTurn) {
            isDragging = true; draggedPieceSource = sf::Vector2i(logCol, logRow);
            selectedSquare = draggedPieceSource; calculateValidMoves(logRow, logCol);
        }
    }
}

void Board::handleMouseUp(sf::Vector2f mPos) { isDragging = false; }

void Board::exportPGN() {
    if (gameRules.moveHistory.empty()) return;
    std::string pgn = "[Event \"Casual Game\"]\n[Site \"chesstracted\"]\n";
    time_t now = time(0); tm* ltm = localtime(&now);
    pgn += "[Date \"" + std::to_string(1900 + ltm->tm_year) + "." + std::to_string(1 + ltm->tm_mon) + "." + std::to_string(ltm->tm_mday) + "\"]\n";
    pgn += "[White \"Player 1\"]\n[Black \"Player 2\"]\n";
    std::string result = "*";
    if (gameRules.gameOver) {
        if (gameRules.resultText.find("WHITE WINS") != std::string::npos) result = "1-0";
        else if (gameRules.resultText.find("BLACK WINS") != std::string::npos) result = "0-1";
        else if (gameRules.resultText.find("DRAW") != std::string::npos) result = "1/2-1/2";
    }
    pgn += "[Result \"" + result + "\"]\n\n";
    int moveNumber = 1;
    for (size_t i = 0; i < gameRules.moveHistory.size(); ++i) {
        if (gameRules.moveHistory[i].isWhiteMove) pgn += std::to_string(moveNumber) + ". " + gameRules.moveHistory[i].notation + " ";
        else { pgn += gameRules.moveHistory[i].notation + " "; moveNumber++; }
    }
    pgn += result;
    std::cout << "\n--- PGN OUTPUT ---\n" << pgn << "\n------------------\n" << std::endl;
}

void Board::savePGNToFile() {
    if (gameRules.moveHistory.empty()) return;

    std::string filename = "";

#ifdef _WIN32
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    time_t t = time(0);
    tm* ltm = localtime(&t);

    strftime(szFile, sizeof(szFile), "game_%Y%m%d_%H%M%S.pgn", ltm);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "PGN Files (*.pgn)\0*.pgn\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "pgn";

    if (GetSaveFileNameA(&ofn) == TRUE) {
        filename = ofn.lpstrFile;
    }
    else {
        return;
    }
#else
    time_t t = time(0);
    tm* ltm = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "game_%Y%m%d_%H%M%S.pgn", ltm);
    filename = buf;
#endif

    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        // Use the centralized helper function to get the full formatted PGN
        // This ensures the file contains headers (Event, Site, Date, etc.) 
        // as well as the move sequence and result.
        outFile << getFullPGNString();

        outFile.close();
        std::cout << "PGN exported to file successfully." << std::endl;
    }
}

std::string Board::getFullPGNString() {
    if (gameRules.moveHistory.empty()) return "";

    // get current local time 
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm ltm;

    // use safe version of localtime based on platform
#ifdef _WIN32
    localtime_s(&ltm, &now_c);
#else
    ltm = *std::localtime(&now_c);
#endif

    std::string p = "";
    p += "[Event \"Casual Game\"]\n";
    p += "[Site \"Chesstracted\"]\n";

    // format date as YYYY.MM.DD
    std::stringstream dateStream;
    dateStream << std::put_time(&ltm, "%Y.%m.%d");
    p += "[Date \"" + dateStream.str() + "\"]\n";

    // add time header 
    std::stringstream timeStream;
    timeStream << std::put_time(&ltm, "%H:%M:%S");
    p += "[Time \"" + timeStream.str() + "\"]\n";

    p += "[White \"White Player\"]\n";
    p += "[Black \"Black Player\"]\n";

    // determine result
    std::string res = "*";
    if (gameRules.gameOver) {
        if (gameRules.resultText.find("WHITE WINS") != std::string::npos) res = "1-0";
        else if (gameRules.resultText.find("BLACK WINS") != std::string::npos) res = "0-1";
        else if (gameRules.resultText.find("DRAW") != std::string::npos) res = "1/2-1/2";
    }
    p += "[Result \"" + res + "\"]\n\n";

    // build move list
    int moveNum = 1;
    for (size_t i = 0; i < gameRules.moveHistory.size(); ++i) {
        if (gameRules.moveHistory[i].isWhiteMove) {
            p += std::to_string(moveNum) + ". " + gameRules.moveHistory[i].notation + " ";
        }
        else {
            p += gameRules.moveHistory[i].notation + " ";
            moveNum++;
        }
    }

    // append final result to the move list
    p += res + "\n";

    return p;
}

void Board::copyToClipboard(const std::string& text) {
#ifdef _WIN32
    // open the system clipboard
    if (!OpenClipboard(nullptr)) return;

    // clear current contents
    EmptyClipboard();

    // allocate global memory for the text
    size_t size = text.size() + 1;
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hg) {
        CloseClipboard();
        return;
    }

    // lock memory and copy the string
    memcpy(GlobalLock(hg), text.c_str(), size);
    GlobalUnlock(hg);

    // Set clipboard data and close
    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();

    std::cout << ">>> Moves copied to clipboard!" << std::endl;
#endif
}

std::vector<std::string> Board::extractMovesFromPGN(const std::string& rawPgn) {
    std::vector<std::string> pureMoves;
    std::string cleanedStr = "";
    bool inHeader = false;

    // remove all headers (anything between '[' and ']')
    for (char c : rawPgn) {
        if (c == '[') inHeader = true;
        if (!inHeader) cleanedStr += c;
        if (c == ']') inHeader = false;
    }

    // tokenize the remaining string by spaces
    std::stringstream ss(cleanedStr);
    std::string token;

    while (ss >> token) {
        // ignore move numbers (e.g., "1.", "12.")
        if (token.find('.') != std::string::npos) continue;

        // ignore game results
        if (token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*") continue;

        // if it passed the checks, it's a valid move notation!
        pureMoves.push_back(token);
    }

    return pureMoves;
}

bool Board::playNotationMove(const std::string& move) {
    if (move.empty()) return false;

    std::string cleanMove = move;

    // strip check/mate symbols (+, ++, #) safely
    while (!cleanMove.empty() && (cleanMove.back() == '+' || cleanMove.back() == '#')) {
        cleanMove.pop_back();
    }

    // handle Promotion (e.g., h8=Q)
    PieceType promotedPiece = PieceType::Empty;
    size_t eqPos = cleanMove.find('=');
    if (eqPos != std::string::npos) {
        char pChar = cleanMove[eqPos + 1];
        if (pChar == 'Q') promotedPiece = gameRules.whiteTurn ? PieceType::W_Queen : PieceType::B_Queen;
        else if (pChar == 'R') promotedPiece = gameRules.whiteTurn ? PieceType::W_Rook : PieceType::B_Rook;
        else if (pChar == 'B') promotedPiece = gameRules.whiteTurn ? PieceType::W_Bishop : PieceType::B_Bishop;
        else if (pChar == 'N') promotedPiece = gameRules.whiteTurn ? PieceType::W_Knight : PieceType::B_Knight;

        // remove the promotion suffix 
        cleanMove = cleanMove.substr(0, eqPos);
    }

    // handle Castling
    if (cleanMove == "O-O" || cleanMove == "O-O-O") {
        int row = gameRules.whiteTurn ? 7 : 0;
        int targetCol = (cleanMove == "O-O") ? 6 : 2;
        int startCol = 4;
        selectedSquare = sf::Vector2i(startCol, row);
        executeMove(row, targetCol, gameRules.grid[row][startCol], PieceType::Empty, move, false);
        return true;
    }

    // strip 'x' (capture indicator) to make string length predictable
    std::string noX = "";
    for (char ch : cleanMove) if (ch != 'x') noX += ch;
    cleanMove = noX;

    // extract target coordinates
    int targetRow = 8 - (cleanMove.back() - '0');
    int targetCol = cleanMove[cleanMove.size() - 2] - 'a';

    // identify piece type and disambiguation (e.g. Rad1 -> Rook on a-file)
    char firstChar = cleanMove[0];
    bool isPawn = !std::isupper(firstChar);
    PieceType movingType = PieceType::Empty;

    char disFile = '\0';
    char disRank = '\0';

    if (isPawn) {
        movingType = gameRules.whiteTurn ? PieceType::W_Pawn : PieceType::B_Pawn;
        if (cleanMove.size() >= 3) { // e.g., exd4 -> ed4 -> size 3. File is 'e'.
            disFile = cleanMove[0];
        }
    }
    else {
        if (firstChar == 'K') movingType = gameRules.whiteTurn ? PieceType::W_King : PieceType::B_King;
        else if (firstChar == 'Q') movingType = gameRules.whiteTurn ? PieceType::W_Queen : PieceType::B_Queen;
        else if (firstChar == 'R') movingType = gameRules.whiteTurn ? PieceType::W_Rook : PieceType::B_Rook;
        else if (firstChar == 'B') movingType = gameRules.whiteTurn ? PieceType::W_Bishop : PieceType::B_Bishop;
        else if (firstChar == 'N') movingType = gameRules.whiteTurn ? PieceType::W_Knight : PieceType::B_Knight;

        // disambiguation logic (e.g. Rad1 -> 'a' is at index 1)
        if (cleanMove.size() >= 4) {
            char d1 = cleanMove[1];
            if (d1 >= 'a' && d1 <= 'h') disFile = d1;
            else if (d1 >= '1' && d1 <= '8') disRank = d1;
        }
    }

    // find the matching piece on the board
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (gameRules.grid[r][c] == movingType) {
                // apply Disambiguation filters
                if (disFile != '\0' && c != (disFile - 'a')) continue;
                if (disRank != '\0' && r != (8 - (disRank - '0'))) continue;

                // check legal move
                if (gameRules.isMoveValid(r, c, targetRow, targetCol)) {
                    selectedSquare = sf::Vector2i(c, r);
                    PieceType targetPiece = gameRules.grid[targetRow][targetCol];
                    bool isCapture = (targetPiece != PieceType::Empty) || (isPawn && c != targetCol);

                    // execute basic move logic
                    executeMove(targetRow, targetCol, movingType, targetPiece, move, isCapture);

                    // auto-resolve promotion bypassing the UI pause!
                    if (isPromoting && promotedPiece != PieceType::Empty) {
                        gameRules.grid[targetRow][targetCol] = promotedPiece;
                        gameRules.pendingPromotionMove.notation = move; // keep full notation e.g., h8=Q#
                        gameRules.pendingPromotionMove.promotedTo = promotedPiece;

                        // insert into history
                        if (gameRules.currentMoveIndex < (int)gameRules.moveHistory.size() - 1) {
                            gameRules.moveHistory.erase(gameRules.moveHistory.begin() + (gameRules.currentMoveIndex + 1), gameRules.moveHistory.end());
                        }
                        gameRules.moveHistory.push_back(gameRules.pendingPromotionMove);
                        gameRules.currentMoveIndex = (int)gameRules.moveHistory.size() - 1;

                        gameRules.whiteTurn = !gameRules.whiteTurn;
                        gameRules.checkGameEnd();
                        isPromoting = false;
                        promotionSquare = sf::Vector2i(-1, -1);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}