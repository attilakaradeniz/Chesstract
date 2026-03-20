// Board.cpp
#include "Board.hpp"
#include <iostream>

Board::Board() {
	loadAssets();
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			grid[i][j] = PieceType::Empty;
		}
	}

	// std::cout << "Chess board logic initialized." << std::endl;
	// setup pawns
	for (int i = 0; i < 8; ++i) {
		grid[1][i] = PieceType::W_Pawn; // white pawns
		grid[6][i] = PieceType::B_Pawn; // black pawns

	}

	// setup rooks, knights, bishops
	grid[0][0] = grid[0][7] = PieceType::W_Rook;
	grid[7][0] = grid[7][7] = PieceType::B_Rook;

	grid[0][1] = grid[0][6] = PieceType::W_Knight;
	grid[7][1] = grid[7][6] = PieceType::B_Knight;

	grid[0][2] = grid[0][5] = PieceType::W_Bishop;
	grid[7][2] = grid[7][5] = PieceType::B_Bishop;

	grid[0][3] = PieceType::W_Queen; grid[0][4] = PieceType::W_King;
	grid[7][3] = PieceType::B_Queen; grid[7][4] = PieceType::B_King;

	std::cout << "Chess board logic initialized (starting position)." << std::endl;

}

void Board::printStatus() {
	std::cout << "Board is ready for piecec." << std::endl;
}

void Board::draw(sf::RenderWindow& window) {
    int size = 45; // Senin resmindeki gerçek taş boyutu

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            // 1. Kareyi çiz (Mevcut kodun)
            sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
            square.setPosition(j * tileSize + offset, i * tileSize + offset);
            square.setFillColor(((i + j) % 2 == 0) ? sf::Color(240, 217, 181) : sf::Color(181, 136, 99));
            window.draw(square);

            // 2. Taşı çiz
            PieceType type = grid[i][j];
            if (type != PieceType::Empty) {
                int row = 0;
                int col = 0;

                // Resmindeki sıralama genelde şöyledir (K, Q, B, N, R, P)
                // Eğer taşlar yanlış çıkarsa buradaki 'col' değerlerini değiştiririz
                if (type == PieceType::W_King) { col = 0; row = 0; }
                if (type == PieceType::W_Queen) { col = 1; row = 0; }
                if (type == PieceType::W_Bishop) { col = 2; row = 0; }
                if (type == PieceType::W_Knight) { col = 3; row = 0; }
                if (type == PieceType::W_Rook) { col = 4; row = 0; }
                if (type == PieceType::W_Pawn) { col = 5; row = 0; }

                if (type == PieceType::B_King) { col = 0; row = 1; }
                if (type == PieceType::B_Queen) { col = 1; row = 1; }
                if (type == PieceType::B_Bishop) { col = 2; row = 1; }
                if (type == PieceType::B_Knight) { col = 3; row = 1; }
                if (type == PieceType::B_Rook) { col = 4; row = 1; }
                if (type == PieceType::B_Pawn) { col = 5; row = 1; }

                // 45x45'lik alanı kesiyoruz
                pieceSprite.setTextureRect(sf::IntRect(col * size, row * size, size, size));

                // ÖLÇEKLENDİRME: 45 piksellik taşı 100 piksellik kareye yayıyoruz
                // 100 / 45 = 2.22 kat büyütüyoruz
                float scale = tileSize / (float)size;
                pieceSprite.setScale(scale, scale);

                pieceSprite.setPosition(j * tileSize + offset, i * tileSize + offset);
                window.draw(pieceSprite);
            }
        }
    }
}
void Board::loadAssets() {
    // Proje klasöründeki assets/pieces.png yoluna bakar
    if (!piecesTexture.loadFromFile("assets/pieces.png")) {
        std::cerr << "HATA: assets/pieces.png yuklenemedi! Dosya yolunu kontrol et." << std::endl;
    }
    // Sprite nesnesine dokuyu bağlıyoruz
    pieceSprite.setTexture(piecesTexture);
}