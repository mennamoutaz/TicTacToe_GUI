#include "mainwindow.h"  // Include the main window class
#include "ui_mainwindow.h"
#include "aiplayer.h"
#include "savemoves.h"
#include "gameboard.h" // Include the UI definitions
#include <QMessageBox>  // For displaying message boxes
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QInputDialog>  // For input dialogs
#include <sqlite3.h>  // SQLite database
#include <chrono>  // For date-time operations
#include <iostream>  // Standard IO operations
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>  // Standard string operations
#include <QTextStream>
#include <QTimer>
#include <qthread.h>
#include <cassert>
#include <QDebug>


// For handling Qt's string input/output
// Declare a global database connection
sqlite3* db;

/*******************************************************************************************************/

// Function to convert time to string
std::string timeToString(std::chrono::system_clock::time_point timePoint) {
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tm;

#ifdef _WIN32
    localtime_s(&tm, &time);  // Use localtime_s on Windows
#else
    localtime_r(&time, &tm);  // Use localtime_r on Linux
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");  // Format the time using put_time
    return oss.str();  // Return the formatted time string
}

/*******************************************************************************************************/


// Function to hash a password using SHA-256
uint64_t customHash(const std::string& str) {
    const uint64_t seed = 0xdeadbeef;
    const uint64_t fnv_prime = 1099511628211ULL;
    uint64_t hash = seed;

    for (char c : str) {
        hash ^= c;
        hash *= fnv_prime;
    }

    return hash;
}

/*******************************************************************************************************/


// Function to hash a password to a string

std::string hashPassword(const std::string& password) {
    uint64_t hash = customHash(password);
    return std::to_string(hash);
}

/*******************************************************************************************************/


// Function to handle user signup

std::string signup(sqlite3* db, const std::string& email, const std::string& password, const std::string& name, int age, const std::string& city) {
    // Check if the email already exists
    std::string checkSQL = "SELECT COUNT(*) FROM players WHERE email = '" + email + "'";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, checkSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "";  // Return empty if statement preparation fails
    }

    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    if (result > 0) {
        // Email already exists
        return "";
    }

    // Get the current date-time
    std::string currentDateStr = timeToString(std::chrono::system_clock::now());

    // SQL query to insert a new player
    std::string insertSQL = "INSERT INTO players (email, password, name, age, city, current_date) VALUES ('" + email + "', '" + password + "', '" + name + "', " + std::to_string(age) + ", '" + city + "', '" + currentDateStr + "')";
    char* errMsg = nullptr;

    // Execute the SQL query
    if (sqlite3_exec(db, insertSQL.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        if (errMsg) {
            std::cerr << "Error inserting data: " << errMsg << std::endl;
            sqlite3_free(errMsg);  // Free error message memory
        }
        return "";  // Return an empty string on failure
    }

    return email;  // Return the email upon successful signup
}


/*******************************************************************************************************/


// Function to handle user login
std::string login(sqlite3* db, const std::string& email, const std::string& password) {
    // SQL query to select password for a given email
    std::string sql = "SELECT password, last_login_date FROM players WHERE email = '" + email + "'";
    sqlite3_stmt* stmt;

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "";  // Return empty if statement preparation fails
    }

    std::string resultEmail = "";  // Default to an empty string if login fails

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Retrieve the password from the database
        std::string dbPassword(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

        if (dbPassword == password) {  // Check if passwords match
            resultEmail = email;  // Store the successful login email

            // Update the last login date
            std::string lastLoginDateStr = timeToString(std::chrono::system_clock::now());
            std::string updateSQL = "UPDATE players SET last_login_date = '" + lastLoginDateStr + "' WHERE email = '" + email + "'";

            // Execute the SQL query to update the last login date
            if (sqlite3_exec(db, updateSQL.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
                std::cerr << "Error updating last login date." << std::endl;
            }
        }
    }
    sqlite3_finalize(stmt);  // Finalize the statement to avoid memory leaks
    return resultEmail;
}


/*******************************************************************************************************/



// Define the MainWindow class constructor and other components
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    moveTimer(nullptr), currentMoveIndex(0),
    ui(new Ui::MainWindow),
    db(nullptr),
    currentPlayer(1),
    moveNumber(1),
    lastGameId(0){
    ui->setupUi(this);   // Set up the UI components
   /*******************************************************************************************************/
    // Set up the SQLite database connection
 ui->player2PasswordLineEdit->setEchoMode(QLineEdit::Password);
    const char* dbPath = "tictactoe24.db";
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        QMessageBox::critical(this, "Database Error", "Cannot open database");
       std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    std::cout << "Database opened successfully at path: " << dbPath << std::endl;

    // Verify tables exist

    verifyTablesExist();
/*******************************************************************************************************/
    // Initialize frames
    player2LoginFrame = new QFrame();
    player2SignupFrame = new QFrame();
    pvpGameFrame = new QFrame();
    //pveGameFrame = new QFrame();
/*******************************************************************************************************/
    // Add frames to stacked widget
    ui->stackedWidget->addWidget(player2LoginFrame);
    ui->stackedWidget->addWidget(player2SignupFrame);
    ui->stackedWidget->addWidget(pvpGameFrame);
    // ui->stackedWidget->addWidget(pveGameFrame);
/*******************************************************************************************************/
    // Connect signals to slots for the buttons
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::onLoginButtonClicked);
    connect(ui->signupButton, &QPushButton::clicked, this, &MainWindow::onSignupButtonClicked);
    connect(ui->switchToSignupButton, &QPushButton::clicked, this, &MainWindow::onSwitchToSignupButtonClicked);
    connect(ui->switchToLoginButton, &QPushButton::clicked, this, &MainWindow::onSwitchToLoginButtonClicked);
    connect(ui->welcomeSignupButton, &QPushButton::clicked, this, &MainWindow::onswitchToSignupFrameClicked);
    connect(ui->PLAY, &QPushButton::clicked, this, &MainWindow::onPLAYClicked);
    connect(ui->welcomeLoginButton, &QPushButton::clicked, this, &MainWindow::onswitchToLoginFrameClicked);
    connect(ui->return1, &QPushButton::clicked, this, &MainWindow::onreturn1Clicked);
    connect(ui->return1_2, &QPushButton::clicked, this, &MainWindow::onreturn2Clicked);
    connect(ui->pvpButton, &QPushButton::clicked, this, &MainWindow::onPVPButtonClicked);
    connect(ui->pveButton, &QPushButton::clicked, this, &MainWindow::onPVEButtonClicked);
    connect(ui->player2LoginButton, &QPushButton::clicked, this, &MainWindow::onPlayer2LoginButtonClicked);
    connect(ui->player2SignupButton, &QPushButton::clicked, this, &MainWindow::onPlayer2SignupButtonClicked);
    connect(ui->switchToPlayer2SignupButton, &QPushButton::clicked, this, &MainWindow::onSwitchToPlayer2SignupButtonClicked);
    connect(ui->switchToPlayer2LoginButton, &QPushButton::clicked, this, &MainWindow::onSwitchToPlayer2LoginButtonClicked);
    connect(ui->logout, &QPushButton::clicked, this, &MainWindow::onlogoutClicked);
    connect(ui->pg, &QPushButton::clicked, this, &MainWindow::onpgClicked);
    connect(ui->showPlayer1StatsButton, &QPushButton::clicked, this, &MainWindow::showPlayer1Stats);
    connect(ui->showGameIdsButton, &QPushButton::clicked, this, &MainWindow::showPlayerGameIds);
    connect(ui->return2, &QPushButton::clicked, this, &MainWindow::onreturn_2Clicked);
    connect(ui->return3, &QPushButton::clicked, this, &MainWindow::onreturn3Clicked);
    connect(ui->return4, &QPushButton::clicked, this, &MainWindow::onreturn4Clicked);
    connect(ui->return5, &QPushButton::clicked, this, &MainWindow::onreturn5Clicked);
    connect(ui->return6, &QPushButton::clicked, this, &MainWindow::onreturn6Clicked);
    connect(ui->showPlayer2StatsButton, &QPushButton::clicked, this, &MainWindow::showPlayer2Stats);
    connect(ui->showPasswordCheckBox, &QCheckBox::stateChanged, this, &MainWindow::on_showPasswordCheckBox_stateChanged);
    connect(ui->showPassword1CheckBox, &QCheckBox::stateChanged, this, &MainWindow::on_showPassword1CheckBox_stateChanged);

    // Set the initial frame to the welcome frame
    ui->stackedWidget->setCurrentIndex(0);  // Index 0 for 'firstframe'
