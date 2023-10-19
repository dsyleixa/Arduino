// ESP32 WebServer: switch LED, r/w grid cells

#define LED_pin LED_BUILTIN   // default: LED_pin=LED_BUILTIN


static const char webpageCode[] PROGMEM = R"EOF(
<!DOCTYPE html>
<html>
  <head>
    <style>  
      #grid { 
        margin-top: 20px;
        display: grid; 
        grid-template-columns: repeat(10, 40px); 
        grid-template-rows: repeat(12, 40px);
        grid-gap: 2px;
      } 
      #grid > div { 
        font-size: 10px;
        padding: .5em; 
        background: lightgray; 
        text-align: center; 
      }
    </style>
</head>

<body>
  <div>
    <label for="remember">
     <input type="checkbox" role="switch" id="toggle-out" name="toggle-out">
      Toggle OUTPUT
    </label>
    <p id="esp-response"></p>
  </div>            
 <div id="grid"></div> 
<script>
  function sendCellToMCU(e) {   
     // Get actual text
   var cell_ID = e.target.innerHTML;
   console.log(cell_ID);
    
    // The MCU must habndle properly this GET AJAX request
    var  url = new URL("http://" + `${window.location.hostname}` + "/clicked_cell?val=");
    url += cell_ID;    
        
    // Send the value of clicked cekll back to the MCU
    console.log(url);
   fetch(url)
    .then(response => response.text())
    .then(data => {
      if (data.includes("ON"))
        e.target.style.background = "red";
      else 
        e.target.style.background = "lightgray";
      console.log(data)
    });
  }
  
  // Fill the "grid" element with "num" cells
  function createTable(data) {
      var el = document.getElementById("grid");
      var arr = data.split(',');
      // Iterate the array and create the grid
         arr.forEach( (item, i) => {
         // Create a nue empty
        var cell = document.createElement("div");        
        if (parseInt(item) === 1)
          cell.style.background = "red";
        // Set text
        cell.innerHTML = i;        
        // Add a click listener for each cell
        cell.addEventListener('click', sendCellToMCU);
        // Add the new cell to grid element
        el.appendChild(cell);
     }); 
  }
  
  function fetchGridValues() {
   var  url = new URL("http://" + `${window.location.hostname}` + "/getGrid");            
    // get actual grid values from MCU
    console.log(url);
   fetch(url)
    .then(response => response.text())
    .then(obj => {
      // Response now will be an array
      console.log(obj)
      createTable(obj);
    });
  }
  
   // This function fetch the GET request to the ESP webserver
  function toggleOut() {
    const pars = new URLSearchParams({
      val:  document.getElementById('toggle-out').checked ? '1' : '0'
    });
    
    var  url = new URL("http://" + `${window.location.hostname}` + "/led?");
    url += pars; 
    console.log(url);

    fetch(url)                    // Do the request
      .then(response => response.text())    // Parse the response 
      .then(text => {                       // DO something with response      
         document.getElementById('esp-response').innerHTML = text;
      });
  }

  // Add event listener to the LED checkbox (the function will be called on every change)
  document.getElementById('toggle-out').addEventListener('change', toggleOut );
  
  // Call the function createTable in order to create the grid dinamically
  // createTable(120);  
  fetchGridValues();
  
</script>
</body>
</html>
)EOF";


#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>


//---------------------------------------------------

IPAddress LOCALIP(192, 168,  2, 202);
IPAddress GATEWAY (192, 168,  2,  1);
IPAddress SUBNET  (255, 255, 255, 0);


const char* ssid = "WLAN-name";
const char* password = "********";

int OUT1 = LOW;
bool gridState[120];

//---------------------------------------------------
WebServer server(80);

//---------------------------------------------------


//=============================================
//Handle functions executed upon client request
//=============================================
void handleRoot()
{
  // http://xxx.xxx.xxx.xxx/
  server.send(200, "text/html", webpageCode);
}
//---------------------------------------
void handleLed()
{
  // http://xxx.xxx.xxx.xxx/led?val=1
  if(server.hasArg("val")) {
    int value = server.arg("val").toInt();    
  }
  server.send(200, "text/plain", "OK");
}
//---------------------------------------
void handleCell()
{
  // http://xxx.xxx.xxx.xxx/clicked_cell?val=15  
  if(server.hasArg("val")) {
    int value = server.arg("val").toInt();    
    gridState[value] = !gridState[value];
    Serial.println((String)value + "=" +(bool)gridState[value]);
    server.send(200, "text/plain", gridState[value] ? "ON" : "OFF");
  }
  else
    server.send(200, "text/plain", "ERROR");
}
//---------------------------------------
void handleGrid()
{
  // http://xxx.xxx.xxx.xxx/getGrid
  String response;
  for (int i=0; i<sizeof(gridState); i++) {
    response += gridState[i];
    if (i < (sizeof(gridState)-1))
        response += ",";
  }
  server.send(200, "text/plain", response);
}
//===================================================
void setup()
{
  pinMode(LED_pin, OUTPUT);      // set the LED pin mode
  memset(gridState, 0, sizeof(gridState) );

  Serial.begin(115200);

  delay(1000);
  
  //-------------------------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.config(LOCALIP, GATEWAY, SUBNET, GATEWAY, GATEWAY);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi");
  while(WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(500); Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //-------------------------------------------------
  server.on("/", handleRoot);
  server.on("/led", handleLed);
  server.on("/clicked_cell", handleCell);
  server.on("/getGrid", handleGrid);
  server.begin(80);
  Serial.println("HTTP server started");
}

//===================================================
void loop(void)
{
  server.handleClient();
}
