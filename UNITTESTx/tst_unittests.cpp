#include "C:\Users\Maha Yasser\OneDrive\Desktop\elhamdullahfinalfinal\elhamdullah4\aiplayer.h"
#include "C:\Users\Maha Yasser\OneDrive\Desktop\elhamdullahfinalfinal\elhamdullah4\gameboard.h"
#include <QTest>
#include <iostream>

// Test fixture for AIPlayer unit tests
class Tests : public QObject {
    Q_OBJECT

private slots:

    //aiplayer tests
    void testAIMoveEmptyBoard();
    void testAIMovePartialBoard();
    void testBuildTree();
    void testMinimaxInitialBoard();
    void testMinimaxMaximizerMove();
    void testMinimaxMinimizerMove();
    void testMinimaxComplexStateDepth5();
    void testMinimaxComplexStateDifferentPlayer();
    void testEvaluateAIWinsHorizontally();
    void testEvaluateAIWinsVertically();
    void testEvaluateAIWinsDiagonally();
    void testEvaluatePlayerWinsHorizontally();
    void testEvaluatePlayerWinsVertically();
    void testEvaluatePlayerWinsDiagonally();
    void testEvaluateDraw();

    //gameboard tests
    void testPlayer1WinsRow();
    void testPlayer2WinsColumn();
    void testPlayer1WinsDiagonal();
    void testPlayer2WinsDiagonal();
    void testPlayer2WinsAntiDiagonal();
    void testPlayer2WinsRow();
    void testPlayer1WinsColumn();
    void testPlayer1WinsAntiDiagonal();
    void testGameDraw();
    void testGameNotOver();
    void testInitialEmptyBoard();
    void testSetGetValue();
    void testOutOfBoundsAccess();



    // Helper function to create a board with a specific state
    GameBoard createBoard(std::initializer_list<std::initializer_list<int>> values) {
        GameBoard board;
        int row = 0;
        for (auto& rowValues : values) {
            int col = 0;
            for (auto value : rowValues) {
                board.setValue(row, col, value);
                ++col;
            }
            ++row;
        }
        return board;
    }
};


void Tests::testAIMoveEmptyBoard() {
    GameBoard board;
    AIPlayer aiPlayer;

    // Simulate an empty board
    aiPlayer.makeMove(board);

    // Assert that AI has made a move
    bool moveMade = false;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board.getValue(i, j) != 0) {
                moveMade = true;
                break;
            }
        }
    }
    QVERIFY(moveMade);
}

void Tests::testAIMovePartialBoard() {
    GameBoard board;
    AIPlayer aiPlayer;

    // Set up a partially filled board
    board.setValue(0, 0, 1);
    board.setValue(0, 1, -1);
    board.setValue(1, 1, 1);

    aiPlayer.makeMove(board);

    // Assert that AI has made a move in one of the empty spaces
    bool moveMade = false;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board.getValue(i, j) != 0) {
                if (!(i == 0 && j == 0) && !(i == 0 && j == 1) && !(i == 1 && j == 1)) {
                    moveMade = true;
                    break;
                }
            }
        }
    }
    QVERIFY(moveMade);
}



// Test AIPlayer build_tree function
void Tests::testBuildTree() {
    GameBoard board;
    AIPlayer aiPlayer;
    TreeNode* root = new TreeNode;

    // Simulate an initial board state and build the tree
    board.setValue(0, 0, 0); // Empty board
    aiPlayer.build_tree(root, -1);

    // Assert that root node has children corresponding to possible moves
    QVERIFY(root->children.size() > 0);

    delete root;
}

// Test AIPlayer minimax function with initial board state (empty board)
void Tests::testMinimaxInitialBoard() {
    GameBoard board;
    AIPlayer aiPlayer;

    TreeNode* root = new TreeNode;
    board.setValue(0, 0, 0); // Empty board
    aiPlayer.build_tree(root, -1);
    int score = aiPlayer.minimax(root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true, 3);

    QVERIFY(score >= -1000);
    delete root;
}

// Test AIPlayer minimax function with one move made by the maximizer
void Tests::testMinimaxMaximizerMove() {
    GameBoard board;
    AIPlayer aiPlayer;

    TreeNode* root = new TreeNode;
    board.setValue(0, 0, 1); // Maximizer's move
    aiPlayer.build_tree(root, -1);
    int score = aiPlayer.minimax(root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false, 3);

    QVERIFY(score <= 1000);
    delete root;
}

