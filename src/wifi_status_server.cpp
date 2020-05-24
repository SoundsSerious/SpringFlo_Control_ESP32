#include <wifi_status_server.h>

AsyncWebServer server(80);
//AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
//AsyncEventSource events("/events"); // event source (Server-Sent events)

const char* ssid     = "SpaceCase";
const char* password = "9252623793";

String header = "";

portMUX_TYPE wifiMutex = portMUX_INITIALIZER_UNLOCKED;


void begin_server(){
  // attach AsyncWebSocket
  //ws.onEvent(onEvent);
  //server.addHandler(&ws);

  // Serial.println("Beginning Server");
  //TickType_t xdelay = 500/ portTICK_PERIOD_MS;
  // WiFi.begin(ssid, password);
  //
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.println("Connecting...");
  // }

  // attach AsyncEventSource
  //server.addHandler(&events);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Got / Request");
      request->send(200, "text/html",html_doc());
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/info", HTTP_GET, [] (AsyncWebServerRequest *request) {
      AsyncJsonResponse * response = new AsyncJsonResponse();
      response->addHeader("Server","Smasher Data");
      JsonVariant& root = response->getRoot();


      root["on_time"] =  millis()/(1000.0*60.0);

      root["temp_raw"] =  temp_raw_former;
      root["temp_filter"] = temperature_former;
      root["temp_goal"] = Temperature_Goal_Former;
      root["temp_past_min"] = temp_past_minute;

      root["pid_f_e"] =  pid_former_error;
      root["pid_f_i"] = pid_former_ei*100.0 /255;
      root["pwm_f"] = current_pwm_former*100.0 /255;

      root["pid_s_e"] =  pid_smasher_error;
      root["pid_s_i"] = pid_smasher_ei*100.0 /255;
      root["pwm_s"] = current_pwm_smasher*100.0 /255;

      root["former_sensor_is_nominal"] = former_sensor_is_nominal;
      root["thermally_ready"] = thermally_ready;
      root["sensor_failure"] = failure_message;
      root["continous_active"] = continous_active;
      root["mode"] = current_mode_name(current_mode);
      root["feed_state"] = FEED_STATE;
      root["mode_state"] = MODE_STATE;

      root["inst_cycle_time"] =  instantnious_cycle_time;
      root["inst_production_rate"] = instantnious_production_rate;
      root["past_hour_production_rate"] = past_hour_production_rate;
      root["cumulative_cycles"] = cumulative_cycles;

      root["current_delay_time"] = DELAY_TIME;
      root["current_eject_time"] = EJECT_TIME;
      root["current_feed_time"] = FEED_TIME;
      root["current_piston_time"] = PISTON_TIME;
      root["current_close_time"] = CLOSE_TIME;
      root["current_process_temp"] = PROCESS_TEMP;

      root["current_motor_rpm"] = MOTOR_RPM;
      root["current_motor_acl"] = MOTOR_ACL;
      root["current_motor_dcl"] = MOTOR_DCL;

      root["current_control_kp"] = K_P;
      root["current_control_ki"] = K_I;

      root["current_smasher_bias"] = smasher_pwm_bias;



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
          StaticJsonDocument<512> doc;
          deserializeJson(doc, body);

          portENTER_CRITICAL(&wifiMutex);
          CLOSE_TIME = doc["close_time"];
          FEED_TIME = doc["feed_time"];
          EJECT_TIME = doc["eject_time"];
          PISTON_TIME = doc["piston_time"];
          DELAY_TIME = doc["delay_time"];
          PROCESS_TEMP = doc["process_temp"];

          MOTOR_RPM = doc["motor_rpm"];
          MOTOR_ACL = doc["motor_acl"];
          MOTOR_DCL = doc["motor_dcl"];

          K_P = doc["control_kp"];
          K_I = doc["control_ki"];

          smasher_pwm_bias = max(min(int(doc["smsh_pwmbias"]),255),0);


          Temperature_Goal_Former = min(PROCESS_TEMP,MAX_TEMP);
          Temperature_Goal_Smasher = min(PROCESS_TEMP,MAX_TEMP);

          stepper.setRPM(MOTOR_RPM);
          int acl = MOTOR_ACL;
          int dcl = MOTOR_DCL;
          stepper.setSpeedProfile(stepper.LINEAR_SPEED,acl,dcl);

          portEXIT_CRITICAL(&wifiMutex);


          Serial.print("Got Close Time: ");Serial.println(CLOSE_TIME);
          Serial.print("Got FEED_TIME: ");Serial.println(FEED_TIME);
          Serial.print("Got EJECT_TIME: ");Serial.println(EJECT_TIME);
          Serial.print("Got PISTON_TIME: ");Serial.println(PISTON_TIME);
          Serial.print("Got DELAY_TIME: ");Serial.println(DELAY_TIME);
          Serial.print("Got Process Temp: ");Serial.println(PROCESS_TEMP);

          Serial.print("Motor RPM: ");Serial.println(MOTOR_RPM);
          Serial.print("Motor ACL: ");Serial.println(MOTOR_ACL);
          Serial.print("Motor DCL: ");Serial.println(MOTOR_DCL);

  });

  // AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/process_setting", [](AsyncWebServerRequest *request, JsonVariant &json) {
  //   Serial.print(json["process_temp"]);
  //   Serial.println(json["close_time"]);
  // });
  // server.addHandler(handler);

   delay(500);
   MDNS.begin("smash");
   delay(500);
   MDNS.addService("http", "tcp", 80);

   server.begin();
}




