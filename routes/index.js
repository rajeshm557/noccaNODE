var express = require('express');
var router = express.Router();
var util = require('util')
var SerialPort = require('serialport');
var port = new SerialPort('/dev/ttyUSB0', {
  baudRate: 115200
});
var macAddr = ["000000000000FFFF", "0013A2004156586B"];
var macSum = [];

for (var i = 0; i < macAddr.length; i++){
	macSum.push(macHex(macAddr[i]));
}


/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('home');
});

router.post('/home', function(req, res) {
  res.send("home no");
});

router.post('/on', function(req, res) {
	var b = '7E00101701' + macAddr[req.body['bot']] + 'FFFE02443604';
	var sum = 0x17 + 0x01 + macSum[req.body['bot']] + 0xFF + 0xFE + 0x02 + 0x44 + 0x36 + 0x04;
	sum = 0xFF - (sum & 0xFF);
	sum = sum.toString(16);
	b += sum;
	port.write(b, 'hex', function(err) {
		if (err) {
			res.send('Error on write: ', err.message);
			return console.log('Error on write: ', err.message);
		}
		console.log('message written');
		setTimeout(function() {
			port.drain(function(errDrain){
					if (errDrain) {
						return console.log('Error on drain: ', errDrain.message);
					}
					console.log('Message drained.');
					setTimeout(function() {
						res.send(b);
					}, 3);
				});
		}, 3);
	});
});

router.post('/off', function(req, res) {
	var b = '7E00101701' + macAddr[req.body['bot']] + 'FFFE02443605';
	var sum = 0x17 + 0x01 + macSum[req.body['bot']] + 0xFF + 0xFE + 0x02 + 0x44 + 0x36 + 0x05;
	sum = 0xFF - (sum & 0xFF);
	sum = sum.toString(16);
	b += sum;
	port.write(b, 'hex', function(err) {
		if (err) {
			res.send('Error on write: ', err.message);
			return console.log('Error on write: ', err.message);
		}
		console.log('message written');
		setTimeout(function() {
			port.drain(function(errDrain){
					if (errDrain) {
						return console.log('Error on drain: ', errDrain.message);
					}
					console.log('Message drained.');
					setTimeout(function() {
						res.send(b);
					}, 3);
				});
		}, 3);
	});
});

router.post('/emgStop', function(req, res) {
	var b = '7E00101701' + macAddr[req.body['bot']] + 'FFFE02443305';
	var sum = 0x17 + 0x01 + macSum[req.body['bot']] + 0xFF + 0xFE + 0x02 + 0x44 + 0x33 + 0x05;
	sum = 0xFF - (sum & 0xFF);
	sum = sum.toString(16);
	b += sum;
	console.log(b);
	port.write(b, 'hex', function(err) {
		if (err) {
			res.send('Error on write: ', err.message);
			return console.log('Error on write: ', err.message);
		}
		console.log('message written');
		setTimeout(function() {
			port.drain(function(errDrain){
					if (errDrain) {
						return console.log('Error on drain: ', errDrain.message);
					}
					console.log('Message drained.');
					setTimeout(function() {
						res.send(b);
					}, 3);
				});
		}, 3);
	});
});

function closeSerial(err){
	if (err) {
		return console.log('Error on close: ', err.message);
	}
	console.log('Port closed.');
}	

function macHex(str){
	var sum = 0;
	for (var i = 0; i < str.length; i += 2){
		sum += parseInt(str.substr(i, 2), 16);
	}	
	return sum;
}	

module.exports = router;
