#import tensorflow as tf
import numpy as np
import chess

def minimax_ai(b, mp, turn, depth):
    return chess.minimax(b, mp, turn, depth, 20000, True)