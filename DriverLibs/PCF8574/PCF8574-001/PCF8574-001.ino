void PCF8574_Write(int adresse, byte daten) {
  Wire.beginTransmission(adresse);
  Wire.write(daten);
  Wire.endTransmission();
  delay(5);
}

byte PCF8574_Read(int adresse) {
  byte datenByte=0xff;
  Wire.requestFrom(adresse,1);
  if(Wire.available()){
    datenByte=Wire.read();
    datenkomm=true;
    }
  else {
    datenkomm=false;
  }
  return datenByte;
}

void setup(){
  Wire.begin();
  PCF8574_Write(PCF8574,0xff);
  //...
}

void loop() {
  byte daten;
  //...
  daten=PCF8574_Read(PCF8574);
  //...
}