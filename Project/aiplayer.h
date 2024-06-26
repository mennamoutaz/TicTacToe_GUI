#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <vector>
#include <QPushButton>

struct TreeNode {
    int moveIndex;
    std::vector<TreeNode*> children;
    int score;

    TreeNode() : moveIndex(-1), score(0) {}
};

class AIPlayer {
public:
    AIPlayer(char symbol); // Constructor that accepts a symbol
    void makeMove(std::vector<QPushButton*>& buttons);

private:
    char playerSymbol; // Symbol assigned to AIPlayer ('X' or 'O')
void buildTree(TreeNode* node, std::vector<QPushButton*>& buttons, int player); // Declaration of buildTree function
    int minimax(TreeNode* node, std::vector<QPushButton*>& buttons, int alpha, int beta, bool maximizingPlayer, int depth);

    int evaluate(const std::vector<QPushButton*>& buttons);
    friend class tests;
    // Add any additional private methods or member variables as needed
};

#endif // AIPLAYER_H
