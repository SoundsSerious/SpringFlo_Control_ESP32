#include <wifi_status_server.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)

const char* ssid     = "SpaceCase";
const char* password = "9252623793";

String header = "";



void begin_server(){
  TickType_t xdelay = 500/ portTICK_PERIOD_MS;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(xdelay);
  }

  // attach AsyncWebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // attach AsyncEventSource
  server.addHandler(&events);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html",html_doc());
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/info", HTTP_GET, [] (AsyncWebServerRequest *request) {
      AsyncJsonResponse * response = new AsyncJsonResponse();
      response->addHeader("Server","Smasher Data");
      JsonVariant& root = response->getRoot();
      root["temp_raw"] =  temp_raw_former;
      root["temp_filter"] = temperature_former;
      root["temp_goal"] = Temperature_Goal_Former;

      root["pid_f_e"] =  pid_former_error;
      root["pid_f_i"] = pid_former_ei;
      root["pwm_f"] = current_pwm_former;

      root["pid_s_e"] =  pid_smasher_error;
      root["pid_s_i"] = pid_smasher_ei;
      root["pwm_s"] = current_pwm_smasher;

      root["former_sensor_is_nominal"] = former_sensor_is_nominal;
      root["thermally_ready"] = thermally_ready;
      root["mode"] = current_mode;
      root["feed_state"] = FEED_STATE;
      root["mode_state"] = MODE_STATE;

      root["current_close_time"] = CLOSE_TIME;
      root["current_process_temp"] = PROCESS_TEMP;

      response->setLength();

      request->send(response);
  });

  server.on("/process_setting", HTTP_POST, [](AsyncWebServerRequest * request){
      int headers = request->headers();
      int i;
      for(i=0;i<headers;i++){
        Serial.printf("HEADER[%s]: %s\n", request->headerName(i).c_str(), request->header(i).c_str());
      }
      request -> send(200);
      },
      NULL,
      [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

          String body = String((char *)data);
          Serial.println(body);
          StaticJsonDocument<200> doc;
          deserializeJson(doc, body);
          CLOSE_TIME = doc["close_time"];
          PROCESS_TEMP = doc["process_temp"];
          Serial.print("Got Close Time: ");Serial.println(CLOSE_TIME);
          Serial.print("Got Process Temp: ");Serial.println(PROCESS_TEMP);
          Temperature_Goal_Former = PROCESS_TEMP;
          Temperature_Goal_Smasher = PROCESS_TEMP;

  });

  // AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/process_setting", [](AsyncWebServerRequest *request, JsonVariant &json) {
  //   Serial.print(json["process_temp"]);
  //   Serial.println(json["close_time"]);
  // });
  // server.addHandler(handler);

   vTaskDelay(xdelay);
   MDNS.begin("smasher");
   vTaskDelay(xdelay);
   MDNS.addService("http", "tcp", 80);


   server.begin();
}

void onRequest(AsyncWebServerRequest *request){
  //Handle Unknown Request
  request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  //Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  //Handle upload
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //Handle WebSocket event
}

String add_field(String visible_name, String tag_name){
  String line;
  line += "<p>";
  line += visible_name;
  line += "<span id=\"";
  line += tag_name;
  line += "\"></span></p>";
  return line;
}



