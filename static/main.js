var canvas = document.getElementById("board");
var ctx = canvas.getContext('2d');
const CANVAS_SCALE = window.devicePixelRatio;

var piece_textures = [];
var validMoves = [];
for (var i = 1; i <= 12; i++) {
    var texture = new Image();
    texture.src = "http://harysdalvi.com/sub/chess/static/img/"+i+".svg.png";
    piece_textures.push(texture);
}

function drawBoard() {
    var flip = (players[-1] == 0 && (toPlay == -1 || players[1] != 0));
    var full_w = canvas.height/CANVAS_SCALE;
    var margin = full_w*0.05;
    var w = full_w*0.9;
    var grad = ctx.createLinearGradient(0, 0, full_w, full_w);
    if (flip) {
        grad.addColorStop(1, "#111121");
        grad.addColorStop(0, "#404050");
    } else {
        grad.addColorStop(1, "#404050");
        grad.addColorStop(0, "#111121");
    }
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, full_w, full_w);
    for (var i = 0; i < 64; i++) {
        var x = i%8;
        var y = Math.floor(i/8);
        var xb = i%8;
        var yb = Math.floor(i/8);
        if (flip) {
            x = 7-x;
            y = 7-y;
        }
        ctx.fillStyle = ["#FFD09E", "#D18C47"][(i+Math.floor(i/8))%2];
        if (lastMove.length > 0 && ((lastMove[0][0] == yb && lastMove[0][1] == xb) || (lastMove[1][0] == yb && lastMove[1][1] == xb)))
            ctx.fillStyle = ["#CDD16A", "#AAA23B"][(i+Math.floor(i/8))%2];
        if (gameRes == 1 && board[yb][xb] == 9-3*toPlay)
            ctx.fillStyle = "#992200";
        if (gameRes == 2 && (board[yb][xb] == 6 || board[yb][xb] == 12))
            ctx.fillStyle = "#CCCC00";
        ctx.fillRect(margin+(w/8.0)*x, margin+(w/8.0)*y, w/8.0, w/8.0);
        if (yb == checkSquare[0] && xb == checkSquare[1]) {
            grad = ctx.createRadialGradient(margin+(w/8.0)*(x+0.5), margin+(w/8.0)*(y+0.5), (w/8.0*0.3),
                margin+(w/8.0)*(x+0.5), margin+(w/8.0)*(y+0.5), (w/8.0*0.5));
            grad.addColorStop(0, "rgba(255, 0, 0, 1.0)");
            grad.addColorStop(1, "rgba(255, 0, 0, 0.0)");
            ctx.fillStyle = grad;
            ctx.fillRect(margin+(w/8.0)*x, margin+(w/8.0)*y, w/8.0, w/8.0);
        }
        if (dragging[1] == xb && dragging[0] == yb)
            ctx.globalAlpha = 0.5;
        if ((playerPromotion[0][1] == xb && playerPromotion[0][0] == yb) || (playerPromotion[1][1] == xb && playerPromotion[1][0] == yb))
            ctx.globalAlpha = 0.0;
        var piece = board[yb][xb];
        if (piece > 0 && startedFirstGame)
            ctx.drawImage(piece_textures[piece-1], margin+(w/8.0)*x, margin+(w/8.0)*y, (w/8.0), (w/8.0));
        ctx.globalAlpha = 1.0;
    }
    for (var i = 0; i < validMoves.length; i++) {
        let move = validMoves[i];
        if (board[move[0]][move[1]] != 0 ||
        (board[dragging[0]][dragging[1]] == 1 && move[1]-dragging[1] != 0) ||
        (board[dragging[0]][dragging[1]] == 7 && move[1]-dragging[1] != 0))
            ctx.fillStyle = "#FF4422";
        else
            ctx.fillStyle = "#77BBFF";
        var x = move[1];
        var y = move[0]
        if (flip) {
            x = 7-x;
            y = 7-y;
        }
        ctx.beginPath();
        ctx.arc(margin+(w/8.0)*(x+0.5), margin+(w/8.0)*(y+0.5), w/64.0, 0, 2*Math.PI);
        ctx.fill();
    }
    ctx.font = 'bold '+Math.floor(36*full_w/900)+'px Arial';
    const letters = ['a','b','c','d','e','f','g','h'];
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    for (var i = 1; i <= 8; i++) {
        var rank = i;
        var file = letters[i-1];
        var x = i;
        var y = 9-i;
        if (flip) {
            x = 9-x;
            y = i;
        }
        ctx.fillStyle = "#EEEEEE";
        ctx.fillText(rank, margin*0.5, margin+(w/8.0)*(y-0.5));
        ctx.fillText(rank, full_w-margin*0.5, margin+(w/8.0)*(y-0.5));
        ctx.fillText(file, margin+(w/8.0)*(x-0.5), margin*0.5);
        ctx.fillText(file, margin+(w/8.0)*(x-0.5), full_w-margin*0.5);
    }
    if (playerPromotion[2] == -1) {
        if (dragging[0] != -1) {
            ctx.drawImage(piece_textures[board[dragging[0]][dragging[1]]-1], margin+(w/8.0)*(dragPos[0]-0.5), margin+(w/8.0)*(dragPos[1]-0.5), (w/8.0), (w/8.0));
        }
    } else {
        if (flip) {
            ctx.drawImage(piece_textures[board[playerPromotion[0][0]][playerPromotion[0][1]]-1],
                margin+(w/8.0)*(7-playerPromotion[1][1]), margin+(w/8.0)*(7-playerPromotion[1][0]), (w/8.0), (w/8.0));
        } else {
            ctx.drawImage(piece_textures[board[playerPromotion[0][0]][playerPromotion[0][1]]-1],
                margin+(w/8.0)*playerPromotion[1][1], margin+(w/8.0)*playerPromotion[1][0], (w/8.0), (w/8.0));
        }
        ctx.fillStyle = "#000000";
        ctx.fillRect(margin+(w/8.0)*1.5, margin+(w/8.0)*1.5, w/4.0, w/4.0);
        ctx.fillRect(margin+(w/8.0)*1.5, margin+(w/8.0)*4.5, w/4.0, w/4.0);
        ctx.fillRect(margin+(w/8.0)*4.5, margin+(w/8.0)*1.5, w/4.0, w/4.0);
        ctx.fillRect(margin+(w/8.0)*4.5, margin+(w/8.0)*4.5, w/4.0, w/4.0);
        ctx.fillStyle = "#FFFFFF";
        ctx.fillRect(margin+(w/8.0)*1.55, margin+(w/8.0)*1.55, w/4.0*0.95, w/4.0*0.95);
        ctx.fillRect(margin+(w/8.0)*1.55, margin+(w/8.0)*4.55, w/4.0*0.95, w/4.0*0.95);
        ctx.fillRect(margin+(w/8.0)*4.55, margin+(w/8.0)*1.55, w/4.0*0.95, w/4.0*0.95);
        ctx.fillRect(margin+(w/8.0)*4.55, margin+(w/8.0)*4.55, w/4.0*0.95, w/4.0*0.95);
        let addPiece = toPlay == 1 ? 0 : 6;
        ctx.drawImage(piece_textures[4+addPiece], margin+(w/8.0)*1.55, margin+(w/8.0)*1.55, w/4.0*0.95, w/4.0*0.95);
        ctx.drawImage(piece_textures[3+addPiece], margin+(w/8.0)*1.55, margin+(w/8.0)*4.55, w/4.0*0.95, w/4.0*0.95);
        ctx.drawImage(piece_textures[1+addPiece], margin+(w/8.0)*4.55, margin+(w/8.0)*1.55, w/4.0*0.95, w/4.0*0.95);
        ctx.drawImage(piece_textures[2+addPiece], margin+(w/8.0)*4.55, margin+(w/8.0)*4.55, w/4.0*0.95, w/4.0*0.95);
    }
}
function formatTime(t) {
    if (t == UNLIMITED_TIME) {
        return '--:--';
    }
    if (t < 10) {
        var sec = Math.floor(t);
        return '0'+sec+'.'+Math.floor(10*(t%1.0));
    }
    var min = Math.floor(t/60);
    var sec = Math.floor(t%60);
    min = ''+min;
    sec = ''+sec;
    if (min.length == 1)
        min = '0'+min;
    if (sec.length == 1)
        sec = '0'+sec;
    return min+':'+sec;
}
function drawTimer() {
    var full_w = canvas.height/CANVAS_SCALE;
    var flip = (players[-1] == 0 && (toPlay == -1 || players[1] != 0));
    ctx.fillStyle = "#333333";
    ctx.fillRect(full_w, 0, full_w*0.25, full_w);
    ctx.fillStyle = "#334444";
    ctx.fillRect(full_w*1.015, full_w*0.09, full_w*0.22, full_w*0.12);
    ctx.fillRect(full_w*1.015, full_w*0.79, full_w*0.22, full_w*0.12);
    var grad = ctx.createLinearGradient(full_w*1.2, 0, full_w, full_w);
    grad.addColorStop(0.1, "#CCFFCC");
    grad.addColorStop(0.2, "#BBDDCC");
    grad.addColorStop(0.8, "#CCFFCC");
    grad.addColorStop(0.9, "#BBDDCC");
    ctx.fillStyle = grad;
    ctx.fillRect(full_w*1.025, full_w*0.1, full_w*0.2, full_w*0.1);
    ctx.fillRect(full_w*1.025, full_w*0.8, full_w*0.2, full_w*0.1);
    var t1, t2;
    if (flip) {
        t1 = timeLeft[0];
        t2 = timeLeft[1];
    } else {
        t1 = timeLeft[1];
        t2 = timeLeft[0];
    }
    ctx.font = 'bold '+Math.floor(36*full_w/700)+'px Courier New';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillStyle = "#000000";
    if ((gameRes == 0 || (t1 <= 0 && t1 != UNLIMITED_TIME)) && ((!flip && toPlay == -1) || (flip && toPlay == 1)))
        ctx.fillStyle = "#CC0000";
    ctx.fillText(formatTime(t1), full_w*1.125, full_w*0.15);
    ctx.fillStyle = "#000000";
    if ((gameRes == 0 || (t2 <= 0 && t2 != UNLIMITED_TIME)) && ((!flip && toPlay == 1) || (flip && toPlay == -1)))
        ctx.fillStyle = "#CC0000";
    ctx.fillText(formatTime(t2), full_w*1.125, full_w*0.85);
    if (gameRes == 0 && (players[1] == 0 || players[-1] == 0)) {
        ctx.fillStyle = "#334444";
        ctx.fillRect(full_w*1.025, full_w*0.6, full_w*0.09, full_w*0.09);
        ctx.fillRect(full_w*1.135, full_w*0.6, full_w*0.09, full_w*0.09);
        ctx.fillStyle = "#CCFFFF";
        ctx.fillRect(full_w*1.03, full_w*0.605, full_w*0.08, full_w*0.08);
        if (wantsDraw[toPlay == 1 ? 0 : 1] || (wantsDraw[toPlay == 1 ? 1 : 0] && players[toPlay] != 0))
            ctx.fillStyle = "#77BBBB";
        else if (wantsDraw[toPlay == 1 ? 1 : 0])
            ctx.fillStyle = "#33FFFF";
        ctx.fillRect(full_w*1.14, full_w*0.605, full_w*0.08, full_w*0.08);
        ctx.fillStyle = "#334444";
        ctx.strokeStyle = "#334444";
        ctx.lineWidth = full_w*0.003;
        ctx.beginPath();
        ctx.moveTo(full_w*1.05, full_w*0.67);
        ctx.lineTo(full_w*1.04, full_w*0.62);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(full_w*1.045, full_w*0.62);
        ctx.quadraticCurveTo(full_w*1.05, full_w*0.63, full_w*1.065, full_w*0.6225);
        ctx.quadraticCurveTo(full_w*1.07, full_w*0.615, full_w*1.085, full_w*0.625);
        ctx.lineTo(full_w*1.09, full_w*0.65);
        ctx.quadraticCurveTo(full_w*1.075, full_w*0.64, full_w*1.07, full_w*0.6475);
        ctx.quadraticCurveTo(full_w*1.065, full_w*0.655, full_w*1.05, full_w*0.645);
        ctx.lineTo(full_w*1.045, full_w*0.62);
        ctx.fill();
        ctx.stroke();
        ctx.lineWidth = 4;
        ctx.font = ''+Math.floor(24*full_w/900)+'px Courier New';
        ctx.strokeText('1', full_w*1.16, full_w*0.635);
        ctx.beginPath();
        ctx.moveTo(full_w*1.15, full_w*0.675);
        ctx.lineTo(full_w*1.21, full_w*0.615);
        ctx.stroke();
        ctx.strokeText('2', full_w*1.2, full_w*0.655);
    }
}
function clickBoard(event) {
    if (gameRes == 0) {
        var flip = (players[-1] == 0 && (toPlay == -1 || players[1] != 0));
        var rect = canvas.getBoundingClientRect();
        var full_w = canvas.height/CANVAS_SCALE;
        var margin = full_w*0.05;
        var square = (full_w-2*margin)/8.0;
        if (playerPromotion[2] == -1) {
            var raw_x = (event.clientX-rect.left)/full_w;
            var raw_y = (event.clientY-rect.top)/full_w;
            var x = Math.floor((event.clientX-margin-rect.left)/square);
            var y = Math.floor((event.clientY-margin-rect.top)/square);
            if (flip) {
                x = 7-x;
                y = 7-y;
            }
            if (players[toPlay] == 0 && PIECE_SIDE[board[y][x]] == toPlay) {
                dragging[0] = y;
                dragging[1] = x;
            }
            getPython('validmoves', {'board': board_to_arg(board), 'moved': moved_to_arg(movedPieces), 'turn': toPlay})
            .then((vm) => {
                validMoves = [];
                for (var i = 0; i < vm.length; i++) {
                    if (dragging[0] == vm[i][0][0] && dragging[1] == vm[i][0][1])
                        validMoves.push(vm[i][1]);
                }
                drawBoard();
            })
            if (1.025 <= raw_x && raw_x <= 1.115 && 0.6 <= raw_y && raw_y <= 0.69 && players[toPlay] == 0) {
                resign();
            }
            if (1.135 <= raw_x && raw_x <= 1.225 && 0.6 <= raw_y && raw_y <= 0.69 && players[toPlay] == 0) {
                askDraw();
            }
        } else {
            var x = (event.clientX-margin-rect.left)/square;
            var y = (event.clientY-margin-rect.top)/square;
            let myMove = [playerPromotion[0], playerPromotion[1], 0];
            if (1.5 <= x && x <= 3.5 && 1.5 <= y && y <= 3.5)
                myMove[2] = 5;
            if (1.5 <= x && x <= 3.5 && 4.5 <= y && y <= 6.5)
                myMove[2] = 4;
            if (4.5 <= x && x <= 6.5 && 4.5 <= y && y <= 6.5)
                myMove[2] = 3;
            if (4.5 <= x && x <= 6.5 && 1.5 <= y && y <= 3.5)
                myMove[2] = 2;
            if (myMove[2] != 0) {
                playerPromotion = [-1, -1, -1];
                makeMove(myMove[0], myMove[1], myMove[2]);
            }
        }
    }
}
function offBoard(event) {
    if (players[toPlay] == 0 && gameRes == 0) {
        var flip = (players[-1] == 0 && (toPlay == -1 || players[1] != 0));
        var rect = canvas.getBoundingClientRect();
        var full_w = canvas.height/CANVAS_SCALE;
        var margin = full_w*0.05;
        var square = (full_w-2*margin)/8.0;
        var x = Math.floor((event.clientX-margin-rect.left)/square);
        var y = Math.floor((event.clientY-margin-rect.top)/square);
        if (flip) {
            x = 7-x;
            y = 7-y;
        }
        for (var i = 0; i < validMoves.length; i++) {
            if (x == validMoves[i][1] && y == validMoves[i][0]) {
                playerMove([dragging[0],dragging[1]], [validMoves[i][0],validMoves[i][1]]);
            }
        }
    }
    dragging[0] = -1;
    dragging[1] = -1;
    dragPos[0] = -1;
    dragPos[1] = -1;
    validMoves = [];
    drawBoard();
}
function dragBoard(event) {
    if (gameRes == 0) {
        var flip = (players[-1] == 0 && (toPlay == -1 || players[1] != 0));
        var rect = canvas.getBoundingClientRect();
        var full_w = canvas.height/CANVAS_SCALE;
        var margin = full_w*0.05;
        var square = (full_w-2*margin)/8.0;
        var raw_x = (event.clientX-rect.left)/full_w;
        var raw_y = (event.clientY-rect.top)/full_w;
        var x = (event.clientX-margin-rect.left)/square;
        var y = (event.clientY-margin-rect.top)/square;
        if (playerPromotion[2] == -1) {
            dragPos[0] = x;
            dragPos[1] = y;
            x = Math.floor(x)
            y = Math.floor(y);
            if (flip) {
                x = 7-x;
                y = 7-y;
            }
            if (players[toPlay] == 0 && ((0 <= x && x <= 7 && 0 <= y && y <= 7 && PIECE_SIDE[board[y][x]] == toPlay)
            || (1.025 <= raw_x && raw_x <= 1.115 && 0.6 <= raw_y && raw_y <= 0.69)
            || (1.135 <= raw_x && raw_x <= 1.225 && 0.6 <= raw_y && raw_y <= 0.69
                && !(wantsDraw[toPlay == 1 ? 0 : 1])))) {
                canvas.style.cursor = "pointer";
            } else {
                canvas.style.cursor = "default";
            }
            drawBoard();
        } else {
            if ((1.5 <= x && x <= 3.5 && 1.5 <= y && y <= 3.5) ||
            (1.5 <= x && x <= 3.5 && 4.5 <= y && y <= 6.5) ||
            (4.5 <= x && x <= 6.5 && 1.5 <= y && y <= 3.5) ||
            (4.5 <= x && x <= 6.5 && 4.5 <= y && y <= 6.5)) {
                canvas.style.cursor = "pointer";
            } else {
                canvas.style.cursor = "default";
            }
        }
    } else {
        canvas.style.cursor = "default";
    }
}
function touchBoard(event) {
    event.preventDefault();
    if (dragging[0] == -1) {
        clickBoard(event.targetTouches[0]);
    } else {
        offBoard(event.targetTouches[0]);
        clickBoard(event.targetTouches[0]);
    }
}