/*************************************************************************************************************************************************************************/
    for (int i = 0; i < 9; ++i) {
        int row=i/3;
        int col=i%3;
        QString buttonName = "button_" + QString::number(row)+"_"+QString::number(col);

        QPushButton* button = findChild<QPushButton*>(buttonName);
        if (button) {
         connect(button, &QPushButton::clicked, this, &MainWindow::handleButtonClick);
        }
    }

}

/*******************************************************************************************************/

MainWindow::~MainWindow() {
    if (db) {
        sqlite3_close(db);  // Properly close the database connection
    }
    delete ui;  // Clean up UI components
}
/*******************************************************************************************************/


/*******************************************************************************************************/
/******************************FUNCTIONS FOR GAME LOGIC*************************************************/
/*******************************************************************************************************/
//Function to initialize the game

void MainWindow::initializeGame() {
    createTablesIfNeeded();
    moveNumber = 1; // Reset the move number at the start of each game
        // Other initialization code...
        if (againstAI) {
        std::string emailPlayer1 = getLoggedInPlayerEmail(); // Replace with your logic
            std::string player2Email = "AI";
               std::string date1 = timeToString(std::chrono::system_clock::now());
        std::string insertSQL = "INSERT INTO games (player1_email, player2_email, date, game_mode) "
                                "VALUES ('" + emailPlayer1 + "', '" + player2Email + "', '" + date1 + "', " + std::to_string(2) + ")";
        char* errMsg = nullptr;
        int status = sqlite3_exec(db, insertSQL.c_str(), nullptr, nullptr, &errMsg);

        if (status != SQLITE_OK) {
            std::cerr << "Error inserting game data: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return;
        }
         lastGameId = sqlite3_last_insert_rowid(db);
            ui->statusLabel->setText("Player 1's Turn (X)");//Set the turn label text.
            currentPlayer = 1;
        } else {
            std::string emailPlayer1 = getLoggedInPlayerEmail(); // Replace with your logic
            std::string emailPlayer2 = getPlayer2Email(); // Replace with your logic
             std::string date1 = timeToString(std::chrono::system_clock::now());
            std::string insertSQL = "INSERT INTO games (player1_email, player2_email, date, game_mode) "
                                    "VALUES ('" + emailPlayer1 + "', '" + emailPlayer2 + "', '" + date1 + "', " + std::to_string(1) + ")";
            char* errMsg = nullptr;
            int status = sqlite3_exec(db, insertSQL.c_str(), nullptr, nullptr, &errMsg);

            if (status != SQLITE_OK) {
                std::cerr << "Error inserting game data: " << errMsg << std::endl;
                sqlite3_free(errMsg);
                return;
            }
            lastGameId = sqlite3_last_insert_rowid(db);


    QMessageBox::StandardButton reply = QMessageBox::question(this, "Choose Symbol", "Player 1, do you want to be X?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        player1Symbol = 'X';
        player2Symbol = 'O';
    } else {
        player1Symbol = 'O';
        player2Symbol = 'X';
    }

    currentPlayer = 1;
    ui->statusLabel->setText("Player 1's turn");
    updateBoardUI();
    }
}

/*******************************************************************************************************/
// Function to handle button clicks
void MainWindow::handleButtonClick()// Slot to handle button clicks.
    {
    QPushButton *button = qobject_cast<QPushButton *>(sender());//Get the button that was clicked.
    if (!button) return;//Return if no button is found.

    QString buttonName = button->objectName();//Get the button name.
    int row = buttonName.split("_")[1].toInt();//Extract row from button name.
    int col = buttonName.split("_")[2].toInt();//Extract column from button name.

    if (board.getValue(row, col) == 0) //Check if the cell is empty.
    {
        board.setValue(row, col, currentPlayer);//: Set the board value to the current player.

        updateBoardUI();// Update the game board UI.
        // Save the move
        std::string playerTurn = (currentPlayer == 1) ? "X" : "O";
        saveMove(lastGameId, board, playerTurn, moveNumber, db);
        moveNumber++;

        if (checkGameState()) {
            return; // If the game is over, return immediately
        }
        currentPlayer = -currentPlayer;//Switch the current player.
        updateTurnLabel();

        // If AI's turn, make AI move
        if (againstAI && currentPlayer == -1)//Check if it is AI's turn.
        {
           QTimer::singleShot(500, this, &MainWindow::makeAIMove);

        }
    }
}
/*******************************************************************************************************/
//Function for handling AI move

