#include "mainwindow.h"  // Include the main window class
#include "ui_mainwindow.h"  // Include the UI definitions
#include <QMessageBox>  // For displaying message boxes
#include <QInputDialog>  // For input dialogs
#include <sqlite3.h>  // SQLite database
#include <chrono>  // For date-time operations
#include <iostream>  // Standard IO operations
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>  // Standard string operations
#include <QTextStream>  // For handling Qt's string input/output
// Declare a global database connection
sqlite3* db;
// Function to convert time to string
std::string timeToString(std::chrono::system_clock::time_point timePoint) {
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tm;
    localtime_s(&tm, &time);  // Use localtime_s to convert time to tm struct in a thread-safe manner

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");  // Format the time using put_time
    return oss.str();  // Return the formatted time string
}

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

// Function to hash a password to a string
std::string hashPassword(const std::string& password) {
    uint64_t hash = customHash(password);
    return std::to_string(hash);
}

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



void MainWindow::loadUserData(const std::string &email) {
    // SQL query to retrieve user data
    std::string sql = "SELECT name, age, total_games, pvp_win_count, pvp_lose_count, pvp_total_games ,last_login_date FROM players WHERE email = '" + email + "'";
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
        int pvpWins = sqlite3_column_int(stmt, 3);
        int pvpLosses = sqlite3_column_int(stmt, 4);
        int totalGamespvp = sqlite3_column_int(stmt, 5);
        std::string lastLoginDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));

        // Set the values to the corresponding labels in your frame
        ui->userNameLabel->setText(QString::fromStdString(name));
        ui->userNameLabel2->setText(QString::fromStdString(name));
        ui->userEmailLabel->setText(QString::fromStdString(email));
        ui->userAgeLabel->setText(QString::number(age));
        ui->userGamesPlayedLabel->setText(QString::number(totalGamespvp));
        ui->userWinsLabel->setText(QString::number(pvpWins));
        ui->userLossesLabel->setText(QString::number(pvpLosses));
        ui->userLastLoginLabel->setText(QString::fromStdString(lastLoginDate));
    }

    sqlite3_finalize(stmt);  // Finalize the statement
}

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
// Define the MainWindow class constructor and other components
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);  // Set up the UI components

    // Set up the SQLite database connection
    if (sqlite3_open("tictactoe22.db", &db) != SQLITE_OK) {
        QMessageBox::critical(this, "Database Error", "Cannot open database");
        return;
    }
    // Ensure the 'players' table exists; create it if it doesn't
    const char* createPlayersTableSQL =
        "CREATE TABLE IF NOT EXISTS players ("
        "id INTEGER PRIMARY KEY, "
        "email TEXT UNIQUE, "
        "password TEXT, "
        "name TEXT, "
        "city TEXT, "
        "age INTEGER, "
        "pvp_win_count INTEGER DEFAULT 0, " // Win count for Player vs. Player games
        "pvp_lose_count INTEGER DEFAULT 0, " // Lose count for Player vs. Player games
        "pvp_total_games INTEGER DEFAULT 0, " // Total games for Player vs. Player
        "pve_win_count INTEGER DEFAULT 0, " // Win count for Player vs. AI games
        "pve_lose_count INTEGER DEFAULT 0, " // Lose count for Player vs. AI games
        "pve_total_games INTEGER DEFAULT 0, " // Total games for Player vs. AI
        "total_games INTEGER DEFAULT 0,     "
        "current_date TEXT, " // Date of account creation
        "last_login_date TEXT" // Last login date
        "last_login_date TEXT" // Last login date
        "last_login_date TEXT" // Last login date
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, createPlayersTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        QMessageBox::critical(this, "Database Error", "Failed to create the 'players' table");
        sqlite3_free(errMsg);
        return;
    }
    // Initialize frames
    player2LoginFrame = new QFrame();
    player2SignupFrame = new QFrame();
    pvpGameFrame = new QFrame();

    // Add frames to stacked widget
    ui->stackedWidget->addWidget(player2LoginFrame);
    ui->stackedWidget->addWidget(player2SignupFrame);
    ui->stackedWidget->addWidget(pvpGameFrame);

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
    // Set the initial frame to the welcome frame
    ui->stackedWidget->setCurrentIndex(0);  // Index 0 for 'firstframe'

    for (int i = 1; i <= 9; ++i) {
        QString buttonName = "button" + QString::number(i);
        QPushButton* button = findChild<QPushButton*>(buttonName);
        if (button) {
            connect(button, &QPushButton::clicked, this, &MainWindow::handleButtonClick);
        }
    }

    initializeGame();

}