if (location.hostname == "localhost") {
    var SERVER_URL = 'http://localhost:8000/';
} else {
    var SERVER_URL = 'http://www.harysdalvi.com/apps/chess/';
}

async function getPython(pyfunc, args={}) {
    var argstring = '';
    const keys = Object.keys(args);
    for (var i = 0; i < keys.length; i++) {
        if (i == 0)
            argstring += '?';
        else
            argstring += '&';
        argstring += keys[i];
        argstring += '=';
        argstring += args[keys[i]];
    }
    const url = SERVER_URL+pyfunc+argstring;
    const response = await fetch(url);
    const myJson = await response.json();
    return myJson;
}

const DEFAULT_BOARD =  [[10,8, 9,11,12, 9, 8,10],
                        [7, 7, 7, 7, 7, 7, 7, 7],
                        [0, 0, 0, 0, 0, 0, 0, 0],
                        [0, 0, 0, 0, 0, 0, 0, 0],
                        [0, 0, 0, 0, 0, 0, 0, 0],
                        [0, 0, 0, 0, 0, 0, 0, 0],
                        [1, 1, 1, 1, 1, 1, 1, 1],
                        [4, 2, 3, 5, 6, 3, 2, 4]];
var board = DEFAULT_BOARD;
var movedPieces = [];
var lastMove = [];
var checkSquare = [-1,-1];
var states = [[DEFAULT_BOARD, []]];
var toPlay = 1;
var players = {};
var gameRes = -2;
players[1] = 0;
players[-1] = 0;
var fifty_move_counter = 50;
var dragging = [-1,-1];
var dragPos = [-1, -1];
const UNLIMITED_TIME = -100*60;
var timeLeft = [UNLIMITED_TIME, UNLIMITED_TIME];
var wantsDraw = [false, false];
var increment = 0;
var playerPromotion = [-1, -1, -1];
var timer;
var startedFirstGame = false;
const PIECE_SIDE = [0,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1];

