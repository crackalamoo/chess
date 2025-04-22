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
#include "bitboard.h"
using namespace std;

#ifndef nullptr
#define nullptr 0
#endif

const int PIECE_VAL[] = { 0, 100, 320, 330, 500, 900, 20000, -100, -320, -330, -500, -900, -20000 };
const int SIMPLIFIED_PIECE_VAL[] = {0, 100, 300, 300, 500, 900, 20000, -100, -300, -300, -500, -900, -20000};
const int PIECE_SIDE[] = { 0, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1 };
const int FILE_SEARCH[] = {3, 4, 2, 5, 1, 6, 0, 7};
const int RANK_SEARCH[] = {3, 4, 1, 6, 0, 7, 2, 5};

typedef short square[2];

struct GameState {
    short board[8][8];
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
    short piece = newState.board[start[0]][start[1]];
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
    int king = 9-3*turn;
    vector< vector<int> > moves;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (PIECE_SIDE[state.board[i][j]] == turn*-1) {
                short piece = state.board[i][j];
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
    short piece = state.board[start[0]][start[1]];
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
                // en passant
                if (state.board[end[0]+1][end[1]] == 7 && end[0] == 2 && state.board[1][end[1]] == 0 && sameSquare(state.lastMoved, ep))
                    return true;
                return false;
            } else {
                return true;
            }
        } else if (end[0] == start[0]-2 && end[1] == start[1]) { // # moving forward two
            if (state.moved[start[0]][start[1]] || state.board[start[0]-1][start[1]] != 0 || state.board[start[0]-2][start[1]] != 0)
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
                // en passant
                if (state.board[end[0]-1][end[1]] == 1 && end[0] == 5 && state.board[6][end[1]] == 0 && sameSquare(state.lastMoved, ep))
                    return true;
                return false;
            } else {
                return true;
            }
        } else if (end[0] == start[0]+2 && end[1] == start[1]) { // # moving forward two
            if (state.moved[start[0]][start[1]] || state.board[start[0]+1][start[1]] != 0 || state.board[start[0]+2][start[1]] != 0)
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
            if (state.board[start[0]][5] != 0 || state.moved[start[0]][4] || state.moved[start[0]][7]
            || state.board[start[0]][7] != castle || inCheck(state, turn) || inCheck(between_board, turn))
                return false;
        } else if (end[0] == start[0] && end[1] == start[1]-2 && end[1] == 2) { // queenside castle
            square between_square = {start[0], start[1]-1};
            GameState between_board = afterMove(state, start, between_square);
            if (state.board[start[0]][3] != 0 || state.board[start[0]][2] != 0 || state.board[start[0]][1] != 0
            || state.moved[start[0]][4] || state.moved[start[0]][0] || state.board[start[0]][0] != castle
            || inCheck(state, turn) || inCheck(between_board, turn))
                return false;
        } else {
            return false;
        }
    }
    return true;
}}

