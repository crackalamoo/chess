from flask import Flask
from chess_core import *
import ai

app = Flask(__name__)

from flask import jsonify, request, render_template

def arg_to_square(arg):
    s = [int(arg[0]), int(arg[1])]
    return s
def arg_to_board(arg):
    arg = arg.split("_")
    b = []
    for i in range(8):
        b.append([])
        for j in range(8):
            b[i].append(int(arg[i*8+j]))
    return b
def arg_to_moved(arg):
    arg = arg.split("_")
    mp = []
    for i in range(len(arg)):
        if not arg[i] == '':
            mp.append(arg_to_square(arg[i]))
    return mp
def arg_to_states(arg):
    arg = arg.split("S")
    states = []
    for i in range(len(arg)):
        if arg[i] != '':
            state = arg[i].split("s")
            states.append([arg_to_board(state[0]), arg_to_moved(state[1])])
    return states

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/minimax')
def js_minimax():
    b = arg_to_board(request.args.get('board'))
    mp = arg_to_moved(request.args.get('moved'))
    states = arg_to_states(request.args.get('states'))
    turn = int(request.args.get('turn'))
    time = int(request.args.get('time'))
    time = int(time/max(15.0, 20.0-len(states)/2.0))
    time = int(time*2)
    if len(states) <= 6:
        time = min(time, 6000)
    if len(states) <= 4:
        time = min(time, 4500)
    if 20 <= len(states) <= 46:
        time = int(time*2)
    res = ai.minimax_ai(b, mp, states, turn, time)
    res = jsonify({"start": res[0], "end": res[1], "promotion": res[2]})
    return res

@app.route('/validmoves')
def js_validMoves():
    b = arg_to_board(request.args.get('board'))
    mp = arg_to_moved(request.args.get('moved'))
    turn = int(request.args.get('turn'))
    return jsonify(validMoves(b, mp, turn))

@app.route('/makemove')
def js_makeMove():
    b = arg_to_board(request.args.get('board'))
    mp = arg_to_moved(request.args.get('moved'))
    start = arg_to_square(request.args.get('start'))
    end = arg_to_square(request.args.get('end'))
    turn = int(request.args.get('turn'))
    states = arg_to_states(request.args.get('states'))
    promotion = int(request.args.get('promotion'))
    fifty_count = int(request.args.get('fifty_count'))
    messages = []
    if validMove(b, mp, start, end, turn):
        if b[start[0]][start[1]] in [1,7] or not b[end[0]][end[1]] == 0:
            fifty_count = 50
        else:
            fifty_count -= 1
        madeMove = afterMove(b, mp, start, end, promotion)
        states.append([madeMove[0],madeMove[1]])
        checkSquare = [-1,-1]
        turn *= -1
        res = gameRes(states, madeMove[0], madeMove[1], turn)
        check = inCheck(madeMove[0], madeMove[1], turn)
        if check:
            messages.append("Check")
            king = 6
            if turn == -1:
                king = 12
            for i in range(8):
                for j in range(8):
                    if b[i][j] == king:
                        checkSquare = [i,j]
            if res == 1:
                messages.append("Checkmate")
        if res == 2:
            messages.append("Stalemate")
        if (fifty_count <= 0):
            messages.append("Drawn by fifty-move rule")
            res = 2
        elif num_repetitions(madeMove[0], madeMove[1], states) >= 3:
            messages.append("Drawn by threefold repetition")
            res = 2
        return jsonify({"board": madeMove[0], "moved": madeMove[1], "res": res, "check": checkSquare, "fifty": fifty_count, "messages": messages})
    else:
        return jsonify({"res": -2})

if __name__ == '__main__':
    app.run(debug=True)