void MainWindow::makeAIMove()//Slot for AI's move.
{
     ai.makeMove(board);//Make the AI move.
    updateBoardUI();// Update the game board UI.
     std::string playerTurn = "O";
     saveMove(lastGameId, board, playerTurn, moveNumber, db);
     moveNumber++;
    if (checkGameState()) {

        return; // If the game is over, return immediately
    }
    currentPlayer = 1; // Switch back to Player 1
    updateTurnLabel();
}


/*******************************************************************************************************/

void MainWindow::updateTurnLabel()
{
    if (againstAI) {
        if (currentPlayer == 1) {
            ui->statusLabel->setText("Player 1's Turn ");
        } else {
            ui->statusLabel->setText("AI's Turn ");
        }
    } else {
        if (currentPlayer == 1) {
            ui->statusLabel->setText("Player 1's Turn ");
        } else {
            ui->statusLabel->setText("Player 2's Turn ");
        }
    }
}
/*******************************************************************************************************/
void MainWindow::updateBoardUI() {
    char boardStr[10]; // Assuming board is 3x3 with '\0' terminator
    int index = 0;

    // Populate boardStr with current board state
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int value = board.getValue(i, j);
            if (value == 1) {
                boardStr[index] = 'X';
            } else if (value == -1) {
                boardStr[index] = 'O';
            } else {
                boardStr[index] = '-';
            }
            index++;
        }
    }
    boardStr[9] = '\0'; // Ensure null-terminated

    // Show the board using the updated boardStr
    showBoard(boardStr);
}

/*******************************************************************************************************/
bool MainWindow::checkGameState()
{   std::string emailPlayer1 = getLoggedInPlayerEmail(); // Replace with your logic
    std::string emailPlayer2 = getPlayer2Email(); // Replace with your logic
    int result = board.checkWin();
    QString winner;
    if (result == 1) {
        QMessageBox::information(this, "Game Over", "Player 1 wins!");
        winner ="Player 1 win";
        askPlayAgain(winner);
        // Update statistics for both players (handleGameOutcome function)
        // Handle game win logic
        handleGameOutcome(emailPlayer1, emailPlayer2,1); // 1 means win


        return true;
    } else if (result == -1) {
       handleGameOutcome(emailPlayer1, emailPlayer2,-1); // 1 means win
         if (againstAI && currentPlayer == -1) {
            QMessageBox::information(this, "Game Over", "AI wins!");
            winner ="AI win";
            askPlayAgain(winner);
        } else {
            QMessageBox::information(this, "Game Over", "Player 2 wins!");
            // Update statistics for both players (handleGameOutcome function)
            std::string emailPlayer1 = getLoggedInPlayerEmail(); // Replace with your logic
            std::string emailPlayer2 = getPlayer2Email(); // Replace with your logic
            winner ="Player 2 win";
            askPlayAgain(winner);
            // Handle game win logic



        }

        return true;
    } else if (result == 2) {
         handleGameOutcome(emailPlayer1, emailPlayer2,2); // 1 means win
        QMessageBox::information(this, "Game Over", "It's a draw!");
        winner ="It's a draw!";
        askPlayAgain(winner);

        // Update statistics for both players (handleGameOutcome function)
        std::string emailPlayer1 = getLoggedInPlayerEmail(); // Replace with your logic
        std::string emailPlayer2 = getPlayer2Email(); // Replace with your logic
        // Handle game win logic



        return true;
    }
    return false;
}
/*******************************************************************************************************/

void MainWindow::resetGame()
{
    board = GameBoard();
    updateBoardUI();
    initializeGame();
}


/*******************************************************************************************************/
void MainWindow::resetGame2()
{

    for (int i = 0; i < 9; ++i) {
        int row = i / 3;
        int col = i % 3;
        QString buttonName = QString("button_%1_%2").arg(row).arg(col);
        QPushButton* button = findChild<QPushButton*>(buttonName);
        if (button) {
            // 'X', 'O', or '-'
            QString buttonText;


            buttonText = "  ";


            button->setText(buttonText);
        }
    }
}
/*******************************************************************************************************/
void MainWindow::resetGame1()
{

    for (int i = 0; i < 9; ++i) {
        int row = i / 3;
        int col = i % 3;
        QString buttonName = QString("pushButton_%1_%2").arg(row).arg(col);
        QPushButton* button = findChild<QPushButton*>(buttonName);
        if (button) {
           // 'X', 'O', or '-'
            QString buttonText;


                buttonText = "  ";


            button->setText(buttonText);
        }
    }
}

/*******************************************************************************************************/


void MainWindow::askPlayAgain(const QString& result) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Game Over", result + "\nDo you want to play again?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        resetGame();
    } else {
        gameActive = false;
        ui->statusLabel->setText("Game over. Thanks for playing!");
    }
}
/*******************************************************************************************************/
void MainWindow::showBoard(const char boardStr[]) {
    // Assuming boardStr is a 10-character array (including '\0')
    // Mapping boardStr to the 3x3 QPushButton grid

    for (int i = 0; i < 9; ++i) {
        int row = i / 3;
        int col = i % 3;
        QString buttonName = QString("button_%1_%2").arg(row).arg(col);
        QPushButton* button = findChild<QPushButton*>(buttonName);

        if (button) {
            char cell = boardStr[i]; // 'X', 'O', or '-'
            QString buttonText;

            if (cell == 'X') {
                buttonText = "X";
            } else if (cell == 'O') {
                buttonText = "O";
            } else {
                buttonText = ""; // or " " if you want to show empty cells visibly
            }

            button->setText(buttonText);
        }
    }
}
/********************************************************************************************************/

void MainWindow::showBoard1(const char boardStr[]) {
    std::cout << "Updating board: " << boardStr << std::endl; // Debugging output

    for (int i = 0; i < 9; ++i) {
        int row = i / 3;
        int col = i % 3;
        QString buttonName = QString("pushButton_%1_%2").arg(row).arg(col);
        QPushButton* button = findChild<QPushButton*>(buttonName);
        if (button) {
            char cell = boardStr[i];
            QString buttonText;

            if (cell == 'X') {
                buttonText = "X";
            } else if (cell == 'O') {
                buttonText = "O";
            } else {
                buttonText = "";
            }

            button->setText(buttonText);
            std::cout << "Setting " << buttonName.toStdString() << " to " << buttonText.toStdString() << std::endl; // Debugging output
        } else {
            std::cerr << "Button " << buttonName.toStdString() << " not found!" << std::endl; // Debugging output
        }
    }
}


/*******************************************************************************************************/
/************************************FUNCTIONS FOR PRESSED BUTTONS**************************************/
/*******************************************************************************************************/

