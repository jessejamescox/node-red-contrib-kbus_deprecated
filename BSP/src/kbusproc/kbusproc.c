//-----------------------------------------------------------------------------
//  Copyright (c) Jesse Cox 2019
//
//  PROPRIETARY RIGHTS are involved in the subject matter of this material.
//  All manufacturing, reproduction, use and sales rights pertaining to this
//  subject matter are governed by the license agreement. The recipient of this
//  software implicitly accepts the terms of the license.  No warranty expressed
//	or implied.  
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
///  \file     	kbusproc.c
///
///  \version  	$Id: 0.0.99
///
///  \brief    	Interface for KBus to work with MQTT messaging.  Designed for Node-Red 
///				interface         
///
///  \author   	Jesse Cox
//----------------------------------------------------------------------------- 

#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h>
#include <mosquitto.h>

//-----------------------------------------------------------------------------
// include files for KBUS WAGO ADI
//-----------------------------------------------------------------------------
#include <dal/adi_application_interface.h>
//-----------------------------------------------------------------------------
// include files for kbus information via dbus
//-----------------------------------------------------------------------------
#include <ldkc_kbus_information.h>          
#include <ldkc_kbus_register_communication.h> 

#define SUBTOPIC_OUTPUT "pd_out"
#define SUBTOPIC_INPUTS "pd_in"
#define MQTT_HOSTNAME "127.0.0.1"
#define MQTT_PORT 1883

struct ioModule 
{
  char    *   type;
  int         numOfChannels;
  int         byteOffsetIn;
  int         byteOffsetOut;
  int         bitOffsetOut;
  uint16_t    channelData[8];
  bool        digChannelData[16];
};

// instances of the IO modules 
struct ioModule aModules[64];
struct ioModule aModulesOld[64];

// vars for ADI-interface
tDeviceInfo deviceList[10];           // the list of devices given by the ADI i.e. kbus, CAN, RS
size_t nrDevicesFound;                // number of devices found
size_t nrKbusFound;                   // position of the kbus in the list
tDeviceId kbusDeviceId;               // device ID from the ADI
tApplicationDeviceInterface * adi;    // pointer to the application interface
tApplicationStateChangedEvent event;  // var for the event interface of the ADI
size_t terminalCount;
u16 terminals[LDKC_KBUS_TERMINAL_COUNT_MAX];
tldkc_KbusInfo_TerminalInfo terminalDescription[LDKC_KBUS_TERMINAL_COUNT_MAX];
  
uint32_t taskId = 0;                  // task Id 

char * bufSocketRxData; // Rx Data from the MQTT 



// callback for the MQTT subscribe
void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){
	int ucAIndex = 0;
  
	// json vars
  	struct json_object* jsonRaw;
  	enum json_type jsType;

	// vars
  	int i = 0;
  	int numOfModules;
  	uint8_t junk = 255;

	// received data
  	bufSocketRxData = (char *)message->payload;
  
	// parse the incoming MQTT message  	
	jsonRaw = json_tokener_parse(bufSocketRxData);

	// get the module position
  	int desiredModulePosition = json_object_get_int(json_object_object_get(jsonRaw, "module"));
	int actualTargetModule = (desiredModulePosition - 1);

	// get the channel position
  	int desiredChannelPosition = json_object_get_int(json_object_object_get(jsonRaw, "channel"));
  	int actualChannelPosition = (desiredChannelPosition - 1);
  	
	// get the data type of the received message value feild
	jsType = json_object_get_type(json_object_object_get(jsonRaw, "value"));

	// analog module
  	if (jsType == 3){
		
		// conver the value from json to int
    	int desiredChannelValueInt = json_object_get_int(json_object_object_get(jsonRaw, "value"));
    	
		// write the kbus
		adi->WriteStart(kbusDeviceId, taskId); // lock PD-out data
    	adi->WriteBytes(kbusDeviceId, taskId, (aModules[actualTargetModule].byteOffsetOut + (desiredChannelPosition * 2)), 2, (uint16_t*) &desiredChannelValueInt);
    	adi->WriteEnd(kbusDeviceId, taskId); // unlock PD-out data
  	}

  	if (jsType == 1){

		// read the value and convert to int (0 or 1)
    	int desiredChannelValueBool = json_object_get_boolean(json_object_object_get(jsonRaw, "value"));
    	
		// set the bool state
		bool hoildBool;
		if (desiredChannelValueBool > 0){
      		hoildBool = true;
    	}
	    else{
      		hoildBool = false;
    	}

		// write the kbus    
		adi->WriteStart(kbusDeviceId, taskId); // lock PD-out data
    	adi->WriteBool(kbusDeviceId, taskId, aModules[actualTargetModule].bitOffsetOut + actualChannelPosition, hoildBool);
    	adi->WriteEnd(kbusDeviceId, taskId); // unlock PD-out data
  }
}

