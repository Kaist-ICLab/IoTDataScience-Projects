'use strict';

// Server_IP는 본인 서버 실행 컴퓨터 IP 기입
const SERVER_IP = '143.248.250.70';
const SERVER_PORT = 80;

//const xlsx = require('xlsx');
const fs = require('fs');
const express = require('express');
const app = express();

// 서버를 실행 (https://143.248.109.205:80으로 접속)
const server = require('http').createServer(app);
server.listen(SERVER_PORT, function(){
  console.log("http server started");
});

// 서버에 접속한 클라이언트에게 index.html 파일을 전송하여 렌더링 해줌
app.use(express.static(__dirname));
app.get('/', function(req, res){
    res.sendFile(__dirname + '/index.html');
});

const io = require('socket.io').listen(server);

var acc_gyro_data = [];
var sound_data = [];

var collectStatus = false;
// 클라이언트가 웹페이지에 접속하였을 경우
io.on('connection', function(client){
    console.log("[LOG] Client Connected");

    // 아두이노 클라이언트에서 웹으로 데이터가 전송된 경우
    client.on('collectorT', function(data){
      var today = new Date();
      var time = today.valueOf();
      var val = {};
      val.timeStamp = Number(time);
      val.acX = data.AcX;
      val.acY = data.AcY;
      val.acZ = data.AcZ;
      val.gyX = data.GyX;
      val.gyY = data.GyY;
      val.gyZ = data.GyZ;
      io.sockets.emit('towebT', val);
    });

    client.on('collectorS', function(data){
      var today = new Date();
      var time = today.valueOf();
      var val = {};
      val.timeStamp = Number(time);
      val.sound = data.soundVal;
      io.sockets.emit('towebB', val);
    })

    // 웹 클라이언트에서 웹으로 아두이노로 전송된 경우
    client.on('fromwebT', function(data){
        //io.sockets.emit('MyServer', data);
      acc_gyro_data.push(data);
      console.log(data);
    });

    client.on('fromwebB', function(data){
      sound_data.push(data);
      console.log(sound_data.length);
    })

    client.on('saveReqT', function(){
      const makeHeader = () => new Promise((resolve, reject) => {
        try {
          const length = [];
          acc_gyro_data.forEach((item) => {
            length.push(Object.keys(item).length);
          });
          const index = length.indexOf(Math.max.apply(null, length));
          return resolve(Object.keys(acc_gyro_data[index]));
        } catch (err) {
          return reject(err);
        }
      });

      const makeBody = (header) => new Promise((resolve, reject) => {
        try {
          const body = [];
          acc_gyro_data.forEach((x) => {
            header.forEach((y) => {
              body.push(typeof x[y] !== 'undefined' ? x[y] : '');
            });
          });
          const headerLen = header.length;
          let result = header.join();
          result += '\n'; // 헤더 뒤에 줄 바꿈.
          body.forEach((item, index) => {
            result += item;
            if (Number(index + 1) % headerLen === 0) {
              result += '\n';
            } else {
              result += ',';
            }
          });
          return resolve(result);
        } catch (err) {
          return reject(err);
        }
      });

      const saveFile = (csv) => new Promise((resolve, reject) => {
        fs.writeFile('gyro.csv', csv, (err) => {
          if (err) return reject(err);
          return resolve('write success');
        });
      });

      makeHeader().then(async (header) => {
        try {
          const csv = await makeBody(header);
          const result = await saveFile(csv);
          console.log(result);
        } catch (err) {
          console.log(err);
        }
      }).catch((e) => {
        console.log(e);
      });
    })

    client.on('saveReqB', function(){
      const makeHeader = () => new Promise((resolve, reject) => {
        try {
          const length = [];
          sound_data.forEach((item) => {
            length.push(Object.keys(item).length);
          });
          const index = length.indexOf(Math.max.apply(null, length));
          return resolve(Object.keys(sound_data[index]));
        } catch (err) {
          return reject(err);
        }
      });

      const makeBody = (header) => new Promise((resolve, reject) => {
        try {
          const body = [];
          sound_data.forEach((x) => {
            header.forEach((y) => {
              body.push(typeof x[y] !== 'undefined' ? x[y] : '');
            });
          });
          const headerLen = header.length;
          let result = header.join();
          result += '\n'; // 헤더 뒤에 줄 바꿈.
          body.forEach((item, index) => {
            result += item;
            if (Number(index + 1) % headerLen === 0) {
              result += '\n';
            } else {
              result += ',';
            }
          });
          return resolve(result);
        } catch (err) {
          return reject(err);
        }
      });

      const saveFile = (csv) => new Promise((resolve, reject) => {
        fs.writeFile('sound.csv', csv, (err) => {
          if (err) return reject(err);
          return resolve('write success');
        });
      });

      makeHeader().then(async (header) => {
        try {
          const csv = await makeBody(header);
          const result = await saveFile(csv);
          console.log(result);
        } catch (err) {
          console.log(err);
        }
      }).catch((e) => {
        console.log(e);
      });
    })
});