// Example slot implementations for login and signup (adjust to fit your application)
void MainWindow::onLoginButtonClicked() {
    // Store the original password
    originalPassword = ui->passwordLineEdit->text();

    // Convert QString to std::string for hashing
    std::string password = hashPassword(originalPassword.toStdString());

    // Ensure other fields are converted correctly
    std::string email = ui->emailLineEdit->text().toStdString();

    std::string result = login(db, email, password);  // Pass as std::string

    if (!result.empty()) {
        QMessageBox::information(this, "Login Successful", "Welcome!");
        ui->stackedWidget->setCurrentIndex(2);  // Return to login frame
        loadUserData(email);
    } else {
        ui->loginErrorLabel->setText("Invalid email or password.");  // No conversion needed
    }
}
/*******************************************************************************************************/


void MainWindow::on_showPassword1CheckBox_stateChanged(int state) {
    if (state == Qt::Checked) {
        // Show the original password
        ui->passwordLineEdit->setEchoMode(QLineEdit::Normal);
    } else {
        // Hide the password (show black dots)
        ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    }
}


/*******************************************************************************************************/
void MainWindow::onSignupButtonClicked() {
    // Convert all QString to std::string where required
    std::string email = ui->signupEmailLineEdit->text().toStdString();
    std::string password = hashPassword(ui->signupPasswordLineEdit->text().toStdString());
    std::string name = ui->signupNameLineEdit->text().toStdString();
    std::string city = ui->signupCityLineEdit->text().toStdString();

    int age = ui->signupAgeLineEdit->text().toInt();  // Conversion to int

    // Corrected function call with std::string and int
    std::string signupResult = signup(db, email, password, name, age, city);

    if (!signupResult.empty()) {
        QMessageBox::information(this, "Signup Successful", "Please log in.");
        ui->stackedWidget->setCurrentIndex(0);  // Return to login frame
    } else {
        ui->signupErrorLabel->setText("Signup failed. Please try again.");
    }
}
/*******************************************************************************************************/
// Slot implementations for switching frames
void MainWindow::onSwitchToSignupButtonClicked() {
    ui->stackedWidget->setCurrentIndex(3);  // Switch to the signup frame
}
/*******************************************************************************************************/
void MainWindow::onSwitchToLoginButtonClicked() {
    ui->stackedWidget->setCurrentIndex(1);  // Switch back to the login frame
}
/*******************************************************************************************************/
// Slot to switch to the signup frame
void MainWindow::onswitchToSignupFrameClicked() {
    ui->stackedWidget->setCurrentIndex(3);  // Assuming frame index 2 is for signup
}
/*******************************************************************************************************/
// Slot to switch to the login frame
void MainWindow::onswitchToLoginFrameClicked() {
    ui->stackedWidget->setCurrentIndex(1);  // Assuming frame index 1 is for login
}
/*******************************************************************************************************/
void MainWindow::onreturn1Clicked() {
    ui->stackedWidget->setCurrentIndex(0);  // Assuming frame index 1 is for login
}
/*******************************************************************************************************/
void MainWindow::onreturn2Clicked() {

    ui->stackedWidget->setCurrentIndex(0);

      // Assuming frame index 1 is for login
}
/*******************************************************************************************************/
void MainWindow::onreturn_2Clicked() {
    ui->signupEmailLineEdit->clear();
    ui->signupPasswordLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->signupNameLineEdit->clear();
    ui->signupAgeLineEdit->clear();
    ui->signupCityLineEdit->clear();
    ui->userEmailLabel->clear();
    ui-> player2EmailLineEdit->clear();
    ui->player2PasswordLineEdit->clear();
    ui->stackedWidget->setCurrentIndex(1);  // Assuming frame index 1 is for login
}
/*******************************************************************************************************/
void MainWindow::onreturn3Clicked() {
    ui-> player2EmailLineEdit->clear();
    ui->player2PasswordLineEdit->clear();
    std::string emailPlayer1 = getLoggedInPlayerEmail();

    loadUserData(emailPlayer1 );

    ui->stackedWidget->setCurrentIndex(2);  // Assuming frame index 1 is for login
}
/*******************************************************************************************************/
void MainWindow::onreturn4Clicked() {
    ui->stackedWidget->setCurrentIndex(4);  // Assuming frame index 1 is for login
}
/*******************************************************************************************************/
void MainWindow::onreturn5Clicked() {
    ui->stackedWidget->setCurrentIndex(2);  // Assuming frame index 1 is for login
}
/*******************************************************************************************************/
void MainWindow::onPLAYClicked() {
    ui->stackedWidget->setCurrentIndex(4);// Assuming frame index 1 is for login
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->playSelectionFrame));
}
/*******************************************************************************************************/

void MainWindow::onPVEButtonClicked()
{resetGame();
    bool ok;
    // Logic to start a Player vs AI game
    QMessageBox::information(this, "PVE", "Player vs AI mode selected!");
    againstAI=1;
    // Navigate to the actual game frame for PvE
     ui->stackedWidget->setCurrentIndex(6);
    resetGame2();

}
/*******************************************************************************************************/

void MainWindow::onPVPButtonClicked()
{resetGame();
    bool ok;
    againstAI=0;
    QString text = QInputDialog::getItem(this, tr("Player 2 Account"),
                                         tr("Does Player 2 have an account?"),
                                         QStringList() << "Yes" << "No", 0, false, &ok);
    if (ok && !text.isEmpty()) {
        if (text == "Yes") {

            ui->stackedWidget->setCurrentIndex(5);

        } else {
            ui->stackedWidget->setCurrentIndex(7);

        }
    }
}

/*******************************************************************************************************/

void MainWindow::on_showPasswordCheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        // Show the original Player 2 password
        ui->player2PasswordLineEdit->setEchoMode(QLineEdit::Normal);
    } else {
        // Hide the password (show black dots)
        ui->player2PasswordLineEdit->setEchoMode(QLineEdit::Password);
    }
}

/*******************************************************************************************************/
void MainWindow::onPlayer2LoginButtonClicked()
{

    // Store the original Player 2 password
    originalPlayer2Password = ui->player2PasswordLineEdit->text();

    // Hash the password
    std::string hashedPassword = hashPassword(originalPlayer2Password.toStdString());

    std::string emailPlayer2 = ui->player2EmailLineEdit->text().toStdString();

    // Fetch email of Player 1
    std::string emailPlayer1 = ui->userEmailLabel->text().toStdString(); // Assuming you have a label for Player 1's email

    // Check if Player 2's email is the same as Player 1's email
    if (emailPlayer2 == emailPlayer1) {
        QMessageBox::critical(this, "Login Error", "Player 2 cannot have the same email as Player 1.");
        return;
    }

    std::string result = login(db, emailPlayer2, hashedPassword);

    if (!result.empty()) {
        QMessageBox::information(this, "Login Successful", "Player 2 Logged In!");
        ui->stackedWidget->setCurrentIndex(6); // Replace with the actual name of your game frame widget
        initializeGame(); // Initialize the game if needed
        loadUserData(emailPlayer2); // Example to load player data
    } else {
        ui->player2LoginErrorLabel->setText("Invalid email or password.");
    }
}

