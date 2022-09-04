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
 
def minimax_ai(b, mp, states, turn, time=7500):
    return chess.minimax(states, b, mp, turn, 2, time, True)

def nn_ai(model, b, mp, states, turn, time=5.0):
    child = monteCarlo(model, b, mp, turn, states, time)
    index = child.move
    m = nn_to_move(index, turn)
    return [m[0], m[1], 5]

def policy_ai(model, b, mp, states, turn):
    scores = nn_policy(model, b, mp, states, turn)
    print(scores[np.argmax(scores)])
    return nn_to_move(np.argmax(scores), turn)

def hybrid_ai(model, b, mp, states, turn, time=5.0):
    child = hybridSearch(model, b, mp, turn, states, time)
    index = child.move
    m = nn_to_move(index, turn)
    return [m[0], m[1], 5]

def nn_policy(model, b, mp, states, turn):
    inp = ai_input(turn, states)
    inp = np.expand_dims(inp[0], axis=0)
    policy = model.predict(inp)[0]
    for i in range(len(policy)):
        m = nn_to_move(i, turn)
        if not chess.validMove(b, mp, m[0], m[1], turn):
            policy[i] = 0
    if not np.sum(policy) == 0:
        policy /= np.sum(policy)
    return policy

class Node:
    def __init__(self, m, p):
        self.move = m
        self.parent = p
        self.children = {}
        self.val_sum = 0
        self.visits = 0
        self.is_leaf = True
        self.worstVal = 20000
        self.res = 0
    def expand(self, b, mp, turn):
        vm = chess.validMoves(b, mp, turn)
        for i in range(len(vm)):
            m = move_to_nn(vm[i][0], vm[i][1], turn, False)
            nc = Node(m, self)
            self.children[m] = nc
        if not len(vm) == 0:
            self.is_leaf = False
    def update(self, val):
        self.val_sum += val
        self.worstVal = min(self.worstVal, val)
        self.visits += 1
    def has_parent(self):
        return self.parent is not None
    def evaluate(self):
        if self.visits == 0:
            return 0
        return self.visits+self.val_sum/self.visits+self.worstVal
    def hybrid_update(self, res):
        self.res = res
    def hybrid_evaluate(self, state, turn):
        if self.is_leaf and self.res == 0:
            return turn*chess.evalState(state[0], state[1])
        if not self.res == 0:
            if self.res == 1:
                return -20000
            return 0
        val = -20000
        for child in self.children.values():
            m = nn_to_move(child.move, turn)
            val = max(val, -child.hybrid_evaluate(chess.afterMove(state[0], state[1], m[0], m[1]), turn*-1))
        return val
    def visit_child(self, model, turn, states):
        lastState = states[len(states)-1]
        uct = nn_policy(model, lastState[0], lastState[1], states, turn)
        uct[uct == 0] -= 10
        for key in self.children.keys():
            child = self.children[key]
            uct[key] *= np.sqrt(2*self.visits)/(child.visits+1)
            if not child.visits == 0:
                uct[key] += child.val_sum/child.visits
        visit = np.argmax(uct)
        return self.children[visit]
    def hybrid_visit(self, model, turn, states):
        lastState = states[len(states)-1]
        uct = nn_policy(model, lastState[0], lastState[1], states, turn)
        uct[uct == 0] -= 10
        for key in self.children.keys():
            child = self.children[key]
            uct[key] *= np.sqrt(2*self.visits)/(child.visits+1)
        visit = np.argmax(uct)
        self.children[visit].visits += 1
        return self.children[visit]
    def best_child(self):
        val = -np.inf
        best = None
        for child in self.children.values():
            if child.visits > 0 and child.evaluate() > val:
                val = child.evaluate()
                best = child
        return best
    def hybrid_best_child(self, b, mp, turn):
        val = -np.inf
        best = None
        for child in self.children.values():
            m = nn_to_move(child.move, turn)
            v = -child.hybrid_evaluate(chess.afterMove(b, mp, m[0], m[1]), turn*-1)
            if v > val:
                val = v
                best = child
        print("Anticipated value:", val)
        return best


