#pragma once
#include <string>
#include "sqlite3.h"
#include <chrono>

class WindowLogger
{
public:
    WindowLogger(const std::string& dbPath);
    ~WindowLogger();

    void LogWindowChange(const std::string& newTitle);
    void FlushCurrentSession();

private:
    sqlite3* db;
    std::string currentTitle;
    std::chrono::system_clock::time_point sessionStart;

    void initDatabase();
    void insertLog(const std::string& title, const std::string& startTime, int duration);
    std::string getCurrentTimeString();
};