String html_doc(){
  String doc;
  doc += "<!doctype html>\n";
  doc += "<html>\n";
  doc += "<head>\n";
  doc += "<title>Spring Flo Former Status</title>\n";
  doc += "<style>line-height:30px;</style>\n";
  doc += "<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>\n";
  doc += "<script>$(document).ready(function(){ $(\"#sendbtn\").click(function(){    let closetime = $('#close_time').val();  let temp = $('#process_temperature').val();    var data = JSON.stringify({ \"process_temp\": parseInt(temp), \"close_time\": parseInt(closetime) });   console.log(data);    $.ajax({   url: '/process_setting',   type: 'POST',   data: data,   contentType: 'application/json; charset=utf-8',   dataType: 'json',   async: false,   success: function(msg) {    }    });   }); });</script>";
  doc += "<script>$(function(){setInterval(requestData, 500);function requestData(){$.getJSON('/info',function(data){if(data){$.each(data, function(key,value) {$(\"#\"+key).text(\":\t\"+value);});}else{console.log('bad data');}});}});</script>";

  doc += "</head>\n";

  doc += "<body>\n";

  doc += "<h3>Thermal</h3>";
  doc += add_field("Temp Goal","temp_goal");
  doc += add_field("Temp Raw","temp_raw");
  doc += add_field("Temp Filtered","temp_filter");

  doc += "<br><h3>Info And Modes</h3>";
  doc += add_field("Thermally Ready","thermally_ready");
  doc += add_field("Current Mode","mode");
  doc += add_field("Mode State","mode_state");
  doc += add_field("Feed State","feed_state");
  doc += add_field("Sensor Good","former_sensor_is_nominal");

  doc += "<br><h3>Control</h3>";
  doc += add_field("Close Time","current_close_time");
  doc += add_field("Process Temp","current_process_temp");

  doc += add_field("PWM Former","pwm_f");
  doc += add_field("PWM Smasher","pwm_s");

  doc += add_field("Former Error","pid_f_e");
  doc += add_field("Smasher Error","pid_s_e");

  doc += add_field("Former Integral Error","pid_f_i");
  doc += add_field("Smasher Integral Error","pid_s_i");

  doc += "<br><h3>Input</h3>";
  doc += "<p>Close Time (ms)</p>";
  doc += "<p><input type=\"number\" id=\"close_time\" placeholder=\"1000\"><\p>";
  doc += "<p>Process Temperature (C)</p>";
  doc += "<p><input type=\"number\" id=\"process_temperature\" placeholder=\"130\"><\p>";
  doc += "<button id=\"sendbtn\">Send JSON</button> ";

  doc += "</body>";
  doc += "</html>";


  return doc;
}
//
// void print_thermal_status_wifi(){
//   WiFiClient client = server.available();   // Listen for incoming clients
//
//   if (client) {                             // If a new client connects,
//     String currentLine = "";                // make a String to hold incoming data from the client
//     while (client.connected()) {            // loop while the client's connected
//       if (client.available()) {             // if there's bytes to read from the client,
//         char c = client.read();             // read a byte, then
//         Serial.write(c);                    // print it out the serial monitor
//         header += c;
//         if (c == '\n') {                    // if the byte is a newline character
//           // if the current line is blank, you got two newline characters in a row.
//           // that's the end of the client HTTP request, so send a response:
//           if (currentLine.length() == 0) {
//             // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//             // and a content-type so the client knows what's coming, then a blank line:
//             client.println("HTTP/1.1 200 OK");
//             client.println("Content-type:text/html");
//             client.println("Connection: close");
//             client.println();
//
//             client.println("<!DOCTYPE html><html><head><title>Smasher Status</title>");
//             client.println("<meta http-equiv=\"refresh\" content=\"60\"></head><body>");
//
//             // client.println("<script type=\"text/javscript\"> function sendJSON(){ let result = document.querySelector('.result'); let close_time = document.querySelector('#close_time');let process_temp = document.querySelector('#process_temp');  let xhr = new XMLHttpRequest();let url = 'smasher.local';xhr.open('POST', url, true);  xhr.setRequestHeader('Content-Type', 'application/json'); var data = JSON.stringify({ 'close_time': close_time.value, 'process_temp': process_temp.value });xhr.send(data);}</script>");
//
//             client.println("<p>Temperature Goal: ");
//             client.print(Temperature_Goal_Former);
//             client.println("</p><p>Raw Temperature: ");
//             client.print(temp_raw_former);
//             client.println("</p><p>Filtered Temperature: ");
//             client.print(temperature_former);
//             client.println("</p>");
//
//             client.println("<p>_________________________________________");
//             client.println("<p>Former Control</p>");
//             client.print("<p>PID P Error %: ");
//             client.println(pid_former_error*100.0 /255);
//             client.print("</p><p>PID I Error %: ");
//             client.println(pid_former_ei*100.0 /255);
//             client.print("</p><p>PWM Output  %: ");
//             client.println(current_pwm_former*100.0 /255);
//             client.println("</p>");
//
//             client.println("<p>_________________________________________");
//             client.println("<p>Smasher Control</p>");
//             client.print("<p>PID P Error %: ");
//             client.println(pid_smasher_error*100.0 /255);
//             client.print("</p><p>PID I Error %: ");
//             client.println(pid_smasher_ei*100.0 /255);
//             client.print("</p><p>PWM Output  %: ");
//             client.println(current_pwm_smasher*100.0 /255);
//             client.println("</p>");
//
//             client.println("<p>_________________________________________");
//             client.print("<p>Thermal Sensor Nominal: ");
//             client.println(former_sensor_is_nominal);
//             client.print("</p><p>Thermally Ready: ");
//             client.println(thermally_ready);
//             client.println("</p>");
//
//             client.println("<p>_________________________________________");
//             client.print("<p>Do Feed: ");
//             client.println(do_feeding);
//             client.print("</p><p>Piston Closed: ");
//             client.print(piston_closed);
//             client.print("</p>");
//
//             client.println("<p>_________________________________________</p>");
//             client.print("<p><input type=\"number\" id=\"close_time\" placeholder=\"" );
//             client.print(CLOSE_TIME);
//             client.println("\"></p>");
//             client.print("<p><input type=\"number\" id=\"process_temp\" placeholder=\"" );
//             client.print(PROCESS_TEMP);
//             client.print("\"></p>");
//             client.println("<p>_________________________________________</p>");
//             client.println("<button onclick=\"function sendJSON(){ let close_time = document.querySelector('#close_time');let process_temp = document.querySelector('#process_temp');  let xhr = new XMLHttpRequest();let url = 'http://smasher.local';xhr.open('POST', url, true);  xhr.setRequestHeader('Content-Type', 'application/json'); var data = JSON.stringify({ 'close_time': close_time.value, 'process_temp': process_temp.value });xhr.send(data);};sendJSON();\">Send JSON</button> ");
//
//
//
//             client.println("</body></html>");
//
//             //break;
//           } else { // if you got a newline, then clear currentLine
//
//             //Process CurrentLine
//             //Serial.print("Got A Message:");
//             //Serial.println(currentLine);
//             //Try To Parse JSON
//             StaticJsonDocument<256> doc;
//             DeserializationError err = deserializeJson(doc, currentLine);
//
//             if (err) { //Check if successful
//               Serial.print(F("deserializeJson() returned "));
//               Serial.println(err.c_str());
//               serializeJsonPretty(doc, Serial);
//             }
//             else{ //It was
//               CLOSE_TIME = doc["close_time"].as<int>(); //ms
//               PROCESS_TEMP = doc["process_temp"].as<int>(); //ms
//             }
//
//             //Clear now
//             currentLine = "";
//           }
//         } else if (c != '\r') {  // if you got anything else but a carriage return character,
//           currentLine += c;      // add it to the end of the currentLine
//         }
//       }
//     }
//     // Clear the header variable
//     header = "";
//     // Close the connection
//     client.stop();
//   }
// }


// void wifi_status_task(void * parameter){
//
//   const TickType_t xDelay = wifi_interval_ms / portTICK_PERIOD_MS;
//
//   begin_server();
//   vTaskDelay(xDelay);
//
//   while(true){
//     try{
//         print_thermal_status_wifi();
//     }
//     catch(int e){}
//     vTaskDelay(xDelay);
//   }
// }

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
