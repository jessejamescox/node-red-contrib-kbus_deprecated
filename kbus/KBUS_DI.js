module.exports = function(RED) {
    "use strict";

    function digitalInput(n) {
       RED.nodes.createNode(this,n);
       var node = this;
       var moduleNum = n.module
       var channelNum = n.channel
       //var bitSize = parseInt(n.outputs);
       //var bitOffset = n.offset;

        this.on('input', function(msg) {
            var inMsg = JSON.parse(msg.payload);
            var actualModuleNum = inMsg.module;
            var actualChannelNum = inMsg.channel;
            var rawInput = inMsg.value;

            if ((actualChannelNum == channelNum) && (actualModuleNum == moduleNum)) {
              var o = {};
              o.payload = rawInput;
              node.send(o);
          }
        });
    }
    RED.nodes.registerType("Digital Input",digitalInput);
};
