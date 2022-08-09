#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <time.h>
using namespace std;

const int PIECE_VAL[] = { 0, 1, 3, 3, 5, 9, 99, -1, -3, -3, -5, -9, -99 };
const int PIECE_SIDE[] = { 0, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1 };
const double CENTER_MASK[8][8]= {{-0.2, -0.1, 0.0, 0.0, 0.0, 0.0, -0.1, -0.2},
                                {-0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.1},
                                {-0.1, 0.0, 0.4, 0.8, 0.8, 0.4, 0.0, -0.1},
                                {-0.1, 0.0, 0.8, 1.0, 1.0, 0.8, 0.0, -0.1},
                                {-0.1, 0.0, 0.8, 1.0, 1.0, 0.8, 0.0, -0.1},
                                {-0.1, 0.0, 0.4, 0.8, 0.8, 0.4, 0.0, -0.1},
                                {-0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.1},
                                {-0.2, -0.1, 0.0, 0.0, 0.0, 0.0, -0.1, -0.2}};
const double CENTER_WEIGHT[] = {0, 0.4, 0.5, 0.25, 0.1, 0.2, -0.1, -0.4, -0.5, -0.25, -0.1, -0.2, 0.1};

typedef int square[2];

struct GameState {
    int board[8][8];
    int moved[8][8];
    square lastMoved;
};

bool sameSquare(square s1, square s2) {
    if (s1[0] == s2[0] && s1[1] == s2[1])
        return true;
    return false;
}

vector<vector<int> > addDirection(vector<vector<int> > directions, vector<vector<int> > newDirs) {
    for (int i = 0; i < newDirs.size(); i++) {
        vector<int> d;
        d.push_back(newDirs.at(i).at(0));
        d.push_back(newDirs.at(i).at(1));
        directions.push_back(d);
    }
    return directions;
}

bool promotionState(GameState state) {
    for (int i = 0; i < 8; i++) {
        if (state.board[0][i] == 1 || state.board[7][i] == 7)
            return true;
    }
    return false;
}