/*******************************************************************************************************/

void MainWindow::onPlayer2SignupButtonClicked()
{    std::string password = hashPassword(ui->player2SignupPasswordLineEdit->text().toStdString());
    std::string email = ui->player2SignupEmailLineEdit->text().toStdString();

    std::string name = ui->player2SignupNameLineEdit->text().toStdString();
    std::string city = ui->player2SignupCityLineEdit->text().toStdString();
    int age = ui->player2SignupAgeLineEdit->text().toInt();

    std::string signupResult = signup(db, email, password, name, age, city);

    if (!signupResult.empty()) {
        QMessageBox::information(this, "Signup Successful", "Player 2 Signed Up! Please log in.");
        ui->stackedWidget->setCurrentIndex(5);
    } else {
        ui->player2SignupErrorLabel->setText("Signup failed. Please try again.");
    }
}
/*******************************************************************************************************/

void MainWindow::onSwitchToPlayer2SignupButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(7);
}
/*******************************************************************************************************/
void MainWindow::onSwitchToPlayer2LoginButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}
/*******************************************************************************************************/
std::string MainWindow::getLoggedInPlayerEmail() {
    // Example function to get the email of the logged in player
    return ui->emailLineEdit->text().toStdString();
}
/*******************************************************************************************************/
std::string MainWindow::getPlayer2Email() {
    // Example function to get the email of Player 2
    return ui->player2EmailLineEdit->text().toStdString();
}
/*******************************************************************************************************/
void MainWindow::onlogoutClicked(){
    ui->signupEmailLineEdit->clear();
    ui->signupPasswordLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->signupNameLineEdit->clear();
    ui->signupAgeLineEdit->clear();
    ui->signupCityLineEdit->clear();
    ui->userEmailLabel->clear();
    ui-> player2EmailLineEdit->clear();
    ui->player2PasswordLineEdit->clear();

    ui->stackedWidget->setCurrentIndex(0);}
/*******************************************************************************************************/
void MainWindow::onpgClicked() {
    resetGame();
    ui-> player2SignupEmailLineEdit->clear();
    ui->player2SignupPasswordLineEdit->clear();
    ui->player2SignupNameLineEdit->clear();
    ui->player2SignupAgeLineEdit->clear();
    ui->player2SignupCityLineEdit->clear();
    ui-> player2EmailLineEdit->clear();
    ui->player2PasswordLineEdit->clear();
    ui->stackedWidget->setCurrentIndex(4);

}
/*******************************************************************************************************/
void MainWindow:: onreturn6Clicked(){

    ui-> player2SignupEmailLineEdit->clear();
    ui->player2SignupPasswordLineEdit->clear();
    ui->player2SignupNameLineEdit->clear();
    ui->player2SignupAgeLineEdit->clear();
    ui->player2SignupCityLineEdit->clear();
    ui-> player2EmailLineEdit->clear();
    ui->player2PasswordLineEdit->clear();
    ui->stackedWidget->setCurrentIndex(4);


}
/*******************************************************************************************************/
/********************************FUNCTIONS FOR PLAYER STATISTICS****************************************/
/*******************************************************************************************************/



bool MainWindow::getPlayerStats(const std::string& email,int& pvp_win_count, int& pvp_lose_count, int& pvp_total_games,
                                int& pve_win_count, int& pve_lose_count, int& pve_total_games,
                                int& total_wins, int& total_loss, int& total_games) {
    sqlite3_stmt* stmt;
    std::string query = "SELECT pvp_win_count, pvp_lose_count, pvp_total_games, "
                        "pve_win_count, pve_lose_count, pve_total_games, "
                        "total_wins, total_loss, total_games FROM players WHERE email = ?";

    // Convert email string to char*
    const char* email_cstr = email.c_str();


    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        // Handle error
        return false;
    }
    // Bind email parameter

    sqlite3_bind_text(stmt, 1, email_cstr, -1, SQLITE_STATIC);

    // Execute the query
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Retrieve data from the query
        pvp_win_count = sqlite3_column_int(stmt, 0);
        pvp_lose_count = sqlite3_column_int(stmt, 1);
        pvp_total_games = sqlite3_column_int(stmt, 2);
        pve_win_count = sqlite3_column_int(stmt, 3);
        pve_lose_count = sqlite3_column_int(stmt, 4);
        pve_total_games = sqlite3_column_int(stmt, 5);
        total_wins = sqlite3_column_int(stmt, 6);
        total_loss = sqlite3_column_int(stmt, 7);
        total_games = sqlite3_column_int(stmt, 8);
    } else {
        // No row found for the given email
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}



/*******************************************************************************************************/


void MainWindow::updatePlayerStats(const std::string& email,  int pvp_win_count, int pvp_lose_count, int pvp_total_games,
                                   int pve_win_count, int pve_lose_count, int pve_total_games) {
    int total_wins = pvp_win_count + pve_win_count;
    int total_loss = pvp_lose_count + pve_lose_count;
    int total_games = pvp_total_games + pve_total_games;

    sqlite3_stmt* stmt;
    std::string query = "UPDATE players SET "
                        "pvp_win_count = ?, pvp_lose_count = ?, pvp_total_games = ?, "
                        "pve_win_count = ?, pve_lose_count = ?, pve_total_games = ?, "
                        "total_wins = ?, total_loss = ?, total_games = ? WHERE email = ?";

    // Convert email string to char*
    const char* email_cstr = email.c_str();

    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        // Handle error
        return;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, pvp_win_count);
    sqlite3_bind_int(stmt, 2, pvp_lose_count);
    sqlite3_bind_int(stmt, 3, pvp_total_games);
    sqlite3_bind_int(stmt, 4, pve_win_count);
    sqlite3_bind_int(stmt, 5, pve_lose_count);
    sqlite3_bind_int(stmt, 6, pve_total_games);
    sqlite3_bind_int(stmt, 7, total_wins);
    sqlite3_bind_int(stmt, 8, total_loss);
    sqlite3_bind_int(stmt, 9, total_games);
    sqlite3_bind_text(stmt, 10, email_cstr, -1, SQLITE_STATIC);

    // Execute the update
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        // Handle error
    }

    sqlite3_finalize(stmt);
}
/*******************************************************************************************************/