vector<vector<int> > validMoves(GameState state, int turn) {
    vector< vector<int> > moves;
    int wpm[][2] = {{-1,-1},{-1,1},{-1,0},{-2,0}};
    int bpm[][2] = {{1,-1},{1,1},{1,0},{2,0}};
    int nm[][2] = {{2,1},{2,-1},{1,2},{1,-2},{-1,2},{-1,-2},{-2,1},{-2,-1}};
    int bm[][2] = {{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7},
                    {-1,1},{-2,2},{-3,3},{-4,4},{-5,5},{-6,6},{-7,7},
                    {1,-1},{2,-2},{3,-3},{4,-4},{5,-5},{6,-6},{7,-7},
                    {-1,-1},{-2,-2},{-3,-3},{-4,-4},{-5,-5},{-6,-6},{-7,-7}};
    int rm[][2] = {{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0},
                    {0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},
                    {-1,0},{-2,0},{-3,0},{-4,0},{-5,0},{-6,0},{-7,0},
                    {0,-1},{0,-2},{0,-3},{0,-4},{0,-5},{0,-6},{0,-7}};
    int km[][2] = {{1,1},{1,-1},{-1,1},{-1,-1},{1,0},{0,1},{-1,0},{0,-1},{0,2},{0,-2}};
    for (int i0 = 0; i0 < 8; i0++) {
        for (int j0 = 0; j0 < 8; j0++) {
            int i = RANK_SEARCH[i0];
            int j = FILE_SEARCH[j0];
            square start = {i,j};
            if (PIECE_SIDE[state.board[i][j]] == turn) {
                short piece = state.board[i][j];
                switch(piece) {
                    case 1:
                        for (int k = 0; k < 4; k++) {
                            square end = {i+wpm[k][0],j+wpm[k][1]};
                            if (validMove(state, start, end, turn)) {
                                vector<int> m;
                                m.push_back(i);
                                m.push_back(j);
                                m.push_back(i+wpm[k][0]);
                                m.push_back(j+wpm[k][1]);
                                moves.push_back(m);
                            }
                        }
                        break;
                    case 7:
                        for (int k = 0; k < 4; k++) {
                            square end = {i+bpm[k][0],j+bpm[k][1]};
                            if (validMove(state, start, end, turn)) {
                                vector<int> m;
                                m.push_back(i);
                                m.push_back(j);
                                m.push_back(i+bpm[k][0]);
                                m.push_back(j+bpm[k][1]);
                                moves.push_back(m);
                            }
                        }
                        break;
                    case 2:
                    case 8:
                        for (int k = 0; k < 8; k++) {
                            square end = {i+nm[k][0],j+nm[k][1]};
                            if (validMove(state, start, end, turn)) {
                                vector<int> m;
                                m.push_back(i);
                                m.push_back(j);
                                m.push_back(i+nm[k][0]);
                                m.push_back(j+nm[k][1]);
                                moves.push_back(m);
                            }
                        }
                        break;
                    case 6:
                    case 12:
                        for (int k = 0; k < 10; k++) {
                            square end = {i+km[k][0],j+km[k][1]};
                            if (validMove(state, start, end, turn)) {
                                vector<int> m;
                                m.push_back(i);
                                m.push_back(j);
                                m.push_back(i+km[k][0]);
                                m.push_back(j+km[k][1]);
                                moves.push_back(m);
                            }
                        }
                        break;
                }
                if (piece == 3 || piece == 9 || piece == 5 || piece == 11) {
                    for (int k = 0; k < 28; k++) {
                        square end = {i+bm[k][0],j+bm[k][1]};
                        if (validMove(state, start, end, turn)) {
                            vector<int> m;
                            m.push_back(i);
                            m.push_back(j);
                            m.push_back(i+bm[k][0]);
                            m.push_back(j+bm[k][1]);
                            moves.push_back(m);
                        }
                    }
                }
                if (piece == 4 || piece == 10 || piece == 5 || piece == 11) {
                    for (int k = 0; k < 28; k++) {
                        square end = {i+rm[k][0],j+rm[k][1]};
                        if (validMove(state, start, end, turn)) {
                            vector<int> m;
                            m.push_back(i);
                            m.push_back(j);
                            m.push_back(i+rm[k][0]);
                            m.push_back(j+rm[k][1]);
                            moves.push_back(m);
                        }
                    }
                }
            }
        }
    }
    return moves;
}

bool endgameState(GameState state) {
    int whiteVal = 0;
    int blackVal = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (state.board[i][j] == 5 || state.board[i][j] == 11)
                return false;
        }
    }
    return true;
}

int myAbs(int n) {
    if (n < 0)
        return n*-1;
    return n;
}

