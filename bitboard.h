#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cstdint>
using namespace std;

const int PAWN_MAP[8][8] = {{90, 90, 90, 90, 90, 90, 90, 90},
                            {60, 50, 50, 50, 50, 50, 50, 60},
                            {10, 10, 20, 30, 30, 20, 10, 10},
                            {5,  5, 10, 25, 25, 10,  5,  5},
                            {0,  0,  0, 25, 25,  0,  0,  0},
                            {5, -5,-10,  0,  0,-10, -5,  5},
                            {10, 5, 10,-20,-25, 10,  5, 10},
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
                            {-10, -5,  5, 10, 10,  5, -5,-10},
                            {-10,  5, 25, 20, 20, 25,  5,-10},
                            {-10,  0, 25, 25, 25, 25,  0,-10},
                            {-10, 10, 10, 15, 15, 10, 10,-10},
                            {-10, 10,  0,  5,  5,  0, 10,-10},
                            {-20,-10,-20,-10,-10,-20,-10,-20}};
const int ROOK_MAP[8][8] = {{ 0,  0,  0,  0,  0,  0,  0,  0},
                            { 5, 10, 10, 10, 10, 10, 10,  5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            {-5,  0,  0,  0,  0,  0,  0, -5},
                            { 0,  0,  0,  5,  5,  0, 0,  0}};
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
                            {  0,  0,-15,-15,-15,-15,  0,  0},
                            { 25, 45, 80,  5,  5, 15, 85, 25}};
const int KING_MAP_ENDGAME[8][8] = {{-50,-40,-30,-20,-20,-30,-40,-50},
                                    {-30,-20,-10,  0,  0,-10,-20,-30},
                                    {-30,-10, 20, 30, 30, 20,-10,-30},
                                    {-30,-10, 30, 40, 40, 30,-10,-30},
                                    {-30,-10, 30, 40, 40, 30,-10,-30},
                                    {-30,-10, 20, 30, 30, 20,-10,-30},
                                    {-30,-30,  0,  0,  0,  0,-30,-30},
                                    {-50,-30,-30,-30,-30,-30,-30,-50}};

// https://alexanderameye.github.io/notes/chess-engine/

struct BitState {
    uint64_t white; // all white pieces
    uint64_t black; // all black pieces
    uint64_t pawns; // all pawns
    uint64_t knights; // all knights
    uint64_t bishops; // all bishops
    uint64_t rooks; // all rooks
    uint64_t queens; // all queens
    uint64_t kings; // all kings
    uint64_t moved; // all moved pieces
    uint8_t en_passant; // an index from 0 to 63

    uint8_t extra_info;
    // 00001 bit: side to move (0=white, 1=black)
    // 00010 bit: white kingside
    // 00100 bit: white queenside
    // 01000 bit: black kingside
    // 10000 bit: black queenside
};

struct BitMove {
    uint8_t start; // an index 0-63
    uint8_t end; // an index 0-63
    uint8_t promotion; // a number 0-3
    uint8_t en_passant; // an index 0-63 for the last pawn moved two squares, or 64 for none
    uint8_t capture; // an index 0-63 for the most recent piece captured, or 64 for none
    uint8_t castling; // bits of the form 000qkQKc representing castling rights and whether or not this move is castling
};

const uint64_t RANK_1 = 0xFF00000000000000;
const uint64_t RANK_2 = 0x00FF000000000000;
const uint64_t RANK_3 = 0x0000FF0000000000;
const uint64_t RANK_4 = 0x000000FF00000000;
const uint64_t RANK_5 = 0x00000000FF000000;
const uint64_t RANK_6 = 0x0000000000FF0000;
const uint64_t RANK_7 = 0x000000000000FF00;
const uint64_t RANK_8 = 0x00000000000000FF;

const uint64_t FILE_A = 0x0101010101010101;
const uint64_t FILE_B = 0x0202020202020202;
const uint64_t FILE_C = 0x0404040404040404;
const uint64_t FILE_D = 0x0808080808080808;
const uint64_t FILE_E = 0x1010101010101010;
const uint64_t FILE_F = 0x2020202020202020;
const uint64_t FILE_G = 0x4040404040404040;
const uint64_t FILE_H = 0x8080808080808080;

