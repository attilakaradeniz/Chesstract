#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <vector>
//#include <string>
#include "ChessRules.hpp" // Crucial link to the logic class

struct PieceSource {
    int col;
    int row;
};

class Board {
public:
    Board();
    void printStatus(); 
    void draw(sf::RenderWindow& window);

    void handleMouseClick(const sf::Vector2i mousePos);

    void undoMove();
    void goToMove(int targetIndex);
    void handleKeyPress(sf::Keyboard::Key key);

    void exportPGN();
    void flipBoard();
    void toggleCoordinates();
    void calculateValidMoves(int startRow, int startCol);
    void savePGNToFile();

    // functions for drag & drop
    void updateMousePos(sf::Vector2f pos) { mousePos = pos; }
    void handleMouseDown(sf::Vector2f mPos);
    void handleMouseUp(sf::Vector2f mPos);

private:
    ChessRules gameRules; 

    // SFML visual assets
    sf::Texture piecesTexture;
    sf::Sprite pieceSprite;
    sf::Font font;

    // Audio assets
    sf::SoundBuffer moveBuffer;
    sf::SoundBuffer captureBuffer;
    sf::Sound moveSound;
    sf::Sound captureSound;

    // UI specific members
    const float tileSize = 100.f;
    float offset = 50.f;
    std::map<PieceType, PieceSource> pieceSourceMap;
    sf::Vector2i selectedSquare = sf::Vector2i(-1, -1);

    bool isFlowFlipped = false;
    bool showCoordinates = true;
    std::vector<sf::Vector2i> validMoves;

    sf::Vector2i lastMoveStart = sf::Vector2i(-1, -1);
    sf::Vector2i lastMoveEnd = sf::Vector2i(-1, -1);

    float scrollOffset = 0.0f;
    bool isDragging = false;
    sf::Vector2i draggedPieceSource;
    sf::Vector2f mousePos;

    // pawn promotion related members
    bool isPromoting = false;
    sf::Vector2i promotionSquare = sf::Vector2i(-1, -1);

    // Helpers
    void loadAssets();
    void setupPieceSources();
    char getPieceChar(PieceType type);
    void applyMoveIndependently(const MoveRecord& record);
};