void MainWindow::handleGameOutcome(const std::string& player1Email, const std::string& player2Email, int res ) {
    // Variables for player 1 statistics
    int pvp_win_count_1, pvp_lose_count_1, pvp_total_games_1;
    int pvp_win_count_2, pvp_lose_count_2, pvp_total_games_2;
    int pve_win_count_1, pve_lose_count_1, pve_total_games_1;
    int pve_win_count_2, pve_lose_count_2, pve_total_games_2;
    int total_games_1=pvp_total_games_1+pve_total_games_1;
    int total_games_2=pvp_total_games_2+pve_total_games_2;
    int total_win_1=pvp_win_count_1+pve_win_count_1;
    int total_win_2=pvp_win_count_2+pve_win_count_2;
    int total_lose_1=pvp_lose_count_1+pve_lose_count_1;
    int total_lose_2=pvp_lose_count_2+pve_lose_count_2;
    res = board.checkWin();
    // Fetch statistics for both players
    if (!getPlayerStats(player1Email, pvp_win_count_1, pvp_lose_count_1, pvp_total_games_1, pve_win_count_1, pve_lose_count_1, pve_total_games_1,total_win_1,total_lose_1,total_games_1)) {
        qDebug() << "Error fetching player 1 stats.";
        return;
    }
    if(!againstAI){
        if (!getPlayerStats(player2Email, pvp_win_count_2, pvp_lose_count_2, pvp_total_games_2,pve_win_count_2, pve_lose_count_2, pve_total_games_2,total_win_2,total_lose_2,total_games_2)) {
            qDebug() << "Error fetching player 2 stats.";
            return;}
    }
    qInfo("%d",againstAI);
    // Update statistics based on game outcome
    if (res == 1) { // Player 1 wins

        if (!againstAI){
            pvp_win_count_1++;
            pvp_total_games_1++;
            pvp_lose_count_2;
            pvp_total_games_2;
        }
        if (againstAI){
            pve_win_count_1++;
            pve_total_games_1++;

        }
    }
    else if (res == -1) {
        if (!againstAI){
            pvp_win_count_2++;
            pvp_total_games_2++;
            pvp_lose_count_1++;
            pvp_total_games_1++;
        }
        if (againstAI){
            pve_lose_count_1++;
            pve_total_games_1++;
        }}
    if (res == 2){
        if (!againstAI){
            pvp_total_games_2++;
            pvp_total_games_1++;
        }
        if (againstAI){
            pve_total_games_1++;
        } }

    // Update the database with new stats
    updatePlayerStats(player1Email, pvp_win_count_1, pvp_lose_count_1, pvp_total_games_1,pve_win_count_1, pve_lose_count_1, pve_total_games_1);
    if(!againstAI){
        updatePlayerStats(player2Email, pvp_win_count_2, pvp_lose_count_2, pvp_total_games_2,pve_win_count_2, pve_lose_count_2, pve_total_games_2);
    }
}
/*******************************************************************************************************/
// Define the function to show player 1's statistics
void MainWindow::showPlayer1Stats() {
    // Get the email of player 1
    std::string emailPlayer1 = getLoggedInPlayerEmail();

    if (emailPlayer1.empty()) {
        QMessageBox::warning(this, "Error", "Player 1 is not logged in");
        return;
    }

    // Variables to store player 1's statistics
    int pvp_win_count, pvp_lose_count, pvp_total_games;
    int pve_win_count, pve_lose_count, pve_total_games;
    int total_wins, total_loss, total_games;

    // Get player 1's statistics
    if (!getPlayerStats(emailPlayer1, pvp_win_count, pvp_lose_count, pvp_total_games,
                        pve_win_count, pve_lose_count, pve_total_games,
                        total_wins, total_loss, total_games)) {
        QMessageBox::warning(this, "Error", "Failed to retrieve player 1's statistics");
        return;
    }

    // Display player 1's statistics
    QMessageBox::information(this, "Player 1 Statistics", QString("PvP Wins: %1\nPvP Losses: %2\nTotal PvP Games: %3\n"
                                                                  "PvE Wins: %4\nPvE Losses: %5\nTotal PvE Games: %6\n"
                                                                  "Total Wins: %7\nTotal Losses: %8\nTotal Games: %9")
                                                              .arg(pvp_win_count)
                                                              .arg(pvp_lose_count)
                                                              .arg(pvp_total_games)
                                                              .arg(pve_win_count)
                                                              .arg(pve_lose_count)
                                                              .arg(pve_total_games)
                                                              .arg(total_wins)
                                                              .arg(total_loss)
                                                              .arg(total_games));
}
/*******************************************************************************************************/
void MainWindow::showPlayer2Stats() {
    // Get the email of player 2
    std::string emailPlayer2 = getPlayer2Email();

    if (emailPlayer2.empty()) {
        QMessageBox::warning(this, "Error", "Player 2 is not logged in");
        return;
    }

    // Variables to store player 2's statistics
    int pvp_win_count, pvp_lose_count, pvp_total_games;
    int pve_win_count, pve_lose_count, pve_total_games;
    int total_wins, total_loss, total_games;

    // Get player 2's statistics
    if (!getPlayerStats(emailPlayer2, pvp_win_count, pvp_lose_count, pvp_total_games,
                        pve_win_count, pve_lose_count, pve_total_games,
                        total_wins, total_loss, total_games)) {
        QMessageBox::warning(this, "Error", "Failed to retrieve player 2's statistics");
        return;
    }

    // Display player 2's statistics
    QMessageBox::information(this, "Player 2 Statistics", QString("PvP Wins: %1\nPvP Losses: %2\nTotal PvP Games: %3\n"
                                                                  "PvE Wins: %4\nPvE Losses: %5\nTotal PvE Games: %6\n"
                                                                  "Total Wins: %7\nTotal Losses: %8\nTotal Games: %9")
                                                              .arg(pvp_win_count)
                                                              .arg(pvp_lose_count)
                                                              .arg(pvp_total_games)
                                                              .arg(pve_win_count)
                                                              .arg(pve_lose_count)
                                                              .arg(pve_total_games)
                                                              .arg(total_wins)
                                                              .arg(total_loss)
                                                              .arg(total_games));
}

/*******************************************************************************************************/
//Function to display player 1 information

