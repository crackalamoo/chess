import ctypes

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
def minimax(b, mp, turn, depth, time):
    cpp_setCalc(time)
    state = cppState(b, mp)
    res = cpp_minimax(state, ctypes.c_int(turn), ctypes.c_int(depth))
    return ((int((res%10000)/1000),int((res%1000)/100)), (int((res%100)/10), int(res%10)), int(res/10000))
def showMoves(b, mp, turn):
    state = cppState(b, mp)
    cpp_showMoves(state, ctypes.c_int(turn))