// TicTacToeWidget.h
#ifndef TICTACTOEWIDGET_H
#define TICTACTOEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include "aiplayer.h" // Include your AIPlayer class header

class TicTacToeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TicTacToeWidget(QWidget *parent = nullptr);

private slots:
    void onButtonClicked();

private:
    QVector<QPushButton*> buttons; // Store pointers to the buttons
    AIPlayer aiPlayer; // AI player instance

    // Declare private helper methods if needed
};

#endif // TICTACTOEWIDGET_H
