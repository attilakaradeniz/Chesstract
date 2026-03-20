// Board.hpp
#ifndef BOARD_HPP
#define	BOARD_HPP
#include <iostream>

// enum for piece types
enum class PieceType {Empty, Pawn, Rook, Knight, Bishop, Queen, King};
enum class PieceColor {None, White, Black};

class Board {
public:
	Board(); // Constructor
	void printStatus(); // to test the output
private:
	PieceType grid[8][8];
};

#endif // !1