MainWindow::~MainWindow() {
    if (db) {
        sqlite3_close(db);  // Properly close the database connection
    }
    delete ui;  // Clean up UI components
}
void MainWindow::initializeGame() {
    memset(board, ' ', sizeof(board));
    currentPlayer = 'X';
    gameActive = true;
    ui->statusLabel->setText("Player X's turn");
    updateBoardUI();
}
void MainWindow::handleButtonClick() {
    if (!gameActive) return;

    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;

    QString buttonName = clickedButton->objectName();
    int index = buttonName.right(1).toInt() - 1;
    int row = index / 3;
    int col = index % 3;

    if (board[row][col] == ' ') {
        board[row][col] = currentPlayer;
        updateBoardUI();
        checkGameStatus();
        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
        if (gameActive) {
            ui->statusLabel->setText(QString("Player %1's turn").arg(currentPlayer));
        }
    }
}

void MainWindow::updateBoardUI() {
    for (int i = 1; i <= 9; ++i) {
        QString buttonName = "button" + QString::number(i);
        QPushButton* button = findChild<QPushButton*>(buttonName);
        int index = i - 1;
        int row = index / 3;
        int col = index % 3;
        if (button) {
            button->setText(QString(board[row][col]));
        }
    }
}


bool MainWindow::getPlayerStats(const std::string& email, int& pvp_win_count, int& pvp_lose_count, int& pvp_total_games) {
    // Assuming you have an SQLite database connection
    sqlite3_stmt* stmt;
    std::string query = "SELECT pvp_win_count, pvp_lose_count, pvp_total_games FROM players WHERE email = ?";

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
    } else {
        // No row found for the given email
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

void MainWindow::updatePlayerStats(const std::string& email, int pvp_win_count, int pvp_lose_count, int pvp_total_games) {
    // Assuming you have an SQLite database connection
    sqlite3_stmt* stmt;
    std::string query = "UPDATE players SET pvp_win_count = ?, pvp_lose_count = ?, pvp_total_games = ? WHERE email = ?";

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
    sqlite3_bind_text(stmt, 4, email_cstr, -1, SQLITE_STATIC);

    // Execute the update
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        // Handle error
    }

    sqlite3_finalize(stmt);
}

void MainWindow::checkGameStatus() {
    bool win = false;
    char winner = ' ';

    // Check rows
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            win = true;
            winner = board[i][0];
        }
    }

    // Check columns
    for (int i = 0; i < 3; ++i) {
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            win = true;
            winner = board[0][i];
        }
    }

    // Check diagonals
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        win = true;
        winner = board[0][0];
    }
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        win = true;
        winner = board[0][2];
    }

    // Check for draw
    bool isDraw = true;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == ' ') {
                isDraw = false;
                break;
            }
        }
        if (!isDraw) break;
    }

    // Update game status and player statistics
    if (win) {
        gameActive = false;
        ui->statusLabel->setText(QString("Player %1 wins!").arg(winner));

        // Update statistics for both players
        std::string emailPlayer1 = getLoggedInPlayerEmail(); // Get email of Player 1
        std::string emailPlayer2 = getPlayer2Email();

        if (emailPlayer1.empty() || emailPlayer2.empty()) {
            qDebug() << "Error: Player emails are not valid.";
            return;
        }


        // Handle game win logic
        handleGameOutcome(emailPlayer1, emailPlayer2, 1, 1); // 1 means win
    } else if (isDraw) {
        gameActive = false;
        ui->statusLabel->setText("It's a draw!");

        // Update statistics for both players
        std::string emailPlayer1 = getLoggedInPlayerEmail(); // Get email of Player 1
        std::string emailPlayer2 = getPlayer2Email();

        if (emailPlayer1.empty() || emailPlayer2.empty()) {
            qDebug() << "Error: Player emails are not valid.";
            return;
        }

        // Check if Player 2's email is the same as Player 1's email
        if (emailPlayer2 == emailPlayer1) {
            QMessageBox::critical(this, "Login Error", "Player 2 cannot have the same email as Player 1.");
            return;
        }

        // Handle game draw logic
        handleGameOutcome(emailPlayer1, emailPlayer2, 2, 0); // 2 means draw
    }
}

void MainWindow::handleGameOutcome(const std::string& player1Email, const std::string& player2Email, int gameStatus, int gameOutcome) {
    // Variables for player 1 statistics
    int pvp_win_count1, pvp_lose_count1, pvp_total_games1;

    // Variables for player 2 statistics
    int pvp_win_count2, pvp_lose_count2, pvp_total_games2;

    // Fetch statistics for both players
    if (!getPlayerStats(player1Email, pvp_win_count1, pvp_lose_count1, pvp_total_games1)) {
        qDebug() << "Error fetching player 1 stats.";
        return;
    }

    if (!getPlayerStats(player2Email, pvp_win_count2, pvp_lose_count2, pvp_total_games2)) {
        qDebug() << "Error fetching player 2 stats.";
        return;
    }

    // Update statistics based on game outcome
    if (gameStatus == 1) { // Player 1 wins
        pvp_win_count1++;
        pvp_total_games1++;
        pvp_lose_count2++;
        pvp_total_games2++;

    } else if (gameStatus == 2) { // Draw
        pvp_total_games1++;
        pvp_total_games2++;
    } else { // Player 2 wins (assuming gameStatus == 0)
        pvp_win_count2++;
        pvp_total_games2++;
        pvp_lose_count1++;
        pvp_total_games1++;
    }

    // Update the database with new stats
    updatePlayerStats(player1Email, pvp_win_count1, pvp_lose_count1, pvp_total_games1);
    updatePlayerStats(player2Email, pvp_win_count2, pvp_lose_count2, pvp_total_games2);
}

