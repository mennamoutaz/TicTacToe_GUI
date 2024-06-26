#include "aiplayer.h"
#include <limits>
#include <iostream>
#include <QPushButton>

AIPlayer::AIPlayer(char symbol) : playerSymbol(symbol) {
    // Constructor initialization
}

void AIPlayer::makeMove(std::vector<QPushButton*>& buttons) {
    std::cout << "AI Move:" << std::endl;

    // Create a TreeNode to build the game tree
    TreeNode* root = new TreeNode;

    // Build the game tree starting from the current board state
    buildTree(root, buttons, -1); // Assuming AI is player -1

    int bestScore = std::numeric_limits<int>::min();
    TreeNode* bestMove = nullptr;

    // Evaluate each possible move and choose the best one
    for (TreeNode* child : root->children) {
        int score = minimax(child, buttons, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false, 9); // Adjust depth of search
        if (score > bestScore) {
            bestScore = score;
            bestMove = child;
        }
    }

    // Apply AI move to the button
    if (bestMove) {
        int index = bestMove->moveIndex;
        if (index >= 0 && index < buttons.size()) {
            buttons[index]->setText(QString(playerSymbol)); // Set AI move to playerSymbol (either 'X' or 'O')
            buttons[index]->setEnabled(false); // Disable button after AI move
        }
    }

    // Free memory
    for (TreeNode* child : root->children) {
        delete child;
    }
    delete root;
}

void AIPlayer::buildTree(TreeNode* node, std::vector<QPushButton*>& buttons, int player) {
    // Iterate through each button to explore possible moves
    for (int i = 0; i < buttons.size(); ++i) {
        if (buttons[i]->text().isEmpty()) { // Check if button is empty
            TreeNode* child = new TreeNode;
            child->moveIndex = i;
            node->children.push_back(child);

            // Simulate move on the button
            buttons[i]->setText(QString(playerSymbol)); // Set AI move to playerSymbol (either 'X' or 'O')
            buttons[i]->setEnabled(false); // Disable button after AI move

            // Recursively build tree for further moves
            buildTree(child, buttons, -player); // Toggle player for opponent's turn

            // Undo move to explore other possibilities
            buttons[i]->setText(""); // Clear button text
            buttons[i]->setEnabled(true); // Enable button again
        }
    }
}

int AIPlayer::minimax(TreeNode* node, std::vector<QPushButton*>& buttons, int alpha, int beta, bool isMax, int depth) {
    if (node->children.empty() || depth == 0) {
        return evaluate(buttons); // Evaluate the board state based on buttons
    }

    if (isMax) {
        int maxScore = std::numeric_limits<int>::min();
        for (TreeNode* child : node->children) {
            int score = minimax(child, buttons, alpha, beta, false, depth - 1);
            maxScore = std::max(maxScore, score);
            alpha = std::max(alpha, score);
            if (alpha >= beta) {
                break;
            }
        }
        return maxScore;
    } else {
        int minScore = std::numeric_limits<int>::max();
        for (TreeNode* child : node->children) {
            int score = minimax(child, buttons, alpha, beta, true, depth - 1);
            minScore = std::min(minScore, score);
            beta = std::min(beta, score);
            if (alpha >= beta) {
                break;
            }
        }
        return minScore;
    }
}

int AIPlayer::evaluate(const std::vector<QPushButton*>& buttons) {
    // Evaluation criteria for Tic-Tac-Toe:
    // - Check for potential wins, blocks, and prioritize based on game state

    // Define winning patterns (indices for buttons)
    const std::vector<std::vector<int>> winPatterns = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // columns
        {0, 4, 8}, {2, 4, 6} // diagonals
    };

    int aiScore = 0;
    int playerScore = 0;

    // Evaluate each winning pattern
    for (const auto& pattern : winPatterns) {
        bool aiFound = true;
        bool playerFound = true;

        for (int index : pattern) {
            if (buttons[index]->text() != QString(playerSymbol)) { // Check AI's symbol
                aiFound = false;
            }
            if (buttons[index]->text() != (playerSymbol == 'X' ? "O" : "X")) { // Check player's symbol
                playerFound = false;
            }
        }

        if (aiFound) {
            aiScore += 10; // Favorable score for AI winning pattern
        }
        if (playerFound) {
            playerScore += 10; // Favorable score for player winning pattern
        }
    }

    // Return the difference in scores (favoring AI's perspective)
    return aiScore - playerScore;
}
