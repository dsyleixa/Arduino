/*            Transmitter
 For more details see: http://projectsfromtech.blogspot.com/
 
 This sketch makes use of Serial1. This functionality is only found on the Arduino Mega and other 3rd party boards.
 Tx1: pin 18   Rx1: pin 19
 Connect the Tx1->Rx  and Rx1 ->Tx pins with the board running Add_Five_Rx.ino
 Connect the Grounds of the two boards
 
 Read integer value from Serial Monitor
 Transmit that value to the Rx board
 Receive integer from Rx and print to Serial Monitor
 
 Note: The Tx and Rx boards are just names I chose. The communication is 2-way. 
 Master-slave would probably be a better option.
 */


int val = 0;
int incoming = 0;
int incoming1 = 0;

void setup()
{
  Serial.begin(115200);  //Begin Serial to talk to the Serial Monitor   
  Serial1.begin(115200); //Begin Serial to talk to the Rx Arduino
  Serial.println("Serial Monitor Connected"); 
  Serial.parseInt();     //clear any garbage in the buffer.  
}

void loop()
{
  //   From the USB
  incoming = Serial.available();  
  while (incoming != 0)           //while there is something to be read
  {
    val = Serial.parseInt();
    if (val != 0)
    {
      Serial.print("Received input... Transmitting:  ");
      Serial.println(val);         //Print value to the Serial Monitor
      Serial1.print(val);          //Send value to the Rx Arduino
    }
    incoming = Serial.available();
  }

  //     From the Rx Arduino
  incoming1 = Serial1.available();  
  while(incoming1 != 0)           //While there is something to be read
  {
    val = Serial1.parseInt();    //Get new value
    Serial.print("Receiving...  ");    
    Serial.println(val);        //Print new value to the Serial Monitor
    incoming1 = Serial1.available();
  }
}


