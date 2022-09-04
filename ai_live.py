import chess_core as chess

T_PLY = 4 # number of ply to analyze
 
def minimax_ai(b, mp, states, turn, time=7500):
    return chess.minimax(states, b, mp, turn, 2, time, True)

def pgn_to_move(pgn, b, mp, turn):
    # possible move formats:
    # e4, exd5, Nc3, Nxe4, e8=Q, exd8=Q, Nexd4, O-O, O-O-O, Qd5+
    possibleStarts = []
    end = [-1,-1]
    promotion = 5
    pgn = pgn.replace("x","").replace("+","").replace("#","")
    piece = 1
    ib = b
    try:
        piece = {"N": 2, "B": 3, "R": 4, "Q": 5, "K": 6}[pgn[0]]
        pgn = pgn[1:]
    except KeyError:
        if pgn[0] == "O":
            row = 0
            if turn == 1:
                row = 7
            if pgn == "O-O":
                return [[row,4], [row,6], 5]
            else:
                return [[row,4], [row,2], 5]
        # otherwise, must be a pawn (1)
    if not pgn.find("=") == -1:
        promotion = {"N": 2, "B": 3, "R": 4, "Q": 5}[pgn[pgn.find("=")+1]]
        pgn = pgn[:-2]
    if turn == -1:
        piece += 6
    end = [8-int(pgn[-1]), {'a':0,'b':1,'c':2,'d':3,'e':4,'f':5,'g':6,'h':7}[pgn[-2]]]
    for i in range(8):
        for j in range(8):
            if ib[i][j] == piece and chess.validMove(b, mp, [i,j], end, turn):
                possibleStarts.append([i,j])
    if len(possibleStarts) == 1:
        start = possibleStarts[0]
    else:
        try:
            file = {'a':0,'b':1,'c':2,'d':3,'e':4,'f':5,'g':6,'h':7}[pgn[0]]
            for i in range(len(possibleStarts)):
                if possibleStarts[i][1] == file:
                    start = possibleStarts[i]
                    break
        except KeyError:
            rank = 8-int(pgn[0])
            for i in range(len(possibleStarts)):
                if possibleStarts[i][0] == rank:
                    start = possibleStarts[i]
                    break
    move = [start, end, promotion]
    return move

def debugBoard(b):
    for i in range(8):
        s = ""
        for j in range(8):
            s += chess.piece_icons[int(b[i][j])]
        print(s)
