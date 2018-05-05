/* ===============================================================
      Project: Piscina 2.0 - Swimming poll filter pump timer control
       Author: Evaldo Nasciment0
      Created: 5st May 2018
  Arduino IDE: 1.8.5
      Website: 
  Description: implement a pump timer control for ESP32 platform.
================================================================== */


int      PointerTimeTable  = 0;
boolean  TimeToTurnOn   = false;
boolean  TimeToTurnOff  = false;
String   TimeNow  = "";
String   InputTimeTable = "00600T00800F01600T01800F10600T10800F11600T11800F20600T20800F21600T21800F30600T30800F31600T31800F40600T40800F41600T41800F50600T50800F51600T51800F60600T60800F61600T61800F";
int      TableSize = InputTimeTable.length();

String  CurrentTimeTable  = "";
char CurrentAction = 0;


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
/* get first table entry */
CurrentTimeTable = InputTimeTable.substring(0,0) + InputTimeTable.substring(1,4);
CurrentAction = InputTimeTable[5];
}

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
       
 //    CurrentTimeTable = InputTimeTable [PointerTimeTable] + InputTimeTable [PointerTimeTable+1:PointerTimeTable+5]
 //    print ("proximo horario da tabela = desligar as  "+CurrentTimeTable)
 //    CurrentAction = InputTimeTable [PointerTimeTable+5]

       
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
       
//     CurrentTimeTable = InputTimeTable [PointerTimeTable] + InputTimeTable [PointerTimeTable+1:PointerTimeTable+5]
//     print ("proximo horario da tabela = LIGAR as  "+CurrentTimeTable)
//     CurrentAction = InputTimeTable [PointerTimeTable+5]
    }

}

/*
 * 
 * Python version
 
from datetime import datetime
from time import sleep

# open text file with time table
arquivo = open('TimeTableFilterPump.txt', 'r')
InputTimeTable = arquivo.read()
arquivo.close()
print ('Tabela horarios',InputTimeTable)

# setup variables
TableSize = len (InputTimeTable)
PointerTimeTable = 1
TimeToTurnOn  = False
TimeToTurnOff = False
TimeNow = ""

# get first table entry
CurrentTimeTable = InputTimeTable [1] + InputTimeTable [2:6]
CurrentAction = InputTimeTable [6]


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


while (True):
    TimeNow = get_time()
#    print ('Hora agora = ', TimeNow)
#    print ('Dia e Hora obtida da tabela = ',CurrentTimeTable)
#    print ('Acao obtida da tabela = ',CurrentAction)
    TimeToTurnOn  = (TimeNow == CurrentTimeTable) & (CurrentAction == 'T')
    TimeToTurnOff = (TimeNow == CurrentTimeTable) & (CurrentAction == 'F')

    sleep (5)
    
    if TimeToTurnOn :
       print ("")
       print ("RelayOn")
       sleep (1)
#      if HighCurrent()
#         Panic()
#         Stop
       PointerTimeTable = PointerTimeTable + 7
       if TableSize <= PointerTimeTable :
           PointerTimeTable = 1
       CurrentTimeTable = InputTimeTable [PointerTimeTable] + InputTimeTable [PointerTimeTable+1:PointerTimeTable+5]
       print ("proximo horario da tabela = desligar as  "+CurrentTimeTable)
       CurrentAction = InputTimeTable [PointerTimeTable+5]

       
    if TimeToTurnOff :
       print ("")
       print ("RelayOff")
       sleep(1)
#      if AnyCurrent()
#         Panic()
#         Stop
       PointerTimeTable = PointerTimeTable + 7
       if TableSize <= PointerTimeTable :
           PointerTimeTable = 1
       CurrentTimeTable = InputTimeTable [PointerTimeTable] + InputTimeTable [PointerTimeTable+1:PointerTimeTable+5]
       print ("proximo horario da tabela = LIGAR as  "+CurrentTimeTable)
       CurrentAction = InputTimeTable [PointerTimeTable+5]
 * 
 * 
 */
