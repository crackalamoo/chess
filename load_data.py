import numpy as np
import chess_core as chess
import tensorflow as tf
from ai import pgn_to_move, ai_input, move_to_nn

NUM_GAMES = 20000

train_X0 = [] # board states
train_X1 = [] # extra info (threefold repetition)
train_y0 = [] # policy
train_y1 = [] # win probability


def move_pgn(pgn, b, mp, turn):
    move = pgn_to_move(pgn, b, mp, turn)
    newBoard = chess.afterMove(b, mp, move[0], move[1], move[2])
    return newBoard

def playGame(game, white, black, termination):
    turn = 1
    sb = [chess.DEFAULT_BOARD, []] # simulated board
    if not termination == "[Termination \"Normal\"]":
        return
    white = white[8:-2]
    black = black[8:-2]
    game = game.split(" ")
    res = game.pop(len(game)-1)
    res = {'1-0': 1, '1/2-1/2': 0, '0-1': -1}[res]
    for i in range(len(game)-1, -1, -1):
        if i%3 == 0:
            game.pop(i)
    gameStates = []
    for i in range(len(game)):
        move = pgn_to_move(game[i], sb[0], sb[1], turn)
        sb = chess.afterMove(sb[0], sb[1], move[0], move[1], move[2])
        gameStates.append(sb)
        inp = ai_input(turn, gameStates)
        train_X0.append(inp[0])
        train_X1.append(inp[1])
        train_y0.append(move_to_nn(move[0], move[1]))
        train_y1.append(res*turn) # win for this player
        turn *= -1
def playGames(num):
    print("Playing through games")
    data = open('data/lichess_arifd2.pgn').read().split('\n')
    num = min(num, int(len(data)/20))
    n = 0
    for l in range(len(data)):
        if data[l].startswith('1'):
            playGame(data[l], data[l-14], data[l-13], data[l-2])
            n += 1
            print(str(n)+"/"+str(num) + " ("+str(int(n/num*100))+"%)", end='\r')
        if n >= num:
            break
    print("Played through " + str(n) + " games (" + str(len(train_y0)) + " positions)")

def movePrint(pgn, sb, turn):
    sb = move_pgn(pgn, sb[0], sb[1], turn)
    print(sb[0], sb[1])

playGames(NUM_GAMES)

print("Saving data...")
train_X0 = np.asarray(train_X0)
train_X1 = np.asarray(train_X1)
train_y0 = np.asarray(train_y0)
train_y1 = np.asarray(train_y1)
print(train_X0.shape)
print(train_X1.shape)
print(train_y0.shape)
print(train_y1.shape)

np.savez_compressed("data/data", X0=train_X0, X1=train_X1, y0=train_y0, y1=train_y1)
print("Saved data")