#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

// Pieces types and colors
enum class PieceType {
    Empty,
    W_Pawn, W_Rook, W_Knight, W_Bishop, W_Queen, W_King,
    B_Pawn, B_Rook, B_Knight, B_Bishop, B_Queen, B_King
};

class Board {
public:
    Board();
    void printStatus();
    void draw(sf::RenderWindow& window);

private:
    PieceType grid[8][8];
    const float tileSize = 100.f;
    float offset = 50.f;

    // SFML Görsel Nesneleri
    sf::Texture piecesTexture;
    sf::Sprite pieceSprite;

    void loadAssets(); // to load asset
};