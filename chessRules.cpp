#include "ChessRules.hpp"
#include <iostream>
#include <cmath>

ChessRules::ChessRules() {
    resetBoardToStart();
}

void ChessRules::resetBoardToStart() {
    //reset all flags
    whiteTurn = true;
    gameOver = false;
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteRook0Moved = false;
    whiteRook7Moved = false;
    blackRook0Moved = false;
    blackRook7Moved = false;
    lastPawnDoubleMove = sf::Vector2i(-1, -1);
    currentMoveIndex = -1;

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

bool ChessRules::isWhite(PieceType type) {
    if (type == PieceType::Empty) return false;
    return (int)type >= (int)PieceType::W_Pawn && (int)type <= (int)PieceType::W_King;
}

bool ChessRules::isSquareAttacked(int targetRow, int targetCol, bool attackerIsWhite) {
    int rowDirs[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
    int colDirs[] = { 0, 0, -1, 1, -1, 1, -1, 1 };

    // sliding Pieces
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

sf::Vector2i ChessRules::findKing(bool white) {
    PieceType targetKing = white ? PieceType::W_King : PieceType::B_King;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (grid[r][c] == targetKing) return sf::Vector2i(c, r);
        }
    }
    return sf::Vector2i(-1, -1);
}

bool ChessRules::isMoveValid(int startRow, int startCol, int endRow, int endCol) {
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

bool ChessRules::needsDisambiguation(int startRow, int startCol, int endRow, int endCol, PieceType type) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (grid[r][c] == type && (r != startRow || c != startCol)) {
                if (isMoveValid(r, c, endRow, endCol)) return true;
            }
        }
    }
    return false;
}

void ChessRules::checkGameEnd() {
    if (!hasLegalMoves(whiteTurn)) {
        gameOver = true;
        sf::Vector2i kingPos = findKing(whiteTurn);
        if (isSquareAttacked(kingPos.y, kingPos.x, !whiteTurn)) resultText = whiteTurn ? "BLACK WINS BY CHECKMATE" : "WHITE WINS BY CHECKMATE";
        else resultText = "DRAW BY STALEMATE";
    }
}

bool ChessRules::hasLegalMoves(bool white) {
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