void MainWindow::loadUserData(const std::string &email) {
    // SQL query to retrieve user data including name, email, age, city, total games, total wins, total losses, and last login date
    std::string sql = "SELECT name, age, total_games, total_wins, total_loss, last_login_date FROM players WHERE email = '" + email + "'";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        // Error handling for query preparation
        QMessageBox::critical(this, "Database Error", "Failed to prepare the SQL statement.");
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Retrieve data from the query
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        int age = sqlite3_column_int(stmt, 1);
        int totalGames = sqlite3_column_int(stmt, 2);
        int totalWins = sqlite3_column_int(stmt, 3);
        int totalLosses = sqlite3_column_int(stmt, 4);
        std::string lastLoginDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt,5));

        // Set the values to the corresponding labels in your UI
        ui->userNameLabel->setText(QString::fromStdString(name));
        ui->userEmailLabel->setText(QString::fromStdString(email));
        ui->userAgeLabel->setText(QString::number(age));
        ui->userGamesPlayedLabel->setText(QString::number(totalGames));
        ui->userWinsLabel->setText(QString::number(totalWins));
        ui->userLossesLabel->setText(QString::number(totalLosses));
        ui->userLastLoginLabel->setText(QString::fromStdString(lastLoginDate));
    }

    sqlite3_finalize(stmt);  // Finalize the statement
}


/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/

void MainWindow::verifyTablesExist() {
    const char* checkTablesSQL = R"(
        SELECT name FROM sqlite_master WHERE type='table' AND name IN ('games', 'moves', 'players');
    )";

    char* errMsg = nullptr;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, checkTablesSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    int tablesFound = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* tableName = sqlite3_column_text(stmt, 0);
        std::cout << "Found table: " << tableName << std::endl;
        tablesFound++;
    }

    if (tablesFound < 3) {
        std::cerr << "Not all required tables found in the database." << std::endl;
        QMessageBox::critical(this, "Database Error", "Required tables not found in database");
    }

    sqlite3_finalize(stmt);
}
/*******************************************************************************************************/
/*****************************************FUNCTIOCNS FOR DATABASE TABLES********************************/
/*******************************************************************************************************/

void MainWindow::createTablesIfNeeded() {


    const char* createPlayersTableSQL = R"(
    CREATE TABLE IF NOT EXISTS players (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        email TEXT UNIQUE,
        password TEXT,
        name TEXT,
        city TEXT,
        age INTEGER,
        pvp_win_count INTEGER DEFAULT 0,
        pvp_lose_count INTEGER DEFAULT 0,
        pvp_total_games INTEGER DEFAULT 0,
        pve_win_count INTEGER DEFAULT 0,
        pve_lose_count INTEGER DEFAULT 0,
        pve_total_games INTEGER DEFAULT 0,
        total_wins INTEGER DEFAULT 0,
        total_loss INTEGER DEFAULT 0,
        total_games INTEGER DEFAULT 0,
        current_date TEXT,
        last_login_date TEXT
        last_login_date TEXT
        last_login_date TEXT
    );
)";

    const char* creategamesTableSQL = R"(
        CREATE TABLE IF NOT EXISTS games (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            player1_email TEXT NOT NULL,
            player2_email TEXT NOT NULL,
            date TEXT NOT NULL,
            game_mode INTEGER NOT NULL

        );
    )";

    const char* createmovesTableSQL = R"(
        CREATE TABLE IF NOT EXISTS moves (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            game_id INTEGER NOT NULL,
            board TEXT NOT NULL,
            player_turn TEXT NOT NULL,
            move_number INTEGER NOT NULL,
        );
    )";

    char* errMsg = nullptr;
    int status = sqlite3_exec(db, creategamesTableSQL, nullptr, nullptr, &errMsg);
    if (status != SQLITE_OK) {
        std::cerr << "Error creating games table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return;
    }

    status = sqlite3_exec(db, createmovesTableSQL, nullptr, nullptr, &errMsg);
    if (status != SQLITE_OK) {
        std::cerr << "Error creating moves table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return;
    }

    status = sqlite3_exec(db, createPlayersTableSQL, nullptr, nullptr, &errMsg);
    if (status != SQLITE_OK) {
        std::cerr << "Error creating Players table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return;
    }

    std::cout << "Tables created successfully or already exist." << std::endl;
}

/*******************************************************************************************************/
/*******************************FUNCTIONS FOR HANDLING GAME HISTORY*************************************/
/*******************************************************************************************************/
//Function to save each move in the game

void MainWindow::saveMove(int gameId, const GameBoard& board, const std::string& playerTurn, int moveNumber, sqlite3* db) {
    char boardStr[10];
    int index = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board.getValue(i, j) == 1) {
                boardStr[index] = 'X';
            } else if (board.getValue(i, j) == -1) {
                boardStr[index] = 'O';
            } else {
                boardStr[index] = '-';
            }
            index++;
        }
    }
    boardStr[9] = '\0';

    qInfo("%d %s %s %d",gameId,boardStr, playerTurn.c_str() ,moveNumber);
    std::string insertMoveSQL = "INSERT INTO moves (game_id, board, player_turn, move_number) "
                                "VALUES (" + std::to_string(gameId) + ", '" + boardStr + "', '" + playerTurn + "', " + std::to_string(moveNumber) + ")";

    char* errMsg = nullptr;
    int status = sqlite3_exec(db, insertMoveSQL.c_str(), nullptr, nullptr, &errMsg);
    if (status != SQLITE_OK) {
        std::cerr << "Error inserting move: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    int lastGameId = sqlite3_last_insert_rowid(db);
    qInfo("%d",lastGameId);
}


std::vector<int> MainWindow::getPlayerGameIds(const std::string& playerEmail, sqlite3* db) {
    std::vector<int> gameIds;
    std::string querySQL = "SELECT id FROM games WHERE player1_email = ? OR player2_email = ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return gameIds;
    }

    sqlite3_bind_text(stmt, 1, playerEmail.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, playerEmail.c_str(), -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int gameId = sqlite3_column_int(stmt, 0);
        gameIds.push_back(gameId);
    }

    sqlite3_finalize(stmt);
    gameIds.size();
    return gameIds;
}




/*******************************************************************************************************/

void MainWindow::showPlayerGameIds() {
    std::string emailPlayer1 = getLoggedInPlayerEmail(); // Replace with your logic to get Player 1's email

    std::vector<int> gameIds = getPlayerGameIds(emailPlayer1, db);

    if (gameIds.empty()) {
        QMessageBox::information(this, "Player 1 Game IDs", "Player 1 has no recorded games.");
        return;
    }
    int* gameIdsArray = new int[gameIds.size()];
    for (size_t i = 0; i < gameIds.size(); ++i) {
        gameIdsArray[i] = gameIds[i];
    }

    QStringList gameIdsStringList;
    for (size_t i = 0; i < gameIds.size(); ++i) {
        gameIdsStringList << QString::number(i + 1);
    }

    bool ok;
    QString gameIdText = QInputDialog::getItem(this, "Select Game ID", "Select a game ID:", gameIdsStringList, 0, false, &ok);
    if (ok && !gameIdText.isEmpty()) {
        int selectedIndex = gameIdText.toInt() - 1;
        int gameId = gameIdsArray[selectedIndex];
        showGameOptions(emailPlayer1, gameId);
    }

    delete[] gameIdsArray;
}



