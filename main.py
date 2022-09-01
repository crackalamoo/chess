import numpy as np
import ai
if __name__ == "__main__":
    import chess as py_chess
    import chess.svg
    import threading
    from PyQt5 import QtCore
    from PyQt5.QtSvg import QSvgWidget
    from PyQt5.QtWidgets import QApplication, QMainWindow
from chess_core import *
import sys
import os

movedPieces = []
board = DEFAULT_BOARD
toPlay = 1
players = {1: 2, -1: 1}
FLIP_BOARD = (players[-1] == 0)
DEBUG = False
UNTRAINED_MODEL = False

fifty_move_counter = 50
saved_states = [[DEFAULT_BOARD, []]]
messages = []

if UNTRAINED_MODEL:
    nn_model = ai.define_model()
else:
    nn_model = ai.load_model()

if __name__ == "__main__":
    class ChessWindow(QMainWindow):
        def __init__(self):
            super().__init__()
            self.resize(900, 900)
            self.setWindowTitle("Chess")
            self.chessboard = py_chess.Board()
            self.widgetSvg = QSvgWidget(parent=self)
            self.widgetSvg.setGeometry(10, 10, 880, 880)
            self.paint()
        def paint(self, event=None, move=None, flip=False, checkSquare=None):
            if flip:
                orientation = py_chess.BLACK
            else:
                orientation = py_chess.WHITE
            svg = chess.svg.board(self.chessboard, orientation=orientation, lastmove=move, check=checkSquare).encode("UTF-8")
            self.widgetSvg.load(svg)
            self.widgetSvg.update()
            self.widgetSvg.show()
        def move(self, move, isPromotion, flip, checkSquare):
            files = ['a','b','c','d','e','f','g','h']
            uci = ""
            uci += files[move[0][1]]
            uci += str(8-move[0][0])
            uci += files[move[1][1]]
            uci += str(8-move[1][0])
            if isPromotion:
                uci += {2: 'n', 3: 'b', 4: 'r', 5: 'q'}[move[2]]
            move = py_chess.Move.from_uci(uci)
            self.chessboard.push(move)
            self.paint(move=move, flip=flip, checkSquare=checkSquare)

def addState(b, mp):
    global saved_states
    saved_states.append([b, mp])


def displayBoard(b, turn, flip=False):
    if __name__ == "__main__":
        #os.system('clear')
        print("")
        print("Move " + str(1+int(len(saved_states)/2)))
        """
        if flip:
            for i in range(len(b)-1, -1, -1):
                s = "  " + str(int(8-i)) + " "
                for j in range(len(b[i])-1, -1, -1):
                    s += piece_icons[b[i][j]]
                print(s)
            print("     h  g  f  e  d  c  b  a")
        else:
            for i in range(len(b)):
                s = "  " + str(int(8-i)) + " "
                for j in range(len(b[i])):
                    s += piece_icons[b[i][j]]
                print(s)
            print("     a  b  c  d  e  f  g  h")"""


def makeMove(start, end, promotion=5, isPromotion=False):
    global toPlay
    global board
    global movedPieces
    global playing
    global fifty_move_counter
    if validMove(board, movedPieces, start, end, toPlay):
        if board[start[0]][start[1]] in [1,7] or not board[end[0]][end[1]] == 0:
            fifty_move_counter = 50
        else:
            fifty_move_counter -= 1
        madeMove = afterMove(board, movedPieces, start, end, promotion)
        board = madeMove[0]
        movedPieces = madeMove[1]
        checkSquare = None
        toPlay *= -1
        res = gameRes(saved_states, board, movedPieces, toPlay)
        check = inCheck(board, movedPieces, toPlay)
        if check:
            messages.append("\033[91mCheck\033[0m")
            king = 6
            if toPlay == -1:
                king = 12
            for i in range(8):
                for j in range(8):
                    if board[i][j] == king:
                        checkSquare = py_chess.square(j, 7-i)
            if res == 1:
                messages.append("\033[91m\033[1mCheckmate\033[0m")
                playing = False
        if res == 2:
            messages.append("\033[93m\033[1mStalemate\033[0m")
            playing = False
        if __name__ == "__main__":
            window.move([start, end, promotion], isPromotion, toPlay==-1 and FLIP_BOARD, checkSquare)
        addState(board, movedPieces)
        print("Made move", start, end)
        if (fifty_move_counter <= 0):
            playing = False
            messages.append("\033[93m\033[1mDrawn by fifty-move rule\033[0m")
        elif num_repetitions(board, movedPieces, saved_states) >= 3:
            playing = False
            messages.append("\033[93m\033[1mDrawn by threefold repetition\033[0m")
    else:
        print("Invalid move")
    if DEBUG:
        input("Press enter")
    threading.Thread(target=getNextMove).start()