def monteCarlo(model, b, mp, turn, states, timeLimit):
    startTime = time.time()
    root = Node(None, None)
    while time.time() - startTime < timeLimit:
        t = turn
        node = root
        sim_states = []
        for i in range(len(states)):
            sim_states.append(states[i])
        curr_state = sim_states[len(sim_states)-1]
        while not node.is_leaf:
            node = node.visit_child(model, t, sim_states)
            nm = nn_to_move(node.move, t)
            curr_state = chess.afterMove(curr_state[0], curr_state[1], nm[0], nm[1])
            sim_states.append(curr_state)
            t *= -1
        node.expand(curr_state[0], curr_state[1], t)
        result = chess.gameRes(sim_states, curr_state[0], curr_state[1], t)
        if result == 0:
            old_eval = chess.evalState(curr_state[0], curr_state[1])
            result = 2/(1.0+np.power(10, -turn*old_eval/400.0))-1
        else:
            result = {1: -1*t*turn, 2: 0, 3: 0}[result]
        while node.has_parent():
            node.update(result)
            node = node.parent
    return root.best_child()

def hybridSearch(model, b, mp, turn, states, timeLimit):
    startTime = time.time()
    root = Node(None, None)
    while time.time() - startTime < timeLimit:
        t = turn
        node = root
        sim_states = []
        for i in range(len(states)):
            sim_states.append(states[i])
        curr_state = sim_states[len(sim_states)-1]
        while not node.is_leaf:
            node = node.hybrid_visit(model, t, sim_states)
            nm = nn_to_move(node.move, t)
            curr_state = chess.afterMove(curr_state[0], curr_state[1], nm[0], nm[1])
            sim_states.append(curr_state)
            t *= -1
        node.update(chess.gameRes(sim_states, curr_state[0], curr_state[1], t))
        node.expand(curr_state[0], curr_state[1], t)
        while node.has_parent():
            node = node.parent
    return root.hybrid_best_child(b, mp, turn)




def one_hot_board(b, turn):
    piece_board = np.asarray(b)
    if turn == -1:
        piece_board[piece_board == 0] -= 20
        piece_board[piece_board > 6] += 20
        piece_board[piece_board < 7] += 6
        piece_board[piece_board > 12] -= 26
        piece_board = piece_board[::-1,:]
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
    #currState = inStates[len(inStates)-1]
    #input2 = np.asarray([turn, chess.num_repetitions(currState[0], currState[1], states)]).astype(int)
    return [input]


def move_to_nn(start, end, turn, tensor=True):
    s0 = start[0]
    e0 = end[0]
    if turn == -1:
        s0=7-s0
        e0=7-e0
    nn = (s0*8+start[1])*(56+8) # 56 queen moves and 8 knight moves from each square
    nn += e0*8+end[1]
    if tensor:
        return tf.one_hot(nn, 4096)
    return nn
def nn_to_move(nn, turn):
    e1 = nn%8
    nn -= e1
    e0 = nn%64
    nn -= e0
    s1 = nn%512
    nn -= s1
    s0 = nn%4096
    nn -= s0
    s0 = int(s0/512)
    s1 = int(s1/64)
    e0 = int(e0/8)
    e1 = int(e1)
    if turn == -1:
        s0=7-s0
        e0=7-e0
    return [[s0,s1], [e0,e1], 5]


def residual_block(x, kernel, filters, reg):
    fx = Conv2D(filters, kernel, activation='relu', padding='same')(x)
    fx = BatchNormalization()(fx)
    fx = Conv2D(filters, kernel, padding='same', kernel_regularizer=reg, bias_regularizer=reg)(fx)
    out = tf.keras.layers.Add()([x,fx])
    out = tf.keras.layers.ReLU()(out)
    out = BatchNormalization()(out)
    return out

def define_model():
    input = Input(shape=(8,8,12*T_PLY))
    reg = L2(0.01)

    b = input[:,:,:,12*(T_PLY-1):]
    b = Flatten()(b)
    r0 = residual_block(input, (3,3), 12*T_PLY, reg)
    r1 = residual_block(input, (5,5), 12*T_PLY, reg)
    for i in range(8):
        r0 = residual_block(r0, (3,3), 12*T_PLY, reg)
        #r1 = residual_block(r1, (5,5), 12*T_PLY, reg)
    r0 = GlobalAveragePooling2D()(r0)
    r1 = GlobalAveragePooling2D()(r1)

    #conv = Concatenate(axis=1)([r0,r1])
    hidden = Concatenate(axis=1)([r0,r1,b])
    policy = Dense(4096, activation='softmax')(hidden)

    model = Model(inputs=[input], outputs=[policy])
    model.compile(
        optimizer="adam",
        loss="categorical_crossentropy",
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
