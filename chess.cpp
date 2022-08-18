#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <time.h>
#include <sys/time.h>
#include <ctime>
#include <cmath>
using namespace std;

const int PIECE_VAL[] = { 0, 100, 320, 330, 500, 900, 20000, -100, -320, -330, -500, -900, -20000 };
const int PIECE_SIDE[] = { 0, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1 };
const int FILE_SEARCH[] = {3, 4, 2, 5, 1, 6, 0, 7};
const int RANK_SEARCH[] = {3, 4, 1, 6, 0, 7, 2, 5};

const int PAWN_MAP[8][8] = {{90, 90, 90, 90, 90, 90, 90, 90},
                            {60, 50, 50, 50, 50, 50, 50, 60},
                            {10, 10, 20, 30, 30, 20, 10, 10},
                            {5,  5, 10, 25, 25, 10,  5,  5},
                            {0,  0,  0, 20, 20,  0,  0,  0},
                            {5, -5,-10,  0,  0,-10, -5,  5},
                            {5, 10, 10,-35,-35, 10, 10,  5},
                            {0,  0,  0,  0,  0,  0,  0,  0}};
const int KNIGHT_MAP[8][8]={{-50,-40,-30,-30,-30,-30,-40,-50},
                            {-40,-20,  0,  0,  0,  0,-20,-40},
                            {-30,  0, 10, 15, 15, 10,  0,-30},
                            {-30,  5, 15, 20, 20, 15,  5,-30},
                            {-30,  0, 15, 20, 20, 15,  0,-30},
                            {-30,  5, 10, 15, 15, 10,  5,-30},
                            {-40,-20,  5,  5,  5,  5,-20,-40},
                            {-50,-40,-30,-30,-30,-30,-40,-50}};
const int BISHOP_MAP[8][8]={{-20,-10,-10,-10,-10,-10,-10,-20},
                            {-10,  0,  0,  0,  0,  0,  0,-10},
                            {-10,  0,  5, 10, 10,  5,  0,-10},
                            {-10,  5,  5, 10, 10,  5,  5,-10},
                            {-10,  0, 10, 10, 10, 10,  0,-10},
                            {-10, 10, 10, 10, 10, 10, 10,-10},
                            {-10,  5,  0,  0,  0,  0,  5,-10},
                            {-20,-10,-10,-10,-10,-10,-10,-20}};
