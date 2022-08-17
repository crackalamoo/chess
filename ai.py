import tensorflow as tf
from tensorflow.keras.layers import Dense, Conv2D, AveragePooling2D, Flatten, BatchNormalization, Dropout, Concatenate
from tensorflow.keras.regularizers import L2
from tensorflow.keras import Model, Input
import numpy as np
import chess
import logging, os
logging.disable(logging.WARNING)
os.environ["TF_CPP_MIN_LOG_LEVEL"] = "2"

def minimax_ai(b, mp, turn):
    #return chess.minimax(b, mp, turn, 2, {-1: 7500, 1: 7500}[turn], True)
    return chess.minimax(b, mp, turn, 2, 7500, True)

def int_board(b):
    ib = np.zeros((8,8))
    for i in range(8):
        for j in range(8):
            ib[i][j] = int.from_bytes(b[i][j], 'little')
    return ib

def one_hot_board(b):
    ohb = b
    #ohb = b.flatten()
    """if black:
        ohb = ohb[::-1][:]
        ohb[ohb > 6] += 50
        ohb[(0 < ohb) & (ohb < 7)] += 6
        ohb[ohb > 20] -= 56"""
    ohb -= 1
    return tf.one_hot(ohb,12)

def define_model_old():
    input = Input(shape=(8,8,12))
    reg = L2(0.01)
    """
    h0 = Flatten()(input)
    h0 = Dense(256, activation='relu')(h0)
    h1 = Flatten()(inputOpponent)
    h1 = Dense(256, activation='relu')(h1)
    model = Concatenate(axis=1)([h0,h1])
    """

    model = Flatten()(input)
    model = Dense(1024, activation='relu')(model)
    model = Dropout(0.2)(model)
    model = Dense(1024, activation='relu', kernel_regularizer=reg, bias_regularizer=reg)(model)
    model = Dropout(0.2)(model)
    model = Dense(32, activation='relu', kernel_regularizer=reg, bias_regularizer=reg)(model)
    model = Dense(1, activation='sigmoid')(model) # probability white wins
    model = Model(inputs=input, outputs=model)

    model.compile(
        optimizer="adam",
        loss="binary_crossentropy",
        metrics=["binary_accuracy"],
    )

    return model

def define_model():
    input = Input(shape=(8,8,12))
    reg = L2(0.01)

    m0 = Conv2D(64, (1,1), activation='relu', padding='same')(input)
    m1 = Conv2D(64, (3,3), activation='relu', padding='same')(input)
    m2 = Conv2D(64, (5,5), activation='relu', padding='same')(input)
    m3 = Conv2D(64, (7,7), activation='relu', padding='same')(input)
    m0 = BatchNormalization()(m0)
    m1 = BatchNormalization()(m1)
    m2 = BatchNormalization()(m2)
    m3 = BatchNormalization()(m3)
    m0 = Conv2D(64, (1,1), activation='relu', padding='same')(m0)
    m1 = Conv2D(64, (3,3), activation='relu', padding='same')(m1)
    m2 = Conv2D(64, (5,5), activation='relu', padding='same')(m2)
    m3 = Conv2D(64, (7,7), activation='relu', padding='same')(m3)
    print(m0.shape)
    m0 = AveragePooling2D((8,8))(m0)
    m1 = AveragePooling2D((8,8))(m1)
    m2 = AveragePooling2D((8,8))(m2)
    m3 = AveragePooling2D((8,8))(m3)
    print(m0.shape)
    m0 = Flatten()(m0)
    m1 = Flatten()(m1)
    m2 = Flatten()(m2)
    m3 = Flatten()(m3)
    print(m0.shape)

    model = Concatenate(axis=1)([m0,m1,m2,m3])
    print(model.shape)
    model = Dense(256, activation='relu')(model)
    model = Dense(32, activation='relu')(model)
    model = Dense(1, activation='sigmoid')(model) # probability white wins
    model = Model(inputs=input, outputs=model)

    model.compile(
        optimizer="adam",
        loss="binary_crossentropy",
        metrics=["binary_accuracy"],
    )

    print(model.summary())

    return model

def pgn_to_move(pgn, b, mp, turn):
    # possible move formats:
    # e4, exd5, Nc3, Nxe4, e8=Q, exd8=Q, Nexd4, O-O, O-O-O, Qd5+
    possibleStarts = []
    end = [-1,-1]
    promotion = 5
    pgn = pgn.replace("x","").replace("+","").replace("#","")
    piece = 1
    ib = int_board(b)
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
def move_pgn(pgn, b, mp, turn):
    move = pgn_to_move(pgn, b, mp, turn)
    newBoard = chess.afterMove(b, mp, move[0], move[1], move[2])
    return newBoard
def debugBoard(b):
    for i in range(8):
        s = ""
        for j in range(8):
            s += chess.piece_icons[int(b[i][j])]
        print(s)

def movePrint(pgn, sb, turn):
    sb = move_pgn(pgn, sb[0], sb[1], turn)
    print(int_board(sb[0]), sb[1])





