var net = require('net');
var spawn = require('child_process').spawn;

var ip = '162.105.81.73';
var port = 10001;
var n = 20;
var clients = [];

server = net.Socket();
server.connect(port, ip);
server.on('data', function(data) {
    killClients();
    clients = [];
    for (var i = 0; i < n; ++i) {
        var client = spawn('./ggp');
        clients.push(client);
    }
});

server.on('close', function () {
    killClients();
});

function killClients() {
    for (var i = 0; i < clients.length; ++i) {
        clients[i].kill('SIGKILL');
    }
}

