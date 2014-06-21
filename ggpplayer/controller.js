var net = require('net');
var spawn = require('child_process').spawn;

var ip = '162.105.81.73';
var port = 10001;
var n = 22;
var clients = [];

server = net.Socket();
server.connect(port, ip);
server.on('connect', function() {
    killClients();
    startClients();
});
server.on('data', function(data) {
    killClients();
    startClients();
});

server.on('close', function () {
    killClients();
});

function startClients() {
    clients = [];
    for (var i = 0; i < n; ++i) {
        var client = spawn('./ggp');
        client.on('exit', function () {
            console.log('client exit');
        });
        client.stderr.on('data', function (data) {
        });
        clients.push(client);
    }
}

function killClients() {
    for (var i = 0; i < clients.length; ++i) {
        clients[i].kill('SIGKILL');
    }
}

