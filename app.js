var express  = require('express');
var app      = express();
var http     = require('http').Server(app);
var io       = require('socket.io')(http);

var SerialPort = require('serialport');
var parsers    = SerialPort.parsers;
var parser     = new parsers.Readline({
  delimiter: '\r\n'
});

var sp = new SerialPort('COM4', {
  baudRate: 9600
});

var port = 3000;

sp.pipe(parser);

sp.on('open', () => console.log('Port open'));

parser.on('data', function(data)
{
	if(data.substring(0,4) == "dist"){
		distance = data.substring(4);
		io.emit('dist', distance);
		console.log('distance: ', distance);
	}
	else{
		distance = data;
		io.emit('dist', distance);
		console.log(distance);
	}
	
});

app.get('/security_on',function(req,res)
{
	sp.write('sec1\n\r', function(err){
		if (err) {
            return console.log('Error on write: ', err.message);
        }
        console.log('send: security mode on');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('\n');
	});
});

app.get('/security_off',function(req,res)
{
	sp.write('sec0\n\r', function(err){
		if (err) {
            return console.log('Error on write: ', err.message);
        }
        console.log('send: security mode off');
		res.writeHead(200, {'Content-Type': 'text/plain'});
		res.end('\n');
	});
});

app.use(express.static(__dirname + '/public'));

http.listen(port, function() {  // server.listen(port, function() {
    console.log('listening on *:' + port);
});