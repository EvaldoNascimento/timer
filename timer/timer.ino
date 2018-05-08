/* ===============================================================
      Project: Timer to control relays
       Author: Evaldo Nasciment0
      Created: 7th May 2018
  Arduino IDE: 1.8.5
      Website: 
  Description: implement a pump timer control for ESP32 platform.
================================================================== */

#include <TimeLib.h>

#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <SPI.h> //responsável pela comunicação serial
#include <LoRa.h> //responsável pela comunicação com o WIFI Lora
#include <Wire.h>  //responsável pela comunicação i2c
#include "SSD1306.h" //responsável pela comunicação com o display
#include "images.h" //contém o logo para usarmos ao iniciar o display

#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`


// use pins that are not shared with other applications [34..39]
#define  PUMP        34
#define  ACCURRENT   35  // ver como funciona o detector de corrent AC - digital ou analogico??
#define  DOOROPEN    36
#define  ONEWIRE     37

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND    915E6  //Frequencia do radio - podemos utilizar ainda : 433E6, 868E6, 915E6

// OneWire  ds(37) //pq nao aceita ONEWIRE definido acima ??

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends



int      PointerTimeTable  	= 0;
boolean  TimeToTurnOn   	= false;
boolean  TimeToTurnOff  	= false;
String   TimeNow  			= "";
String   InputTimeTable 	= "00600T00800F01600T01800F10600T10800F11600T11800F20600T20800F21600T21800F30600T30800F31600T31800F40600T40800F41600T41800F50600T50800F51600T51800F60600T60800F61600T61800F";
int      TableSize 			= InputTimeTable.length();
int      attempt 			= 0;

String  CurrentTimeTable  = "";
char CurrentAction = 0;

SSD1306 display(0x3c, 4, 15); //construtor do objeto que controlaremos o display


/* esta função deve retornar a hora do sistema no formato wHHMM 

# get system day of week and time
# wHHMM
#   w = day of week - 0-Mon, 1-Tue, 2-Wed .. 6-Sun
#   HH hour - 00..23
#   MM minute - 00..59
def get_time():
    SystemTime = datetime.now()
    CurrentHour =  str(SystemTime.hour)
    if len (CurrentHour) == 1 :
       CurrentHour = "0" + CurrentHour
    CurrentMinute =  str(SystemTime.minute)
    if len (CurrentMinute) == 1 :
       CurrentMinute = "0" + CurrentMinute
    TimeNow = str(SystemTime.weekday()) + CurrentHour + CurrentMinute
    return TimeNow
*/

// functions declarations
void logo (void);
boolean SendLora (char buffer);
char ReceiveLora();


String get_time(){
  return ("00600");
}

boolean HighCurrent() {
  return (false);
}

boolean AnyCurrent() {
  return (false);
}


void Panic(){
  
}

void Halt(){
  
}