function board_to_arg(b) {
    s = ""
    for (var i = 0; i < 8; i++) {
        for (var j = 0; j < 8; j++) {
            s += b[i][j]+"_";
        }
    }
    return s;
}
function moved_to_arg(mp) {
    s = ""
    for (var i = 0; i < mp.length; i++) {
        s += ''+mp[i][0]+''+mp[i][1]+"_";
    }
    return s;
}
function square_to_arg(s) {
    return ""+s[0]+s[1];
}
function states_to_arg(st) {
    s = "";
    for (var i = Math.max(st.length-4, 0); i < st.length; i++) {
        s += board_to_arg(st[i][0]);
        s += "s";
        s += moved_to_arg(st[i][1]);
        s += "S";
    }
    return s;
}

function playerMove(start, end) {
    if ((board[start[0]][start[1]] == 1 && end[0] == 0) || (board[start[0]][start[1]] == 7 && end[0] == 7)) {
        playerPromotion[0] = start;
        playerPromotion[1] = end;
        playerPromotion[2] = 0;
        drawBoard();
    } else {
        makeMove(start, end, 5);
    }
}
async function makeMove(start, end, promotion) {
    getPython('makemove', {'board': board_to_arg(board), 'moved': moved_to_arg(movedPieces),
        'start': square_to_arg(start), 'end': square_to_arg(end), 'promotion': promotion,
        'states': states_to_arg(states), 'turn': toPlay, 'fifty_count': fifty_move_counter})
    .then((myJson) => {
        gameRes = myJson['res'];
        if (gameRes != -2) {
            board = myJson['board'];
            movedPieces = myJson['moved'];
            fifty_move_counter = myJson['fifty'];
            toPlay *= -1;
            lastMove = [start,end];
            var msg = myJson['messages'];
            checkSquare = myJson['check'];
            states.push([board,movedPieces]);
            drawBoard();
            drawTimer();
            if (gameRes == 0)
                getNextMove();
            for (var i = 0; i < msg.length; i++)
                console.log(msg[i]);
        } else {
            console.log(board);
            console.log(start);
            console.log(end);
            console.log("Invalid move");
            getNextMove();
        }
    });
}
function tickTimer() {
    let player = toPlay == 1 ? 0 : 1;
    if (timeLeft[player] != UNLIMITED_TIME) {
        timeLeft[player] -= 0.1;
        if (timeLeft[player] <= 0) {
            gameRes = 1;
            timeLeft[player] = 0;
        }
    }
    if (gameRes != 0) {
        window.clearInterval(timer);
        drawBoard();
        document.settings.style.visibility = '';
    }
    drawTimer();
}

