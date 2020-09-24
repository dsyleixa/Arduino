 // --------------------------------------
 // i2c_scanner
 //    
 // This sketch tests the standard 7-bit addresses
 // Devices with higher bit address might not be seen properly.
 //
     
 #include <Wire.h>
    
 byte error, address;
 int nDevices, nDeverr;
 
 void setup()
 {            
      //Wire.pins(D2, D1);   // change only if not default 
      Wire.begin();
 
      Serial.begin(115200);
      while (!Serial);                 // Leonardo: wait for serial monitor

      Serial.println("\nI2C Scanner");     
                
 }
     
     
 void loop()  {    
      static uint32_t  i2cclock = 10000; 
      int iores;
      
      nDevices = 0;
      nDeverr  = 0;
      
      if(i2cclock==10000) i2cclock=100000; 
      else 
      if(i2cclock==100000) i2cclock=400000;
      /*else 
      if(i2cclock==400000) i2cclock=1000000;
      */
      else 
      if(i2cclock==400000) i2cclock=10000;
      
      
      Wire.setClock(i2cclock);
      Serial.print("\nScanning at "); Serial.print(i2cclock/1000); Serial.println("k");
      
      for(address = 0; address < 128; address++ )   {

        if (address%16 == 0)  {   
          Serial.println();  
          Serial.print( (address+1)/16);
          Serial.print("  ");
        }

        
        if(address==0 || address==127) {
           Serial.print("** ");
           continue;
        }
        
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
     
        if (error == 0)
        {          
          if (address<16) Serial.print("0"); 
          Serial.print(address,HEX); Serial.print(" ");         
          nDevices++;
        }
        else if (error==1)  //  data too long for transmit buffer
        {
          Serial.print("E1 ");   
          nDeverr++;
        }         
        else if (error==3)  //  NACK on transmit of data
        {
          Serial.print("E3 ");   
          nDeverr++;
        } 
        else if (error==4)  //  other error
        {
          Serial.print("E4 ");   
          nDeverr++;
        }    
        else // error==2 == no device (NACK on transmit of address)
        {      
           Serial.print("-- ");   
        }               
      }
      
      Serial.println();
      Serial.print("Scan result for i2cclock "); Serial.print(i2cclock/1000); Serial.println("k");
      Serial.print("found: "); Serial.print(nDevices); Serial.println(" devices ");
      Serial.print("error: "); Serial.print(nDeverr ); Serial.println(" devices \n");
      delay(5000);      
 }
