/*     
 For more details see: http://projectsfromtech.blogspot.com/
 

 Read char value from Serial Monitor
 Display that value on the Serial Monitor
 */


char val = 0;
int incoming = 0;

void setup()
{
  Serial.begin(115200);     
  Serial.println("Serial Monitor Connected");  
}

void loop()
{
  incoming = Serial.available();  
  while (incoming == 0)     //keep checking until something is available
  {
    incoming = Serial.available();
  }
  val = Serial.read();  //Reads the input from the Serial Monitor
                        

  if (val == 46)               //If val is a period                           
    Serial.println(val);       //display val with a line return afterward
  else
    Serial.print(val);         //display val
}


