import numpy as np
import ai
from chess import *
import os

piece_icons = ['[ ]', u'[\N{WHITE CHESS PAWN}]', u'[\N{WHITE CHESS KNIGHT}]', u'[\N{WHITE CHESS BISHOP}]', u'[\N{WHITE CHESS ROOK}]', u'[\N{WHITE CHESS QUEEN}]', u'[\N{WHITE CHESS KING}]',
    u'[\N{BLACK CHESS PAWN}]', u'[\N{BLACK CHESS KNIGHT}]', u'[\N{BLACK CHESS BISHOP}]', u'[\N{BLACK CHESS ROOK}]', u'[\N{BLACK CHESS QUEEN}]', u'[\N{BLACK CHESS KING}]']
movedPieces = []
board = np.asarray(DEFAULT_BOARD)
FLIP_BOARD = True
toPlay = 1

def displayBoard(b, turn, flip=False):
    os.system('clear')
    print("")
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
        print("     a  b  c  d  e  f  g  h")
    if inCheck(board, movedPieces, turn):
        print("Check")


def makeMove(start, end):
    global toPlay
    global board
    global movedPieces
    madeMove = afterMove(board, movedPieces, start, end)
    board = madeMove[0]
    movedPieces = madeMove[1]
    toPlay = {1: -1, -1: 1}[toPlay]


def inputMove(source):
    if source == 0:
        myMove = []
        while not len(myMove) == 2 or not validMove(board, movedPieces, myMove[0], myMove[1], toPlay):
            myMove = tupleMove(input("Move: ").split(" "))
            if not len(myMove) == 2 or not validMove(board, movedPieces, myMove[0], myMove[1], toPlay):
                displayBoard(board, toPlay, {1: False, -1: True and FLIP_BOARD}[toPlay])
                print("Invalid move")
        makeMove(myMove[0], myMove[1])
def tupleMove(m):
    if not len(m) == 2 or not len(m[0]) == 2 or not len(m[1]) == 2:
        return ()
    try:
        start = (8-int(m[0][1]), {'a':0,'b':1,'c':2,'d':3,'e':4,'f':5,'g':6,'h':7}[m[0][0]])
        end = (8-int(m[1][1]), {'a':0,'b':1,'c':2,'d':3,'e':4,'f':5,'g':6,'h':7}[m[1][0]])
    except (ValueError, KeyError) as e:
        return ()
    return (start, end)

                




os.system('clear')
displayBoard(board, toPlay)

for i in range(30):
    inputMove(0)
    displayBoard(board, toPlay, {1: False, -1: True and FLIP_BOARD}[toPlay])