int main(void){

	// generic vars
	int i = 0;

  	// mosquitto iunstance
  	struct mosquitto *mosq = NULL;

  	char * chArray2 = 	"[0,0]";
  	char * chArray4 = 	"[0,0,0,0]";
  	char * chArray8 = 	"[0,0,0,0,0,0,0,0]";
  	char * chArray16 = 	"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]";

  	// connect to ADI-interface
  	adi = adi_GetApplicationInterface();

  	// init interface
  	adi->Init();

  	// scan devices
	adi->ScanDevices();
  	adi->GetDeviceList(sizeof(deviceList), deviceList, &nrDevicesFound);

  	// find kbus device
  	nrKbusFound = -1;
  	for (i = 0; i < nrDevicesFound; ++i){
    	if (strcmp(deviceList[i].DeviceName, "libpackbus") == 0){
      	nrKbusFound = i;
    	}
  	}

  	// kbus not found > exit
  	if (nrKbusFound == -1){
    	printf("No KBUS device found \n");    
    	adi->Exit(); // disconnect ADI-Interface    
    	return -1; // exit program
  	}

  	// open kbus device 
  	kbusDeviceId = deviceList[nrKbusFound].DeviceId;
  	if (adi->OpenDevice(kbusDeviceId) != DAL_SUCCESS){
    	printf("Kbus device open failed\n");
    	adi->Exit(); // disconnect ADI-Interface    
    	return -2; // exit program
  	}

	// set application state to "Running".
    event.State = ApplicationState_Running;   
    if (adi->ApplicationStateChanged(event) != DAL_SUCCESS){
        // Set application state to "Running" failed
        printf("Set application state to 'Running' failed\n");  
    	adi->CloseDevice(kbusDeviceId); // close kbus device    
      	adi->Exit(); // disconnect ADI-Interface      
      	return -3; // exit programm
    }
  
  //*** get kbus config via dbus
    if (KbusInfo_Failed == ldkc_KbusInfo_Create()){
    	printf(" ldkc_KbusInfo_Create() failed\n");
    	adi->CloseDevice(kbusDeviceId); // close kbus device    
      	adi->Exit(); // disconnect ADI-Interface 
    	return -11;
  	}
  
  	// get status 
  	tldkc_KbusInfo_Status status;
  	if (KbusInfo_Failed == ldkc_KbusInfo_GetStatus(&status)){
		printf(" ldkc_KbusInfo_GetStatus() failed\n");
    	adi->CloseDevice(kbusDeviceId); // close kbus device    
    	adi->Exit(); // disconnect ADI-Interface 
    	ldkc_KbusInfo_Destroy();
    	return -12;
  	}

	// terminal count and position vars
    unsigned char ucPosition;
    unsigned char ucIndex;
    unsigned char ucCntOfTrmnlType;
    unsigned char ucMaxPosition;

	// get the terminal descriptions from the kbus registration
    if ( KbusInfo_Failed == ldkc_KbusInfo_GetTerminalInfo(OS_ARRAY_SIZE(terminalDescription), terminalDescription, &terminalCount)){
    	printf(" ldkc_KbusInfo_GetTerminalInfo() failed\n");
    	adi->CloseDevice(kbusDeviceId); // close kbus device    
      	adi->Exit(); // disconnect ADI-Interface 
    	ldkc_KbusInfo_Destroy();
    	return -13;
  	}

	// get the kbus size and offsets    
	if ( KbusInfo_Failed == ldkc_KbusInfo_GetTerminalList(OS_ARRAY_SIZE(terminals), terminals, NULL)){
    	printf(" ldkc_KbusInfo_GetTerminalList() failed\n");
    	adi->CloseDevice(kbusDeviceId); // close kbus device    
      	adi->Exit(); // disconnect ADI-Interface 
    	ldkc_KbusInfo_Destroy(); 
        return -14;
    }
    
	// set the termial counts and reset position	
	ucPosition    = 1;
    ucMaxPosition = terminalCount;

    for (ucIndex = 0; ucPosition <= ucMaxPosition; ucPosition++, ucIndex++){
		
		// vars        
		char * chHoldDataArray;
        const u32 idx = ucPosition - 1;

		if( 0 == (terminals[idx] & 0x8000) ){
        
		// compare second part of order number in "dec" format for complex IO-Terminals
        if (terminals[idx] < 500 && terminals[idx] > 400){
			// set the object data        	
			aModules[idx].type = "ai";
          	aModules[idx].numOfChannels = terminalDescription[idx].AdditionalInfo.ChannelCount; 
          	aModules[idx].byteOffsetOut = NULL;
          	aModules[idx].byteOffsetIn = (terminalDescription[idx].OffsetInput_bits / 8);
        }
        else if (terminals[idx] < 600 && terminals[idx] > 500){
         	// set the object data 
			aModules[idx].type = "ao";
          	aModules[idx].numOfChannels = terminalDescription[idx].AdditionalInfo.ChannelCount; 
          	aModules[idx].byteOffsetOut = (terminalDescription[idx].OffsetOutput_bits / 8);
          	aModules[idx].byteOffsetIn = NULL;
        }
        else if (terminalDescription[idx].SizeOutput_bits != 0 && terminalDescription[idx].SizeInput_bits != 0){
			// set the object data          	
			aModules[idx].type = "sp"; 
          	aModules[idx].numOfChannels = terminalDescription[idx].AdditionalInfo.ChannelCount; 
          	aModules[idx].byteOffsetOut = (terminalDescription[idx].OffsetOutput_bits / 8);
          	aModules[idx].byteOffsetIn = terminalDescription[idx].OffsetInput_bits / 8;
        }
      }
      else{
        // update the channel cout on the digital modules
        if (terminalDescription[idx].SizeOutput_bits != 0){
          	// set the object data
			aModules[idx].type = "do";
          	aModules[idx].numOfChannels = terminalDescription[idx].SizeOutput_bits; 
          	terminalDescription[idx].AdditionalInfo.ChannelCount = terminalDescription[idx].SizeOutput_bits;
          	aModules[idx].byteOffsetOut = (terminalDescription[idx].OffsetOutput_bits / 8);
          	aModules[idx].bitOffsetOut = terminalDescription[idx].OffsetOutput_bits;
        }
        if (terminalDescription[idx].SizeInput_bits != 0){
          	// set the object data
			aModules[idx].type = "di";
          	aModules[idx].numOfChannels = terminalDescription[idx].SizeInput_bits;
          	terminalDescription[idx].AdditionalInfo.ChannelCount = terminalDescription[idx].SizeInput_bits;
          	aModules[idx].byteOffsetIn = (terminalDescription[idx].OffsetInput_bits / 8); 
        }
      }
      
      switch(terminalDescription[idx].AdditionalInfo.ChannelCount){
      	case 2 :
        	chHoldDataArray = chArray2;
          	break;
        case 4 :
          	chHoldDataArray = chArray4;
          	break;
        case 8 :
          	chHoldDataArray = chArray8;
          	break;
        case 16 :
          	chHoldDataArray = chArray16;
          	break;
      }

    }// for - module

	// creat the loop for testing
    int loops = 0;

    // initialize the mosquitto library
    mosquitto_lib_init();

    // new mosquitto instance 
    mosq = mosquitto_new (NULL, true, NULL);
    if (!mosq)
    {
      printf (stderr, "Can't initialize Mosquitto library\n");
      exit (-1);
    }
    sleep(1);

/**********************************************************************************************/
/**********************************************************************************************/
//******************* MAIN LOOP ***************************************************************/
/**********************************************************************************************/    
/**********************************************************************************************/
    while(1){
    	uint32_t retval = 0;
      
      	// use function "libpackbus_Push" to trigger one KBUS cycle.
		if (adi->CallDeviceSpecificFunction("libpackbus_Push", &retval) != DAL_SUCCESS){
		  // CallDeviceSpecificFunction failed
		  printf("CallDeviceSpecificFunction failed\n");  
		  adi->CloseDevice(kbusDeviceId); // close kbus device    
		  adi->Exit(); // disconnect ADI-Interface      
		  return -4; // exit programm
		}
		
		if (retval != DAL_SUCCESS){
			// Function 'libpackbus_Push' failed
		    printf("Function 'libpackbus_Push' failed\n");  
			adi->CloseDevice(kbusDeviceId); // close kbus device    
		  	adi->Exit(); // disconnect ADI-Interface      
		  	return -5; // exit programm
		}

		// Trigger Watchdog
		adi->WatchdogTrigger();

		if (loops == 0){
		    // Establish a connection to the MQTT server. Do not use a keep-alive ping
		    int ret = mosquitto_connect (mosq, MQTT_HOSTNAME, MQTT_PORT, 0);
		    if (ret){
		    //printf (stderr, "Can't connect to Mosquitto server\n");
		    //exit (-1);
		    }

		    int sub = mosquitto_subscribe (mosq, NULL, SUBTOPIC_OUTPUT, 0);
		    if (sub){
		      printf("Problem Subscribing");
		    }
		  }

		  // Specify the function to call when a new message is received
		  mosquitto_message_callback_set (mosq, my_message_callback);

		  // Wait for new messages
		  char * mosq_loop = mosquitto_loop(mosq, -1, 1);

		  // GetTerminalInfo
		  unsigned char ucPosition;
		  unsigned char ucIndex;
		  unsigned char ucCntOfTrmnlType;
		  unsigned char ucMaxPosition;

		  unsigned char charucChanPosition;
		  unsigned char chIndex;
		  unsigned char ucMaxChannels;
		  unsigned char ucChanPosition;

		  ucPosition    = 1;
		  ucMaxPosition = terminalCount;

		// loop the read/write data for each module
		for (ucIndex = 0; ucPosition <= ucMaxPosition; ucPosition++, ucIndex++){
		  	
			// vars
			char * sendString;
		  	const u32 idx = ucPosition - 1;
	      	const u32 ucMaxChannels = aModules[idx].numOfChannels;

			// reset data
			chIndex = 0;
		  	ucChanPosition = 1;

		      /***************** ANALOG AND SPEC MODULES *********************/
			if ( 0 == (terminals[idx] & 0x8000)){ 
		        
				/***** ANALOG INPUTS *****/
		        if (terminals[idx] < 500 && terminals[idx] > 400){
							          
					// reset the position data
					chIndex = 0;
		          	ucChanPosition = 1;
		          	
					// read inputs  
		          	adi->ReadStart(kbusDeviceId, taskId);   // lock PD-In data 
		         	adi->ReadBytes(kbusDeviceId, taskId, aModules[idx].byteOffsetIn, (aModules[idx].numOfChannels * 2), (uint16_t *) &aModules[idx].channelData[0]);
		         	adi->ReadEnd(kbusDeviceId, taskId); // unlock PD-In data 
		         	 
					for (chIndex = 0; ucChanPosition <= ucMaxChannels; chIndex ++, ucChanPosition++){
		         	   	if (aModules[idx].channelData[chIndex] != aModulesOld[idx].channelData[chIndex]){
		              	
						// vars
						struct json_object* jsonSend;
		              	char *sendString = "";
						
						// build the json object for the output message		              
						jsonSend = json_object_new_object();
		              	json_object_object_add(jsonSend, "module", json_object_new_int(idx + 1));
		              	json_object_object_add(jsonSend, "channel", json_object_new_int(chIndex + 1));
		              	json_object_object_add(jsonSend, "value", json_object_new_int(aModulesOld[idx].channelData[chIndex]));
						
						// convert the json to stirng for send			              	
						sendString = json_object_to_json_string(jsonSend);
		              	
						// send the string to broker						
						mosquitto_publish(mosq, NULL, SUBTOPIC_INPUTS, strlen(sendString), sendString, 0, 0);
		              
						// update the comparison values
						aModulesOld[idx].channelData[chIndex] = aModules[idx].channelData[chIndex];
		            }
		          }//for
		        }

		       	if (terminals[idx] < 600 && terminals[idx] > 500){
		          	chIndex = 0;
		          	ucChanPosition = 1;
		          	
					// read inputs  
		          	for (chIndex = 0; ucChanPosition <= ucMaxChannels; chIndex ++, ucChanPosition++){
		            	//printf("Found an analog input channel\n");
		            	const u32 cdx = ucChanPosition - 1;

		          	}//for
		        }

		      }

		      /***************** DIGITALMODULES *********************/
		      else{
		      	/***** DIGITAL INPUTS *****/
		      	if (terminalDescription[idx].SizeInput_bits != 0){
		          	
					// reset data					
					chIndex = 0;
		          	ucChanPosition = 1;

		        	// read inputs  
		          	for (chIndex = 0; ucChanPosition <= ucMaxChannels; chIndex ++, ucChanPosition++){

						// vars
		            	const u32 cdx = ucChanPosition - 1;

						// read inputs by channel		            
						adi->ReadStart(kbusDeviceId, taskId); // lock PD-In data 
		            	adi->ReadBool(kbusDeviceId, taskId, (terminalDescription[idx].OffsetInput_bits + chIndex), (bool *) &aModules[idx].digChannelData[cdx]);
		            	adi->ReadEnd(kbusDeviceId, taskId); // unlock PD-In data 

		            	if (aModules[idx].digChannelData[cdx] != aModulesOld[idx].digChannelData[cdx]){
		              
							// vars
							struct json_object* jsonSend;
		              	
							// build the json object							
							jsonSend = json_object_new_object();
		              		json_object_object_add(jsonSend, "module", json_object_new_int(idx + 1));
		              		json_object_object_add(jsonSend, "channel", json_object_new_int(cdx + 1));
		              		json_object_object_add(jsonSend, "value", json_object_new_boolean(aModules[idx].digChannelData[cdx]));

							// convert json object to string
							sendString = json_object_to_json_string(jsonSend);
		              
							//send the json string to broker
		              		mosquitto_publish(mosq, NULL, SUBTOPIC_INPUTS, strlen(sendString), sendString, 0, 0);
		              
							// set comparison data
		              		aModulesOld[idx].digChannelData[cdx] = aModules[idx].digChannelData[cdx];
		          }
		        }//for - channels
		      }// if - digital inputs             
		    }// if - analog inputs 
		  }//for - modules

		// capture cycle times for reset logic
		loops++;
		if (loops == 1000){
		  mosquitto_disconnect(mosq);
		  loops = 0;
		}

		// delay the re-cycle 1ms
		usleep(1000);		

    }//while
/**********************************************************************************************/

/**********************************************************************************************/

  adi->CloseDevice(kbusDeviceId); // close kbus device    
  adi->Exit(); // disconnect ADI-Interface 
  ldkc_KbusInfo_Destroy();
  return 0; // exit program

}// main
