var http = require('http');
var util = require('util');
var fs = require('fs');
var spawn = require('child_process').spawn;
var net = require('net');
var clients = [];
var controllers = [];
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
            for (var i = 0; i < controllers.length; ++i) {
                controllers[i].write('reset');
            }
            fs.writeFileSync('gdl/rule.txt', request.rule);
            ggp.game = request.id;
            ggp.rule = request.rule;
            ggp.playclock = request.playclock;
            ggp.role = request.role;
            ggp.res = res;
            ggp.move = null;
            ggp.state = null;
            ggp.buffer = '';
            ggp.data_length = null;
            ggp.msgs = [];
            ggp.timer = null;
            if (ggp.exe) {
                ggp.exe.kill('SIGKILL');
            }
            ggp.exe = spawn('./ggp');
            ggp.exe.stderr.on('data', function (data) {
                receiveExeData(String(data));
            });
            ggp.exe.on('exit', function(code) {
                console.log('ggp exit');
                ggp.exe = null;
                if (ggp.timer) {
                    clearTimeout(ggp.timer);
                    ggp.timer = null;
                }                    
            });
            ggp.exe.stdin.write(request.rule + '\n');
            ggp.exe.stdin.write(request.role + '\n');
            for (var i = 0; i < clients.length; ++i) {
                if (!clients[i].game) {
                    clients[i].game = ggp.game;
                    var msg = 'rule ' + ggp.role + ' ' + ggp.rule;
                    clients[i].sock.write(msg.length + ' ' + msg);
                }
            }
            break;
        case 'play' :
            if (ggp.exe) {
                //console.log('ggp in: ' + request.move);
                ggp.msgs.splice(1);
                ggp.exe.stdin.write("server " + request.move + '\n');
                ggp.res = res;
                ggp.timer = setTimeout(function() {
                    if (ggp.move) {
                        console.log('move: ' + ggp.move);
                        ggp.res.end(ggp.move);
                    } else {
                        console.log('no move');
                    }
                    ggp.move = null;
                }, (ggp.playclock - 2) * 1000);
            }
            break;
        case 'stop' :
            if (ggp.exe) {
                ggp.exe.kill('SIGKILL');
            }
            ggp.exe = null;
            res.end('done');
            break;
        case 'abort' :
            break;
        };
    });
}).listen(80);

function receiveExeData(data) {
    if (ggp.data_length === null) {
        var space_pos = data.indexOf(' ');
        if (space_pos != -1) {
            var length = parseInt(data.substring(0, space_pos));
            if (!isNaN(length)) {
                ggp.data_length = length;
                data = data.substring(space_pos + 1);
            } 
        }
    } 
    if (ggp.data_length !== null) {
        ggp.buffer += data;
        if (ggp.buffer.length === ggp.data_length) {
            handleExeMessage(ggp.buffer);
            ggp.buffer = '';
            ggp.data_length = null;
        } else if (ggp.buffer.length > ggp.data_length) {
            handleExeMessage(ggp.buffer.substring(0, ggp.data_length));
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
    if (cmd === 'ready') {
        ggp.res.end('ready');
    } else if (cmd === 'move') {
        //console.log('Exe Message: ' + message)
        ggp.move = message;
    } else if (cmd === 'state') {        
        ggp.state = message;
        for (var i = 0; i < clients.length; ++i) {
            if (clients[i].game === ggp.game && !clients[i].state) {
                clients[i].state = ggp.state;
                var msg = 'state ' + clients[i].state;
                clients[i].sock.write(msg.length + ' ' + msg);
            }
        }
    } else if (cmd === 'updated') {
        ggp.msgs.splice(0, 1);
        if (ggp.msgs.length > 0) {
            ggp.exe.stdin.write(ggp.msgs[0]);
        }
    } else if (cmd === 'debug') {
        console.log('debug: ' + message)
    } else if (cmd === 'stat') {
        console.log('stat: ' + message);
    }
}

function receiveClientData(client, data) {
    if (client.data_length === null) {
        var space_pos = data.indexOf(' ');
        if (space_pos != -1) {
            var length = parseInt(data.substring(0, space_pos));
            if (!isNaN(length)) {
                client.data_length = length;
                data = data.substring(space_pos + 1);
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
    //console.log('From Client ' + client.sock.remotePort + ':' + message);
    var i = message.indexOf(' ');
    var cmd = message.substring(0, i);
    var message = message.substring(i + 1);
    if (!ggp.exe) {
        client.game = null;
        client.state = null;
    } else if (client.game === ggp.game) {
        var update_state = false;
        if (cmd === 'uct') {
            var msg = 'client ' + client.state + ';' + message + '\n';
            ggp.msgs.push(msg);
            if (ggp.msgs.length === 1) {
                ggp.exe.stdin.write(ggp.msgs[0]);
            }
            update_state = true;
        } else if (cmd === 'ready') {
            if (!client.state) {
                update_state = true;
            }
        }
        if (update_state && ggp.state) {
            client.state = ggp.state;
            var msg = 'state ' + client.state;
            client.sock.write(msg.length + ' ' + msg);
        }
    } else {
        client.game = ggp.game;
        client.state = null;
        var msg = 'rule ' + ggp.role + ' ' + ggp.rule;
        client.sock.write(msg.length + ' ' + msg);
    }
}

var server = net.createServer(function (sock) {
    var client = {};
    client.sock = sock;
    client.game = null;
    client.state = null;
    client.buffer = '';
    client.data_length = null;
    clients.push(client);
    console.log('connected ' + sock.remoteAddress + ':' + sock.remotePort);
    sock.on('data', function(data) {
        for (var i = 0; i < clients.length; ++i) {
            if (clients[i].sock === sock) {
                receiveClientData(clients[i], String(data));
                break;
            }
        }
    });
    sock.on('close', function(has_err) {
        console.log('close ' + sock.remoteAddress);
        var client = null;
        for (var i = 0; i < clients.length; ++i) {
            if (clients[i].sock === sock) {
                console.log('close ' + sock.remoteAddress);
                clients.splice(i, 1);
                break;
            }
        }
    });
    if (ggp.exe) {
        client.game = ggp.game;
        var msg = 'rule ' + ggp.role + ' ' + ggp.rule;
        sock.write(msg.length + " " + msg);
    }
});

server.listen(10000);

net.createServer(function (sock) {
    sock.on('close', function() {
        for (var i = 0; i < controllers.length; ++i) {
            if (controllers[i] === sock) {
                controllers.splice(i, 1);
                break;
            }
        }
    });
    controllers.push(sock);
    console.log('controller ' + sock.remoteAddress + ' connected');
}).listen(10001);

process.on('uncaughtException', function(err) {
    console.log(util.inspect(err));
});