async function getNextMove() {
    let player = toPlay == 1 ? 0 : 1;
    if (timeLeft[player] != UNLIMITED_TIME)
        timeLeft[player] += increment;
    switch (players[toPlay]) {
        case 1:
            var playTime = Math.floor(timeLeft[player]*1000);
            if (timeLeft[player] == UNLIMITED_TIME)
                playTime = 100000;
            getPython("minimax", {"board": board_to_arg(board), "moved": moved_to_arg(movedPieces), "turn": toPlay,
            "states": states_to_arg(states), "time": playTime})
            .then((move) => makeMove(move['start'], move['end'], move['promotion']));
            break;
        case 2:
            var playTime = Math.floor(timeLeft[player]*1000);
            if (timeLeft[player] == UNLIMITED_TIME)
                playTime = 100000;
            getPython("nn", {"board": board_to_arg(board), "moved": moved_to_arg(movedPieces), "turn": toPlay,
            "states": states_to_arg(states), "time": playTime})
            .then((move) => makeMove(move['start'], move['end'], move['promotion']));
            break;
        case 3:
            getPython("policy", {"board": board_to_arg(board), "moved": moved_to_arg(movedPieces),
            "turn": toPlay, "states": states_to_arg(states)})
            .then((move) => makeMove(move['start'], move['end'], move['promotion']));
            break;
        case 4:
            var playTime = Math.floor(timeLeft[player]*1000);
            if (timeLeft[player] == UNLIMITED_TIME)
                playTime = 100000;
            getPython("hybrid", {"board": board_to_arg(board), "moved": moved_to_arg(movedPieces), "turn": toPlay,
            "states": states_to_arg(states), "time": playTime})
            .then((move) => makeMove(move['start'], move['end'], move['promotion']));
            break;
    }
}

