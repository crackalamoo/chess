#include <stdio.h>
#include <iostream>
#include <vector>
using namespace std;

const int PIECE_VAL[] = { 0, 10, 30, 30, 50, 90, 999, -10, -30, -50, -90, -999 };
const int PIECE_SIDE[] = { 0, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1 };
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

extern "C" {


GameState afterMove(GameState state, square start, square end) {
    GameState newState;
    square captured = {end[0], end[1]};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            newState.board[i][j] = state.board[i][j];
            newState.moved[i][j] = state.moved[i][j];
        }
    }
    int piece = newState.board[start[0]][start[1]];
    if (piece == 1 && end[1]-start[1] != 0 && newState.board[end[0]][end[1]] == 0) // white en passant
        newState.board[end[0]+1][end[1]] = 0;
        captured[0] = end[0]+1;
    if (piece == 7 && end[1]-start[1] != 0 && newState.board[end[0]][end[1]] == 0) // black en passant
        newState.board[end[0]-1][end[1]] = 0;
        captured[0] = end[0]-1;
    newState.board[end[0]][end[1]] = piece;
    newState.board[start[0]][start[1]] = 0;
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
    newState.moved[start[0]][start[1]] = 0;
    newState.moved[captured[0]][captured[1]] = 0;
    newState.moved[end[0]][end[1]] = 1;
    newState.lastMoved[0] = end[0];
    newState.lastMoved[1] = end[1];
    return newState;
}

bool validMove(GameState state, square start, square end, int turn, bool useCheck);

bool inCheck(GameState state, int turn) {
    int enemy = 12;
    if (turn == -1) enemy = 6;
    int map1[][2] = {{-1,-1},{-1,1}};
    int map2[][2] = {{2,1},{2,-1},{1,2},{1,-2},{-1,2},{-1,-2},{-2,1},{-2,-1}};
    int map3[][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    int map4[][2] = {{1,0},{0,1},{-1,0},{0,-1}};
    int map5[][2] = {{1,0},{0,1},{-1,0},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    int map6[][2] = {{1,0},{0,1},{-1,0},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    int map7[][2] = {{1,-1},{1,1}};
    int (*dir_map)[2];
    int sizes[] = {2,8,4,4,8,8,2,8,4,4,8,8};
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (PIECE_SIDE[state.board[i][j]] == turn*-1) {
                int piece = state.board[i][j];
                int steps = 1;
                square startPos = {i,j};
                switch(piece) {
                    case 1:
                        dir_map = map1;
                        break;
                    case 7:
                        dir_map = map7;
                        break;
                    case 2:
                    case 8:
                        dir_map = map2;
                        break;
                    case 3:
                    case 9:
                        dir_map = map3;
                        break;
                    case 4:
                    case 10:
                        dir_map = map4;
                        break;
                    case 5:
                    case 11:
                        dir_map = map5;
                        break;
                    case 6:
                    case 12:
                        dir_map = map6;
                        break;
                }
                for (int d = 0; d < sizes[piece]; d++) {
                    square pos = {i,j};
                    for (int k = 0; k < steps; k++) {
                        pos[0] += *(dir_map+d)[0];
                        pos[1] += *(dir_map+d)[1];
                        if (validMove(state, startPos, pos, turn, false) && state.board[pos[0]][pos[1]] == enemy) {
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
    square pos = {end[0]-dy,end[1]-dx};
    while (pos != start) {
        if (pos[0] < 0 || pos[0] >= 8 || pos[1] < 0 || pos[1] >= 8 || state.board[pos[0]][pos[1]] != 0)
            return false;
        pos[0] -= dy;
        pos[1] -= dx;
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
    int map1[][2] = {{-1,-1},{-1,0},{-1,1},{-2,0}};
    int map2[][2] = {{2,1},{2,-1},{1,2},{1,-2},{-1,2},{-1,-2},{-2,1},{-2,-1}};
    int map3[][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    int map4[][2] = {{1,0},{0,1},{-1,0},{0,-1}};
    int map5[][2] = {{1,0},{0,1},{-1,0},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    int map6[][2] = {{1,0},{0,1},{-1,0},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1},{0,2},{0,-2}};
    int map7[][2] = {{1,-1},{1,0},{1,1},{2,0}};
    int (*dir_map)[2];
    int sizes[] = {4,8,4,4,8,10,4,8,4,4,8,10};
    dir_map = map1;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (PIECE_SIDE[state.board[i][j]] == turn) {
                int piece = state.board[i][j];
                int steps = 1;
                square startPos = {i,j};
                switch(piece) {
                    case 1:
                        dir_map = map1;
                        break;
                    case 7:
                        dir_map = map7;
                        break;
                    case 2:
                    case 8:
                        dir_map = map2;
                        break;
                    case 3:
                    case 9:
                        dir_map = map3;
                        break;
                    case 4:
                    case 10:
                        dir_map = map4;
                        break;
                    case 5:
                    case 11:
                        dir_map = map5;
                        break;
                    case 6:
                    case 12:
                        dir_map = map6;
                        break;
                }
                for (int d = 0; d < sizes[piece]; d++) {
                    square pos = {i,j};
                    for (int k = 0; k < steps; k++) {
                        pos[0] += *(dir_map+d)[0];
                        pos[1] += *(dir_map+d)[1];
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