#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "gameboard.h"
#include "aiplayer.h"
#include <sqlite3.h> // SQLite database
#include <string> // Standard string operations
#include <QMainWindow>
#include <QFrame> // Include QFrame header from QtWidgets module
#include <vector>
#include <QTimer>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
  void showBoard(const QString& boardStr);
  void showMoveByMove(int gameId, const QString& playerEmail, sqlite3* db); // Declaration of showMoveByMove
  std::tuple<std::string, std::string, std::string, int, std::vector<std::string>> getGameDetails(int gameId, sqlite3* db);
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
    void showNextMove();
    void onmbmClicked(int gameId);
    void on_showPasswordCheckBox_stateChanged(int state);
    void on_showPassword1CheckBox_stateChanged(int state);
    void onreturn_2Clicked();
    void onreturn3Clicked();
    void onreturn4Clicked();
    void onreturn5Clicked();
      void onreturn6Clicked();
    void resetGame2();


private:
    Ui::MainWindow *ui; // Reference to the UI elements
    QFrame *player2LoginFrame; // Define QFrame pointer
    QFrame *player2SignupFrame; // Define QFrame pointer
    QFrame *pvpGameFrame; // Define QFrame pointer
    std::string getLoggedInPlayerEmail();
    std::string getPlayer2Email();
    void showPlayer2Stats();
    void showPlayer1Stats();
    bool getPlayerStats(const std::string& email, int& pvp_win_count, int& pvp_lose_count, int& pvp_total_games,
                        int& pve_win_count, int& pve_lose_count, int& pve_total_games,
                        int& total_wins, int& total_loss, int& total_games);

    void updatePlayerStats(const std::string& email,  int pvp_win_count, int pvp_lose_count, int pvp_total_games,
                           int pve_win_count, int pve_lose_count, int pve_total_games);
    void checkGameStatus();
    void handleGameOutcome(const std::string& player1Email, const std::string& player2Email, int result);
    void askPlayAgain(const QString& result);
    void handleGameWin(const std::string& player1Email, const std::string& player2Email);
    void handleGameDraw(const std::string& player1Email, const std::string& player2Email);
    void initializeGame(); // You should implement this function for game initialization
    void updateTurnLabel();
    void makeAIMove();
    void resetGame();
    void resetGame1();
    GameBoard board;
    AIPlayer ai;
    sqlite3 *db;
    int currentPlayer;
    bool gameActive;
    bool checkGameState();
    bool againstAI;
    int state;
    char player1Symbol;
    char player2Symbol;
    std::vector<std::pair<std::string, std::string>> moves;
    QTimer* moveTimer;
    int currentMoveIndex;
    void updateBoardUI();
    void saveMove(int gameId, const GameBoard& board, const std::string& playerTurn, int moveNumber, sqlite3* db);
    void verifyTablesExist();
    void createTablesIfNeeded();
    int lastGameId; // Declare lastGameId if not already declared
    int moveNumber; // Add this line to declare moveNumber
    int try_lastGameId;
    QString player1name;
    void showBoard(const char boardStr[]) ;
    std::vector<int> getPlayerGameIds(const std::string& playerEmail, sqlite3* db);
    void showPlayerGameIds();
    void showGameOptions(const std::string& playerEmail,int gameId);
    void showFinalMove(int gameId) ;
    void showMoveByMove(int gameId, const std::string& playerEmail) ;
    void showBoard1(const char boardStr[]);
    QString originalPassword;
    QString originalPlayer2Password;
    void showGameDetails(int gameId);

};

#endif // MAINWINDOW_H
