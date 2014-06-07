var http = require('http');
var util = require('util');
var fs = require('fs');
var spawn = require('child_process').spawn;
var net = require('net');
var clients = [];
var sock_client = {};

var ggp = {};

var getFirstString = function(data, i) {
    while (data[i] === ' ') ++i;
    var j = i;
    while (data[j] !== ' ' && data[j] !== ')') ++j;
    return [i, j];
};

var getFirstBracket = function(data, i) {
    while (data[i] === ' ') ++i;    
    var j = i;
    var cnt = 0;
    do {
        if (data[j] === '(') ++cnt;
        if (data[j] === ')') --cnt;
        ++j;
    } while (cnt > 0);
    return [i, j];
}

var parse = function(data) {
    data = data.toLowerCase();
    var ret = {};
    var i = 0;
    while (data[i] === ' ' || data[i] === '(') {
        ++i;
    }
    var s = getFirstString(data, i);
    i = s[1];
    ret.cmd = data.substring(s[0], s[1]);
    switch (ret.cmd) {
    case 'info' :        
        break;
    case 'start' : 
        s = getFirstString(data, i);
        i = s[1];
        ret.id = data.substring(s[0], s[1]);
        s = getFirstString(data, i);
        i = s[1];
        ret.role = data.substring(s[0], s[1]);
        s = getFirstBracket(data, i);
        i = s[1];
        ret.rule = data.substring(s[0] + 1, s[1] - 1).replace(/\r\n/g, '\n').replace(/\r/g, '\n').replace(/\n/g, '\r\n'); // delete outer brachet
        s = getFirstString(data, i);
        i = s[1];
        ret.startclock = Number(data.substring(s[0], s[1]));
        s = getFirstString(data, i);
        i = s[1];
        ret.playclock = Number(data.substring(s[0], s[1]));
        break;
    case 'play' :
    case 'stop' :
        s = getFirstString(data, i);
        i = s[1];
        ret.id = data.substring(s[0], s[1]);
        while (data[i] === ' ') ++i;
        if (data[i] === '(') {
            s = getFirstBracket(data, i);
            i = s[1];
            ret.move = data.substring(s[0] + 1, s[1] - 1); // delete outer bracket            
        } else {
            s = getFirstString(data, i);
            i = s[1];
            ret.move = data.substring(s[0], s[1]);
        }
        break;
    case 'abort' :
        s = getFirstString(data, i);
        i = s[1];
        ret.id = data.substring(s[0], s[1]);
        break;
    };
    return ret;
};

http.createServer(function (req, res) {
    var body = '';
//    console.log('http version: ' + req.httpVersion);
    req.on('data', function(chunk) {
        body += chunk;
    });
    req.on('end', function () {
        res.writeHead(200, {
                'Content-Type' : 'text/acl'
            ,   'Access-Control-Allow-Origin' : '*'
        });
        console.log(body);
        var request = parse(body);    
        switch (request.cmd) {
        case 'info' :
            res.end('( ( name ailab ) ( status available ) )');
            break;
        case 'start' :
            fs.writeFile('gdl/rule.txt', request.rule, function(err) {
                if (err) {
                    console.log(err);
                }
                ggp.game = request.id;
                ggp.exe = spawn('./ggp');
                ggp.res = res;
                ggp.exe.stdout.on('data', function (data) {
                    data = String(data);
                    console.log('ggp out: ' + data);
                    receiveExeData(data);
                });
                ggp.exe.on('close', function(code) {
                    ggp.exe = null;
                    if (ggp.timer) {
                        clearTimeout(ggp.timer);
                        ggp.timer = null;
                    }                    
                });
                ggp.exe.stdin.write(request.role + '\n');
                ggp.exe.stdin.write(request.playclock + '\n');
            });
            ggp.timer = setTimeout(function() {
                ggp.res.end('ready');
            }, request.startclock - 2);
            break;
        case 'play' :
            if (ggp.exe) {
                console.log('ggp in: ' + request.move);
                ggp.exe.stdout.once('data', function (data) {
                    data = String(data);
                    console.log('ggp out: ' + data);
                    var end = data[data.length - 1];
                    if (end === '\r' || end === '\n') {
                        res.end(data.substring(0, data.length - 1));
                    } else {
                        res.write(data);
                    }
                });
                ggp.exe.stdin.write(request.move + '\n');
            }
            break;
        case 'stop' :
            ggp.exe = null;
            res.end('done');
            break;
        case 'abort' :
            break;
        };
//        console.log(JSON.stringify(request));        
    });
}).listen(80);

