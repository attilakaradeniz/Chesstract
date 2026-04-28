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

    // SQL query
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // filling the parameters (question marks) with actual vallues 
    std::string seq = generateSequence(history);
    sqlite3_bind_text(stmt, 1, white.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, black.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, result.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, seq.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, fullPgn.c_str(), -1, SQLITE_TRANSIENT);

    // execute query
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (success) std::cout << "Game saved to collection!" << std::endl;
    return success;
}

std::vector<GameEntry> GameDatabase::searchByOpening(const std::string& openingSeq) {
    std::vector<GameEntry> results;

    // Append the SQL wildcard character '%' to the sequence
    // Example: "e4|e5" becomes "e4|e5%" (matches anything starting with e4|e5)
    std::string querySeq = openingSeq + "%";

    // SQL query
    const char* sql = "SELECT ID, Date, White, Black, Result, Sequence, FullPGN FROM Games WHERE Sequence LIKE ?;";
    sqlite3_stmt* stmt;

    // prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare search statement: " << sqlite3_errmsg(db) << std::endl;
        return results; // Return empty list on error
    }

    // binding the query string to the placeholder (?)
    sqlite3_bind_text(stmt, 1, querySeq.c_str(), -1, SQLITE_TRANSIENT);

    // execute and iterate through the matching rows
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        GameEntry entry;

        // extract ID
        entry.id = sqlite3_column_int(stmt, 0);

        // extract Date safely
        const unsigned char* dateText = sqlite3_column_text(stmt, 1);
        if (dateText) entry.date = reinterpret_cast<const char*>(dateText);

        // extract White Player safely
        const unsigned char* whiteText = sqlite3_column_text(stmt, 2);
        if (whiteText) entry.whitePlayer = reinterpret_cast<const char*>(whiteText);

        // extract Black Player safely
        const unsigned char* blackText = sqlite3_column_text(stmt, 3);
        if (blackText) entry.blackPlayer = reinterpret_cast<const char*>(blackText);

        // extract Result safely
        const unsigned char* resultText = sqlite3_column_text(stmt, 4);
        if (resultText) entry.result = reinterpret_cast<const char*>(resultText);

        // extract Sequence safely
        const unsigned char* seqText = sqlite3_column_text(stmt, 5);
        if (seqText) entry.notationSequence = reinterpret_cast<const char*>(seqText);

        // extract Full PGN safely
        const unsigned char* pgnText = sqlite3_column_text(stmt, 6);
        if (pgnText) entry.fullPgn = reinterpret_cast<const char*>(pgnText);

        // add the populated entry to results list
        results.push_back(entry);
    }

    // clean up
    sqlite3_finalize(stmt);

    return results;
}

 GameEntry GameDatabase::getGameById(int id) {
    GameEntry entry;
    entry.id = -1; // default to -1 to indicate "not found" if the query fails

    const char* sql = "SELECT ID, Date, White, Black, Result, Sequence, FullPGN FROM Games WHERE ID = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare getGameById statement: " << sqlite3_errmsg(db) << std::endl;
        return entry;
    }

    // bind the integer ID to the query
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        entry.id = sqlite3_column_int(stmt, 0);

        const unsigned char* dateText = sqlite3_column_text(stmt, 1);
        if (dateText) entry.date = reinterpret_cast<const char*>(dateText);

        const unsigned char* whiteText = sqlite3_column_text(stmt, 2);
        if (whiteText) entry.whitePlayer = reinterpret_cast<const char*>(whiteText);

        const unsigned char* blackText = sqlite3_column_text(stmt, 3);
        if (blackText) entry.blackPlayer = reinterpret_cast<const char*>(blackText);

        const unsigned char* resultText = sqlite3_column_text(stmt, 4);
        if (resultText) entry.result = reinterpret_cast<const char*>(resultText);

        const unsigned char* seqText = sqlite3_column_text(stmt, 5);
        if (seqText) entry.notationSequence = reinterpret_cast<const char*>(seqText);

        const unsigned char* pgnText = sqlite3_column_text(stmt, 6);
        if (pgnText) entry.fullPgn = reinterpret_cast<const char*>(pgnText);
    }
    else {
        std::cerr << "Game with ID " << id << " not found." << std::endl;
    }

    sqlite3_finalize(stmt);
    return entry;
} 
