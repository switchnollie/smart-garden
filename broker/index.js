const aedes = require('aedes')();
const server = require('net').createServer(aedes.handle);
const logging = require('aedes-logging');
const port = 1883;

logging({ instance: aedes, servers: [server] });

server.listen(port, "0.0.0.0", () => {
  console.log('broker started and listening on port ', port)
})