import tensorflow as tf
from tensorflow.keras.layers import Dense, Conv2D, GlobalAveragePooling2D, Flatten, BatchNormalization, Dropout, Concatenate
from tensorflow.keras.regularizers import L2
from tensorflow.keras import Model, Input
import numpy as np
import chess_core as chess
import logging, os
import time
logging.disable(logging.WARNING)
os.environ["TF_CPP_MIN_LOG_LEVEL"] = "2"

T_PLY = 4 # number of ply to analyze

def minimax_ai(b, mp, turn):
    #return chess.minimax(b, mp, turn, 2, {-1: 7500, 1: 7500}[turn], True)
    return chess.minimax(b, mp, turn, 2, 7500, True)

def nn_ai(model, b, mp, states, turn):
    root = monteCarlo(model, b, mp, states, turn, 20, 7.5)
    index = root.select_child()[0]
    m = nn_to_move(index)
    return [m[0], m[1], 5]

def nn_result(model, b, mp, states, turn):
    inp = ai_input(turn, states)
    res = model.predict([np.expand_dims(inp[0],axis=0), np.expand_dims(inp[1],axis=0)])
    policy = res[0].reshape(res[0].shape[1])
    value = res[1].reshape(res[1].shape[1])
    for i in range(len(policy)):
        m = nn_to_move(i)
        if not chess.validMove(b, mp, m[0], m[1], turn):
            policy[i] = 0
    if np.sum(policy) == 0:
        if chess.inCheck(b, mp, turn):
            value = -1
        else:
            value = 0
    else:
        policy /= np.sum(policy)
    return policy, value

def ucb(parent, child):
    prior_score = child.prior * np.sqrt(parent.visits)/(child.visits+1)
    value_score = -child.value()
    return value_score + prior_score
class Node:
    def __init__(self, prior, turn):
        self.visits = 0
        self.turn = turn
        self.prior = prior
        self.value_sum = 0
        self.children = {}
        self.state = None
    def expanded(self):
        return (len(self.children) > 0)
    def value(self):
        if self.visits == 0:
            return 0
        return self.value_sum / self.visits
    def select_action(self, temperature):
        visit_counts = np.array([child.visit_count for child in self.children.values()])
        actions = [action for action in self.children.keys()]
        if temperature == 0:
            action = actions[np.argmax(visit_counts)]
        elif temperature == float("inf"):
            action = np.random.choice(actions)
        else:
            visit_counts = visit_counts ** (1/temperature)
            visit_counts /= sum(visit_counts)
            action = np.random.choice(actions, p=visit_counts)
        return action
    def select_child(self):
        best = -np.inf
        index = -1
        pick = None
        for action, child in self.children.items():
            score = ucb(self, child)
            if score > best:
                best = score
                index = action
                pick = child
        return index, pick
    def expand(self, state, turn, priors):
        self.turn = turn
        self.state = state
        for a, p in enumerate(priors):
            if not p == 0:
                self.children[a] = Node(p, turn*-1)

def monteCarlo(model, b, mp, states, turn, sims, timeLimit):
    startTime = time.time()
    root = Node(0, turn)
    priors, value = nn_result(model, b, mp, states, turn)
    root.expand([b,mp], turn, priors)
    for i in range(sims):
        node = root
        path = [node]
        while node.expanded() and len(path) < 10 and time.time() - startTime < timeLimit:
            action, node = node.select_child()
            path.append(node)
        parent = path[-2]
        state = parent.state
        m = nn_to_move(action)
        nextState = chess.afterMove(state[0], state[1], m[0], m[1])
        states.append(nextState)
        outcome = chess.gameRes(nextState[0], nextState[1], parent.turn*-1)
        # value means chance of win for the player whose move it is at nextState
        priors, value = nn_result(model, nextState[0], nextState[1], states, parent.turn*-1)
        states.pop(len(states)-1)
        if outcome == 0 and time.time() - startTime < timeLimit:
            node.expand(nextState, parent.turn*-1, priors)
        for node in reversed(path):
            node.value_sum += value*node.turn*turn
            node.visits += 1
        if time.time() - startTime >= timeLimit:
            break
    return root


    

def one_hot_board(b, turn):
    piece_board = np.asarray(b)
    if turn == -1:
        piece_board[piece_board == 0] -= 20
        piece_board[piece_board > 6] += 20
        piece_board[piece_board < 7] += 6
        piece_board[piece_board > 12] -= 26
    piece_board -= 1
    ohb = tf.one_hot(piece_board,12).numpy()
    return ohb

def ai_input(turn, states):
    inStates = []
    for i in range(len(states)):
        inStates.append(states[i])
    while len(inStates) > T_PLY:
        inStates.pop(0)
    while len(inStates) < T_PLY:
        inStates.insert(0, [chess.DEFAULT_BOARD, []])
    input = one_hot_board(inStates[0][0], turn)
    for i in range(1, len(inStates)):
        nextBoard = one_hot_board(inStates[i][0], turn)
        input = np.concatenate((input, nextBoard), axis=2)
    input = input.astype(int)
    currState = inStates[len(inStates)-1]
    input2 = np.asarray([turn, chess.num_repetitions(currState[0], currState[1], states)]).astype(int)
    return [input, input2]

def move_to_nn(start, end):
    nn = (start[0]*8+start[1])*(56+8) # 56 queen moves and 8 knight moves from each square
    nn += end[0]*8+end[1]
    return tf.one_hot(nn, 4096)
def nn_to_move(nn):
    e1 = nn%8
    nn -= e1
    e0 = nn%64
    nn -= e0
    s1 = nn%512
    nn -= s1
    s0 = nn%4096
    nn -= s0
    return [[int(s0/512),int(s1/64)], [int(e0/8),int(e1)], 5]

def define_model():
    input = Input(shape=(8,8,12*T_PLY))
    input2 = Input(shape=(2,))
    reg = L2(0.01)

    m0 = Conv2D(64, (1,1), padding='same')(input)
    m1 = Conv2D(64, (3,3), padding='same')(input)
    m2 = Conv2D(64, (5,5), padding='same')(input)
    m3 = Conv2D(64, (7,7), padding='same')(input)
    m0 = Conv2D(64, (1,1), padding='same', kernel_regularizer=reg, bias_regularizer=reg, activation='relu')(m0)
    m1 = Conv2D(64, (3,3), padding='same', kernel_regularizer=reg, bias_regularizer=reg, activation='relu')(m1)
    m2 = Conv2D(64, (5,5), padding='same', kernel_regularizer=reg, bias_regularizer=reg, activation='relu')(m2)
    m3 = Conv2D(64, (7,7), padding='same', kernel_regularizer=reg, bias_regularizer=reg, activation='relu')(m3)
    m0 = GlobalAveragePooling2D()(m0)
    m1 = GlobalAveragePooling2D()(m1)
    m2 = GlobalAveragePooling2D()(m2)
    m3 = GlobalAveragePooling2D()(m3)

    hidden = Concatenate(axis=1)([m0,m1,m2,m3,input2])
    policy = Dense(4096, activation='softmax')(hidden)
    value = Dense(32, activation='relu')(hidden)
    value = Dense(1, activation='tanh')(value)
    model = Model(inputs=[input, input2], outputs=[policy, value])

    model.compile(
        optimizer="adam",
        loss=["categorical_crossentropy", "mse"],
        metrics=["accuracy"],
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