String html_doc(){
  String doc = F("<!doctype html>\n");
  doc += "<html>\n";
  doc += "<head>\n";
  doc += "<title>Spring Flo Former Status</title>\n";
  doc += "<style>line-height:10px;</style>\n";
  doc += "<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>\n";
  doc += "<script>$(document).ready(function(){ $(\"#sendbtn\").click(function(){    let closetime = $('#close_time').val(); let feed_time = $('#feed_time').val(); let eject_time = $('#eject_time').val(); let piston_time = $('#piston_time').val(); let delay_time = $('#delay_time').val();  let temp = $('#process_temperature').val();  let motor_rpm = $('#motor_rpm').val(); let motor_acl = $('#motor_acl').val();  let motor_dcl = $('#motor_dcl').val(); let control_kp = $('#control_kp').val();  let control_ki = $('#control_ki').val(); let smsh_pwmbias = $('#smsh_pwmbias').val();   var data = JSON.stringify({ \"process_temp\": parseInt(temp), \"close_time\": parseInt(closetime), \"feed_time\": parseInt(feed_time),\"eject_time\": parseInt(eject_time),\"piston_time\": parseInt(piston_time),\"delay_time\": parseInt(delay_time),\"motor_rpm\": parseInt(motor_rpm),\"motor_acl\": parseInt(motor_acl),\"motor_dcl\": parseInt(motor_dcl),\"control_kp\": parseFloat(control_kp),\"control_ki\": parseFloat(control_ki), \"smsh_pwmbias\": parseInt(smsh_pwmbias) });   console.log(data);    $.ajax({   url: '/process_setting',   type: 'POST',   data: data,   contentType: 'application/json; charset=utf-8',   dataType: 'json',   async: false,   success: function(msg) {    }    });   }); });</script>";
  doc += "<script>$(function(){setInterval(requestData, 1500);function requestData(){$.getJSON('/info',function(data){if(data){ console.log(data); $.each(data, function(key,value) {$(\"#\"+key).text(\":\t\"+value);});}else{console.log('bad data');}});}});</script>";

  doc += "</head>\n";

  doc += "<body>\n";
  doc += "<div style=\"display: table-row\">";

  doc += "<div style=\"width: 50%; display: table-cell;\">";
  doc += add_field("On Time (min)","on_time");
  doc += "<h3>Thermal (C) </h3>";
  doc += add_field("Process Temp","current_process_temp");
  doc += add_field("Temp Goal","temp_goal");
  doc += add_field("Temp Raw","temp_raw");
  doc += add_field("Temp Filtered","temp_filter");
  doc += add_field("Temp Past Min","temp_past_min");

  doc += "<br><h3>Control</h3>";
  doc += add_field("Control KP","current_control_kp");
  doc += add_field("Control KI","current_control_ki");

  doc += add_field("PWM Former","pwm_f");
  doc += add_field("PWM Smasher","pwm_s");
  doc += add_field("PWM Smasher Bias","current_smasher_bias");

  doc += add_field("Former Error","pid_f_e");
  doc += add_field("Smasher Error","pid_s_e");

  doc += add_field("Former Integral Error","pid_f_i");
  doc += add_field("Smasher Integral Error","pid_s_i");

  doc += "<br><h3>Info And Modes</h3>";
  doc += add_field("Sensor Good","former_sensor_is_nominal");
  doc += add_field("Thermally Ready","thermally_ready");
  doc += add_field("Sensor Failure","sensor_failure");
  doc += add_field("Current Mode","mode");
  doc += add_field("Active State","continous_active");
  doc += add_field("Mode State","mode_state");
  doc += add_field("Feed State","feed_state");


  doc += "<br><h3>Metrics</h3>";
  doc += add_field("Instantanious Cycle Time (s)","inst_cycle_time");
  doc += add_field("Instantanious Production Rate (units/hr)","inst_production_rate");
  doc += add_field("Production Rate Last Hour(units/hr)","past_hour_production_rate");
  doc += add_field("Cumuative Parts Made","cumulative_cycles");

  doc += "<br><h3>Timing</h3>";
  doc += add_field("Close Time","current_close_time");
  doc += add_field("Feed Time","current_feed_time");
  doc += add_field("Eject Time","current_eject_time");
  doc += add_field("Piston Time","current_piston_time");
  doc += add_field("Delay Time","current_delay_time");

  doc += "<br><h3>Motor</h3>";
  doc += add_field("Motor RPM","current_motor_rpm");
  doc += add_field("Motor ACL","current_motor_acl");
  doc += add_field("Motor DCL","current_motor_dcl");
  doc += "</div>";

  doc += "<div style=\"display: table-cell;\">";
  doc += "<br><h3>Input</h3>";
  doc += add_input("Process Temperature","process_temperature",PROCESS_TEMP);
  doc += add_input("Close Time (ms)","close_time",CLOSE_TIME);
  doc += add_input("Feed Time (ms)","feed_time",FEED_TIME);
  doc += add_input("Eject Time (ms)","eject_time",EJECT_TIME);
  doc += add_input("Piston Time (ms)","piston_time",PISTON_TIME);
  doc += add_input("Delay Time (ms)","delay_time",DELAY_TIME);

  doc += add_input("Motor Speed (rpm)","motor_rpm",MOTOR_RPM);
  doc += add_input("Motor ACL (rpm/s)","motor_acl",MOTOR_ACL);
  doc += add_input("Motor DCL (rpm/s)","motor_dcl",MOTOR_DCL);

  doc += add_input("Control KP","control_kp",K_P);
  doc += add_input("Control KI","control_ki",float(K_I));

  doc += add_input("Smasher PWM Bias (0-255)","smsh_pwmbias",smasher_pwm_bias);

  doc += "<br><button id=\"sendbtn\">Send JSON</button> ";
  doc += "</div>";

  doc += "</div>";
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


String add_input(String visible_name, String tag_name, float defaultvalue){
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
