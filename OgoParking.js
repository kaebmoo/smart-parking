var parkingLot = new Array();

var app = angular.module("schoolofrock", []);
app.controller("parkingController", function($scope, $http, $window) {

  $scope.Available = "Hello World...";

  client = new Paho.MQTT.Client("db.ogonan.com", Number(8083), "/wss", "ogoparking_"+parseInt(Math.random() * 100,10));
  var options = {
    useSSL: true,
    cleanSession: true,
    onSuccess: onConnect,
    userName : "seal",
    password : "sealwiththekiss"
  };

  client.onConnectionLost = onConnectionLost;
  client.onMessageArrived = onMessageArrived;
  if (!client.isConnected) {
    alert("Problem connection , kindly contact system admin .");
    document.getElementById("mqttstatus").innerHTML = "MQTT Status: <mqttdisconnect>Disconnect</mqttdisconnect>";
    $('#reconnectButton').toggleClass('hidden');
  }
  // connect the client
  client.connect(options);

  // called when the client connects
  function onConnect() {
    // Once a connection has been made, make a subscription and send a message.
    console.log("onConnect");
    document.getElementById("mqttstatus").innerHTML = "MQTT Status: <mqttconnected>Connected</mqttconnected>";
    $('#reconnectButton').toggleClass('hidden');
    //angular.element(document.querySelector('#reconnectButton')).addClass('disabled');


    //client.subscribe("room/+/timer");
    client.subscribe("sensor/+/status");

  }

  // called when the client loses its connection
  function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
      console.log("onConnectionLost:"+responseObject.errorMessage);
      document.getElementById("mqttstatus").innerHTML = "MQTT Status: <mqttdisconnect>Disconnect</mqttdisconnect>";
      $('#reconnectButton').toggleClass('hidden');
    }
  }

  function onMessageArrived(message) {
    var topic;
    var sensorNumber;
    var accumulation;

    topic = message.topic;
    //console.log(topic);
    console.log("onMessageArrived: " + message.topic + " " + message.payloadString);

    if(topic.search("status") != -1) {
      sensorNumber = topic.match(/\d+/)[0].toString();
      console.log("Lot: " + sensorNumber);
      if (message.payloadString == 0) {
        parkingLot[sensorNumber] = 0;
      }
      else if(message.payloadString == 1) {
        parkingLot[sensorNumber] = 1;
      }

      // https://stackoverflow.com/questions/6120931/how-to-count-the-number-of-certain-element-in-an-array
      console.log("countAvailable: " + parkingLot.filter(i => i === 0).length);
      $scope.Available = 'จำนวนที่จอดว่าง: ' + parkingLot.filter(i => i === 0).length;
      console.log("Available: " + $scope.Available);
      $scope.$apply();
    }
  }


});
