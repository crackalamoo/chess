#import tensorflow as tf
import numpy as np
import chess

def minimax_ai(b, mp, turn):
    #return chess.minimax(b, mp, turn, 2, {-1: 7500, 1: 7500}[turn], True)
    return chess.minimax(b, mp, turn, 2, 7500, True)