def inputMove(source):
    print({1: "White", -1: "Black"}[toPlay] + " to play")
    isPromotion = False
    if source == 0:
        myMove = []
        promotion = 5
        while not len(myMove) == 2 or not validMove(board, movedPieces, myMove[0], myMove[1], toPlay):
            inputStr = input("Move: ")
            try:
                myMove = ai.pgn_to_move(inputStr, board, movedPieces, toPlay)
                promotion = myMove[2]
                myMove = [myMove[0], myMove[1]]
            except:
                myMove = []
                print("Error understanding move")
            if not len(myMove) == 2 or not validMove(board, movedPieces, myMove[0], myMove[1], toPlay):
                print("Invalid move")
                if len(inputStr) > 0 and inputStr[0] == '/':
                    try:
                        print(eval(input("Command: ")))
                    except Exception as e:
                        print(str(e))
        if (board[myMove[0][0]][myMove[0][1]] == 1 and myMove[1][0] == 0) or (board[myMove[0][0]][myMove[0][1]] == 7 and myMove[1][0] == 7):
            isPromotion = True
            if inputStr.find("=") == -1:
                promotion = 0
                while not (2 <= promotion <= 5):
                    try:
                        promotion = {"N": 2, "B": 3, "R": 4, "Q": 5}[input("N/B/R/Q: ").upper()]
                    except KeyError:
                        print("Invalid promotion")
        makeMove(myMove[0], myMove[1], promotion, isPromotion)
    if source == 1:
        print("Thinking...")
        myMove = ai.minimax_ai(board, movedPieces, saved_states, toPlay)
        if (board[myMove[0][0]][myMove[0][1]] == 1 and myMove[1][0] == 0) or (board[myMove[0][0]][myMove[0][1]] == 7 and myMove[1][0] == 7):
            isPromotion = True
        makeMove(myMove[0], myMove[1], myMove[2], isPromotion)
    if source == 2:
        print("Thinking...")
        myMove = ai.nn_ai(nn_model, board, movedPieces, saved_states, toPlay)
        if (board[myMove[0][0]][myMove[0][1]] == 1 and myMove[1][0] == 0) or (board[myMove[0][0]][myMove[0][1]] == 7 and myMove[1][0] == 7):
            isPromotion = True
        makeMove(myMove[0], myMove[1], myMove[2], isPromotion)
    if source == 3:
        print("Thinking...")
        myMove = ai.policy_ai(nn_model, board, movedPieces, saved_states, toPlay)
        if (board[myMove[0][0]][myMove[0][1]] == 1 and myMove[1][0] == 0) or (board[myMove[0][0]][myMove[0][1]] == 7 and myMove[1][0] == 7):
            isPromotion = True
        makeMove(myMove[0], myMove[1], myMove[2], isPromotion)
    if source == 4:
        print("Thinking...")
        myMove = ai.hybrid_ai(nn_model, board, movedPieces, saved_states, toPlay)
        if (board[myMove[0][0]][myMove[0][1]] == 1 and myMove[1][0] == 0) or (board[myMove[0][0]][myMove[0][1]] == 7 and myMove[1][0] == 7):
            isPromotion = True
        makeMove(myMove[0], myMove[1], myMove[2], isPromotion)
def tupleMove(m):
    if not len(m) == 2 or not len(m[0]) == 2 or not len(m[1]) == 2:
        return ()
    try:
        start = (8-int(m[0][1]), {'a':0,'b':1,'c':2,'d':3,'e':4,'f':5,'g':6,'h':7}[m[0][0]])
        end = (8-int(m[1][1]), {'a':0,'b':1,'c':2,'d':3,'e':4,'f':5,'g':6,'h':7}[m[1][0]])
    except (ValueError, KeyError) as e:
        return ()
    return (start, end)

                
def vm(start, end):
    return validMove(board, movedPieces, start, end, toPlay)
def sm():
    showMoves(board, movedPieces, toPlay)
    print("(opponent)")
    showMoves(board, movedPieces, toPlay*-1)

playing = True
def getNextMove():
    global messages
    global playing
    global toPlay

    print("Playing", toPlay)

    if playing:
        inputMove(players[toPlay])
        displayBoard(board, toPlay, {1: False, -1: True and FLIP_BOARD}[toPlay])
        for i in range(len(messages)):
            print(messages[i])
        messages = []
    if not playing:
        inputMove(0)

if __name__ == "__main__":
    os.system('clear')
    displayBoard(board, toPlay)
    app = QApplication([])
    window = ChessWindow()
    threading.Thread(target=getNextMove).start()
    QtCore.QCoreApplication.processEvents()
    window.show()
    sys.exit(app.exec())
