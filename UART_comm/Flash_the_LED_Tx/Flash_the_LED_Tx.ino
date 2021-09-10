/*            Transmitter
For more details see: http://projectsfromtech.blogspot.com/

Connect the Tx pin of this board to the Rx of the board running Flash_the_LED__Tx.ino
Connect the Grounds of the two boards

Read integer value from Serial Monitor
Transmit that value to the Rx board
*/


int val = 0;
int incoming = 0;

void setup()
{
  Serial.begin(115200);     
  Serial.println("Serial Monitor Connected");    //Is not an integer so it won't interfere
}

void loop()
{
  incoming = Serial.available();  
  while (incoming == 0)     //keep checking until something is available
  {
    incoming = Serial.available();
  }
  Serial.print("Received value... Transmitting:  ");
  val = Serial.parseInt();  //Reads integers as integer rather than ASCI. Anything else returns 0
                            //Note that this is a user input via the Serial Monitor
                            
   Serial.println(val);       //This is being sent to the Rx board

}