extern "C" int evaluateState(GameState state) {
    int val = 0;
    square whiteKingSquare = {0,0};
    square blackKingSquare = {0,0};
    int whiteBishops = 0;
    int blackBishops = 0;
    bool isEndgame = endgameState(state);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            short piece = state.board[i][j];
            val += PIECE_VAL[piece];
            if (state.moved[i][j]) {
                if (piece == 2 || piece == 3)
                    val += 35;
                else if (piece == 8 || piece == 9)
                    val -= 35;
                if (!isEndgame) {
                    if (piece == 5)
                        val -= 35;
                    else if (piece == 11)
                        val += 35;
                }
            }
            switch(piece) {
                case 0: break;
                case 1: val += PAWN_MAP[i][j]; break;
                case 2: val += KNIGHT_MAP[i][j]; break;
                case 3:
                    val += BISHOP_MAP[i][j];
                    whiteBishops++;
                    break;
                case 4: val += ROOK_MAP[i][j]; break;
                case 5: val += QUEEN_MAP[i][j]; break;
                case 7: val -= PAWN_MAP[7-i][j]; break;
                case 8: val -= KNIGHT_MAP[7-i][j]; break;
                case 9:
                    val -= BISHOP_MAP[7-i][j];
                    blackBishops++;
                    break;
                case 10: val -= ROOK_MAP[7-i][j]; break;
                case 11: val -= QUEEN_MAP[7-i][j]; break;
                case 6:
                    whiteKingSquare[0] = i;
                    whiteKingSquare[1] = j;
                    if (isEndgame)
                        val += KING_MAP_ENDGAME[i][j];
                    else
                        val += KING_MAP[i][j];
                    break;
                case 12:
                    blackKingSquare[0] = i;
                    blackKingSquare[1] = j;
                    if (isEndgame)
                        val -= KING_MAP_ENDGAME[7-i][j];
                    else
                        val -= KING_MAP[7-i][j];
                    break;
            }
        }
    }
    if (state.board[6][3] == 1 && state.board[6][4] == 1)
        val -= 305;
    if (state.board[1][3] == 7 && state.board[1][4] == 7)
        val += 305;
    if (whiteBishops >= 2)
        val += 35;
    if (blackBishops >= 2)
        val -= 35;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            switch (state.board[i][j]) {
                case 2:
                case 3:
                case 4:
                    val += 1.5*(16-myAbs(blackKingSquare[0]-i) - myAbs(blackKingSquare[1]-j)); break;
                case 8:
                case 9:
                case 10:
                    val -= 1.5*(16-myAbs(whiteKingSquare[0]-i) - myAbs(whiteKingSquare[1]-j)); break;
                case 5:
                    val += 0.5*(16-myAbs(blackKingSquare[0]-i) - myAbs(blackKingSquare[1]-j)); break;
                case 11:
                    val -= 0.5*(16-myAbs(whiteKingSquare[0]-i) - myAbs(whiteKingSquare[1]-j)); break;
                default: break;
            }
        }
    }
    if (val > 450) {
        if (isEndgame)
            val -= KING_MAP_ENDGAME[whiteKingSquare[0]][whiteKingSquare[1]];
        else
            val -= KING_MAP[whiteKingSquare[0]][whiteKingSquare[1]];
        square choices[8] = {{blackKingSquare[0]+1,blackKingSquare[1]},{blackKingSquare[0]-1,blackKingSquare[1]},{blackKingSquare[0],blackKingSquare[1]+1},{blackKingSquare[0],blackKingSquare[1]-1},
            {blackKingSquare[0]+1,blackKingSquare[1]+1},{blackKingSquare[0]+1,blackKingSquare[1]-1},{blackKingSquare[0]-1,blackKingSquare[1]+1},{blackKingSquare[0]-1,blackKingSquare[1]-1}};
        for (int i = 0; i < 8; i++) {
            if (!validMove(state, blackKingSquare, choices[i], -1))
                val += 15;
        }
        val -= 35*(myAbs(whiteKingSquare[0]-blackKingSquare[0])+myAbs(whiteKingSquare[1]-blackKingSquare[1]));
    }
    else if (val < -450) {
        if (isEndgame)
            val += KING_MAP_ENDGAME[blackKingSquare[0]][blackKingSquare[1]];
        else
            val += KING_MAP[blackKingSquare[0]][blackKingSquare[1]];
        square choices[8] = {{whiteKingSquare[0]+1,whiteKingSquare[1]},{whiteKingSquare[0]-1,whiteKingSquare[1]},{whiteKingSquare[0],whiteKingSquare[1]+1},{whiteKingSquare[0],whiteKingSquare[1]-1},
            {whiteKingSquare[0]+1,whiteKingSquare[1]+1},{whiteKingSquare[0]+1,whiteKingSquare[1]-1},{whiteKingSquare[0]-1,whiteKingSquare[1]+1},{whiteKingSquare[0]-1,whiteKingSquare[1]-1}};
        for (int i = 0; i < 8; i++) {
            if (!validMove(state, whiteKingSquare, choices[i], 1))
                val -= 15;
        }
        val += 35*(myAbs(whiteKingSquare[0]-blackKingSquare[0])+myAbs(whiteKingSquare[1]-blackKingSquare[1]));
    }
    // val += (rand()%10)-5;
    return val;
}

int evaluateMaterial(GameState state) {
    int val = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            val += SIMPLIFIED_PIECE_VAL[state.board[i][j]];
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

bool sameState(GameState s1, GameState s2) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (s1.board[i][j] != s2.board[i][j])
                return false;
        }
    }
    if ((s1.board[0][4] == 12 && s1.moved[0][4] != s2.moved[0][4])
    || (s1.board[7][4] == 6 && s1.moved[7][4] != s2.moved[0][4])) {
        return false;
    }
    if ((s1.board[0][4] == 12 && s1.board[0][7] == 10 && s1.moved[0][7] != s2.moved[0][7])
    || (s1.board[0][4] == 12 && s1.board[0][0] == 10 && s1.moved[0][0] != s2.moved[0][0])
    || (s1.board[7][4] == 6 && s1.board[7][7] == 10 && s1.moved[7][7] != s2.moved[7][7])
    || (s1.board[7][4] == 6 && s1.board[7][0] == 10 && s1.moved[7][0] != s2.moved[7][0])) {
        return false;
    }
    return true;
}
int num_repetitions(GameState states[], int size, GameState state) {
    int counter = 0;
    for (int i = 0; i < size; i++) {
        if (sameState(state, states[i]))
            counter++;
    }
    return counter;
}
int num_reps_extra(GameState states[], int size, GameState state, GameState extra) {
    int counter = 0;
    for (int i = 0; i < size; i++) {
        if (sameState(state, states[i]))
            counter++;
    }
    if (sameState(state, extra))
        counter++;
    return counter;
}