void MainWindow::handleGameWin(const std::string& player1Email, const std::string& player2Email) {
    // Notify user of the win
    ui->statusLabel->setText("Player 1 wins!");

    // Handle game win logic
    handleGameOutcome(player1Email, player2Email, 1, 1); // 1 means win
}

void MainWindow::handleGameDraw(const std::string& player1Email, const std::string& player2Email) {
    // Notify user of the draw
    ui->statusLabel->setText("It's a draw!");

    // Handle game draw logic
    handleGameOutcome(player1Email, player2Email, 2, 0); // 2 means draw
}

// Example slot implementations for login and signup (adjust to fit your application)
void MainWindow::onLoginButtonClicked() {
    // Convert QString to std::string for hashing
    std::string password = hashPassword(ui->passwordLineEdit->text().toStdString());

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

// Slot implementations for switching frames
void MainWindow::onSwitchToSignupButtonClicked() {
    ui->stackedWidget->setCurrentIndex(3);  // Switch to the signup frame
}

void MainWindow::onSwitchToLoginButtonClicked() {
    ui->stackedWidget->setCurrentIndex(1);  // Switch back to the login frame
}
// Slot to switch to the signup frame
void MainWindow::onswitchToSignupFrameClicked() {
    ui->stackedWidget->setCurrentIndex(3);  // Assuming frame index 2 is for signup
}

// Slot to switch to the login frame
void MainWindow::onswitchToLoginFrameClicked() {
    ui->stackedWidget->setCurrentIndex(1);  // Assuming frame index 1 is for login
}
void MainWindow::onreturn1Clicked() {
    ui->stackedWidget->setCurrentIndex(0);  // Assuming frame index 1 is for login
}
void MainWindow::onreturn2Clicked() {
    ui->stackedWidget->setCurrentIndex(0);  // Assuming frame index 1 is for login
}
void MainWindow::onPLAYClicked() {
    ui->stackedWidget->setCurrentIndex(4);// Assuming frame index 1 is for login
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->playSelectionFrame));
}




void MainWindow::onPVEButtonClicked()
{
    // Logic to start a Player vs AI game
    QMessageBox::information(this, "PVE", "Player vs AI mode selected!");
    // Navigate to the actual game frame for PvE
}

void MainWindow::onPVPButtonClicked()
{
    bool ok;
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

void MainWindow::onPlayer2LoginButtonClicked()
{
    std::string emailPlayer2 = ui->player2EmailLineEdit->text().toStdString();
    std::string password = hashPassword(ui->player2PasswordLineEdit->text().toStdString());

    // Fetch email of Player 1 (You should have a member variable or a way to access it)
    std::string emailPlayer1 = ui->userEmailLabel->text().toStdString(); // Assuming you have a label for Player 1's email

    // Check if Player 2's email is the same as Player 1's email
    if (emailPlayer2 == emailPlayer1) {
        QMessageBox::critical(this, "Login Error", "Player 2 cannot have the same email as Player 1.");
        return;
    }

    std::string result = login(db, emailPlayer2, password);

    if (!result.empty()) {
        QMessageBox::information(this, "Login Successful", "Player 2 Logged In!");

        ui->stackedWidget->setCurrentIndex(6); // Replace with the actual name of your game frame widget
        initializeGame(); // Initialize the game if needed

        loadUserData(emailPlayer2); // Example to load player data
    } else {
        ui->player2LoginErrorLabel->setText("Invalid email or password.");
    }
}

void MainWindow::onPlayer2SignupButtonClicked()
{
    std::string email = ui->player2SignupEmailLineEdit->text().toStdString();
    std::string password = hashPassword(ui->player2SignupPasswordLineEdit->text().toStdString());
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

void MainWindow::onSwitchToPlayer2SignupButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(7);
}

void MainWindow::onSwitchToPlayer2LoginButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}
std::string MainWindow::getLoggedInPlayerEmail() {
    // Example function to get the email of the logged in player
    return ui->emailLineEdit->text().toStdString();
}

std::string MainWindow::getPlayer2Email() {
    // Example function to get the email of Player 2
    return ui->player2EmailLineEdit->text().toStdString();
}
