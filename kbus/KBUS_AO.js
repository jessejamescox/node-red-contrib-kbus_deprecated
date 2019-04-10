module.exports = function(RED) {
    "use strict";

    function analogOutput(n) {
       RED.nodes.createNode(this,n);
       //var context = this.context();
       var node = this;
       var moduleNum = n.module;
       var channelNum = n.channel;
       var sensorType = n.sensorType;
       var resolution = n.resolution;
       var low = n.low;
       var high = n.high;
       var prec = n.precision;
       var selectedProcess = n.selectedProcess;

       // scales number
       function scale(x, i_lo, i_hi, o_lo, o_hi)    {
            var multiplier = (o_hi - o_lo) / (i_hi - i_lo);
            var scaledVal = (multiplier * limit(i_lo, x, i_hi)) + o_lo;
            return(scaledVal);
        }
        function limit(i_lo, x, i_hi){
            var last = 0;
            if (x<i_lo){
                return(i_lo);
            } else {
                if (x>i_hi){
                    return(i_hi);
                } else {
                    return(x);
                }
            }
        }
        function toFixed( num, precision ) {
        return (+(Math.round(+(num + 'e' + precision)) + 'e' + -precision)).toFixed(precision);
        }

        this.on('input', function(msg) {
            var rawInput = parseInt(msg.payload);
            var rawMinOutput = 0;
            var rawMaxOutput = 0;
            var outputMsg = {};
            var actualSensorValue;
            var val_10vdc = 0;
            var val_int16 = 0;
            var scaledHold = 0;
            var outValue = 0;

            // set the max value
            switch(resolution)  {
                case "12_Bit":
                    rawMinOutput = 0;
                    rawMaxOutput = 32767;
                    break;
                case "13_Bit":
                    rawMinOutput = 0;
                    rawMaxOutput = 32767;
                    break;
                case "13_Bit_signed":
                    rawMinOutput = -32768;
                    rawMaxOutput = 32767;
                    break;
                case "14_Bit":
                    rawMinOutput = 0;
                    rawMaxOutput = 32767;
                    break;
                case "15_Bit":
                    rawMinOutput = 0;
                    rawMaxOutput = 32767;
                    break;
                case "15_Bit_signed":
                    rawMinOutput = -32767;
                    rawMaxOutput = 32767;
                    break;
                case "16_Bit":
                    rawMinOutput = 0;
                    rawMaxOutput = 65535;
                    break;                
            }

            switch(sensorType)  {
                case "0-20mA":
                    //actualSensorValue = scale(msg.payload, 0, 20, rawMinOutput, rawMaxOutput);
                    break;
                case "4-20mA":
                    //actualSensorValue = scale(msg.payload, 4, 20, rawMinOutput, rawMaxOutput);
                    break;
                case "0-10VDC":
                    //actualSensorValue = scale(msg.payload, 0, 10, rawMinOutput, rawMaxOutput);
                    break;
                case "+/-10VDC": 
                    if (msg.payload < (high-low)/2){
                        var valX = 0;
                        valX = scale(msg.payload, low, (high-low)/2, 0, rawMaxOutput); 
                        val_10vdc = valX * -1 + 65535;
                    } else {
                        val_10vdc = scale(msg.payload, (high-low)/2, high, 0, rawMaxOutput); 
                        val_10vdc = val_10vdc + 32767;           
                    }
                    actualSensorValue = val_10vdc;   
                    break;
                case "0-30VDC":
                    //actualSensorValue = scale(msg.payload, 0, 30, rawMinOutput, rawMaxOutput);
                    break;
            }
            // operation based on processSelected
            switch(selectedProcess) {
                case "Raw":
                    outValue = msg.payload;
                    break;
                case "SensorVal":
                    outValue = actualSensorValue;
                    break;
                case "Scaled":
                    if (sensorType == "+/-10VDC"){
                        scaledHold = actualSensorValue;                        
                    } else {
                        scaledHold = scale(msg.payload, low, high, rawMinOutput, rawMaxOutput);
                    }
                    outValue = toFixed(scaledHold, 0); 
                    break;
            }
            outputMsg = {payload: {module: moduleNum, channel: channelNum, value: outValue}};
            node.send(outputMsg);
        });
    }
    RED.nodes.registerType("Analog Output",analogOutput);
};
