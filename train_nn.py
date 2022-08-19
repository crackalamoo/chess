#import chess_core as chess
from re import T
from ai import *
import tensorflow as tf
import numpy as np

print("Loaded modules")

BATCH_SIZE = 256
NUM_EPOCHS = 5 # 15 so far
TEST_SIZE = 10000
DATA_FILE = "data/data2000.npz"
LOAD_MODEL = True

class TrainCallback(tf.keras.callbacks.Callback):
    def __init__(self, m, tX0, tX1, ty0, ty1):
        super().__init__()
        self.model = m
        self.tX0 = tX0
        self.tX1 = tX1
        self.ty0 = ty0
        self.ty1 = ty1
    def on_epoch_begin(self, epoch, logs=None):
        print("Training model")
    def on_epoch_end(self, epoch, logs=None):
        print("\nEvaluating model")
        results = self.model.evaluate([self.tX0, self.tX1], [self.ty0, self.ty1], batch_size=BATCH_SIZE)
        print("test loss, test acc", results)
        print("")

loaded_data = np.load(DATA_FILE)
train_X0 = loaded_data['X0']
train_X1 = loaded_data['X1']
train_y0 = loaded_data['y0']
train_y1 = loaded_data['y1']
print("Loaded data")
test_X0 = train_X0[-TEST_SIZE:]
test_X1 = train_X1[-TEST_SIZE:]
test_y0 = train_y0[-TEST_SIZE:]
test_y1 = train_y1[-TEST_SIZE:]
train_X0 = train_X0[:-TEST_SIZE]
train_X1 = train_X1[:-TEST_SIZE]
train_y0 = train_y0[:-TEST_SIZE]
train_y1 = train_y1[:-TEST_SIZE]

print(train_X0.shape)
print(train_X1.shape)
print(train_y0.shape)
print(train_y1.shape)

if LOAD_MODEL:
    model = load_model()
    print("Loaded model")
else:
    model = define_model()
    print("Defined model")

print("Evaluating model (before training)")
results = model.evaluate([test_X0, test_X1], [test_y0, test_y1], batch_size=BATCH_SIZE)
print("|| test loss, test acc", results)


history = model.fit(
    [train_X0, train_X1],
    [train_y0, train_y1],
    batch_size=BATCH_SIZE,
    epochs=NUM_EPOCHS,
    callbacks=[TrainCallback(model, test_X0, test_X1, test_y0, test_y1)]
)

model.save("model")