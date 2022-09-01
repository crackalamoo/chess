#import chess_core as chess
from ai import *
import tensorflow as tf
import numpy as np

print("Loaded modules")

BATCH_SIZE = 256
NUM_EPOCHS = 1
TEST_SIZE = 20000
DATA_FILE = "data/gm5.npz" # done gm1, gm2, gm3, gm4, gm5
LOAD_MODEL = True

class TrainCallback(tf.keras.callbacks.Callback):
    def __init__(self, m, tX0, ty):
        super().__init__()
        self.model = m
        self.tX0 = tX0
        #self.tX1 = tX1
        self.ty = ty
        #self.ty1 = ty1
    def on_epoch_begin(self, epoch, logs=None):
        print("Training model")
    def on_epoch_end(self, epoch, logs=None):
        print("\nEvaluating model")
        results = self.model.evaluate([self.tX0], [self.ty], batch_size=BATCH_SIZE)
        print("test loss, test acc", results)
        print("")

loaded_data = np.load(DATA_FILE)
train_X0 = loaded_data['X']
#train_X1 = loaded_data['X1']
train_y = loaded_data['y']
#train_y1 = loaded_data['y1']
print("Loaded data")
test_X0 = train_X0[-TEST_SIZE:]
#test_X1 = train_X1[-TEST_SIZE:]
test_y = train_y[-TEST_SIZE:]
#test_y1 = train_y1[-TEST_SIZE:]
train_X0 = train_X0[:-TEST_SIZE]
#train_X1 = train_X1[:-TEST_SIZE]
train_y = train_y[:-TEST_SIZE]
#train_y1 = train_y1[:-TEST_SIZE]

print(train_X0.shape)
#print(train_X1.shape)
print(train_y.shape)
#print(train_y1.shape)

if LOAD_MODEL:
    model = load_model()
    print("Loaded model")
else:
    model = define_model()
    print("Defined model")

def test_model(model):
    wrong = 0
    for i in range(1000):
        pred = np.argmax(model.predict(np.expand_dims(test_X0[i], axis=0))[0])
        actual = np.argmax(test_y[i])
        if not pred == actual:
            wrong += 1
            print(test_X0[i])
            print(nn_to_move(pred, 1), nn_to_move(actual, 1))
    print(wrong/float(1000))
#test_model(model)

print("Evaluating model (before training)")
results = model.evaluate(test_X0, test_y, batch_size=BATCH_SIZE)
print("|| test loss, test acc", results)


history = model.fit(
    train_X0,
    train_y,
    batch_size=BATCH_SIZE,
    epochs=NUM_EPOCHS,
    callbacks=[TrainCallback(model, test_X0, test_y)]
)

print("Completed training")
print("Saving model")

model.save("model")

print("Saved model")