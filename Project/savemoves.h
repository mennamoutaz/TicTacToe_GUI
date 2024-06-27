#ifndef SAVEMOVES_H
#define SAVEMOVES_H
#include <vector>
#include <limits>
#include <string>
#include <iostream>
#include "sqlite3.h"
#include <cstdint>
#include <conio.h>
#include <chrono>
#include <ctime>
#include <thread>
#include "gameboard.h"
#include <map>
#include <cstdint>

#include "savemoves.h"

using namespace std;

 void showMoveByMove(int gameId, const QString& playerEmail, sqlite3* db); // Declaration of showMoveByMove
std::vector<int> getPlayerGameIds(const std::string& playerEmail, sqlite3* db);
void showFinalMove(int gameId, const std::string& playerEmail, sqlite3* db);
void showBoard(const char boardStr[]) ;
#endif // SAVEMOVES_H
