var express = require('express');
var LatLon = require('geodesy').LatLonEllipsoidal;
var vec3D = require('geodesy').Vector3d;
var util = require('util');
var SerialPort = require('serialport');

var router = express.Router();
var port = new SerialPort('/dev/ttyUSB0', {
  baudRate: 115200
});

const macAddr = ["000000000000FFFF", "0013A2004156586B"];

const rows = {
	A : { p1:[0, 0], p2:[0, 180]},
	B : { p1:[34, 56], p2:[36, 50]}
};

var rowAp1 = (new LatLon(rows.A.p1[0], rows.A.p1[1])).toCartesian();
var rowAp2 = (new LatLon(rows.A.p2[0], rows.A.p2[1])).toCartesian();
var pt = (new LatLon(0, 1)).toCartesian();

var dist = pt.minus(rowAp1).cross(pt.minus(rowAp2)).length()/rowAp1.minus(rowAp2).length();

console.log(util.inspect(dist));

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

var macSum = [];

for (var i = 0; i < macAddr.length; i++){
	macSum.push(macHex(macAddr[i]));
}

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('home');
});

router.post('/on', function(req, res) {
	sendRemoteATcmd(req.body['bot'], '36', '04', function(data){
		res.send(data);
	});
});

router.post('/off', function(req, res) {
	sendRemoteATcmd(req.body['bot'], '36', '05', function(data){
		res.send(data);
	});
});

router.post('/emgStop', function(req, res) {
	sendRemoteATcmd(req.body['bot'], '33', '05', function(data){
		res.send(data);
	});
});
router.post('/reset', function(req, res) {
	sendRemoteATcmd(req.body['bot'], '33', '04', function(data){
		sendRemoteATcmd(req.body['bot'], '36', '05', function(data){
			res.send(data);
		});	
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

function sendRemoteATcmd(botID, pinNo, atr, callBack){
	var b = '7E00101701' + macAddr[botID] + 'FFFE0244' + pinNo + atr;
	var sum = 0x17 + 0x01 + macSum[botID] + 0xFF + 0xFE + 0x02 + 0x44 + parseInt(pinNo, 16) + parseInt(atr, 16);
	sum = 0xFF - (sum & 0xFF);
	sum = sum.toString(16);
	b += sum;
	port.write(b, 'hex', function(err) {
		if (err) {
			console.log('Error on write: ', err.message);
			callBack('Error on write: ', err.message);
		}else {
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
								callBack('no confirmation received from robot');
							}else if(newFrame151){
								newFrame151 = false;
								clearInterval(timer);
								callBack(packetInfo);
							}	
						}, 5);	
					});
			}, 3);
		}	
	});
}	

function macHex(str){
	var sum = 0;
	for (var i = 0; i < str.length; i += 2){
		sum += parseInt(str.substr(i, 2), 16);
	}	
	return sum;
}	

module.exports = router;