extern "C" int gameRes(GameState states[], int statesSize, GameState state, int turn) {
    int reps = num_repetitions(states, statesSize, state);
    if (reps >= 3)
        return 3; // threefold repetition
    vector<vector<int> > moves = validMoves(state, turn);
    if (moves.size() > 0)
        return 0; // game continuing
    if (inCheck(state, turn))
        return 1; // checkmate
    return 2; // stalemate
}

struct MoveRoot {
    public:
        square start;
        square end;
        int depth;
        int trueDepth;
        int material;
        unordered_map<string, int> tp_table;
};

string stateToString(GameState state, int turn=0) {
    // turn 0 means don't care about turn
    string s = "";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            switch(state.board[i][j]) {
                case 0: s += ".";
                case 1: s += "p";
                case 2: s += "n";
                case 3: s += "b";
                case 4: s += "r";
                case 5: s += "q";
                case 6: s += "k";
                case 7: s += "P";
                case 8: s += "N";
                case 9: s += "B";
                case 10: s += "R";
                case 11: s += "Q";
                case 12: s += "K";
            }
            if (state.moved[i][j]) {
                s += "m";
            } else {
                s += "_";
            }
        }
    }
    if (turn == 1) {
        s += "w";
    } else if (turn == -1) {
        s += "b";
    }
    return s;
}

int alphaBeta(GameState state, int depth, int turn, MoveRoot* root, int trueDepth, int alpha, int beta, int timeLimit) {
    if (root->tp_table.find(stateToString(state, turn)) != root->tp_table.end()) {
        return root->tp_table[stateToString(state, turn)];
    }
    GameState states[] = {};
    int res = gameRes(states, 0, state, turn);
    if (depth == 0 && res == 0) {
        return turn*evaluateState(state);
    }
    if (res != 0) {
        if (res == 1)
            return -20000;
        else
            return 0;
    }
    vector<vector<int> > moves = validMoves(state, turn);
    int value = -100000;
    int bottomDepth = -8;
    for (int i = 0; i < moves.size(); i++) {
        GameState newState = afterVecMove(state, moves.at(i));
        int nextDepth = depth-1;
        int timeElapsed = get_millis()-start_calc;
        int extraPoints = 0;
        //bool threefold = (num_reps_extra(states, statesSize, newState, state) >= 3);
        //bool threefold = false;
        if (trueDepth >= bottomDepth) {
            int movingPiece = state.board[moves.at(i).at(0)][moves.at(i).at(1)] - (6*(turn==-1));
            int capturePiece = state.board[moves.at(i).at(2)][moves.at(i).at(3)];
            int curr_material = turn*evaluateMaterial(newState);
            bool capture = (PIECE_SIDE[capturePiece] == turn*-1);
            if (capture && trueDepth == root->depth)
                extraPoints += (curr_material > 0)*(12 - movingPiece*2);
            /* bool check = false;
            if (trueDepth > 0) {
                check = inCheck(newState, turn*-1);
                if (check)
                    extraPoints += 20;
            } */
            if (capture && timeElapsed < timeLimit) {
                nextDepth = max(nextDepth, 1);
            }
        }
        int curr_val;
        curr_val = -alphaBeta(newState, nextDepth, -1*turn, root, trueDepth-1, -1*beta, -1*alpha, timeLimit);
        curr_val += extraPoints;
        if (curr_val > value) {
            value = curr_val;
            if (trueDepth == root->depth) {
                root->start[0] = moves.at(i).at(0);
                root->start[1] = moves.at(i).at(1);
                root->end[0] = moves.at(i).at(2);
                root->end[1] = moves.at(i).at(3);
                /* cout << "best move: d" << trueDepth << " "
                << moves.at(i).at(0) << moves.at(i).at(1) << "->" << moves.at(i).at(2) << moves.at(i).at(3)
                << " v=" << value << " v0=" << evaluateState(newState) << endl; */
            }
        }
        alpha = max(alpha, value);
        // if (alpha >= beta || timeElapsed >= timeLimit) {
        if (alpha >= beta) {
            if (trueDepth == root->depth && timeElapsed >= timeLimit)
                cout << "Out of time" << endl;
            break;
        }
    }
    root->trueDepth = min(root->trueDepth, trueDepth);
    if (trueDepth == root->depth)
        cout << "Anticipated value: " << value << endl;
    root->tp_table[stateToString(state, turn)] = value;
    return value;

}

