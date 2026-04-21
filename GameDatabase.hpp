#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"
#include "ChessRules.hpp" // MoveRecord

struct GameEntry {
    int id;
    std::string date;
    std::string whitePlayer;
    std::string blackPlayer;
    std::string result;
    std::string notationSequence; // e4|c5|Nf3...
    std::string fullPgn;
};

class GameDatabase {
public:
    GameDatabase(const std::string& dbPath);
    ~GameDatabase();

    // save game
    bool saveGame(const std::string& white, const std::string& black,
        const std::string& result, const std::vector<MoveRecord>& history,
        const std::string& fullPgn);

	// opening sequence search
    std::vector<GameEntry> searchByOpening(const std::string& openingSeq);

    // to load a game from db
	GameEntry getGameById(int id);

private:
    sqlite3* db;
    void createTable();
    std::string generateSequence(const std::vector<MoveRecord>& history, int depth = 12);

}; 