const uint64_t NW_EDGE = RANK_8 | FILE_A;
const uint64_t NE_EDGE = RANK_8 | FILE_H;
const uint64_t SW_EDGE = RANK_1 | FILE_A;
const uint64_t SE_EDGE = RANK_1 | FILE_H;

uint64_t flipVertical(uint64_t x) {
    return  ( (x << 56)                     ) |
            ( (x << 40) & 0x00ff000000000000) |
            ( (x << 24) & 0x0000ff0000000000) |
            ( (x <<  8) & 0x000000ff00000000) |
            ( (x >>  8) & 0x00000000ff000000) |
            ( (x >> 24) & 0x0000000000ff0000) |
            ( (x >> 40) & 0x000000000000ff00) |
            ( (x >> 56) );
}

void showBitboard(uint64_t board) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            uint64_t bit = 1;
            bit = bit << (i*8+j);
            if (board & bit) {
                cout << "1 ";
            } else {
                cout << "0 ";
            }
        }
        cout << endl;
    }
    cout << endl;
}

BitState afterBitMove(BitState state, BitMove move) {
    BitState newState = state;
    uint64_t startboard = 1;
    startboard = startboard << move.start;
    uint64_t endboard = 1;
    endboard = endboard << move.end;
    uint64_t tofrom = startboard ^ endboard;
    newState.white = state.white ^ tofrom;
    newState.black = state.black ^ tofrom;
    newState.pawns = state.pawns ^ tofrom;
    newState.knights = state.knights ^ tofrom;
    newState.bishops = state.bishops ^ tofrom;
    newState.rooks = state.rooks ^ tofrom;
    newState.queens = state.queens ^ tofrom;
    newState.kings = state.kings ^ tofrom;
    newState.moved = state.moved ^ tofrom;
    newState.en_passant = move.en_passant;
    newState.extra_info = move.castling * 2;
    newState.extra_info |= ~(state.extra_info & 1);

    uint64_t promoted = endboard & newState.pawns & (RANK_1 | RANK_8);
    if (promoted) {
        newState.pawns ^= promoted;
        if (move.promotion == 0) {
            newState.queens |= promoted;
        } else if (move.promotion == 1) {
            newState.rooks |= promoted;
        } else if (move.promotion == 2) {
            newState.bishops |= promoted;
        } else if (move.promotion == 3) {
            newState.knights |= promoted;
        }
    }

    return newState;
}

BitMove createBitMove(int start, int end, bool isCapture, bool isRook=false) {
    BitMove move;
    move.start = start;
    move.end = end;
    move.promotion = 0;
    move.en_passant = 64;
    move.capture = isCapture ? end : 64;
    move.castling = 0;
    if (isRook) {
        if (start == 0)
            move.castling &= ~8;
        else if (start == 7)
            move.castling &= ~4;
        else if (start == 56)
            move.castling &= ~2;
        else if (start == 63)
            move.castling &= ~1;
    }
    return move;
}

int num_shifted(uint64_t x) {
    int count = 0;
    while (x) {
        x >>= 1;
        count++;
    }
    return count - 1;
}

string numSquare(int sqr) {
    string files[] = {"a", "b", "c", "d", "e", "f", "g", "h"};
    string ranks[] = {"8", "7", "6", "5", "4", "3", "2", "1"};
    int rank = sqr / 8;
    int file = sqr % 8;
    return files[file] + ranks[rank];
}

string bitSquare(uint64_t sqr) {
    return numSquare(num_shifted(sqr));
}

// bitboard move generation

void scanRay(vector<BitMove> * moves, uint64_t start, int bitShift, uint64_t canmove, uint64_t enemy,
uint8_t extrainfo, uint64_t edge, bool isRook=false) {
    if (start & edge)
        return;
    uint64_t ray = start;
    int start_i = num_shifted(start);
    int ray_i = start_i;
    while (ray & ~edge) {
        if (bitShift < 0)
            ray >>= -bitShift;
        else
            ray <<= bitShift;
        ray_i += bitShift;
        if (ray & canmove) {
            BitMove move = createBitMove(start_i, ray_i, ray & enemy, isRook);
            if (!isRook)
                move.castling = extrainfo & (~1);
            moves->push_back(move);
            if (ray & enemy)
                break;
        } else {
            break;
        }
    }
}

