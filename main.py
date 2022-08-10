import numpy as np
import ai
from chess import *
import os

piece_icons = ['[ ]', u'[\N{WHITE CHESS PAWN}]', u'[\N{WHITE CHESS KNIGHT}]', u'[\N{WHITE CHESS BISHOP}]', u'[\N{WHITE CHESS ROOK}]', u'[\N{WHITE CHESS QUEEN}]', u'[\N{WHITE CHESS KING}]',
    u'[\N{BLACK CHESS PAWN}]', u'[\N{BLACK CHESS KNIGHT}]', u'[\N{BLACK CHESS BISHOP}]', u'[\N{BLACK CHESS ROOK}]', u'[\N{BLACK CHESS QUEEN}]', u'[\N{BLACK CHESS KING}]']
movedPieces = []
board = DEFAULT_BOARD
toPlay = 1
players = {1: 1, -1: 1}
FLIP_BOARD = (players[1] == 0 and players[-1] == 0)

def displayBoard(b, turn, flip=False):
    os.system('clear')
    print("")
    if flip:
        for i in range(len(b)-1, -1, -1):
            s = "  " + str(int(8-i)) + " "
            for j in range(len(b[i])-1, -1, -1):
                s += piece_icons[ord(b[i][j])]
            print(s)
        print("     h  g  f  e  d  c  b  a")
    else:
        for i in range(len(b)):
            s = "  " + str(int(8-i)) + " "
            for j in range(len(b[i])):
                s += piece_icons[ord(b[i][j])]
            print(s)
        print("     a  b  c  d  e  f  g  h")
    res = gameRes(board, movedPieces, turn)
    check = inCheck(board, movedPieces, turn)
    if check:
        print("Check")
        if res == 1:
            print("Checkmate")
    if res == 2:
        print("Stalemate")
    return res


def makeMove(start, end, promotion=5):
    global toPlay
    global board
    global movedPieces
    if validMove(board, movedPieces, start, end, toPlay):
        madeMove = afterMove(board, movedPieces, start, end, promotion)
        board = madeMove[0]
        movedPieces = madeMove[1]
        toPlay *= -1
        print("Made move", start, end)
    else:
        print("Invalid move")


def inputMove(source):
    print({1: "White", -1: "Black"}[toPlay] + " to play")
    if source == 0:
        myMove = []
        while not len(myMove) == 2 or not validMove(board, movedPieces, myMove[0], myMove[1], toPlay):
            inputStr = input("Move: ")
            myMove = tupleMove(inputStr.split(" "))
            if not len(myMove) == 2 or not validMove(board, movedPieces, myMove[0], myMove[1], toPlay):
                displayBoard(board, toPlay, {1: False, -1: True and FLIP_BOARD}[toPlay])
                print("Invalid move")
                if len(inputStr) > 0 and inputStr[0] == '/':
                    print(eval(input("Command: ")))
        promotion = 5
        if (board[myMove[0][0]][myMove[0][1]] == 1 and myMove[1][0] == 0) or (board[myMove[0][0]][myMove[0][1]] == 7 and myMove[1][0] == 7):
            promotion = 0
            while not (2 <= promotion <= 5):
                try:
                    promotion = {"N": 2, "B": 3, "R": 4, "Q": 5}[input("N/B/R/Q: ").upper()]
                except KeyError:
                    print("Invalid promotion")
        makeMove(myMove[0], myMove[1], promotion)
    if source == 1:
        print("Thinking...")
        myMove = minimax(board, movedPieces, toPlay, 3, 7500)
        makeMove(myMove[0], myMove[1], myMove[2])
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

os.system('clear')
displayBoard(board, toPlay)
playing = True

while playing:
    inputMove(players[toPlay])
    r = displayBoard(board, toPlay, {1: False, -1: True and FLIP_BOARD}[toPlay])
    if not r == 0:
        playing = False