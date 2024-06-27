#ifndef GAMEBOARD_H
#define GAMEBOARD_H

class GameBoard {
public:
    GameBoard();

    void display() const;
    int checkWin() const;
    int getValue(int row, int col) const;
    void setValue(int row, int col, int value);

private:
    friend class tests;
    int board[3][3];
};

#endif // GAMEBOARD_H