vector<BitMove> pseudoLegal(BitState state) // ignoring check/pins
{
    vector<BitMove> moves;
    bool black = state.extra_info & 1;
    uint64_t pieces = black ? state.black : state.white;
    uint64_t enemy = black ? state.white : state.black;
    uint64_t empty = ~(pieces | enemy);
    uint64_t pawns = state.pawns & pieces;
    uint64_t knights = state.knights & pieces;
    uint64_t bishops = state.bishops & pieces;
    uint64_t rooks = state.rooks & pieces;
    uint64_t queens = state.queens & pieces;
    uint64_t kings = state.kings & pieces;
    uint64_t ep_sqr = 0;
    if (state.en_passant < 64)
        ep_sqr = 1ull << state.en_passant;
    uint64_t canmove = empty | enemy;
    uint64_t diagonal = bishops | queens;
    uint64_t adjacent = rooks | queens;
    
    for (int i = 0; i < 64; i++) {
        uint64_t sqr = 1ull << i;
        if (sqr & pieces) {
            if (sqr & pawns) {
                uint64_t bmoves = 0;
                uint64_t bmove_check[4];
                if (black) {
                    bmove_check[0] = sqr << 8;
                    bmove_check[1] = sqr << 16;
                    bmove_check[2] = sqr << 7;
                    bmove_check[3] = sqr << 9;
                    bmoves |= (sqr << 8) & empty;
                    if ((sqr & RANK_7) && ((sqr << 8) & empty))
                        bmoves |= (sqr << 16) & empty;
                    bmoves |= (sqr << 7) & (enemy | (ep_sqr << 8)) & ~FILE_H;
                    bmoves |= (sqr << 9) & (enemy | (ep_sqr << 8)) & ~FILE_A;
                } else {
                    bmove_check[0] = sqr >> 8;
                    bmove_check[1] = sqr >> 16;
                    bmove_check[2] = sqr >> 7;
                    bmove_check[3] = sqr >> 9;
                    bmoves |= (sqr >> 8) & empty;
                    if ((sqr & RANK_2) && ((sqr >> 8) & empty))
                        bmoves |= (sqr >> 16) & empty;
                    bmoves |= (sqr >> 7) & (enemy | (ep_sqr >> 8)) & ~FILE_A;
                    bmoves |= (sqr >> 9) & (enemy | (ep_sqr >> 8)) & ~FILE_H;
                }
                for (int j = 0; j < 4; j++) {
                    uint64_t move = bmove_check[j];
                    if (move & bmoves) {
                        int move_index = num_shifted(move);
                        BitMove bitMove = createBitMove(i, move_index, move & enemy);
                        bitMove.promotion = 0; // change to add non-queen promotion
                        if (((sqr << 16) | (sqr >> 16)) & move)
                            bitMove.en_passant = move_index;
                        bitMove.castling = state.extra_info / 2;
                        uint64_t behind = black ? (move >> 8) : (move << 8);
                        if (behind & ep_sqr)
                            bitMove.capture = state.en_passant;
                        moves.push_back(bitMove);
                    }
                }
            } else if (sqr & knights) {
                uint64_t bmoves = 0;
                uint64_t bmove_check[] = {sqr << 17, sqr << 15, sqr << 10, sqr << 6, sqr >> 17, sqr >> 15, sqr >> 10, sqr >> 6};
                bmoves |= ((sqr >> 17) & (~FILE_H & ~RANK_1 & ~RANK_2))
                    | ((sqr >> 15) & (~FILE_A & ~RANK_1 & ~RANK_2))
                    | ((sqr >> 10) & (~FILE_H & ~FILE_G & ~RANK_1))
                    | ((sqr >> 6) & (~FILE_A & ~FILE_B & ~RANK_1))
                    | ((sqr << 17) & (~FILE_A & ~RANK_8 & ~RANK_7))
                    | ((sqr << 15) & (~FILE_H & ~RANK_8 & ~RANK_7))
                    | ((sqr << 10) & (~FILE_A & ~FILE_B & ~RANK_8))
                    | ((sqr << 6) & (~FILE_H & ~FILE_G & ~RANK_8));
                bmoves &= canmove;
                for (int j = 0; j < 8; j++) {
                    uint64_t move = bmove_check[j];
                    if (move & bmoves) {
                        BitMove bitMove = createBitMove(i, num_shifted(move), move & enemy);
                        bitMove.castling = state.extra_info & (~1);
                        moves.push_back(bitMove);
                    }
                }
            } else if (sqr & kings) {
                uint64_t bmoves = 0;
                uint64_t bmove_check[] = {sqr << 8, sqr << 9, sqr << 7, sqr << 1,
                    sqr >> 1, sqr >> 8, sqr >> 9, sqr >> 7, sqr << 2, sqr >> 2};
                bmoves |= ((sqr >> 8) & (~RANK_1))
                    | ((sqr >> 9) & (~RANK_1 & ~FILE_A))
                    | ((sqr >> 7) & (~RANK_1 & ~FILE_H))
                    | ((sqr << 8) & (~RANK_8))
                    | ((sqr << 9) & (~RANK_8 & ~FILE_A))
                    | ((sqr << 7) & (~RANK_8 & ~FILE_H))
                    | ((sqr >> 1) & (~FILE_A))
                    | ((sqr << 1) & (~FILE_H));
                uint8_t kingside = 2;
                uint8_t queenside = 4;
                if (black) {
                    kingside = 8;
                    queenside = 16;
                }
                if (((sqr << 1) & empty) && (state.extra_info & kingside))
                    bmoves |= (sqr << 1);
                if (((sqr >> 1) & empty) && (state.extra_info & queenside))
                    bmoves |= (sqr >> 1);
                bmoves &= (canmove);
                // white kingside 2, white queenside 4, black kingside 8, black queenside 16, is castling 1
                for (int j = 0; j < 10; j++) {
                    uint64_t move = bmove_check[j];
                    if (move & bmoves) {
                        BitMove bitMove = createBitMove(i, num_shifted(move), move & enemy);
                        bitMove.castling = state.extra_info & (~1);
                        bitMove.castling &= ~kingside;
                        bitMove.castling &= ~queenside;
                        if (j >= 8)
                            bitMove.castling |= 1;
                        moves.push_back(bitMove);
                    }
                }
            } else {
                if (sqr & diagonal) {
                    scanRay(&moves, sqr, 9, canmove, enemy, state.extra_info, SE_EDGE);
                    scanRay(&moves, sqr, 7, canmove, enemy, state.extra_info, SW_EDGE);
                    scanRay(&moves, sqr, -9, canmove, enemy, state.extra_info, NW_EDGE);
                    scanRay(&moves, sqr, -7, canmove, enemy, state.extra_info, NE_EDGE);
                }
                if (sqr & adjacent) {
                    bool isRook = sqr & rooks;
                    scanRay(&moves, sqr, 8, canmove, enemy, state.extra_info, RANK_1, isRook);
                    scanRay(&moves, sqr, 1, canmove, enemy, state.extra_info, FILE_H, isRook);
                    scanRay(&moves, sqr, -8, canmove, enemy, state.extra_info, RANK_8, isRook);
                    scanRay(&moves, sqr, -1, canmove, enemy, state.extra_info, FILE_A, isRook);
                }
            }
        }
    }
    
    return moves;
}

bool bitCheck(BitState state) {
    BitState enemyState;
    enemyState.white = state.white;
    enemyState.black = state.black;
    enemyState.kings = state.kings;
    enemyState.queens = state.queens;
    enemyState.rooks = state.rooks;
    enemyState.bishops = state.bishops;
    enemyState.knights = state.knights;
    enemyState.pawns = state.pawns;
    bool blackToMove = state.extra_info & 1;
    vector<BitMove> enemyMoves = pseudoLegal(enemyState);
    uint64_t notEnemy = blackToMove ? state.white : state.black;
    for (int i = 0; i < enemyMoves.size(); i++) {
        uint64_t endboard = 1;
        endboard = endboard << enemyMoves.at(i).end;
        if (endboard & state.kings & notEnemy)
            return true;
    }

    return false;
}

vector<BitMove> bitLegal(BitState state) {
    vector<BitMove> moves = pseudoLegal(state);
    vector<BitMove> legalMoves;
    for (int i = 0; i < moves.size(); i++) {
        BitState newState = afterBitMove(state, moves[i]);
        if (!bitCheck(newState))
            legalMoves.push_back(moves[i]);
    }
    return legalMoves;
}