/*************************************************************************************************************************/

void MainWindow::showMoveByMove(int gameId, const std::string& playerEmail) {
    resetGame1();  // Reset the board before showing the moves

    std::string querySQL = "SELECT board, player_turn FROM moves WHERE game_id = ? ORDER BY move_number";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, gameId);

    moves.clear(); // Clear any previous moves
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* boardStr = sqlite3_column_text(stmt, 0);
        const unsigned char* playerTurn = sqlite3_column_text(stmt, 1);
        if (boardStr && playerTurn) {
            moves.emplace_back(reinterpret_cast<const char*>(boardStr), reinterpret_cast<const char*>(playerTurn));
        }
    }

    if (moves.empty()) {
        QMessageBox::information(this, "Game Moves", "No moves found for game " + QString::number(gameId));
    } else {
        QMessageBox::information(this, "Game Moves", "Press OK to start showing moves.");
        currentMoveIndex = 0;

        moveTimer = new QTimer(this);
        connect(moveTimer, &QTimer::timeout, this, &MainWindow::showNextMove);
        moveTimer->start(1000); // Adjust the interval as needed (1000 ms = 1 second)
    }

    sqlite3_finalize(stmt);
}
/*******************************************************************************************************/
void MainWindow::showNextMove() {
    if (currentMoveIndex < moves.size()) {
        const std::string& boardStr = moves[currentMoveIndex].first;
        const std::string& playerTurn = moves[currentMoveIndex].second;

        showBoard1(boardStr.c_str());

        QString moveDescription = "Player's move: " + QString::fromStdString(playerTurn);
        QMessageBox::information(this, "Move Details", moveDescription);

        ++currentMoveIndex;
    } else {
        moveTimer->stop();
    }
}
void MainWindow::showFinalMove(int gameId) {
    std::string querySQL = "SELECT board FROM moves WHERE game_id = ? ORDER BY move_number DESC LIMIT 1";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, gameId);

    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* boardStr = sqlite3_column_text(stmt, 0);
        if (boardStr) {
            showBoard1(reinterpret_cast<const char*>(boardStr));
        } else {
            QMessageBox::information(this, "Game Moves", "No final move found for game " + QString::number(gameId));
        }
    } else {
        QMessageBox::information(this, "Game Moves", "No final move found for game " + QString::number(gameId));
    }

    sqlite3_finalize(stmt);
}

/*******************************************************************************************************/
void MainWindow::showGameOptions(const std::string& playerEmail, int gameId) {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Game Options",
                                                              "yes:: view the final move    No:: view moves step by step?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Show final move for the selected game ID
          ui->stackedWidget->setCurrentIndex(8);

        showFinalMove(gameId);
          showGameDetails(gameId);
    } else {
        ui->stackedWidget->setCurrentIndex(8);

        onmbmClicked(gameId);
        showGameDetails(gameId);
    }
}
/*******************************************************************************************************/

void MainWindow::onmbmClicked(int gameId) {
    std::string emailPlayer1 = getLoggedInPlayerEmail();
    showMoveByMove(gameId, emailPlayer1);
}

/*******************************************************************************************************/
std::tuple<std::string, std::string, std::string, int, std::vector<std::string>> MainWindow::getGameDetails(int gameId, sqlite3* db) {
    std::string querySQL = "SELECT player1_email, player2_email, date, game_mode FROM games WHERE id = ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return std::make_tuple("", "", "", -1, std::vector<std::string>());
    }

    sqlite3_bind_int(stmt, 1, gameId);

    std::string player1Email, player2Email, date;
    int gameMode = -1;

    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        player1Email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        player2Email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        gameMode = sqlite3_column_int(stmt, 3);
    } else {
        std::cerr << "Error executing SQL statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    // Fetch moves for this game
    std::vector<std::string> playerTurns;
    querySQL = "SELECT player_turn FROM moves WHERE game_id = ?";
    rc = sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return std::make_tuple(player1Email, player2Email, date, gameMode, playerTurns);
    }

    sqlite3_bind_int(stmt, 1, gameId);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* playerTurn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        playerTurns.push_back(playerTurn);
    }

    sqlite3_finalize(stmt);

    return std::make_tuple(player1Email, player2Email, date, gameMode, playerTurns);
}
/*******************************************************************************************************/
void MainWindow::showGameDetails(int gameId) {
    std::string emailPlayer1 = getLoggedInPlayerEmail(); // Replace with your logic to get Player 1's email

    std::vector<int> gameIds = getPlayerGameIds(emailPlayer1, db);

    if (gameIds.empty()) {
        ui->label_12->setText("No games found for Player 1.");
        return;
    }

    int selectedIndex = -1;
    for (size_t i = 0; i < gameIds.size(); ++i) {
        if (gameIds[i] == gameId) {
            selectedIndex = i + 1; // Store the index (1-based)
            break;
        }
    }

    if (selectedIndex != -1) {
        auto [player1Email, player2Email, date, gameMode, playerTurns] = getGameDetails(gameId, db);

        QString details = QString("Game Index: %1\nPlayer 1 Email: %2\nPlayer 2 Email: %3\nDate: %4\nGame Mode: %5\n  ")
                              .arg(selectedIndex)
                              .arg(QString::fromStdString(player1Email))
                              .arg(QString::fromStdString(player2Email))
                              .arg(QString::fromStdString(date))
                              .arg(gameMode);
        for (size_t i = 0; i < 2; ++i) {
            QString moveDetails = QString("player_symbol %1: %2 \n").arg(i + 1).arg(QString::fromStdString(playerTurns[i]));
            details.append(moveDetails);
        }

        // Show the moves and determine the winner or draw
        int result = board.checkWin();
        QString winnerMessage;
        if (result == 1) {
            winnerMessage = "Player 1 wins!";
        } else if (result == -1) {
            if (againstAI) {
                winnerMessage = "AI wins!";
            } else {
                winnerMessage = "Player 2 wins!";
            }
        } else if (result == 2) {
            winnerMessage = "It's a draw!";
        }

        details.append(QString("Result: %1").arg(winnerMessage));

        // Assuming ui->label_12 is your QLabel widget for displaying game details
        ui->label_12->setText(details);
    } else {
        ui->label_12->setText("Game details not found.");
    }
}