// Test AIPlayer minimax function with one move made by the minimizer
void Tests::testMinimaxMinimizerMove() {
    GameBoard board;
    AIPlayer aiPlayer;

    TreeNode* root = new TreeNode;
    board.setValue(0, 0, -1); // Minimizer's move
    aiPlayer.build_tree(root, 1);
    int score = aiPlayer.minimax(root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true, 3);

    QVERIFY(score >= -1000);
    delete root;
}

// Test AIPlayer minimax function with a complex board state and deeper depth
void Tests::testMinimaxComplexStateDepth5() {
    GameBoard board;
    AIPlayer aiPlayer;

    TreeNode* root = new TreeNode;
    board.setValue(0, 0, 1);
    board.setValue(1, 1, -1);
    board.setValue(2, 2, 1);
    aiPlayer.build_tree(root, -1);
    int score = aiPlayer.minimax(root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true, 5);

    QVERIFY(score >= -1000);
    delete root;
}

// Test AIPlayer minimax function with another complex state and different starting player
void Tests::testMinimaxComplexStateDifferentPlayer() {
    GameBoard board;
    AIPlayer aiPlayer;

    TreeNode* root = new TreeNode;
    board.setValue(0, 0, -1);
    board.setValue(0, 1, 1);
    board.setValue(0, 2, -1);
    board.setValue(1, 0, 1);
    aiPlayer.build_tree(root, -1);
    int score = aiPlayer.minimax(root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false, 4);

    QVERIFY(score <= 1000);
    delete root;
}


void Tests::testEvaluateAIWinsHorizontally() {
    AIPlayer aiPlayer;
    GameBoard board = createBoard({
        { -1, -1, -1 },
        {  0,  0,  0 },
        {  0,  0,  0 }
    });
    QCOMPARE(aiPlayer.evaluate(board), 1000);
}

void Tests::testEvaluateAIWinsVertically() {
    AIPlayer aiPlayer;
    GameBoard board = createBoard({
        { -1,  0,  0 },
        { -1,  0,  0 },
        { -1,  0,  0 }
    });
    QCOMPARE(aiPlayer.evaluate(board), 1000);
}

void Tests::testEvaluateAIWinsDiagonally() {
    AIPlayer aiPlayer;
    GameBoard board = createBoard({
        { -1,  0,  0 },
        {  0, -1,  0 },
        {  0,  0, -1 }
    });
    QCOMPARE(aiPlayer.evaluate(board), 1000);
}

void Tests::testEvaluatePlayerWinsHorizontally() {
    AIPlayer aiPlayer;
    GameBoard board = createBoard({
        {  1,  1,  1 },
        {  0,  0,  0 },
        {  0,  0,  0 }
    });
    QCOMPARE(aiPlayer.evaluate(board), -1000);
}

void Tests::testEvaluatePlayerWinsVertically() {
    AIPlayer aiPlayer;
    GameBoard board = createBoard({
        {  0,  0,  1 },
        {  0,  0,  1 },
        {  0,  0,  1 }
    });
    QCOMPARE(aiPlayer.evaluate(board), -1000);
}

void Tests::testEvaluatePlayerWinsDiagonally() {
    AIPlayer aiPlayer;
    GameBoard board = createBoard({
        {  1,  0,  0 },
        {  0,  1,  0 },
        {  0,  0,  1 }
    });
    QCOMPARE(aiPlayer.evaluate(board), -1000);
}

void Tests::testEvaluateDraw() {
    AIPlayer aiPlayer;
    GameBoard board = createBoard({
        {  1, -1,  1 },
        { -1,  1, -1 },
        { -1,  1, -1 }
    });
    QCOMPARE(aiPlayer.evaluate(board), 0);
}


void Tests::testPlayer1WinsRow() {
    GameBoard board;
    // Set up a board where Player 1 wins by a row
    board.setValue(0, 0, 1);
    board.setValue(0, 1, 1);
    board.setValue(0, 2, 1);
    QCOMPARE(board.checkWin(), 1);
}

void Tests::testPlayer2WinsRow() {
    GameBoard board;
    // Set up a board where Player 2 wins by a row
    board.setValue(1, 0, -1);
    board.setValue(1, 1, -1);
    board.setValue(1, 2, -1);
    QCOMPARE(board.checkWin(), -1);
}

void Tests::testPlayer1WinsColumn() {
    GameBoard board;
    // Set up a board where Player 1 wins by a column
    board.setValue(0, 2, 1);
    board.setValue(1, 2, 1);
    board.setValue(2, 2, 1);
    QCOMPARE(board.checkWin(), 1);
}

