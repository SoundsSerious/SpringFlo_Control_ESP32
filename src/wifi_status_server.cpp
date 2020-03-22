#include <wifi_status_server.h>

WiFiServer server(80);
const char* ssid     = "SpaceCase";
const char* password = "9252623793";
String header = "";

void begin_server(){
  TickType_t xdelay = 500/ portTICK_PERIOD_MS;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(xdelay);
  }
  server.begin();

   vTaskDelay(xdelay);
   MDNS.begin("smasher");
   vTaskDelay(xdelay);
   MDNS.addService("http", "tcp", 80);
}

void print_thermal_status_wifi(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            client.println("<!DOCTYPE html><html><head><title>Smasher Status</title>");
            client.println("<meta http-equiv=\"refresh\" content=\"60\"></head><body>");

            // client.println("<script type=\"text/javscript\"> function sendJSON(){ let result = document.querySelector('.result'); let close_time = document.querySelector('#close_time');let process_temp = document.querySelector('#process_temp');  let xhr = new XMLHttpRequest();let url = 'smasher.local';xhr.open('POST', url, true);  xhr.setRequestHeader('Content-Type', 'application/json'); var data = JSON.stringify({ 'close_time': close_time.value, 'process_temp': process_temp.value });xhr.send(data);}</script>");

            client.println("<p>Temperature Goal: ");
            client.print(Temperature_Goal_Former);
            client.println("</p><p>Raw Temperature: ");
            client.print(temp_raw_former);
            client.println("</p><p>Filtered Temperature: ");
            client.print(temperature_former);
            client.println("</p>");

            client.println("<p>_________________________________________");
            client.println("<p>Former Control</p>");
            client.print("<p>PID P Error %: ");
            client.println(pid_former_error*100.0 /255);
            client.print("</p><p>PID I Error %: ");
            client.println(pid_former_ei*100.0 /255);
            client.print("</p><p>PWM Output  %: ");
            client.println(current_pwm_former*100.0 /255);
            client.println("</p>");

            client.println("<p>_________________________________________");
            client.println("<p>Smasher Control</p>");
            client.print("<p>PID P Error %: ");
            client.println(pid_smasher_error*100.0 /255);
            client.print("</p><p>PID I Error %: ");
            client.println(pid_smasher_ei*100.0 /255);
            client.print("</p><p>PWM Output  %: ");
            client.println(current_pwm_smasher*100.0 /255);
            client.println("</p>");

            client.println("<p>_________________________________________");
            client.print("<p>Thermal Sensor Nominal: ");
            client.println(former_sensor_is_nominal);
            client.print("</p><p>Thermally Ready: ");
            client.println(thermally_ready);
            client.println("</p>");

            client.println("<p>_________________________________________");
            client.print("<p>Do Feed: ");
            client.println(do_feeding);
            client.print("</p><p>Piston Closed: ");
            client.print(piston_closed);
            client.print("</p>");

            client.println("<p>_________________________________________</p>");
            client.print("<p><input type=\"number\" id=\"close_time\" placeholder=\"" );
            client.print(CLOSE_TIME);
            client.println("\"></p>");
            client.print("<p><input type=\"number\" id=\"process_temp\" placeholder=\"" );
            client.print(PROCESS_TEMP);
            client.print("\"></p>");
            client.println("<p>_________________________________________</p>");
            client.println("<button onclick=\"function sendJSON(){ let close_time = document.querySelector('#close_time');let process_temp = document.querySelector('#process_temp');  let xhr = new XMLHttpRequest();let url = 'http://smasher.local';xhr.open('POST', url, true);  xhr.setRequestHeader('Content-Type', 'application/json'); var data = JSON.stringify({ 'close_time': close_time.value, 'process_temp': process_temp.value });xhr.send(data);};sendJSON();\">Send JSON</button> ");



            client.println("</body></html>");

            //break;
          } else { // if you got a newline, then clear currentLine

            //Process CurrentLine
            //Serial.print("Got A Message:");
            //Serial.println(currentLine);
            //Try To Parse JSON
            StaticJsonDocument<256> doc;
            DeserializationError err = deserializeJson(doc, currentLine);

            if (err) { //Check if successful
              Serial.print(F("deserializeJson() returned "));
              Serial.println(err.c_str());
              serializeJsonPretty(doc, Serial);
            }
            else{ //It was
              CLOSE_TIME = doc["close_time"].as<int>(); //ms
              PROCESS_TEMP = doc["process_temp"].as<int>(); //ms
            }

            //Clear now
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
}