function askDraw() {
    wantsDraw[toPlay == 1 ? 0 : 1] = true;
    if (wantsDraw[0] && wantsDraw[1])
        gameRes = 2;
}
function resign() {
    gameRes = 1;
}

function startGame() {
    board = DEFAULT_BOARD;
    movedPieces = [];
    lastMove = [];
    checkSquare = [-1, -1];
    states = [[DEFAULT_BOARD, []]];
    wantsDraw = [false, false];
    toPlay = 1;
    fifty_move_counter = 50;
    gameRes = 0;
    window.clearInterval(timer);
    timer = setInterval(tickTimer, 100);
    document.settings.style.visibility = 'hidden';
    startedFirstGame = true;
    drawBoard();
    drawTimer();
    getNextMove();
}

function startFromForm() {
    players[1] = parseInt(document.settings.white.value);
    players[-1] = parseInt(document.settings.black.value);
    timeLeft[0] = 60*parseInt(document.settings.time.value);
    timeLeft[1] = 60*parseInt(document.settings.time.value);
    increment = parseInt(document.settings.increment.value);
    startGame();
}

function whenResize() {
    var size = Math.min(document.getElementById("boardHolder").clientWidth*0.75, window.innerHeight*0.8);
    canvas.style.width = (size*1.25)+'px';
    canvas.style.height = size+'px';
    canvas.width = size*1.25*CANVAS_SCALE;
    canvas.height = size*CANVAS_SCALE;
    canvas.getContext('2d').scale(CANVAS_SCALE,CANVAS_SCALE);
    drawBoard();
    drawTimer();
}
window.addEventListener("resize", whenResize);
window.onload = whenResize;
canvas.addEventListener("mousedown", clickBoard);
canvas.addEventListener("touchstart", touchBoard);
canvas.addEventListener("mousemove", dragBoard);
canvas.addEventListener("mouseup", offBoard);
//canvas.addEventListener("touchend", offBoard);