import numpy as np
import chess_core as chess
import tensorflow as tf
from ai import pgn_to_move, ai_input, move_to_nn

START_GAME = 20000*7
END_GAME = 20000*8

train_X0 = [] # board states
train_y = [] # policy
FILENAME = 'data/RecentGames.pgn'
SAVETO = 'data/gm8'#.npz

def move_pgn(pgn, b, mp, turn):
    move = pgn_to_move(pgn, b, mp, turn)
    newBoard = chess.afterMove(b, mp, move[0], move[1], move[2])
    return newBoard

def playGame(game, termination):
    turn = 1
    sb = [chess.DEFAULT_BOARD, []] # simulated board
    if not termination == "[Termination \"Normal\"]":
        return 0
    game = game.replace('\n', '').replace(". ", ".").replace(".", ". ")
    game = game.split(" ")
    res = game.pop(len(game)-1)
    res = {'1-0': 1, '1/2-1/2': 0, '0-1': -1}[res]
    if res == 0:
        return 0
    for i in range(len(game)-1, -1, -1):
        if i%3 == 0:
            game.pop(i)
    gameStates = []
    for i in range(len(game)):
        move = pgn_to_move(game[i], sb[0], sb[1], turn)
        sb = chess.afterMove(sb[0], sb[1], move[0], move[1], move[2])
        gameStates.append(sb)
        inp = ai_input(turn, gameStates)
        if res == turn: # only learn from the winner
            train_X0.append(inp[0])
            train_y.append(move_to_nn(move[0], move[1], turn))
        turn *= -1
    return 1
def playGames(start, num):
    print("Loading " + FILENAME)
    data = open(FILENAME).read().replace('\r','\n').split('\n')
    print("Playing through games")
    num = min(num, int((len(data)-start)/20))
    n = 0
    played = 0
    for l in range(len(data)):
        if n >= num:
            break
        if data[l].startswith('1.'):
            if n >= start:
                game = data[l]
                i = 1
                while l+i < len(data) and not data[l+i].startswith("[Event"):
                    game += data[l+i]
                    i += 1
                #playGame(game, data[l-2])
                played += playGame(game, "[Termination \"Normal\"]")
            n += 1
            print("  "+str(n-start)+"/"+str(num-start) + " ("+str(int((n-start)/(num-start)*100))+"%) ", end='\r')
    print("Played through " + str(played) + " games (" + str(len(train_y)) + " positions)")

def movePrint(pgn, sb, turn):
    sb = move_pgn(pgn, sb[0], sb[1], turn)
    print(sb[0], sb[1])

playGames(START_GAME, END_GAME)

print("Saving to " + SAVETO + ".npz")
print("Saving data...")
train_X0 = np.asarray(train_X0)
train_y = np.asarray(train_y)
print(train_X0.shape)
print(train_y.shape)

np.savez_compressed(SAVETO, X=train_X0, y=train_y)
print("Saved data")