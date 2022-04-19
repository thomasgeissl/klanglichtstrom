const Server = require("node-osc").Server;
const easymidi = require("easymidi");
const portName = "IAC Driver Bus 1";
const output = new easymidi.Output(portName);

output.send("noteon", {
  note: 64,
  velocity: 127,
  channel: 3,
});

var oscServer = new Server(9001, "0.0.0.0", () => {
  console.log("OSC Server is listening");
});

oscServer.on("message", function (msg) {
  console.log(`Message: ${msg}`);
  output.send("cc", {
    controller: 1,
    value: 1,
    channel: 1,
  });
});