void Tests::testPlayer2WinsColumn() {
    GameBoard board;
    // Set up a board where Player 2 wins by a column
    board.setValue(0, 0, -1);
    board.setValue(1, 0, -1);
    board.setValue(2, 0, -1);
    QCOMPARE(board.checkWin(), -1);
}

void Tests::testPlayer1WinsDiagonal() {
    GameBoard board;
    // Set up a board where Player 1 wins by the main diagonal
    board.setValue(0, 0, 1);
    board.setValue(1, 1, 1);
    board.setValue(2, 2, 1);
    QCOMPARE(board.checkWin(), 1);
}

void Tests::testPlayer2WinsDiagonal(){
    GameBoard board;
    // Set up a board where Player 2 wins by the main diagonal
    board.setValue(0, 0, -1);
    board.setValue(1, 1, -1);
    board.setValue(2, 2, -1);
    QCOMPARE(board.checkWin(), -1);
}


void Tests::testPlayer1WinsAntiDiagonal() {
    GameBoard board;
    // Set up a board where Player 1 wins by the anti-diagonal
    board.setValue(0, 2, 1);
    board.setValue(1, 1, 1);
    board.setValue(2, 0, 1);
    QCOMPARE(board.checkWin(), 1);
}


void Tests::testPlayer2WinsAntiDiagonal() {
    GameBoard board;
    // Set up a board where Player 2 wins by the anti-diagonal
    board.setValue(0, 2, -1);
    board.setValue(1, 1, -1);
    board.setValue(2, 0, -1);
    QCOMPARE(board.checkWin(), -1);
}

void Tests::testGameNotOver() {
    GameBoard board;
    // Set up a board where the game is not over yet
    board.setValue(0, 0, 1);
    board.setValue(1, 1, -1);
    board.setValue(2, 2, 0);
    QCOMPARE(board.checkWin(), 0);
}

void Tests::testGameDraw() {
    GameBoard board;
    // Set up a board where the game is a draw
    board.setValue(0, 0, 1);
    board.setValue(0, 1, -1);
    board.setValue(0, 2, 1);
    board.setValue(1, 0, -1);
    board.setValue(1, 1, 1);
    board.setValue(1, 2, -1);
    board.setValue(2, 0, -1);
    board.setValue(2, 1, 1);
    board.setValue(2, 2, -1);
    QCOMPARE(board.checkWin(), 2);
}




void Tests::testInitialEmptyBoard() {
    GameBoard board;
    // Test the initial empty board state
    QCOMPARE(board.checkWin(), 0);
}

void Tests::testSetGetValue() {
    GameBoard board;

    // Set up a specific board state for testing
    board.setValue(0, 0, 1);
    board.setValue(1, 1, -1);
    board.setValue(2, 2, 0);

    // Verify the values retrieved from the board
    QCOMPARE(board.getValue(0, 0), 1); // Expected value: 1
    QCOMPARE(board.getValue(1, 1), -1); // Expected value: -1
    QCOMPARE(board.getValue(2, 2), 0); // Expected value: 0

    // Modify values and verify again
    board.setValue(0, 0, -1);
    board.setValue(1, 1, 1);
    board.setValue(2, 2, 1);

    QCOMPARE(board.getValue(0, 0), -1); // Expected value: -1 after modification
    QCOMPARE(board.getValue(1, 1), 1); // Expected value: 1 after modification
    QCOMPARE(board.getValue(2, 2), 1); // Expected value: 1 after modification
}

// Additional test case to verify out-of-bound access handling
void Tests::testOutOfBoundsAccess() {
    GameBoard board;

    // Set values within bounds
    board.setValue(0, 0, 1);
    board.setValue(1, 1, -1);
    board.setValue(2, 2, 0);

    // Verify in-bound accesses
    QCOMPARE(board.getValue(0, 0), 1); // Expected value: 1
    QCOMPARE(board.getValue(1, 1), -1); // Expected value: -1
    QCOMPARE(board.getValue(2, 2), 0); // Expected value: 0

    // Attempt to access out-of-bound indices
    QCOMPARE(board.getValue(3, 0), 0); // Expected value: 0 (default for out-of-bounds)
    QCOMPARE(board.getValue(0, 3), 0); // Expected value: 0 (default for out-of-bounds)
}



// Include the QTEST_MAIN macro to compile the unit tests
QTEST_MAIN(Tests)

#include "tst_unittests.moc" // Include the moc file for Qt Test