function receiveExeData(data) {
    if (ggp.data_length === null) {
        var space_pos = data.indexOf(' ');
        if (space_pos != -1) {
            var length = parseInt(data.substring(0, space_pos));
            if (!isNaN(length)) {
                ggp.data_length = length;
                ggp.buffer = data.substring(space_pos + 1);
            } 
        }
    } 
    if (ggp.data_length !== null) {
        ggp.buffer += data;
        if (ggp.buffer.length === ggp.data_length) {
            handleExeMessage(ggp, ggp.buffer);
            ggp.buffer = '';
            ggp.data_length = null;
        } else if (ggp.buffer.length > ggp.data_length) {
            handleExeMessage(ggp, ggp.buffer.substring(0, ggp.data_length));
            var rest = ggp.buffer.substring(ggp.data_length);
            ggp.data_length = null;
            ggp.buffer = '';
            receiveExeData(rest);
        }
    }
}

function handleExeMessage(message) {
    var i = message.indexOf(' ');
    var cmd = message.substring(0, i);
    message = message.substring(i + 1);
    if (cmd === 'server') {
        ggp.move = message;
    } else if (cmd === 'client') {        
        ggp.message = message;
    }
}

function receiveClientData(client, data) {
    if (client.data_length === null) {
        var space_pos = data.indexOf(' ');
        if (space_pos != -1) {
            var length = parseInt(data.substring(0, space_pos));
            if (!isNaN(length)) {
                client.data_length = length;
                client.buffer = data.substring(space_pos + 1);
            } 
        }
    } 
    if (client.data_length !== null) {
        client.buffer += data;
        if (client.buffer.length === client.data_length) {
            handleClientMessage(client, client.buffer);
            client.buffer = '';
            client.data_length = null;
        } else if (client.buffer.length > client.data_length) {
            handleClientMessage(client, client.buffer.substring(0, client.data_length));
            var rest = client.buffer.substring(client.data_length);
            client.data_length = null;
            client.buffer = '';
            receiveClientData(client, rest);
        }
    }
}

function handleClientMessage(client, message) {
    var i = message.indexOf(' ');
    var game = message.substring(0, i);
    message = message.substring(i + 1);
    if (game === ggp.game) {
        ggp.exe.write(message);
    }
    var msg = ggp.game + " " + ggp.message;
    client.sock.write(msg.length + " " + msg);
}

var server = net.createServer(function (sock) {
    sock.on('connect', function() {
        var client = {};
        client.sock = sock;
        client.free = true;
        client.rule = '';
        client.state = '';
        client.buffer = '';
        client.data_length = null;
        clients.push(client);
    });
    sock.on('data', function(data) {
        for (var i = 0; i < clients.length; ++i) {
            if (clients[i].sock === sock) {
                receiveData(clients[i], data);
                break;
            }
        }
    });
    sock.on('close', function(has_err) {
        var client = null;
        for (var i = 0; i < clients.length; ++i) {
            if (clients[i].sock === sock) {
                console.log('close ' + sock.remoteAddress);
                clients.splice(i, 1);
                break;
            }
        }
    });
});

server.listen(10000);

process.on('uncaughtException', function(err) {
    console.log(util.inspect(err));
});
