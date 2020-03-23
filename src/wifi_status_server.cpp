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
      root["pid_f_i"] = pid_former_ei*100.0 /255;
      root["pwm_f"] = current_pwm_former*100.0 /255;

      root["pid_s_e"] =  pid_smasher_error;
      root["pid_s_i"] = pid_smasher_ei*100.0 /255;
      root["pwm_s"] = current_pwm_smasher*100.0 /255;

      root["former_sensor_is_nominal"] = former_sensor_is_nominal;
      root["thermally_ready"] = thermally_ready;
      root["mode"] = current_mode_name(current_mode);
      root["feed_state"] = FEED_STATE;
      root["mode_state"] = MODE_STATE;


      root["current_delay_time"] = DELAY_TIME;
      root["current_eject_time"] = EJECT_TIME;
      root["current_feed_time"] = FEED_TIME;
      root["current_piston_time"] = PISTON_TIME;
      root["current_close_time"] = CLOSE_TIME;
      root["current_process_temp"] = PROCESS_TEMP;

      response->setLength();

      request->send(response);
  });

  server.on("/process_setting", HTTP_POST, [](AsyncWebServerRequest * request){
      // int headers = request->headers();
      // int i;
      // for(i=0;i<headers;i++){
      //   Serial.printf("HEADER[%s]: %s\n", request->headerName(i).c_str(), request->header(i).c_str());
      //}
      request -> send(200);
      },
      NULL,
      [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

          String body = String((char *)data);
          Serial.println(body);
          StaticJsonDocument<200> doc;
          deserializeJson(doc, body);

          CLOSE_TIME = doc["close_time"];
          FEED_TIME = doc["feed_time"];
          EJECT_TIME = doc["eject_time"];
          PISTON_TIME = doc["piston_time"];
          DELAY_TIME = doc["delay_time"];
          PROCESS_TEMP = doc["process_temp"];

          Serial.print("Got Close Time: ");Serial.println(CLOSE_TIME);
          Serial.print("Got FEED_TIME: ");Serial.println(FEED_TIME);
          Serial.print("Got EJECT_TIME: ");Serial.println(EJECT_TIME);
          Serial.print("Got PISTON_TIME: ");Serial.println(PISTON_TIME);
          Serial.print("Got DELAY_TIME: ");Serial.println(DELAY_TIME);
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




String html_doc(){
  String doc;
  doc += "<!doctype html>\n";
  doc += "<html>\n";
  doc += "<head>\n";
  doc += "<title>Spring Flo Former Status</title>\n";
  doc += "<style>line-height:10px;</style>\n";
  doc += "<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>\n";
  doc += "<script>$(document).ready(function(){ $(\"#sendbtn\").click(function(){    let closetime = $('#close_time').val(); let feed_time = $('#feed_time').val(); let eject_time = $('#eject_time').val(); let piston_time = $('#piston_time').val(); let delay_time = $('#delay_time').val();  let temp = $('#process_temperature').val();    var data = JSON.stringify({ \"process_temp\": parseInt(temp), \"close_time\": parseInt(closetime), \"feed_time\": parseInt(feed_time),\"eject_time\": parseInt(eject_time),\"piston_time\": parseInt(piston_time),\"delay_time\": parseInt(delay_time)  });   console.log(data);    $.ajax({   url: '/process_setting',   type: 'POST',   data: data,   contentType: 'application/json; charset=utf-8',   dataType: 'json',   async: false,   success: function(msg) {    }    });   }); });</script>";
  doc += "<script>$(function(){setInterval(requestData, 500);function requestData(){$.getJSON('/info',function(data){if(data){$.each(data, function(key,value) {$(\"#\"+key).text(\":\t\"+value);});}else{console.log('bad data');}});}});</script>";

  doc += "</head>\n";

  doc += "<body>\n";

  doc += "<h3>Thermal</h3>";
  doc += add_field("Process Temp","current_process_temp");
  doc += add_field("Temp Goal","temp_goal");
  doc += add_field("Temp Raw","temp_raw");
  doc += add_field("Temp Filtered","temp_filter");

  doc += "<br><h3>Info And Modes</h3>";
  doc += add_field("Thermally Ready","thermally_ready");
  doc += add_field("Current Mode","mode");
  doc += add_field("Mode State","mode_state");
  doc += add_field("Feed State","feed_state");
  doc += add_field("Sensor Good","former_sensor_is_nominal");

  doc += "<br><h3>Timing</h3>";
  doc += add_field("Close Time","current_close_time");
  doc += add_field("Feed Time","current_feed_time");
  doc += add_field("Eject Time","current_eject_time");
  doc += add_field("Piston Time","current_piston_time");
  doc += add_field("Delay Time","current_delay_time");

  doc += "<br><h3>Control</h3>";
  doc += add_field("PWM Former","pwm_f");
  doc += add_field("PWM Smasher","pwm_s");

  doc += add_field("Former Error","pid_f_e");
  doc += add_field("Smasher Error","pid_s_e");

  doc += add_field("Former Integral Error","pid_f_i");
  doc += add_field("Smasher Integral Error","pid_s_i");

  doc += "<br><h3>Input</h3>";
  doc += add_input("Process Temperature","process_temperature",PROCESS_TEMP);
  doc += add_input("Close Time (ms)","close_time",CLOSE_TIME);
  doc += add_input("Feed Time (ms)","feed_time",FEED_TIME);
  doc += add_input("Eject Time (ms)","eject_time",EJECT_TIME);
  doc += add_input("Piston Time (ms)","piston_time",PISTON_TIME);
  doc += add_input("Delay Time (ms)","delay_time",DELAY_TIME);
  doc += "<button id=\"sendbtn\">Send JSON</button> ";

  doc += "</body>";
  doc += "</html>";


  return doc;
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


String add_input(String visible_name, String tag_name, int defaultvalue){
  String line;
  line += "<p>";
  line += visible_name;
  line += "</p>";
  line += "<input type=\"number\" id=\"";
  line += tag_name;
  line += "\" placeholder=\"";
  line += String(defaultvalue);
  line += "\" value=\"";
  line += String(defaultvalue);
  line += "\">";
  return line;
}
