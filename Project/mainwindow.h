#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <sqlite3.h> // SQLite database
#include <string> // Standard string operations
#include <QMainWindow>
#include "aiplayer.h"
#include <QFrame> // Include QFrame header from QtWidgets module
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadUserData(const std::string &email);
    // Ensure the following slots are declared
    void onLoginButtonClicked(); // Slot for the login button
    void onSignupButtonClicked(); // Slot for the signup button
    void onSwitchToSignupButtonClicked(); // Slot to switch to the signup frame
    void handleButtonClick();
    void onSwitchToLoginButtonClicked(); // Slot to switch to the login frame
    void onswitchToSignupFrameClicked(); // Slot to switch to the signup frame
    void onswitchToLoginFrameClicked(); // Slot to switch to the login frame
    void onreturn1Clicked();
    void onreturn2Clicked();
    void onPLAYClicked();
    void onPVPButtonClicked();
    void onlogoutClicked();
    void onpgClicked();
    void onPVEButtonClicked();
    void onPlayer2LoginButtonClicked();
    void onPlayer2SignupButtonClicked();
    void onSwitchToPlayer2SignupButtonClicked();
    void onSwitchToPlayer2LoginButtonClicked();

private:
    Ui::MainWindow *ui; // Reference to the UI elements
    QFrame *player2LoginFrame; // Define QFrame pointer
    QFrame *player2SignupFrame; // Define QFrame pointer
    QFrame *pvpGameFrame; // Define QFrame pointer
    std::string getLoggedInPlayerEmail();
    std::string getPlayer2Email();
    void showPlayer2Stats();
    void showPlayer1Stats();
    bool getPlayerStats(const std::string& email, int& pvp_win_count, int& pvp_lose_count, int& pvp_total_games);
    void updatePlayerStats(const std::string& email, int pvp_win_count, int pvp_lose_count, int pvp_total_games);
    void checkGameStatus();
    void handleGameOutcome(const std::string& player1Email, const std::string& player2Email, int gameStatus, int gameOutcome);
    void makeAIMove();
    void askPlayAgain(const QString& result);
    void handleGameWin(const std::string& player1Email, const std::string& player2Email);
    void handleGameDraw(const std::string& player1Email, const std::string& player2Email);
    void initializeGame(); // You should implement this function for game initialization
    void onUserClickedButton(int index) ;
    AIPlayer aiPlayer;
    std::vector<QPushButton*> buttons;
    // Tic Tac Toe game logic
    char board[3][3]; // 3x3 board for the game
    char currentPlayer;
    bool gameActive;
    bool againstAI ;
  std::vector<QPushButton*> buttonsVector; // Vector to hold pointers to all buttons representing the game board
    char player1Symbol;
    char player2Symbol;


     friend class tests;

    void updateBoardUI();
};

#endif // MAINWINDOW_H
