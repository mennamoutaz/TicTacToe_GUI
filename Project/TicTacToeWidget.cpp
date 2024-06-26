// TicTacToeWidget.cpp
#include "tictactoewidget.h"
#include <QGridLayout>

TicTacToeWidget::TicTacToeWidget(QWidget* parent)
    : QWidget(parent), aiPlayer('O') // Initialize AIPlayer with symbol 'O'
{
    QGridLayout* gridLayout = new QGridLayout(this);
    // Create 3x3 grid of buttons
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            QPushButton* button = new QPushButton(this);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            connect(button, &QPushButton::clicked, this, &TicTacToeWidget::onButtonClicked);
            gridLayout->addWidget(button, row, col);
            buttons.append(button);
        }
    }
    setLayout(gridLayout);
}

void TicTacToeWidget::onButtonClicked() {
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;

    int index = buttons.indexOf(clickedButton);
    if (index == -1 || !clickedButton->text().isEmpty()) return; // Invalid move

    // User move (Player X)
    clickedButton->setText("X");
    clickedButton->setEnabled(false); // Disable button after move

    // Check game state (win, tie, etc.) after user move

    // Convert QVector<QPushButton*> to std::vector<QPushButton*>
    std::vector<QPushButton*> stdButtons(buttons.begin(), buttons.end());

    // Trigger AI move (Player O)
    aiPlayer.makeMove(stdButtons); // This should trigger AI to make its move automatically
}
