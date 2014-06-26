var net = require('net');
var spawn = require('child_process').spawn;
var exec = require('child_process').exec;

var ip = '162.105.81.43';
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
    data = String(data);
    if (data === 'start') {
        killClients();
        startClients();
    } else if (data === 'reset') {
        killClients();
        setTimeout(function() {
            child = exec('git pull; make', {}, function() {
                startClients();
            });
        }, 1000);
    }
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
        client.stdout.on('data', function (data) {
        });
        client.stderr.on('data', function (data) {
        });
        clients.push(client);
        console.log('client start');
    }
}

function killClients() {
    for (var i = 0; i < clients.length; ++i) {
        clients[i].kill('SIGKILL');
    }
}

