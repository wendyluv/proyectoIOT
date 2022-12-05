  
if(typeof exports == "undefined"){
  exports = this;
}
var client = require('./index.js');
const mqtt = require('mqtt')
const fs = require('fs')
const { Command } = require('commander')

const program = new Command()
program
  .option('-p, --protocol <type>', 'connect protocol: mqtt, mqtts, ws, wss. default is mqtt', 'mqtt')
  .parse(process.argv)

const host = 'industrial.api.ubidots.com'
const port = '1883'
const clientId = `BBFF-e151f0d2b981b22d3548b43e1693ae9badf`

// connect options
const OPTIONS = {
  clientId,
  clean: true,
  connectTimeout: 4000,
  username: 'BBFF-rhBHNkJi0blvHmtCLAcD65AkKNVIAQ',
  password: 'BBFF-rhBHNkJi0blvHmtCLAcD65AkKNVIAQ',
  reconnectPeriod: 1000,
} 
// protocol list
const PROTOCOLS = ['mqtt', 'mqtts', 'ws', 'wss']

// default is mqtt, unencrypted tcp connection
let connectUrl = `mqtt://${host}:${port}`
const topic = 'v1.6/devices/proyecto/distancia'

client = mqtt.connect(connectUrl, OPTIONS)

client.on('connect', () => {
  console.log(`${program.protocol}: Connected`)
  
}
)
client.subscribe([topic], () => {
  console.log(`${program.protocol}: Subscribe to topic '${topic}'`)
}
)
client.publish(topic, '{  "temperatura": {"value": 15}}', { qos: 0, retain: false }, (error) => {
    if (error) {
      console.error(error)
    }
}
)
client.on('reconnect', (error) => {
  console.log(`Reconnecting(${program.protocol}):`, error)
})

client.on('error', (error) => {
  console.log(`Cannot connect(${program.protocol}):`, error)
})

client.on('message', (topic, payload) => {
  console.log('Received Message:', topic, payload.toString())
})  
 
Example = function() {
  this.init();
};

const express = require('express');
const app = express();

var http = require('http') , https = require('https')

app.use( express.static('C:/Users/gwen_/OneDrive/Documents/Iteso/IOT/mqtt/MQTT-Client-Examples-master/mqtt-client-Node.js/')) ;


app.listen(443);

exports.client = client