import numpy as np
import chess
import tensorflow as tf
from ai import one_hot_board, pgn_to_move

NUM_GAMES = 20000

train_X = []
train_y = []


def move_pgn(pgn, b, mp, turn):
    move = pgn_to_move(pgn, b, mp, turn)
    newBoard = chess.afterMove(b, mp, move[0], move[1], move[2])
    return newBoard

def playGame(game, white, black, termination):
    turn = 1
    sb = [chess.DEFAULT_BOARD, []]
    if not termination == "[Termination \"Normal\"]":
        return
    white = white[8:-2]
    black = black[8:-2]
    game = game.split(" ")
    res = game.pop(len(game)-1)
    res = {'1-0': 1, '1/2-1/2': 0.5, '0-1': 0}[res]
    for i in range(len(game)-1, -1, -1):
        if i%3 == 0:
            game.pop(i)
    for i in range(len(game)):
        sb = move_pgn(game[i], sb[0], sb[1], turn)
        turn *= -1
        train_X.append(one_hot_board(sb[0], sb[1]))
        train_y.append(res)
def playGames(num):
    print("Playing through games")
    data = open('data/lichess_arifd2.pgn').read().split('\n')
    n = 0
    for l in range(len(data)):
        if data[l].startswith('1'):
            playGame(data[l], data[l-14], data[l-13], data[l-2])
            n += 1
            print(str(n)+"/"+str(num), end='\r')
        if n >= num:
            break
    print("Played through " + str(n) + " games (" + str(len(train_y)) + " positions)")

def movePrint(pgn, sb, turn):
    sb = move_pgn(pgn, sb[0], sb[1], turn)
    print(sb[0], sb[1])

playGames(NUM_GAMES)

train_X = np.asarray(train_X)
train_y = np.asarray(train_y)
print(train_X.shape)
print(train_y.shape)

np.savez_compressed("data/data", X=train_X, y=train_y)