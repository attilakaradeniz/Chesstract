#include "GameDatabase.hpp"
#include <iostream>

GameDatabase::GameDatabase(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }
    else {
        createTable();
    }
}

GameDatabase::~GameDatabase() {
    sqlite3_close(db);
}

void GameDatabase::createTable() {
    const char* sql = "CREATE TABLE IF NOT EXISTS Games ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Date TEXT,"
        "White TEXT,"
        "Black TEXT,"
        "Result TEXT,"
        "Sequence TEXT,"
        "FullPGN TEXT);";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

std::string GameDatabase::generateSequence(const std::vector<MoveRecord>& history, int depth) {
    std::string seq = "";
    int actualDepth = std::min((int)history.size(), depth);
    for (int i = 0; i < actualDepth; ++i) {
        seq += history[i].notation;
        if (i < actualDepth - 1) seq += "|";
    }
    return seq;
}

bool GameDatabase::saveGame(const std::string& white, const std::string& black,
    const std::string& result, const std::vector<MoveRecord>& history,
    const std::string& fullPgn) {
    const char* sql = "INSERT INTO Games (Date, White, Black, Result, Sequence, FullPGN) "
        "VALUES (datetime('now'), ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    // 1. SQL query
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // 2. filling the parameters (question marks) with actual vallues 
    std::string seq = generateSequence(history);
    sqlite3_bind_text(stmt, 1, white.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, black.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, result.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, seq.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, fullPgn.c_str(), -1, SQLITE_TRANSIENT);

    // 3. execute query
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (success) std::cout << "Game saved to collection!" << std::endl;
    return success;
}
