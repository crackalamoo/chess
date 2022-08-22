var canvas = document.getElementById("board");
var ctx = canvas.getContext('2d');
const CANVAS_SCALE = window.devicePixelRatio;

var piece_textures = [];
for (var i = 1; i <= 12; i++) {
    var texture = new Image();
    texture.src = "static/img/"+i+".svg.png";
    piece_textures.push(texture);
}

function drawBoard(flip=true) {
    var full_w = canvas.width/CANVAS_SCALE;
    var margin = full_w*0.05;
    var w = full_w*0.9;
    ctx.fillStyle = "#222222";
    ctx.fillRect(0, 0, full_w, full_w);
    for (var i = 0; i < 64; i++) {
        var x = i%8;
        var y = Math.floor(i/8);
        if (flip) {
            x = 7-x;
            y = 7-y;
        }
        ctx.fillStyle = ["#eddab9", "#ae8868"][(i+Math.floor(i/8))%2];
        ctx.fillRect(margin+(w/8.0)*x, margin+(w/8.0)*y, w/8.0, w/8.0);
        ctx.fillStyle = "#000000";
        //ctx.fillText(board[Math.floor(i/8)][i%8], (w/8.0)*(0.3+i%8), (w/8.0)*(0.5+Math.floor(i/8)));
        var piece = board[Math.floor(i/8)][i%8];
        if (piece > 0)
            ctx.drawImage(piece_textures[piece-1], margin+(w/8.0)*x, margin+(w/8.0)*y, (w/8.0), (w/8.0));
    }
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.font = '18px Arial';
    const letters = ['a','b','c','d','e','f','g','h'];
    for (var i = 1; i <= 8; i++) {
        var rank = i;
        var file = letters[i-1];
        var x = i;
        var y = 9-i;
        if (flip) {
            x = 9-x;
            y = i;
        }
        ctx.fillStyle = "#FFFFFF";
        ctx.fillText(rank, margin*0.5, margin+(w/8.0)*(y-0.5));
        ctx.fillText(rank, full_w-margin*0.5, margin+(w/8.0)*(y-0.5));
        ctx.fillText(file, margin+(w/8.0)*(x-0.5), margin*0.5);
        ctx.fillText(file, margin+(w/8.0)*(x-0.5), full_w-margin*0.5);
    }
}

const SERVER_URL = 'http://localhost:8000/';

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
var states = [[DEFAULT_BOARD, []]];
var toPlay = 1;
var players = {};
players[1] = 1;
players[-1] = 1;
var fifty_move_counter = 50;

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
        s += ''+mp[i][0]+mp[i][1]+"_";
    }
    return s;
}
function square_to_arg(s) {
    return ""+s[0]+s[1];
}
function states_to_arg(st) {
    s = "";
    for (var i = 0; i < st.length; i++) {
        s += board_to_arg(st[i][0]);
        s += "s";
        s += moved_to_arg(st[i][1]);
        s += "S";
    }
    return s;
}

async function makeMove(start, end, promotion) {
    getPython('makemove', {'board': board_to_arg(board), 'moved': moved_to_arg(movedPieces),
        'start': square_to_arg(start), 'end': square_to_arg(end), 'promotion': promotion,
        'states': states_to_arg(states), 'turn': toPlay, 'fifty_count': fifty_move_counter})
    .then((myJson) => {
        if (myJson['res'] != -1) {
            board = myJson['board'];
            movedPieces = myJson['moved'];
            fifty_move_counter = myJson['fifty'];
            toPlay *= -1;
            var msg = myJson['messages'];
            var res = myJson['res'];
            // res, checkSquare, messages
            states.push([board,movedPieces]);
            drawBoard();
            if (res == 0)
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

async function getNextMove() {
    if (players[toPlay] == 1) {
        getPython("minimax", {"board": board_to_arg(board), "moved": moved_to_arg(movedPieces), "turn": toPlay})
        .then((move) => makeMove(move['start'], move['end'], move['promotion']));
    }
}

function startGame() {
    board = DEFAULT_BOARD;
    movedPieces = [];
    states = [[DEFAULT_BOARD, []]];
    toPlay = 1;
    fifty_move_counter = 50;
    getNextMove();
}

startGame();

function whenResize() {
    var size = Math.min(window.innerWidth*0.75, window.innerHeight*0.8);
    canvas.style.width = size+'px';
    canvas.style.height = size+'px';
    canvas.width = size*CANVAS_SCALE;
    canvas.height = size*CANVAS_SCALE;
    canvas.getContext('2d').scale(CANVAS_SCALE,CANVAS_SCALE);
    drawBoard();
}
window.addEventListener("resize", whenResize);
whenResize();