const int ROOK_MAP[8][8] = {{ 0,  0,  0,  0,  0,  0,  0,  0},
                            { 5, 10, 10, 10, 10, 10, 10,  5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            { 0,  0,  0,  5,  5,  0,  0,  0}};
const int QUEEN_MAP[8][8] ={{-20,-10,-10, -5, -5,-10,-10,-20},
                            {-10,  0,  0,  0,  0,  0,  0,-10},
                            {-10,  0,  5,  5,  5,  5,  0,-10},
                            { -5,  0,  5,  5,  5,  5,  0, -5},
                            {  0,  0,  5,  5,  5,  5,  0, -5},
                            {-10,  5,  5,  5,  5,  5,  0,-10},
                            {-10,  0,  5,  0,  0,  0,  0,-10},
                            {-20,-10,-10, -5, -5,-10,-10,-20}};
const int KING_MAP[8][8] = {{-30,-40,-40,-50,-50,-40,-40,-30},
                            {-30,-40,-40,-50,-50,-40,-40,-30},
                            {-30,-40,-40,-50,-50,-40,-40,-30},
                            {-30,-40,-40,-50,-50,-40,-40,-30},
                            {-20,-30,-30,-40,-40,-30,-30,-20},
                            {-10,-20,-20,-20,-20,-20,-20,-10},
                            { 20, 20,-10,-10,-10,-10, 20, 20},
                            { 20, 30, 20,  0,  0, 10, 40, 20}};
const int KING_MAP_ENDGAME[8][8] = {{-50,-40,-30,-20,-20,-30,-40,-50},
                                    {-30,-20,-10,  0,  0,-10,-20,-30},
                                    {-30,-10, 20, 30, 30, 20,-10,-30},
                                    {-30,-10, 30, 40, 40, 30,-10,-30},
                                    {-30,-10, 30, 40, 40, 30,-10,-30},
                                    {-30,-10, 20, 30, 30, 20,-10,-30},
                                    {-30,-30,  0,  0,  0,  0,-30,-30},
                                    {-50,-30,-30,-30,-30,-30,-30,-50}};

typedef int square[2];

struct GameState {
    int board[8][8];
    bool moved[8][8];
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


int get_millis() {
    timeval time_now;
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
    return (int)(msecs_time);
}
int start_calc;
int calc_time = 7500;

class Directions {
    public:
        vector<vector<int> > whitePawnMoves;
        vector<vector<int> > blackPawnMoves;
        vector<vector<int> > knightMoves;
        vector<vector<int> > bishopMoves;
        vector<vector<int> > rookMoves;
        vector<vector<int> > queenMoves;
        vector<vector<int> > castleMoves;
        Directions(bool attacks) {
            int wpm[][2] = {{-1,-1},{-1,1},{-1,0},{-2,0}};
            int bpm[][2] = {{1,-1},{1,1},{1,0},{2,0}};
            int km[][2] = {{2,1},{2,-1},{1,2},{1,-2},{-1,2},{-1,-2},{-2,1},{-2,-1}};
            int bm[][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
            int rm[][2] = {{1,0},{0,1},{-1,0},{0,-1}};
            int cm[][2] = {{0,2},{0,-2}};
            for (int i = 0; i < 2; i++) {
                vector<int> d1;
                vector<int> d2;
                for (int j = 0; j < 2; j++) {
                    d1.push_back(wpm[i][j]);
                    d2.push_back(bpm[i][j]);
                }
                whitePawnMoves.push_back(d1);
                blackPawnMoves.push_back(d2);
            }
            if (!attacks) {
                for (int i = 0; i < 2; i++) {
                    vector<int> d6;
                    for (int j = 0; j < 2; j++) {
                        d6.push_back(cm[i][j]);
                    }
                    castleMoves.push_back(d6);
                }
                for (int i = 2; i < 4; i++) {
                    vector<int> d1;
                    vector<int> d2;
                    for (int j = 0; j < 2; j++) {
                        d1.push_back(wpm[i][j]);
                        d2.push_back(bpm[i][j]);
                    }
                    whitePawnMoves.push_back(d1);
                    blackPawnMoves.push_back(d2);
                }
            }
            for (int i = 0; i < 4; i++) {
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
        }
};
Directions dirHolder(false);
Directions atkHolder(true);

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
    newState.board[end[0]][end[1]] = piece;
    newState.moved[end[0]][end[1]] = 1;
    if ((piece == 1 && end[0] == 0) || (piece == 7 && end[0] == 7)) { // promotion
        newState.board[end[0]][end[1]] = promotion;
        if (PIECE_SIDE[piece] == -1)
            newState.board[end[0]][end[1]] += 6;
    }
    newState.lastMoved[0] = end[0];
    newState.lastMoved[1] = end[1];
    return newState;
}

bool validMove(GameState state, square start, square end, int turn, bool useCheck);

bool inCheck(GameState state, int turn) {
    int king = 6;
    if (turn == -1) king = 12;
    vector< vector<int> > moves;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (PIECE_SIDE[state.board[i][j]] == turn*-1) {
                int piece = state.board[i][j];
                vector< vector<int> > directions;
                int steps = 1;
                square startPos = {i,j};
                switch(piece) {
                    case 1:
                        directions = atkHolder.whitePawnMoves;
                        break;
                    case 7:
                        directions = atkHolder.blackPawnMoves;
                        break;
                    case 2:
                    case 8:
                        directions = atkHolder.knightMoves;
                        break;
                    case 3:
                    case 9:
                        directions = atkHolder.bishopMoves;
                        steps = 7;
                        break;
                    case 4:
                    case 10:
                        directions = atkHolder.rookMoves;
                        steps = 7;
                        break;
                    case 6:
                    case 12:
                        directions = atkHolder.queenMoves;
                        break;
                    case 5:
                    case 11:
                        directions = atkHolder.queenMoves;
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
            if (state.board[end[0]][end[1]] == 0) {
                if (state.board[end[0]+1][end[1]] == 7 && end[0] == 2 && state.board[1][end[1]] == 0 && sameSquare(state.lastMoved, ep))
                    return true;
                return false;
            }
        } else if (end[0] == start[0]-2 && end[1] == start[1]) { // # moving forward two
            if (state.moved[start[0]][start[1]] == 1 || state.board[start[0]-1][start[1]] != 0 || state.board[start[0]-2][start[1]] != 0)
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
            if (state.board[end[0]][end[1]] == 0) {
                if (state.board[end[0]-1][end[1]] == 1 && end[0] == 5 && state.board[6][end[1]] == 0 && sameSquare(state.lastMoved, ep))
                    return true;
                return false;
            }
        } else if (end[0] == start[0]+2 && end[1] == start[1]) { // # moving forward two
            if (state.moved[start[0]][start[1]] == 1 || state.board[start[0]+1][start[1]] != 0 || state.board[start[0]+2][start[1]] != 0)
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
            {start[0]+1,start[1]+1},{start[0]+1,start[1]-1},{start[0]-1,start[1]+1},{start[0]-1,start[1]-1}};
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
    vector<int> m;
    for (int i0 = 0; i0 < 8; i0++) {
        for (int j0 = 0; j0 < 8; j0++) {
            int i = RANK_SEARCH[i0];
            int j = FILE_SEARCH[j0];
            if (PIECE_SIDE[state.board[i][j]] == turn) {
                int piece = state.board[i][j];
                vector< vector<int> > directions;
                int steps = 1;
                square startPos = {i,j};
                switch(piece) {
                    case 1:
                        directions = dirHolder.whitePawnMoves;
                        break;
                    case 7:
                        directions = dirHolder.blackPawnMoves;
                        break;
                    case 2:
                    case 8:
                        directions = dirHolder.knightMoves;
                        break;
                    case 3:
                    case 9:
                        directions = dirHolder.bishopMoves;
                        steps = 7;
                        break;
                    case 4:
                    case 10:
                        directions = dirHolder.rookMoves;
                        steps = 7;
                        break;
                    case 6:
                    case 12:
                        directions = addDirection(directions, dirHolder.castleMoves);
                        directions = addDirection(directions, dirHolder.queenMoves);
                        break;
                    case 5:
                    case 11:
                        directions = dirHolder.queenMoves;
                        steps = 7;
                        break;
                }
                for (int d = 0; d < directions.size(); d++) {
                    square pos = {i,j};
                    for (int k = 0; k < steps; k++) {
                        pos[0] += directions.at(d).at(0);
                        pos[1] += directions.at(d).at(1);
                        if (validMove(state, startPos, pos, turn)) {
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

bool hasValidMove(GameState state, int turn) {
    for (int i0 = 0; i0 < 8; i0++) {
        for (int j0 = 0; j0 < 8; j0++) {
            int i = RANK_SEARCH[i0];
            int j = FILE_SEARCH[j0];
            if (PIECE_SIDE[state.board[i][j]] == turn) {
                int piece = state.board[i][j];
                vector< vector<int> > directions;
                int steps = 1;
                square startPos = {i,j};
                switch(piece) {
                    case 1:
                        directions = dirHolder.whitePawnMoves;
                        break;
                    case 7:
                        directions = dirHolder.blackPawnMoves;
                        break;
                    case 2:
                    case 8:
                        directions = addDirection(directions, dirHolder.knightMoves);
                        break;
                    case 3:
                    case 9:
                        directions = addDirection(directions, dirHolder.bishopMoves);
                        steps = 7;
                        break;
                    case 4:
                    case 10:
                        directions = addDirection(directions, dirHolder.rookMoves);
                        steps = 7;
                        break;
                    case 6:
                    case 12:
                        directions = addDirection(directions, dirHolder.castleMoves);
                        directions = addDirection(directions, dirHolder.queenMoves);
                        break;
                    case 5:
                    case 11:
                        directions = addDirection(directions, dirHolder.queenMoves);
                        steps = 7;
                        break;
                }
                for (int d = 0; d < directions.size(); d++) {
                    square pos = {i,j};
                    for (int k = 0; k < steps; k++) {
                        pos[0] += directions.at(d).at(0);
                        pos[1] += directions.at(d).at(1);
                        if (validMove(state, startPos, pos, turn)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;

}

bool endgameState(GameState state) {
    int whiteVal = 0;
    int blackVal = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (state.board[i][j] == 5 || state.board[i][j] == 11)
                return false;
            if (PIECE_SIDE[state.board[i][j]] == 1)
                whiteVal += PIECE_VAL[state.board[i][j]];
            if (PIECE_SIDE[state.board[i][j]] == -1)
                blackVal -= PIECE_VAL[state.board[i][j]];
        }
    }
    whiteVal -= PIECE_VAL[6];
    blackVal += PIECE_VAL[12];
    return (whiteVal <= 1300 && blackVal <= 1300);
}

int myAbs(int n) {
    return n*(n < 0);
}

extern "C" int evaluateState(GameState state) {
    int val = 0;
    square whiteKingSquare;
    square blackKingSquare;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int piece = state.board[i][j];
            val += PIECE_VAL[piece];
            switch(piece) {
                case 0: break;
                case 1: val += PAWN_MAP[i][j]; break;
                case 2: val += KNIGHT_MAP[i][j]; break;
                case 3: val += BISHOP_MAP[i][j]; break;
                case 4: val += ROOK_MAP[i][j]; break;
                case 5: val += QUEEN_MAP[i][j]; break;
                case 7: val -= PAWN_MAP[7-i][j]; break;
                case 8: val -= KNIGHT_MAP[7-i][j]; break;
                case 9: val -= BISHOP_MAP[7-i][j]; break;
                case 10: val -= ROOK_MAP[7-i][j]; break;
                case 11: val -= QUEEN_MAP[7-i][j]; break;
                case 6:
                    whiteKingSquare[0] = i;
                    whiteKingSquare[1] = j;
                    if (endgameState(state))
                        val += KING_MAP_ENDGAME[i][j];
                    else
                        val += KING_MAP[i][j];
                    break;
                case 12:
                    blackKingSquare[0] = i;
                    blackKingSquare[1] = j;
                    if (endgameState(state))
                        val -= KING_MAP_ENDGAME[7-i][j];
                    else
                        val -= KING_MAP[7-i][j];
                    break;
            }
        }
    }
    if (val > 350) {
        square choices[8] = {{blackKingSquare[0]+1,blackKingSquare[1]},{blackKingSquare[0]-1,blackKingSquare[1]},{blackKingSquare[0],blackKingSquare[1]+1},{blackKingSquare[0],blackKingSquare[1]-1},
            {blackKingSquare[0]+1,blackKingSquare[1]+1},{blackKingSquare[0]+1,blackKingSquare[1]-1},{blackKingSquare[0]-1,blackKingSquare[1]+1},{blackKingSquare[0]-1,blackKingSquare[1]-1}};
        for (int i = 0; i < 8; i++) {
            if (!validMove(state, blackKingSquare, choices[i], -1))
                val += 20;
        }
        val -= 10*(myAbs(whiteKingSquare[0]-blackKingSquare[0])+myAbs(whiteKingSquare[1]-blackKingSquare[1]));
    }
    if (val < -350) {
        square choices[8] = {{whiteKingSquare[0]+1,whiteKingSquare[1]},{whiteKingSquare[0]-1,whiteKingSquare[1]},{whiteKingSquare[0],whiteKingSquare[1]+1},{whiteKingSquare[0],whiteKingSquare[1]-1},
            {whiteKingSquare[0]+1,whiteKingSquare[1]+1},{whiteKingSquare[0]+1,whiteKingSquare[1]-1},{whiteKingSquare[0]-1,whiteKingSquare[1]+1},{whiteKingSquare[0]-1,whiteKingSquare[1]-1}};
        for (int i = 0; i < 8; i++) {
            if (!validMove(state, whiteKingSquare, choices[i], 1))
                val -= 20;
        }
        val += 10*(myAbs(whiteKingSquare[0]-blackKingSquare[0])+myAbs(whiteKingSquare[1]-blackKingSquare[1]));
    }
    val += (rand()%14)-7;
    return val;
}

int evaluateMaterial(GameState state) {
    int val = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            val += PIECE_VAL[state.board[i][j]];
        }
    }
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

struct MoveRoot {
    public:
        square start;
        square end;
        int depth;
        int trueDepth;
        int material;
};

int alphaBeta(GameState state, int depth, int turn, MoveRoot* root, int trueDepth, int alpha, int beta, int timeLimit) {
    if (depth == 0 && hasValidMove(state, turn)) {
        return turn*evaluateState(state);
    }
    if (!hasValidMove(state, turn)) {
        if (inCheck(state, turn))
            return -20000;
        else
            return 0;
    }
    vector<vector<int> > moves = validMoves(state, turn);
    int value = -100000;
    for (int i = 0; i < moves.size(); i++) {
        GameState newState = afterVecMove(state, moves.at(i));
        int nextDepth = depth-1;
        if (trueDepth >= 0 || (trueDepth == -1 && get_millis()-start_calc < timeLimit*0.25)) {
            int capturePiece = state.board[moves.at(i).at(2)][moves.at(i).at(3)];
            if (PIECE_SIDE[capturePiece] == turn*-1 && (evaluateMaterial(newState) == root->material || get_millis()-start_calc < timeLimit*0.3)) {
                nextDepth = 1;
                if (trueDepth >= 1 || get_millis()-start_calc < timeLimit*0.75)
                    nextDepth += 1;
                if (trueDepth >= 2)
                    nextDepth += 1;
            }
        }
        int curr_val = -1*alphaBeta(newState, nextDepth, -1*turn, root, trueDepth-1, -1*beta, -1*alpha, timeLimit)*pow(0.95, root->trueDepth-trueDepth);
        if (curr_val > value) {
            value = curr_val;
            if (trueDepth == root->depth) {
                root->start[0] = moves.at(i).at(0);
                root->start[1] = moves.at(i).at(1);
                root->end[0] = moves.at(i).at(2);
                root->end[1] = moves.at(i).at(3);
            }
            //cout << "best move: d" << trueDepth << " " << moves.at(i).at(0) << moves.at(i).at(1) << "->" << moves.at(i).at(2) << moves.at(i).at(3) << endl;
        }
        alpha = max(alpha, value);
        if (alpha >= beta || get_millis()-start_calc >= timeLimit) {
            if (trueDepth == root->depth && get_millis()-start_calc >= timeLimit)
                cout << "Out of time" << endl;
            break;
        }
    }
    root->trueDepth = min(root->trueDepth, trueDepth);
    if (trueDepth == root->depth)
        cout << " Anticipated value: " << value << endl;
    return value;

}

extern "C" void set_calc_time(int c) {
    calc_time = c;
}

extern "C" int minimax(GameState s, int turn, int depth, bool moreEndgameDepth=true) {
    srand(time(0));
    square noSquare = {-1,-1};
    MoveRoot* root = new MoveRoot();
    int searchDepth = depth;
    if (moreEndgameDepth && endgameState(s))
        searchDepth += 1;
    root->depth = searchDepth;
    root->trueDepth = searchDepth;
    root->material = evaluateMaterial(s);
    int score0 = evaluateState(s);
    cout << "turn: " << turn << endl;
    cout << "score: " << score0 << endl;
    start_calc = get_millis();
    alphaBeta(s, searchDepth, turn, root, searchDepth, -100000, 100000, calc_time);
    cout << "to depth " << root->trueDepth << endl;
    int res = 50000+root->start[0]*1000+root->start[1]*100+root->end[0]*10+root->end[1];
    cout << "res = " << res << endl;
    delete root;
    return res;
}

extern "C" int showMoves(GameState state, int turn) {
    vector<vector<int> > moves = validMoves(state, turn);
    for (int i = 0; i < moves.size(); i++) {
        cout << "(" << moves.at(i).at(0) << moves.at(i).at(1) << ") -> (" << moves.at(i).at(2) << moves.at(i).at(3) << ")" << endl;
    }
    return 0;
}