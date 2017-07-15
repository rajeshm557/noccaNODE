var express = require('express');
var router = express.Router();
var util = require('util');
var SerialPort = require('serialport');
var port = new SerialPort('/dev/ttyUSB0', {
  baudRate: 115200
});

var packetStarted = false;
var packetInfo = {
	length: 0,
	packetType: 0,
	macAddr: "",
	data: "",
	timestamp: 0
};
var newFrame144 = false;
var newFrame151 = false;

//console.log(util.inspect(packetInfo));

port.on('readable', function () {
	var tStamp = Date.now();
	var buff = port.read();
	var index = 0;
	if(buff.toString('ascii', index, ++index) == '~' && !packetStarted){
		packetStarted = true;
	}	
	index++;
	if(packetInfo.timestamp != tStamp && packetStarted){
		packetInfo.length = parseInt(buff.toString('hex', index, ++index), 16);
	}
	if(packetInfo.timestamp != tStamp || (packetInfo.packetType == 0 && packetInfo.length != 0 && packetStarted)){
		packetInfo.packetType = parseInt(buff.toString('hex', index, ++index), 16);
		if(packetInfo.packetType == 151){
			index++;
		}	
	}	
	if(packetInfo.timestamp != tStamp || (packetInfo.macAddr == '' && packetInfo.packetType != 0 && packetInfo.length != 0 && packetStarted)){
		packetInfo.macAddr = buff.toString('hex', index, index + 8).toUpperCase();
		index = index + 10;
		if(packetInfo.packetType == 144){
			index++;
		}
	}	
	if(packetInfo.timestamp != tStamp || (packetInfo.data == '' && packetInfo.macAddr != '' && packetInfo.packetType != 0 && packetInfo.length != 0 && packetStarted)){
		packetInfo.data = buff.toString('ascii', index, buff.length - 1);
	}
	if(packetInfo.timestamp != tStamp){
		packetInfo.timestamp = tStamp;
	}
	if(packetStarted){
		packetStarted = false;
	}	
	if(packetInfo.packetType == 151){
		newFrame151 = true;
		newFrame144 = false;
	}else if(packetInfo.packetType == 144){
		newFrame144 = true;
		newFrame151 = false;
	}
	console.log(packetInfo);
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
		newFrame151 = false;
		setTimeout(function() {
			port.drain(function(errDrain){
					if (errDrain) {
						return console.log('Error on drain: ', errDrain.message);
					}
					console.log('Message drained.');
					var t0 = Date.now();
					var timer = setInterval(function(){
						if((Date.now() - t0) > 2000){
							clearInterval(timer);
							res.send('no confirmation received from robot');
						}else if(newFrame151){
							newFrame151 = false;
							clearInterval(timer);
							res.send(packetInfo);
						}	
					}, 5);		
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
		newFrame151 = false;
		setTimeout(function() {
			port.drain(function(errDrain){
					if (errDrain) {
						return console.log('Error on drain: ', errDrain.message);
					}
					console.log('Message drained.');
					var t0 = Date.now();
					var timer = setInterval(function(){
						if((Date.now() - t0) > 2000){
							clearInterval(timer);
							res.send('no confirmation received from robot');
						}else if(newFrame151){
							newFrame151 = false;
							clearInterval(timer);
							res.send(packetInfo);
						}	
					}, 5);	
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
	port.write(b, 'hex', function(err) {
		if (err) {
			res.send('Error on write: ', err.message);
			return console.log('Error on write: ', err.message);
		}
		console.log('message written');
		newFrame151 = false;
		setTimeout(function() {
			port.drain(function(errDrain){
					if (errDrain) {
						return console.log('Error on drain: ', errDrain.message);
					}
					console.log('Message drained.');
					var t0 = Date.now();
					var timer = setInterval(function(){
						if((Date.now() - t0) > 2000){
							clearInterval(timer);
							res.send('no confirmation received from robot');
						}else if(newFrame151){
							newFrame151 = false;
							clearInterval(timer);
							res.send(packetInfo);
						}	
					}, 5);	
				});
		}, 3);
	});
});

router.post('/readSerial', function(req, res) {
	var t0 = Date.now();
	var timer = setInterval(function(){
		if((Date.now() - t0) > 2000){
			clearInterval(timer);
			res.send('no data received from robot');
		}else if(newFrame144){
			newFrame144 = false;
			clearInterval(timer);
			res.send(packetInfo);
		}	
	}, 5);	
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
