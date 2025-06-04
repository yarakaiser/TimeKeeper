#include "WindowLogger.h"
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>

WindowLogger::WindowLogger(const std::string& dbPath)
    : db(nullptr), sessionStart(std::chrono::system_clock::now()) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to open SQLite DB\n";
    }
    initDatabase();
}

WindowLogger::~WindowLogger() {
    FlushCurrentSession();
    if (db) sqlite3_close(db);
}

void WindowLogger::initDatabase() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS WindowLog (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            start_time TEXT NOT NULL,
            duration INTEGER
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to create table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

void WindowLogger::LogWindowChange(const std::string& newTitle) {
    //std::cout << "LogWindowChange called with: " << newTitle << "\n";

    auto now = std::chrono::system_clock::now();
    if (!currentTitle.empty()) {
        int duration = static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(now - sessionStart).count());
        insertLog(currentTitle, getCurrentTimeString(), duration);
    }
    currentTitle = newTitle;
    sessionStart = now;
}

void WindowLogger::FlushCurrentSession() {
    if (currentTitle.empty()) return;

    auto now = std::chrono::system_clock::now();
    int duration = static_cast<int>(
        std::chrono::duration_cast<std::chrono::seconds>(now - sessionStart).count());
    insertLog(currentTitle, getCurrentTimeString(), duration);

    currentTitle.clear();
}

void WindowLogger::insertLog(const std::string& title, const std::string& startTime, int duration) {

    std::cout << "Logged: " << title << ", Start: " << startTime << ", Duration: " << duration << "s\n";

    const char* sql = "INSERT INTO WindowLog (title, start_time, duration) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, startTime.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, duration);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    else {
        std::cerr << "Failed to prepare statement\n";
    }

}

std::string WindowLogger::getCurrentTimeString() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << st.wYear << "-"
        << std::setw(2) << st.wMonth << "-" << std::setw(2) << st.wDay << " "
        << std::setw(2) << st.wHour << ":" << std::setw(2) << st.wMinute << ":"
        << std::setw(2) << st.wSecond;
    return oss.str();
}