# BrickChess0.5

This is BrickChess0.5, an online chess engine. [harysdalvi.com/chess](http://www.harysdalvi.com/chess) hosts BrickChess0.5 Lite as well as more details about this engine. To access the full version of BrickChess0.5, download this repository and run `main.py` or `jsclient-local.py`. `main.py` includes the neural network versions, but also requires modifying the `players` variable in the code.

`main.py` requires `python-chess` and `PyQt5`. `jsclient-local.py` requires `flask`. You may have to compile `chess.cpp` using `compile.command`
or a bash equivalent. You also need `numpy` and `tensorflow` for the neural network.