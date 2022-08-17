import ctypes
from math import *

DEFAULT_BOARD = [[10,8, 9, 11,12,9, 8,10],
                [7, 7, 7, 7, 7, 7, 7, 7],
                [0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0],
                [1, 1, 1, 1, 1, 1, 1, 1],
                [4, 2, 3, 5, 6, 3, 2, 4]]
for i in range(8):
    for j in range(8):
        DEFAULT_BOARD[i][j] = bytes([DEFAULT_BOARD[i][j]])

piece_icons = ['[ ]', u'[\N{WHITE CHESS PAWN}]', u'[\N{WHITE CHESS KNIGHT}]', u'[\N{WHITE CHESS BISHOP}]', u'[\N{WHITE CHESS ROOK}]', u'[\N{WHITE CHESS QUEEN}]', u'[\N{WHITE CHESS KING}]',
    u'[\N{BLACK CHESS PAWN}]', u'[\N{BLACK CHESS KNIGHT}]', u'[\N{BLACK CHESS BISHOP}]', u'[\N{BLACK CHESS ROOK}]', u'[\N{BLACK CHESS QUEEN}]', u'[\N{BLACK CHESS KING}]']


class GameState(ctypes.Structure):
    _fields_ = [("board", (ctypes.c_char*8)*8), ("moved", (ctypes.c_bool*8)*8), ("lastMoved", ctypes.c_int*2)]

def cppBoard(b):
    input_board = []
    cpp_row = ctypes.c_char*8
    cpp_board = cpp_row*8
    for i in range(8):
        input_board.append(cpp_row(*b[i]))
    input_board = cpp_board(*input_board)
    return input_board
def cppSquare(s):
    cpp_square = ctypes.c_int*2
    return cpp_square(*s)
def cppMoved(mp):
    mp_board = []
    cpp_row = ctypes.c_bool*8
    for i in range(8):
        row = []
        for j in range(8):
            if (i,j) in mp:
                row.append(1)
            else:
                row.append(0)
        mp_board.append(cpp_row(*row))
    mp_board = (cpp_row*8)(*mp_board)
    return mp_board
def cppState(b, mp):
    state = GameState()
    state.board = cppBoard(b)
    state.moved = cppMoved(mp)
    if len(mp) == 0:
        state.lastMoved = cppSquare((-1,-1))
    else:
        state.lastMoved = cppSquare(mp[len(mp)-1])
    return state
def pythonBoard(state):
    b = []
    for i in range(8):
        b.append([])
        for j in range(8):
            b[i].append(state.board[i][j])
    return b
def pythonMoved(state):
    mp = []
    lastMoved = []
    lastMoved.append(state.lastMoved[0])
    lastMoved.append(state.lastMoved[1])
    for i in range(8):
        for j in range(8):
            if state.moved[i][j] == 1 and not (i,j) == (lastMoved[0],lastMoved[1]):
                mp.append((i,j))
    mp.append((lastMoved[0],lastMoved[1]))
    return mp



chessFuncs = ctypes.CDLL('_chess.so')
cpp_afterMove = chessFuncs.afterMove
cpp_afterMove.restype = GameState
cpp_validMove = chessFuncs.validMove
cpp_validMove.restype = ctypes.c_bool
cpp_inCheck = chessFuncs.inCheck
cpp_inCheck.restype = ctypes.c_bool
cpp_minimax = chessFuncs.minimax
cpp_minimax.restype = ctypes.c_int
cpp_showMoves = chessFuncs.showMoves
cpp_showMoves.restype = ctypes.c_int
cpp_gameRes = chessFuncs.gameRes
cpp_gameRes.restype = ctypes.c_int
cpp_setCalc = chessFuncs.set_calc_time
cpp_evalState = chessFuncs.evaluateState
cpp_evalState.restype = ctypes.c_int


def afterMove(b, mp, start, end, promotion=5):
    oldState = cppState(b, mp)
    newState = cpp_afterMove(oldState, cppSquare(start), cppSquare(end), ctypes.c_int(promotion))
    return (pythonBoard(newState), pythonMoved(newState))
def validMove(b, mp, start, end, turn, useCheck=True):
    state = cppState(b, mp)
    return cpp_validMove(state, cppSquare(start), cppSquare(end), ctypes.c_int(turn), ctypes.c_bool(useCheck))
def inCheck(b, mp, turn):
    state = cppState(b, mp)
    return cpp_inCheck(state, ctypes.c_int(turn))
def gameRes(b, mp, turn):
    state = cppState(b, mp)
    return cpp_gameRes(state, ctypes.c_int(turn))
def minimax(b, mp, turn, depth, time, moreEndgameDepth):
    cpp_setCalc(time)
    state = cppState(b, mp)
    res = cpp_minimax(state, ctypes.c_int(turn), ctypes.c_int(depth), ctypes.c_bool(moreEndgameDepth))
    return ((int((res%10000)/1000),int((res%1000)/100)), (int((res%100)/10), int(res%10)), int(res/10000))
def showMoves(b, mp, turn):
    state = cppState(b, mp)
    cpp_showMoves(state, ctypes.c_int(turn))
def evalState(b, mp):
    state = cppState(b, mp)
    return cpp_evalState(state)

def validMoves(b, mp, turn):
    for i in range(8):
        for j in range(8):
            b[i][j] = int.from_bytes(b[i][j], 'little')
    moves = []
    whitePawnDirs = [[-1,-1],[-1,1],[-1,0],[-2,0]]
    blackPawnDirs = [[1,-1],[1,1],[1,0],[2,0]]
    knightDirs = [[2,1],[2,-1],[1,2],[1,-2],[-1,2],[-1,-2],[-2,1],[-2,-1]]
    bishopDirs = [[1,1],[-1,1],[1,-1],[-1,-1]]
    rookDirs = [[1,0],[0,1],[-1,0],[0,-1]]
    queenDirs = [[1,1],[-1,1],[1,-1],[-1,-1],[1,0],[0,1],[-1,0],[0,-1]]
    kingDirs = [[1,1],[-1,1],[1,-1],[-1,-1],[1,0],[0,1],[-1,0],[0,-1],[0,2],[0,-2]]
    for i in range(8):
        for j in range(8):
            if abs(b[i][j]) == turn:
                piece = b[i][j]
                directions = []
                steps = 1
                if piece == 1:
                    directions = whitePawnDirs
                elif piece == 7:
                    directions = blackPawnDirs
                elif piece in [2,8]:
                    directions = knightDirs
                elif piece in [3,9]:
                    directions = bishopDirs
                elif piece in [4,10]:
                    directions = rookDirs
                elif piece in [5,11]:
                    directions = queenDirs
                elif piece in [6,12]:
                    directions = kingDirs
                if piece in [3,4,5,9,10,11]:
                    steps = 7
                startPos = [i,j]
                for d in range(len(directions)):
                    pos = [i,j]
                    for k in range(steps):
                        pos[0] += directions[d][0]
                        pos[1] += directions[d][1]
                        if (validMove(b, mp, startPos, pos, turn)):
                            moves.append([[startPos[0],startPos[1]],[pos[0],pos[1]]])
    return moves