void setup() {
	/*
	GPIO
	Serial
	OneWire
	RTC
	Lora
	Oled
	TimeTable
	Get LocalTime from GW
	Sync LocalTime with TimeTable 
	*/


  // GPIO setup ===========================================================================================================
  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
  pinMode(PUMP, OUTPUT);  		// initialize digital PUMP As an output.
  pinMode(ACCURRENT, INPUT);  	// initialize digital pin LED_BUILTIN as an input.
  pinMode(DOOROPEN,  INPUT);  	// initialize digital pin LED_BUILTIN as an input.
  
  
  
  // Serial setup =========================================================================================================
  Serial.begin(9600);
  
  
  
  // OneWire setup ========================================================================================================
  
  
  
  // RTC setup ============================================================================================================
  while (!Serial) ; // Needed for Leonardo only
  //setSyncProvider(RTC.get);   // the function to get the time from the RTC
  t_time t = setTime (21,00,00,7,5,2018);
  if (timeStatus() != timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");      

  
  
  // OLED setup ===========================================================================================================
  // Initialize the OLED display using Wire library

  //configura os pinos como saida
  pinMode(16,OUTPUT); //RST do oled
  pinMode(25,OUTPUT);
  
  digitalWrite(16, LOW);    // reseta o OLED
  delay(50); 
  digitalWrite(16, HIGH); // enquanto o OLED estiver ligado, GPIO16 deve estar HIGH

  display.init(); //inicializa o display
  display.flipScreenVertically(); 
  display.setFont(ArialMT_Plain_10); //configura a fonte para um tamanho maior

  //imprime o logo na tela
  logo();

  delay(1500);
  display.clear(); //apaga todo o conteúdo da tela do display
  
  
  
  // LORA setup ============================================================================================================
  
  SPI.begin(SCK,MISO,MOSI,SS); //inicia a comunicação serial com o Lora
  LoRa.setPins(SS,RST,DI00); //configura os pinos que serão utlizados pela biblioteca (deve ser chamado antes do LoRa.begin)
  
  //inicializa o Lora com a frequencia específica.
  if (!LoRa.begin(BAND)) {
    display.drawString(0, 0, "Starting LoRa failed!");
    display.display(); //mostra o conteúdo na tela
    while (1);
  }
  //indica no display que inicilizou corretamente.
  display.drawString(0, 0, "LoRa Initial success!");
  display.display(); //mostra o conteúdo na tela
  delay(1000);
 
  
  // Get LocalTime  from the GW ============================================================================================
  attempt = 0;
  String message = "SYNC";
  sendMessage (message);
  Serial.println("Sending " + message);
  lastSendTime = millis();            // timestamp the message
  interval = random(2000) + 1000;    // 2-3 seconds
 
  //while ((!SendLora ('Sync')) | (attempt <=3)) {
	//attempt += 1;
	//delay (1000);
  }
  char NewTime = ReceiveLora();
  //Sync(RTC, NewTime);
  
  
  // Get a new InputTimeTable  from the GW =================================================================================
  attempt = 0;
  
  
  while ((!SendLora ('Table')) | (attempt <=3)) {
	attempt += 1;
	delay (1000);
  }
  InputTimeTable = ReceiveLora();
  TableSize 	 = InputTimeTable.length();

  /* 
  // TimeTable setup ========================================================================================================
  //  get first table entry
  CurrentTimeTable = InputTimeTable.substring(0,0) + InputTimeTable.substring(1,4);
  CurrentAction = InputTimeTable[5];
 

 // Sync LocalTime with TimeTable ==========================================================================================  
 // once LocalTime is received from the GW, find next TimeTable Entry just before the LocalTime - 

 // find the current day of week inside the InputTimeTable
 char CurrentWeekDay = str (weekday());
 char CurrentTime = hour() + minute();
 
 PointerTimeTable = 0;
 
 //String   InputTimeTable 	= "00600T00800F01600T01800F10600T10800F11600T11800F20600T20800F21600T21800F30600T30800F31600T31800F40600T40800F41600T41800F50600T50800F51600T51800F60600T60800F61600T61800F";

 // procurar pelo 1o weekday ate que seja igual na tabela
 // qdo achar verificar a hora
 //     se for menor que a hora atual obtem a ação e executa
 
 while (CurrentWeekDay <> InputTimeTable[PointerTimeTable]) {
	 PointerTimeTable += 6;
 }
 CurrentTimeTable = InputTimeTable.substring(PointerTimeTable+1, PointerTimeTable+4); 	// get HHMM
 CurrentAction = InputTimeTable[PointerTimeTable+5]; 									// get Action: T = ON   F = OFF
 
 if CurrentTime > CurrentTimeTable {
	PointerTimeTable += 6; // point to the next entry 
 }
 CurrentTimeTable = InputTimeTable.substring(PointerTimeTable+1, PointerTimeTable+4); 	// get HHMM
 CurrentAction = InputTimeTable[PointerTimeTable+5]; 									// get Action: T = ON   F = OFF
 
*/
boolean SendLora(String buffer) {
	 
  LoRa.beginPacket(); 		//beginPacket : abre um pacote para adicionarmos os dados para envio
  LoRa.write(buffer); 		//print: adiciona os dados no pacote
  if (LoRa.endPacket())  {  	//endPacket : fecha o pacote e envia 
	  return (true); } 
  else {
	  return (false); }//retorno= 1:sucesso | 0: falha

}
void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}


String ReceiveLora(void) {
   // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  
  //parsePacket: checa se um pacote foi recebido
  //retorno:  pacote recebido. Se retornar "" nenhum pacote foi recebido
  int packetSize = LoRa.parsePacket();
  //caso tenha recebido pacote chama a função para configurar os dados 
  char packet =""
  if (packetSize) { 
    packSize = String(packetSize,DEC); //transforma o tamanho do pacote em String para imprimirmos
	for (int i = 0; i < packetSize; i++) { 
		packet += (char) LoRa.read(); //recupera o dado recebido e concatena na variável "packet"
	}
 }
 // delay(10);
 return (packet)
}	 

  void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

void logo() {
  //apaga o conteúdo do display
  display.clear();
  //imprime o logo presente na biblioteca "images.h"
  display.drawXbm(0,5,logo_width,logo_height,logo_bits);
  display.display();
}

int get_temperature (boolean ReturnCelcius) {
	
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");

  if ReturnCelcius {
	  
	  return (celsius);
  }	  else {
	  return (fahrenheit(;
  }

}

// MAIN LOOP ===============================================================================================================
void loop() {
    String TimeNow = get_time();
    TimeToTurnOn  = (TimeNow == CurrentTimeTable) && (CurrentAction == 'T');
    TimeToTurnOff = (TimeNow == CurrentTimeTable) && (CurrentAction == 'F');

    delay (5000);
    
    if (TimeToTurnOn) {
//       print ("")
//       print ("RelayOn")
       delay (1000);
       if (HighCurrent() == true) {
          Panic();
          Halt();
       }
       PointerTimeTable = PointerTimeTable + 7;
       if (TableSize <= PointerTimeTable) {
           PointerTimeTable = 0;
       }
       CurrentTimeTable = InputTimeTable.substring(PointerTimeTable,PointerTimeTable) + InputTimeTable.substring(PointerTimeTable+1,PointerTimeTable+5);
       CurrentAction = InputTimeTable[PointerTimeTable+5];
    }
       
    if (TimeToTurnOff) {
 //    print ("")
 //    print ("RelayOff")
       delay (1000);
       if (AnyCurrent() == true) {
           Panic();
           Halt ();
       }
       PointerTimeTable = PointerTimeTable + 7;
       if (TableSize <= PointerTimeTable) {
           PointerTimeTable = 0;
       }
       CurrentTimeTable = InputTimeTable.substring(PointerTimeTable,PointerTimeTable) + InputTimeTable.substring(PointerTimeTable+1,PointerTimeTable+5);
       CurrentAction = InputTimeTable[PointerTimeTable+5];
       
    }

}