extern "C" {


GameState afterMove(GameState state, square start, square end, int promotion=5) {
    GameState newState;
    square captured = {end[0], end[1]};
    copy(&state.board[0][0], &state.board[0][0]+64, &newState.board[0][0]);
    copy(&state.moved[0][0], &state.moved[0][0]+64, &newState.moved[0][0]);
    int piece = newState.board[start[0]][start[1]];
    if (piece == 1 && end[1]-start[1] != 0 && newState.board[end[0]][end[1]] == 0) // white en passant
        captured[0] = end[0]+1;
    if (piece == 7 && end[1]-start[1] != 0 && newState.board[end[0]][end[1]] == 0) // black en passant
        captured[0] = end[0]-1;
    if (piece == 6 || piece == 12) {
        if (end[1]-start[1] == 2) { // kingside castle
            newState.board[end[0]][end[1]-1] = newState.board[end[0]][end[1]+1];
            newState.board[end[0]][end[1]+1] = 0;
            newState.moved[end[0]][end[1]-1] = 1;
        } else if (end[1]-start[1] == -2) { // queenside castle
            newState.board[end[0]][end[1]+1] = newState.board[end[0]][end[1]-2];
            newState.board[end[0]][end[1]-2] = 0;
            newState.moved[end[0]][end[1]+1] = 1;
        }
    }
    newState.board[start[0]][start[1]] = 0;
    newState.moved[start[0]][start[1]] = 0;
    newState.board[captured[0]][captured[1]] = 0;
    newState.moved[captured[0]][captured[1]] = 0;
    if (promotionState(newState)) {
        if (PIECE_SIDE[piece] == -1)
            promotion += 6;
        piece = promotion;
    }
    newState.board[end[0]][end[1]] = piece;
    newState.moved[end[0]][end[1]] = 1;
    newState.lastMoved[0] = end[0];
    newState.lastMoved[1] = end[1];
    return newState;
}

bool validMove(GameState state, square start, square end, int turn, bool useCheck);

bool inCheck(GameState state, int turn) {
    int king = 6;
    if (turn == -1) king = 12;
    vector< vector<int> > moves;
    int wpm[][2] = {{-1,-1},{-1,1}};
    int bpm[][2] = {{1,-1},{1,1}};
    int km[][2] = {{2,1},{2,-1},{1,2},{1,-2},{-1,2},{-1,-2},{-2,1},{-2,-1}};
    int bm[][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    int rm[][2] = {{1,0},{0,1},{-1,0},{0,-1}};
    vector<vector<int> > whitePawnMoves;
    vector<vector<int> > blackPawnMoves;
    vector<vector<int> > knightMoves;
    vector<vector<int> > bishopMoves;
    vector<vector<int> > rookMoves;
    vector<vector<int> > queenMoves;
    for (int i = 0; i < 2; i++) {
        vector<int> d1;
        vector<int> d2;
        vector<int> d3;
        vector<int> d4;
        vector<int> d5;
        for (int j = 0; j < 2; j++) {
            d1.push_back(wpm[i][j]);
            d2.push_back(bpm[i][j]);
            d3.push_back(km[i][j]);
            d4.push_back(bm[i][j]);
            d5.push_back(rm[i][j]);
        }
        whitePawnMoves.push_back(d1);
        blackPawnMoves.push_back(d2);
        knightMoves.push_back(d3);
        bishopMoves.push_back(d4);
        rookMoves.push_back(d5);
        queenMoves.push_back(d4);
        queenMoves.push_back(d5);
    }
    for (int i = 2; i < 4; i++) {
        vector<int> d3;
        vector<int> d4;
        vector<int> d5;
        for (int j = 0; j < 2; j++) {
            d3.push_back(km[i][j]);
            d4.push_back(bm[i][j]);
            d5.push_back(rm[i][j]);
        }
        knightMoves.push_back(d3);
        bishopMoves.push_back(d4);
        rookMoves.push_back(d5);
        queenMoves.push_back(d4);
        queenMoves.push_back(d5);
    }
    for (int i = 4; i < 8; i++) {
        vector<int> d3;
        for (int j = 0; j < 2; j++) {
            d3.push_back(km[i][j]);
        }
        knightMoves.push_back(d3);
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (PIECE_SIDE[state.board[i][j]] == turn*-1) {
                int piece = state.board[i][j];
                vector< vector<int> > directions;
                int steps = 1;
                square startPos = {i,j};
                switch(piece) {
                    case 1:
                        directions = addDirection(directions, whitePawnMoves);
                        break;
                    case 7:
                        directions = addDirection(directions, blackPawnMoves);
                        break;
                    case 2:
                    case 8:
                        directions = addDirection(directions, knightMoves);
                        break;
                    case 3:
                    case 9:
                        directions = addDirection(directions, bishopMoves);
                        steps = 7;
                        break;
                    case 4:
                    case 10:
                        directions = addDirection(directions, rookMoves);
                        steps = 7;
                        break;
                    case 6:
                    case 12:
                        directions = addDirection(directions, bishopMoves);
                        directions = addDirection(directions, rookMoves);
                        break;
                    case 5:
                    case 11:
                        directions = addDirection(directions, bishopMoves);
                        directions = addDirection(directions, rookMoves);
                        steps = 7;
                        break;
                }
                for (int d = 0; d < directions.size(); d++) {
                    square pos = {i,j};
                    for (int k = 0; k < steps; k++) {
                        pos[0] += directions.at(d).at(0);
                        pos[1] += directions.at(d).at(1);
                        if (validMove(state, startPos, pos, turn*-1, false) && state.board[pos[0]][pos[1]] == king) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool unbrokenLine(GameState state, square start, square end, int dx, int dy) {
    square pos = {start[0]+dy,start[1]+dx};
    while (!sameSquare(pos, end)) {
        if (pos[0] < 0 || pos[0] >= 8 || pos[1] < 0 || pos[1] >= 8 || state.board[pos[0]][pos[1]] != 0)
            return false;
        pos[0] += dy;
        pos[1] += dx;
    }
    return true;
}

bool validMove(GameState state, square start, square end, int turn, bool useCheck=true) {
    if (end[0] < 0 || end[0] >= 8 || end[1] < 0 || end[1] >= 8) // trying to move off the board
        return false;
    int piece = state.board[start[0]][start[1]];
    square moveTo = {end[0], end[1]};
    if (PIECE_SIDE[piece] != turn) // trying to move opponent piece
        return false;
    if (PIECE_SIDE[piece] == PIECE_SIDE[state.board[end[0]][end[1]]]) // trying to take piece of same color
        return false;
    if (useCheck && inCheck(afterMove(state, start, end), turn))
        return false;
    if (piece == 1) { // white pawn
        if (end[0] == start[0]-1 && end[1] == start[1]) { // moving forward one
            if (state.board[end[0]][end[1]] != 0)
                return false;
        } else if (end[0] == start[0]-1 && (end[1] == start[1]+1 || end[1] == start[1]-1)) { // capturing diagonally
            square ep = {end[0]+1, end[1]};
            if (state.board[end[0]][end[1]] == 0 && !(state.board[end[0]+1][end[1]] == 7 && sameSquare(state.lastMoved, ep)))
                return false;
        } else if (end[0] == start[0]-2 && end[1] == start[1]) { // # moving forward two
            if (state.moved[start[0]][start[1]] == 1 || state.board[start[0]-1][start[1]] != 0)
                return false;
        } else {
            return false;
        }
    } else if (piece == 7) { // black pawn
        if (end[0] == start[0]+1 && end[1] == start[1]) { // moving forward one
            if (state.board[end[0]][end[1]] != 0)
                return false;
        } else if (end[0] == start[0]+1 && (end[1] == start[1]+1 || end[1] == start[1]-1)) { // capturing diagonally
            square ep = {end[0]-1, end[1]};
            if (state.board[end[0]][end[1]] == 0 && !(state.board[end[0]+1][end[1]] == 1 && sameSquare(state.lastMoved, ep)))
                return false;
        } else if (end[0] == start[0]+2 && end[1] == start[1]) { // # moving forward two
            if (state.moved[start[0]][start[1]] == 1 || state.board[start[0]+1][start[1]] != 0)
                return false;
        } else {
            return false;
        }
    } else if (piece == 2 || piece == 8) { // knight
        square choices[8] = {{start[0]+1,start[1]+2},{start[0]+1,start[1]-2},
            {start[0]+2,start[1]+1},{start[0]+2,start[1]-1},{start[0]-1,start[1]+2},{start[0]-1,start[1]-2},
            {start[0]-2,start[1]+1},{start[0]-2,start[1]-1}};
        for (int i = 0; i < 8; i++) {
            if (sameSquare(moveTo, choices[i]))
                return true;
        }
        return false;
    } else if (piece == 3 || piece == 9) { // bishop
        if (!unbrokenLine(state, start, end, 1, 1) && !unbrokenLine(state, start, end, -1, -1)
        && !unbrokenLine(state, start, end, -1, 1) && !unbrokenLine(state, start, end, 1, -1))
            return false;
    } else if (piece == 4 || piece == 10) { // rook
        if (!unbrokenLine(state, start, end, 1, 0) && !unbrokenLine(state, start, end, -1, 0)
        && !unbrokenLine(state, start, end, 0, 1) && !unbrokenLine(state, start, end, 0, -1))
            return false;
    } else if (piece == 5 || piece == 11) { // queen
        if (!unbrokenLine(state, start, end, 1, 1) && !unbrokenLine(state, start, end, -1, -1)
        && !unbrokenLine(state, start, end, -1, 1) && !unbrokenLine(state, start, end, 1, -1)
        && !unbrokenLine(state, start, end, 1, 0) && !unbrokenLine(state, start, end, -1, 0)
        && !unbrokenLine(state, start, end, 0, 1) && !unbrokenLine(state, start, end, 0, -1))
            return false;
    } else if (piece == 6 || piece == 12) { // king
        square choices[8] = {{start[0]+1,start[1]},{start[0]-1,start[1]},{start[0],start[1]+1},{start[0],start[1]-1},
            {start[0]+1,start[1]+1},{start[0]+1,start[1]-1},{start[0]-1,start[1]+1},{start[0]-1,end[0]-1}};
        for (int i = 0; i < 8; i++) {
            if (sameSquare(moveTo, choices[i]))
                return true;
        }
        int castle = 4;
        if (piece == 12) castle = 10;
        if (end[0] == start[0] && end[1] == start[1]+2 && end[1] == 6) { // kingside castle
            square between_square = {start[0], start[1]+1};
            GameState between_board = afterMove(state, start, between_square);
            if (state.board[start[0]][5] != 0 || state.moved[start[0]][4] == 1 || state.moved[start[0]][7] == 1
            || state.board[start[0]][7] != castle || inCheck(state, turn) || inCheck(between_board, turn))
                return false;
        } else if (end[0] == start[0] && end[1] == start[1]-2 && end[1] == 2) { // queenside castle
            square between_square = {start[0], start[1]-1};
            GameState between_board = afterMove(state, start, between_square);
            if (state.board[start[0]][5] != 0 || state.board[start[0]][1] != 0 || state.moved[start[0]][4] == 1 || state.moved[start[0]][7] == 1
            || state.board[start[0]][7] != castle || inCheck(state, turn) || inCheck(between_board, turn))
                return false;
        } else {
            return false;
        }
    }
    return true;
}}

vector<vector<int> > validMoves(GameState state, int turn) {
    vector< vector<int> > moves;
    int wpm[][2] = {{-1,0},{-1,-1},{-1,1},{-2,0}};
    int bpm[][2] = {{1,0},{1,-1},{1,1},{2,0}};
    int km[][2] = {{2,1},{2,-1},{1,2},{1,-2},{-1,2},{-1,-2},{-2,1},{-2,-1}};
    int bm[][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    int rm[][2] = {{1,0},{0,1},{-1,0},{0,-1}};
    int cm[][2] = {{0,2},{0,-2}};
    vector<vector<int> > whitePawnMoves;
    vector<vector<int> > blackPawnMoves;
    vector<vector<int> > knightMoves;
    vector<vector<int> > bishopMoves;
    vector<vector<int> > rookMoves;
    vector<vector<int> > queenMoves;
    vector<vector<int> > castleMoves;
    for (int i = 0; i < 2; i++) {
        vector<int> d1;
        vector<int> d2;
        vector<int> d3;
        vector<int> d4;
        vector<int> d5;
        vector<int> d6;
        for (int j = 0; j < 2; j++) {
            d1.push_back(wpm[i][j]);
            d2.push_back(bpm[i][j]);
            d3.push_back(km[i][j]);
            d4.push_back(bm[i][j]);
            d5.push_back(rm[i][j]);
            d6.push_back(cm[i][j]);
        }
        whitePawnMoves.push_back(d1);
        blackPawnMoves.push_back(d2);
        knightMoves.push_back(d3);
        bishopMoves.push_back(d4);
        rookMoves.push_back(d5);
        queenMoves.push_back(d4);
        queenMoves.push_back(d5);
        castleMoves.push_back(d6);
    }
    for (int i = 2; i < 4; i++) {
        vector<int> d1;
        vector<int> d2;
        vector<int> d3;
        vector<int> d4;
        vector<int> d5;
        for (int j = 0; j < 2; j++) {
            d1.push_back(wpm[i][j]);
            d2.push_back(bpm[i][j]);
            d3.push_back(km[i][j]);
            d4.push_back(bm[i][j]);
            d5.push_back(rm[i][j]);
        }
        whitePawnMoves.push_back(d1);
        blackPawnMoves.push_back(d2);
        knightMoves.push_back(d3);
        bishopMoves.push_back(d4);
        rookMoves.push_back(d5);
        queenMoves.push_back(d4);
        queenMoves.push_back(d5);
    }
    for (int i = 4; i < 8; i++) {
        vector<int> d3;
        for (int j = 0; j < 2; j++) {
            d3.push_back(km[i][j]);
        }
        knightMoves.push_back(d3);
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (PIECE_SIDE[state.board[i][j]] == turn) {
                int piece = state.board[i][j];
                vector< vector<int> > directions;
                int steps = 1;
                square startPos = {i,j};
                switch(piece) {
                    case 1:
                        directions = addDirection(directions, whitePawnMoves);
                        break;
                    case 7:
                        directions = addDirection(directions, blackPawnMoves);
                        break;
                    case 2:
                    case 8:
                        directions = addDirection(directions, knightMoves);
                        break;
                    case 3:
                    case 9:
                        directions = addDirection(directions, bishopMoves);
                        steps = 7;
                        break;
                    case 4:
                    case 10:
                        directions = addDirection(directions, rookMoves);
                        steps = 7;
                        break;
                    case 6:
                    case 12:
                        directions = addDirection(directions, castleMoves);
                        directions = addDirection(directions, bishopMoves);
                        directions = addDirection(directions, rookMoves);
                        break;
                    case 5:
                    case 11:
                        directions = addDirection(directions, bishopMoves);
                        directions = addDirection(directions, rookMoves);
                        steps = 7;
                        break;
                }
                for (int d = 0; d < directions.size(); d++) {
                    square pos = {i,j};
                    for (int k = 0; k < steps; k++) {
                        pos[0] += directions.at(d).at(0);
                        pos[1] += directions.at(d).at(1);
                        if (validMove(state, startPos, pos, turn)) {
                            vector<int> m;
                            m.clear();
                            m.push_back(startPos[0]);
                            m.push_back(startPos[1]);
                            m.push_back(pos[0]);
                            m.push_back(pos[1]);
                            moves.push_back(m);
                        }
                    }
                }
            }
        }
    }
    return moves;
}

double evaluateState(GameState state) {
    double val = 0.0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int piece = state.board[i][j];
            val += PIECE_VAL[piece];
            if (piece == 2 || piece == 3 || piece == 4 || piece == 8 || piece == 9 || piece == 10)
                val += 0.15*PIECE_SIDE[piece]*state.moved[i][j];
            if (piece == 1)
                val += (6-i)*0.15;
            else if (piece == 7)
                val -= (i-1)*0.15;
            val += CENTER_WEIGHT[piece]*CENTER_MASK[i][j];
        }
    }
    val += 0.05*(double)rand() / RAND_MAX;
    return val;
}

GameState afterVecMove(GameState state, vector<int> move, int promotion=5) {
    square start = {move.at(0), move.at(1)};
    square end = {move.at(2), move.at(3)};
    return afterMove(state, start, end, promotion);
}

void debugBoard(GameState state) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cout << state.board[i][j] << " ";
        }
        cout << endl;
    }
}

extern "C" int gameRes(GameState state, int turn) {
    vector<vector<int> > moves = validMoves(state, turn);
    if (moves.size() > 0)
        return 0;
    if (inCheck(state, turn))
        return 1;
    return 2;
}

class Tree {
    public:
        GameState state;
        double val;
        int turn;
        square start;
        vector<Tree*> children;
        Tree(GameState s, int t, square st) {
            state = s;
            val = evaluateState(state);
            turn = t;
            start[0] = st[0];
            start[1] = st[1];
        }
        ~Tree() {
            children.clear();
            delete &children;
            delete &state;
        }
};

void makeChildren(Tree* tree, int depth) {
    if (depth == 0)
        return;
    tree->children.clear();
    vector<vector<int> > moves = validMoves(tree->state, tree->turn);
    GameState newState;
    square st;
    for (int i = 0; i < moves.size(); i++)  {
        newState = afterVecMove(tree->state, moves.at(i));
        st[0] = moves.at(i).at(0);
        st[1] = moves.at(i).at(1);
        tree->children.push_back(new Tree(newState, tree->turn*-1, st));
        if (promotionState(newState)) {
            int p0 = 2;
            for (int p = p0; p <= p0+2; p++) {
                newState = afterVecMove(tree->state, moves.at(i), p);
                tree->children.push_back(new Tree(newState, tree->turn*-1, st));
            }
        }
    }
    if (moves.size() == 0 && gameRes(tree->state, tree->turn) == 2)
        tree->val = tree->turn*-500;
    for (int i = 0; i < tree->children.size(); i++) {
        makeChildren(tree->children.at(i), depth-1);
    }
}
Tree* evalChildren(Tree* tree) {
    if (tree->children.size() == 0)
        return NULL;
    for (int i = 0; i < tree->children.size(); i++) {
        evalChildren(tree->children.at(i));
    }
    Tree* maxChild = tree->children.at(0);
    double maxVal = maxChild->val;
    for (int i = 0; i < tree->children.size(); i++) {
        if ((tree->turn == 1 && tree->children.at(i)->val > maxVal) || (tree->turn == -1 && tree->children.at(i)->val < maxVal)) {
            maxChild = tree->children.at(i);
            maxVal = maxChild->val;
        }
    }
    tree->children.clear();
    tree->children.push_back(maxChild);
    tree->val = maxVal;
    return maxChild;
}

extern "C" int minimax(GameState s, int turn, int depth) {
    srand(time(0));
    square noSquare = {-1,-1};
    Tree* root = new Tree(s, turn, noSquare);
    makeChildren(root, depth);
    Tree* nextState = evalChildren(root);
    int res = nextState->start[0]*1000+nextState->start[1]*100+nextState->state.lastMoved[0]*10+nextState->state.lastMoved[1];
    return res;
}

extern "C" int showMoves(GameState state, int turn) {
    vector<vector<int> > moves = validMoves(state, turn);
    for (int i = 0; i < moves.size(); i++) {
        cout << "(" << moves.at(i).at(0) << moves.at(i).at(1) << ") -> (" << moves.at(i).at(2) << moves.at(i).at(3) << ")" << endl;
    }
    return 0;
}