const Server = require("node-osc").Server;
const easymidi = require("easymidi");

const portName = "IAC Driver Bus 1";
const oscPort = 9001;
const controller = 1;
const channel = 1;

const output = new easymidi.Output(portName);

var oscServer = new Server(oscPort, "127.0.0.1", () => {
  console.log("OSC Server is listening");
});

oscServer.on("message", function (msg) {
  console.log(`Message: ${msg}`);
  console.log(msg.split(","));
  //   TODO: map value
  output.send("cc", {
    controller,
    value: 1,
    channel,
  });
});
