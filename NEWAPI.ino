#include <SPI.h>
#include <mcp_can.h>

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

#define SOM '<'
#define EOM '>'

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

char cp = 0;
boolean canFound = false;
boolean canStart = false;
char canIn[50];
uint8_t DLC=0;

char tmp1[50],tmp2[50],tmp3[50],tmp4[50];

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN.begin(CAN_500KBPS) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN.setMode(MODE_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
 
  Serial.println("Initializing CAN Bus");
}

void loop() {
  int sa;
  unsigned char canBuff;
  sa = Serial.available(); // count serial buffer bytes
  
  if (sa > 0) { // if buffer not empty process buffer content
    for (int i=0; i < sa; i++){
     canBuff = Serial.read(); // read one char from the serial buffer
       
     if (canStart ) {
      canIn[cp] = canBuff;
      cp++;
     }
     
     if (canBuff == SOM) { // got a start of command
      canStart = true;
     }
       
     if (canBuff == EOM && canStart) { // check for last command char )
      canFound = true;
      canIn[--cp] = NULL;
      break; // end for-loop
     }
    }
  }
  if(canFound){
    /*for(int i=0; i<cp; i++){
      Serial.print(canIn[i]);
    }*/
    
    sscanf(canIn,"%[^,],%[^,],%[^,],%[^,]",tmp1,tmp2,tmp3,tmp4);
        
    unsigned long CAN_ID = hexstring2int(tmp1);
    uint8_t ID_TYPE = hexstring2int(tmp2);
    uint8_t DLC = hexstring2int(tmp3);
    unsigned char DATA[8];
    
    if(ID_TYPE==0){
      Serial.print("\nStandard CANID:0x");
      Serial.println(CAN_ID,HEX);
    }
    else{
      Serial.print("\nExtended CANID:0x");
      Serial.println(CAN_ID,HEX);
    }
    Serial.print("DLC:");
    Serial.println(DLC,HEX);
    Serial.print("DATA:");
    for (int i = 0; i <DLC; i++) {
        sscanf((tmp4 + 2*i), "%02x", &DATA[i]);
        printf("bytearray %d: %02x\n", i, DATA[i]);
        Serial.print("0x");
        Serial.print(DATA[i],HEX);
        Serial.print(" ");
    }

    CAN.sendMsgBuf(CAN_ID,0,DLC,DATA);
    delay(1000);
    cp =0;
    canIn[cp] = NULL;
    canStart = false;
    canFound = false;
  }  
  /*unsigned char len = 0;
  unsigned char buf[8];
  if (CAN_MSGAVAIL == CAN.checkReceive()) {         // check if data coming
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned long canId = CAN.getCanId();

        SERIAL.println("-----------------------------");
        SERIAL.print("get data from ID: 0x");
        SERIAL.println(canId, HEX);

        for (int i = 0; i < len; i++) { // print the data
            SERIAL.print(buf[i],HEX);
            SERIAL.print("\t");
        }
//<7E0,0,8,1020304050607080>
//<CF00400,1,8,1020304050607080>
  }*/
}
uint32_t hexstring2int(char *hex)
{
    uint32_t val = 0;
    while (*hex)
    {
        // get current character then increment
        uint8_t byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}
