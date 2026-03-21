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
        grid[1][i] = PieceType::W_Pawn; // White pawns
        grid[6][i] = PieceType::B_Pawn; // Black pawns
    }

    // Setup pieces (Rooks, Knights, Bishops)
    grid[0][0] = grid[0][7] = PieceType::W_Rook;
    grid[7][0] = grid[7][7] = PieceType::B_Rook;

    grid[0][1] = grid[0][6] = PieceType::W_Knight;
    grid[7][1] = grid[7][6] = PieceType::B_Knight;

    grid[0][2] = grid[0][5] = PieceType::W_Bishop;
    grid[7][2] = grid[7][5] = PieceType::B_Bishop;

    // Setup Kings and Queens
    grid[0][3] = PieceType::W_Queen; grid[0][4] = PieceType::W_King;
    grid[7][3] = PieceType::B_Queen; grid[7][4] = PieceType::B_King;

    std::cout << "Chess board logic initialized (starting position)." << std::endl;
}

void Board::loadAssets() {
    // Load the texture from the assets folder
    if (!piecesTexture.loadFromFile("assets/pieces.png")) {
        std::cerr << "Error: Could not load assets/pieces.png!" << std::endl;
    }
    piecesTexture.setSmooth(true);
    pieceSprite.setTexture(piecesTexture);

    // Initialize the lookup table for piece coordinates on the sprite sheet
    setupPieceSources();
}

void Board::setupPieceSources() {
    // Map each piece type to its grid position (column, row) in the sprite sheet
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

    // Simple rule: Cannot move to the same square
    if (startRow == endRow && startCol == endCol) return false;

    // Specific piece logic
    switch (movingPiece) {
    case PieceType::W_Knight:
    case PieceType::B_Knight: {
        int rowDiff = std::abs(startRow - endRow);
        int colDiff = std::abs(startCol - endCol);
        // Knight moves in an 'L' shape: (2,1) or (1,2)
        return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
    }
                            // Add logic for other pieces here
    default:
        return true; // Temporary: Allow other pieces to move anywhere
    }
}

void Board::handleMouseClick(sf::Vector2i mousePos) {
    int col = (mousePos.x - static_cast<int>(offset)) / static_cast<int>(tileSize);
    int row = (mousePos.y - static_cast<int>(offset)) / static_cast<int>(tileSize);

    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    // CASE 1: Piece is already selected (Movement Phase)
    if (selectedSquare != sf::Vector2i(-1, -1)) {
        PieceType movingPiece = grid[selectedSquare.y][selectedSquare.x];
        PieceType targetPiece = grid[row][col];

        if (isMoveValid(selectedSquare.y, selectedSquare.x, row, col)) {
            // Check for friendly fire: Cannot capture your own piece
            if (targetPiece != PieceType::Empty && isWhite(movingPiece) == isWhite(targetPiece)) {
                // Change selection instead of moving
                selectedSquare = sf::Vector2i(col, row);
                return;
            }

            // Execute move and switch turn
            grid[row][col] = movingPiece;
            grid[selectedSquare.y][selectedSquare.x] = PieceType::Empty;
            whiteTurn = !whiteTurn; // Switch turn from White to Black or vice versa
            std::cout << "Move executed. Next turn: " << (whiteTurn ? "White" : "Black") << std::endl;
        }
        selectedSquare = sf::Vector2i(-1, -1);
    }
    // CASE 2: No piece selected (Selection Phase)
    else {
        PieceType clickedPiece = grid[row][col];
        if (clickedPiece != PieceType::Empty) {
            // Check if player is selecting their own color based on the current turn
            if (isWhite(clickedPiece) == whiteTurn) {
                selectedSquare = sf::Vector2i(col, row);
                std::cout << "Selected: " << (whiteTurn ? "White " : "Black ") << "piece." << std::endl;
            }
            else {
                std::cout << "Wait for your turn! Currently: " << (whiteTurn ? "White's" : "Black's") << " turn." << std::endl;
            }
        }
    }
}

void Board::draw(sf::RenderWindow& window) {
    const int sourceSize = 45; // Pixel size of each piece in our 270x90 sprite
    const float scale = tileSize / static_cast<float>(sourceSize);
    pieceSprite.setScale(scale, scale);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            // 1. Draw the board square
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(j * tileSize + offset, i * tileSize + offset);
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(240, 217, 181) : sf::Color(181, 136, 99));
            window.draw(square);

            // 2. Draw selection highlight (under the piece)
            if (selectedSquare == sf::Vector2i(j, i)) {
                sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                highlight.setPosition(j * tileSize + offset, i * tileSize + offset);
                highlight.setFillColor(sf::Color(255, 255, 0, 80)); // Semi-transparent yellow
                window.draw(highlight);
            }

            // 3. Draw the piece
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
    // Check if the piece belongs to the White team
    return (type == PieceType::W_Pawn || type == PieceType::W_Rook ||
        type == PieceType::W_Knight || type == PieceType::W_Bishop ||
        type == PieceType::W_Queen || type == PieceType::W_King);
}



