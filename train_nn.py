import chess
from ai import *
import tensorflow as tf
import numpy as np

BATCH_SIZE = 64
NUM_EPOCHS = 5
TEST_SIZE = 20000

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

loaded_data = np.load("data/data.npz")
train_X = loaded_data['X']
train_y = loaded_data['y']
test_X = train_X[-TEST_SIZE:]
test_y = train_y[-TEST_SIZE:]
train_X = train_X[:-TEST_SIZE]
train_y = train_y[:-TEST_SIZE]

print(train_X.shape)
print(train_y.shape)

model = define_model()

print("Evaluating model")
results = model.evaluate(test_X, test_y, batch_size=BATCH_SIZE)
print("|| test loss, test acc", results)


history = model.fit(
    train_X,
    train_y,
    batch_size=BATCH_SIZE,
    epochs=NUM_EPOCHS,
    callbacks=[TrainCallback(model, test_X, test_y)]
)

model.save("model")