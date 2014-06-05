var net = require('net');

var server = net.createServer(function (sock) {
    var data = '';
    sock.on('data', function(chunk) {
        data += chunk;
        sock.write('hello');
    });
    sock.on('end', function() {
        console.log(data);
    });
});

server.listen(10000);