// bitboard reimplementation

BitState gamestate_to_bitstate(GameState state, int turn) {
    BitState bitstate;
    bitstate.white = 0;
    bitstate.black = 0;
    bitstate.pawns = 0;
    bitstate.knights = 0;
    bitstate.bishops = 0;
    bitstate.rooks = 0;
    bitstate.queens = 0;
    bitstate.kings = 0;
    bitstate.moved = 0;
    bitstate.en_passant = 64;
    bitstate.extra_info = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (state.board[i][j] != 0) {
                short piece = state.board[i][j];
                uint64_t bit = 1;
                bit = bit << (i*8+j);
                if (PIECE_SIDE[piece] == 1) {
                    bitstate.white |= bit;
                } else {
                    bitstate.black |= bit;
                }
                if (piece == 1 || piece == 7) {
                    bitstate.pawns |= bit;
                } else if (piece == 2 || piece == 8) {
                    bitstate.knights |= bit;
                } else if (piece == 3 || piece == 9) {
                    bitstate.bishops |= bit;
                } else if (piece == 4 || piece == 10) {
                    bitstate.rooks |= bit;
                } else if (piece == 5 || piece == 11) {
                    bitstate.queens |= bit;
                } else if (piece == 6 || piece == 12) {
                    bitstate.kings |= bit;

                    // check castling rights
                    if (!state.moved[i][j]) {
                        int castle = 4;
                        uint8_t kingside = 2;
                        uint8_t queenside = 4;
                        if (piece == 12) {
                            castle = 10;
                            kingside = 8;
                            queenside = 16;
                        }
                        // kingside castle
                        if (state.board[i][j+3] == castle && !state.moved[i][j+3]) {
                            bitstate.extra_info |= kingside;
                        }
                        // queenside castle
                        if (state.board[i][j-4] == castle && !state.moved[i][j-4]) {
                            bitstate.extra_info |= queenside;
                        }
                    }
                }
                if (state.moved[i][j]) {
                    bitstate.moved |= bit;
                }
            }
        }
    }
    square lastmv = {state.lastMoved[0], state.lastMoved[1]};
    if ((state.board[lastmv[0]][lastmv[1]] == 1 && lastmv[0] == 4)
    || (state.board[lastmv[0]][lastmv[1]] == 7 && lastmv[0] == 3)) {
        bitstate.en_passant = lastmv[0]*8+lastmv[1];
    }
    if (turn == -1) {
        cout << "converting as black" << endl;
        bitstate.extra_info |= 1;
    }
    cout << numSquare(bitstate.en_passant) << endl;
    return bitstate;
}



// exported functions

extern "C" void set_calc_time(int c) {
    calc_time = c;
}

extern "C" int showMoves(GameState state, int turn) {
    vector<vector<int> > moves = validMoves(state, turn);
    for (int i = 0; i < moves.size(); i++) {
        cout << "(" << moves.at(i).at(0) << moves.at(i).at(1) << ") -> (" << moves.at(i).at(2) << moves.at(i).at(3) << ")" << endl;
    }
    return 0;
}

extern "C" void showBitMoves(GameState state, int turn) {
    BitState bboard = gamestate_to_bitstate(state, turn);
    vector<BitMove> moves = bitLegal(bboard);
    for (int i = 0; i < moves.size(); i++) {
        uint8_t start = moves.at(i).start;
        uint8_t end = moves.at(i).end;
        cout << "(" << numSquare((int)(start)) << ") -> (" << numSquare((int)(end)) << ")" << endl;
    }
    cout << "info: " << bboard.extra_info << endl;
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
    root->tp_table = unordered_map<string, int>();
    cout << "current evaluation: " << evaluateState(s) << endl;
    int score0 = evaluateState(s);
    start_calc = get_millis();
    alphaBeta(s, searchDepth, turn, root, searchDepth, -100000, 100000, calc_time);
    int res = 50000+root->start[0]*1000+root->start[1]*100+root->end[0]*10+root->end[1];
    delete root;
    return res;
}