# BrickChess0.5

This is BrickChess0.5, an online chess engine. [harysdalvi.com/chess](http://www.harysdalvi.com/chess) hosts BrickChess0.5 Lite as well as more details about this engine. To access the full version of BrickChess0.5, download this repository and run `main.py` or `jsclient-local.py`. `main.py` includes the neural network versions which can be specified with arguments.

`jsclient-local.py` requires `flask`. You may have to compile `chess.cpp` using `compile.command`
or a bash equivalent. You also need `numpy` and `tensorflow` for the neural network.

`main.py` is the file to run chess through the terminal rather than through the browser. You may specify optional arguments for `main.py`. The first argument represents the white player, and the second argument represents the black player. Options are `0` for human player, `1` for minimax, `2` for MCTS, `3` for policy network, and `4` for hybrid. If you would like an SVG board, you will need `python-chess` and `PyQt5`. If you would like to see the board in the terminal instead, use `0` as the third argument. For example, `python3 main.py 1 0` starts a game with a minimax engine as white and a human as black. `python3 main.py 1 2 0` starts a game with an MCTS engine as white, a minimax engine as black, and a unicode board in the terminal rather than an SVG board with `python-chess` and `PyQt5`.