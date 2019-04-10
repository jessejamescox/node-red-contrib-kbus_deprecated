module.exports = function(RED) {
    "use strict";

    function tempInput(n) {
       RED.nodes.createNode(this,n);
       //var context = this.context();
       var node = this;
       var moduleNum = n.module;
       var channelNum = n.channel;
       var sensorType = n.sensorType;
       var signalType = n.signalType;  
       var outValue;     

       function toFixed( num, precision ) {
        return (+(Math.round(+(num + 'e' + precision)) + 'e' + -precision)).toFixed(precision);
        }

        this.on('input', function(msg) {
            var outputMsg = {};
            var inMsg = JSON.parse(msg.payload);
            var actualModuleNum = inMsg.module;
            var actualChannelNum = inMsg.channel;
            var rawInput = inMsg.value;


            if ((actualChannelNum == channelNum) && (actualModuleNum == moduleNum)) {
                if (signalType == "Celsius")    {
                    outValue = toFixed((rawInput / 10), 2);
                }
                if (signalType == "Farenheit")  {
                    outValue = toFixed((rawInput / 10 * (9/5) + 32), 2);              
                }    
                outputMsg.payload = parseFloat(outValue);       
                node.send(outputMsg);
            }
        });
    }
    RED.nodes.registerType("Temperature Input",tempInput);
};