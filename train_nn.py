import chess
from ai import *
import tensorflow as tf

BATCH_SIZE = 32

class TrainCallback(tf.keras.callbacks.Callback):
    def __init__(self, m, tX, ty):
        super().__init__()
        self.model = m
        self.tX = tX
        self.ty = ty
    def on_epoch_begin(self, epoch, logs=None):
        print("Training model")
    def on_epoch_end(self, epoch, logs=None):
        print("\nEvaluating model")
        results = self.model.evaluate(self.tX, self.ty, batch_size=BATCH_SIZE)
        print("test loss, test acc", results)
        print("")

train_X = []
train_y = []

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
        train_X.append(one_hot_board(int_board(sb[0])))
        train_y.append(res)
def playGames(num):
    print("Playing through games")
    data = open('lichess_arifd2.pgn').read().split('\n')
    n = 0
    for l in range(len(data)):
        if data[l].startswith('1'):
            playGame(data[l], data[l-14], data[l-13], data[l-2])
            n += 1
        if n >= num:
            break
    print("Played through " + str(n) + " games (" + str(len(train_y)) + " positions)")

model = define_model()

playGames(5000)

train_X = np.asarray(train_X)
train_y = np.asarray(train_y)
test_X = train_X[-10000:]
test_y = train_y[-10000:]
train_X = train_X[:-10000]
train_y = train_y[:-10000]
print(train_X.shape)
print(train_y.shape)

print("Evaluating model")
results = model.evaluate(test_X, test_y, batch_size=BATCH_SIZE)
print("|| test loss, test acc", results)


history = model.fit(
    train_X,
    train_y,
    batch_size=BATCH_SIZE,
    epochs=5,
    callbacks=[TrainCallback(model, test_X, test_y)]
)

if True:
    sim_board = [chess.DEFAULT_BOARD, []]
    sb1 = move_pgn('e4', sim_board[0], sim_board[1], 1)
    sb1 = move_pgn('e5', sb1[0], sb1[1], -1)
    prediction = model.predict(np.expand_dims(one_hot_board(int_board(sb1[0])), axis=0))
    print(prediction)