void wifi_status_task(void * parameter){

  const TickType_t xDelay = wifi_interval_ms / portTICK_PERIOD_MS;

  begin_server();
  vTaskDelay(xDelay);

  while(true){
    try{
        print_thermal_status_wifi();
    }
    catch(int e){}
    vTaskDelay(xDelay);
  }
}

// WiFiClient client = server.available();   // Listen for incoming clients
//
//  if (client) {                             // If a new client connects,
//    Serial.println("New Client.");          // print a message out in the serial port
//    String currentLine = "";                // make a String to hold incoming data from the client
//    while (client.connected()) {            // loop while the client's connected
//      if (client.available()) {             // if there's bytes to read from the client,
//        char c = client.read();             // read a byte, then
//        Serial.write(c);                    // print it out the serial monitor
//        header += c;
//        if (c == '\n') {                    // if the byte is a newline character
//          // if the current line is blank, you got two newline characters in a row.
//          // that's the end of the client HTTP request, so send a response:
//          if (currentLine.length() == 0) {
//            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//            // and a content-type so the client knows what's coming, then a blank line:
//            client.println("HTTP/1.1 200 OK");
//            client.println("Content-type:text/html");
//            client.println("Connection: close");
//            client.println();
//
//            // turns the GPIOs on and off
//            if (header.indexOf("GET /26/on") >= 0) {
//              Serial.println("GPIO 26 on");
//              output26State = "on";
//              digitalWrite(output26, HIGH);
//            } else if (header.indexOf("GET /26/off") >= 0) {
//              Serial.println("GPIO 26 off");
//              output26State = "off";
//              digitalWrite(output26, LOW);
//            } else if (header.indexOf("GET /27/on") >= 0) {
//              Serial.println("GPIO 27 on");
//              output27State = "on";
//              digitalWrite(output27, HIGH);
//            } else if (header.indexOf("GET /27/off") >= 0) {
//              Serial.println("GPIO 27 off");
//              output27State = "off";
//              digitalWrite(output27, LOW);
//            }
//
//            // Display the HTML web page
//            client.println("<!DOCTYPE html><html>");
//            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
//            client.println("<link rel=\"icon\" href=\"data:,\">");
//            // CSS to style the on/off buttons
//            // Feel free to change the background-color and font-size attributes to fit your preferences
//            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
//            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
//            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
//            client.println(".button2 {background-color: #555555;}</style></head>");
//
//            // Web Page Heading
//            client.println("<body><h1>ESP32 Web Server</h1>");
//
//            // Display current state, and ON/OFF buttons for GPIO 26
//            client.println("<p>GPIO 26 - State " + output26State + "</p>");
//            // If the output26State is off, it displays the ON button
//            if (output26State=="off") {
//              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
//            } else {
//              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
//            }
//
//            // Display current state, and ON/OFF buttons for GPIO 27
//            client.println("<p>GPIO 27 - State " + output27State + "</p>");
//            // If the output27State is off, it displays the ON button
//            if (output27State=="off") {
//              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
//            } else {
//              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
//            }
//            client.println("</body></html>");
//
//            // The HTTP response ends with another blank line
//            client.println();
//            // Break out of the while loop
//            break;
//          } else { // if you got a newline, then clear currentLine
//            currentLine = "";
//          }
//        } else if (c != '\r') {  // if you got anything else but a carriage return character,
//          currentLine += c;      // add it to the end of the currentLine
//        }
//      }
