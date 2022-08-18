import tensorflow as tf
from tensorflow.keras.layers import Dense, Conv2D, GlobalAveragePooling2D, Flatten, BatchNormalization, Dropout, Concatenate
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

def one_hot_board(b, mp):
    wm = chess.validMoves(b, mp, 1) # white moves
    bm = chess.validMoves(b, mp, -1) # black moves
    piece_board = np.asarray(b)
    piece_board -= 1
    ohb = tf.one_hot(piece_board,14).numpy()
    for i in range(len(wm)):
        ohb[wm[i][1][0]][wm[i][1][1]][12] = 1
    for i in range(len(bm)):
        ohb[bm[i][1][0]][bm[i][1][1]][13] = 1
    return ohb

def define_model_old():
    input = Input(shape=(8,8,14))
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
    input = Input(shape=(8,8,14))
    reg = L2(0.01)

    m0 = Conv2D(64, (1,1), padding='same')(input)
    m1 = Conv2D(64, (3,3), padding='same')(input)
    m2 = Conv2D(64, (5,5), padding='same')(input)
    m3 = Conv2D(64, (7,7), padding='same')(input)
    #m0 = BatchNormalization()(m0)
    #m1 = BatchNormalization()(m1)
    #m2 = BatchNormalization()(m2)
    #m3 = BatchNormalization()(m3)
    m0 = Conv2D(32, (1,1), padding='same', kernel_regularizer=reg, bias_regularizer=reg)(m0)
    m1 = Conv2D(32, (3,3), padding='same', kernel_regularizer=reg, bias_regularizer=reg)(m1)
    m2 = Conv2D(32, (5,5), padding='same', kernel_regularizer=reg, bias_regularizer=reg)(m2)
    m3 = Conv2D(32, (7,7), padding='same', kernel_regularizer=reg, bias_regularizer=reg)(m3)
    m0 = GlobalAveragePooling2D()(m0)
    m1 = GlobalAveragePooling2D()(m1)
    m2 = GlobalAveragePooling2D()(m2)
    m3 = GlobalAveragePooling2D()(m3)

    model = Concatenate(axis=1)([m0,m1,m2,m3])
    model = Dense(128, activation='relu')(model)
    model = Dropout(0.2)(model)
    model = Dense(32, activation='relu')(model)
    model = Dense(1, activation='sigmoid', use_bias=False)(model) # probability white wins
    model = Model(inputs=input, outputs=model)

    model.compile(
        optimizer="adam",
        loss="binary_crossentropy",
        metrics=["binary_accuracy"],
    )

    print(model.summary())

    return model
def load_model():
    return tf.keras.models.load_model("model")

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



