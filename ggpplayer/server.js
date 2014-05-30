var http = require('http');
var util = require('util');
var fs = require('fs');
var spawn = require('child_process').spawn;

var ggp_exe;

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
                ggp_exe = spawn('./ggp');
                ggp_exe.stdout.once('data', function (data) {
                    data = String(data);
                    console.log('ggp out: ' + data);
                    var end = data[data.length - 1];
                    if (end === '\r' || end === '\n') {
                        res.end(data.substring(0, data.length - 1));
                    } else {
                        res.write(data);
                    }

                });
                ggp_exe.on('close', function(code) {
                    ggp_exe = null;
                });
                ggp_exe.stdin.write(request.role + '\n');
                ggp_exe.stdin.write(request.playclock + '\n');
            });
            break;
        case 'play' :
            if (ggp_exe) {
                console.log('ggp in: ' + request.move);
                ggp_exe.stdout.once('data', function (data) {
                    data = String(data);
                    console.log('ggp out: ' + data);
                    var end = data[data.length - 1];
                    if (end === '\r' || end === '\n') {
                        res.end(data.substring(0, data.length - 1));
                    } else {
                        res.write(data);
                    }
                });
                ggp_exe.stdin.write(request.move + '\n');
            }
            break;
        case 'stop' :
            ggp_exe = null;
            break;
        case 'abort' :
            break;
        };
//        console.log(JSON.stringify(request));        
    });
}).listen(80);

process.on('uncaughtException', function(err) {
    console.log(util.inspect(err));
});
