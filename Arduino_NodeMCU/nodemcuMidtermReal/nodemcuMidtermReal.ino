#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <EspSoftwareSerial.h>
#include <math.h>

SoftwareSerial se_read(D5, D6); // write only
SoftwareSerial se_write(D0, D1); // read only

struct ProjectData {
  int32_t is_button_pressed;
  float temp;
  int32_t light_lux;
  float sound;
  int32_t door;
} cur_project_data;

struct ServerData {
  int32_t is_button_pressed;
  float temp;
  int32_t light_lux;
  float sound;
  int32_t door;
} server_data;

const char GET_SERVER_DATA = 1;
const char GET_SERVER_DATA_RESULT = 2;
const char UPDATE_PROJECT_DATA = 3;

// wifi configuration
const char SSID[] = "mountz";
const char PASSWORD[] = "phunovalnwza";

// for nodemcu communication
uint32_t last_sent_time = 0;
char expected_data_size = 0;
char cur_data_header = 0;
char buffer[256];
int8_t cur_buffer_length = -1;

void send_to_arduino(char code, void *data, char data_size) {
  char *b = (char*)data;
  int sent_size = 0;
  while (se_write.write(code) == 0) {
    delay(1);
  }
  while (sent_size < data_size) {
    sent_size += se_write.write(b, data_size);
    Serial.print(sent_size);
    delay(1);
  }
}

void wifi_initialization() {
  Serial.println("WIFI INITIALIZING.");

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    yield();
    delay(10);
  }

  Serial.println("WIFI INITIALIZED.");
}

void serial_initialization() {
  Serial.begin(115200);
  se_read.begin(38400);
  se_write.begin(38400);

  while (!se_read.isListening()) {
    se_read.listen();
  }

  Serial.println();
  Serial.println("SERIAL INITIALIZED.");
}

String set_builder(const char *key, int32_t value) {
  String str = "http://ku-exceed-backend.appspot.com/api/";
  str = str + key;
  str = str + "/set/?value=";
  str = str + value;
  return str;
}

void update_data_to_server_callback(String const &str) {
  Serial.println("update_data_to_server_callback FINISHED!");
}

bool GET(const char *url, void (*callback)(String const &str,int32_t &value), int32_t &value) {
  HTTPClient main_client;
  main_client.begin(url);
  if (main_client.GET() == HTTP_CODE_OK) {
    Serial.println("GET REQUEST RESPONSE BEGIN");
    if (callback != 0) {
      callback(main_client.getString(),value);
    }
    return true;
  }
  Serial.println("GET REQUEST RESPONSE BEGIN");
  return false;
}
bool GET(const char *url, void (*callback)(String const &str,float &value), float &value) {
  HTTPClient main_client;
  main_client.begin(url);
  if (main_client.GET() == HTTP_CODE_OK) {
    Serial.println("GET REQUEST RESPONSE BEGIN");
    if (callback != 0) {
      callback(main_client.getString(),value);
    }
    return true;
  }
  Serial.println("GET REQUEST RESPONSE BEGIN");
  return false;
}

bool POST(const char *url, void (*callback)(String const &str)) {
  HTTPClient main_client;
  main_client.begin(url);
  if (main_client.GET() == HTTP_CODE_OK) {
    Serial.println("GET REQUEST RESPONSE BEGIN");
    if (callback != 0) {
      callback(main_client.getString());
    }
    return true;
  }
  Serial.println("GET REQUEST RESPONSE BEGIN");
  return false;
}

int get_request_int(String const &str) {
  int32_t tmp = str.toInt();
  return tmp;
}

float get_request_float(String const &str) {
  float tmp = str.toFloat();
  return tmp;
}
void get_request(String const &str, int32_t &value){
  value = get_request_int(str);
}
void get_request(String const &str, float &value){
  value = get_request_float(str);  
}

void get_request_raw_callback(String const &str) {
  Serial.println("GET REQUEST RESPONSE BEGIN");
  Serial.println("========================");
  Serial.println(str.c_str());
  Serial.println("========================");
  Serial.println("GET REQUEST RESPONSE END");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  serial_initialization();
  wifi_initialization();

  Serial.print("sizeof(ServerData): ");
  Serial.println((int)sizeof(ServerData));
  Serial.print("ESP READY!");
}
void loop() {
  /* 1) Server polling data from server every 1500 ms
     2) Arduino always get local data
  */

  uint32_t cur_time = millis();
  if (cur_time - last_sent_time > 1000) {
    //int variable
    GET("http://ku-exceed-backend.appspot.com/api/exceed-door/view/", get_request,server_data.door);
    Serial.print("door : ");
    Serial.println(server_data.door);
    //float variable
    GET("http://ku-exceed-backend.appspot.com/api/exceed-temperature/view/", get_request,server_data.temp); 
    Serial.print("temp : ");
    Serial.println(server_data.temp);
    
    //GET("http://ku-exceed-backend.appspot.com/api/exceed_value/set/?value=test", 0);
    last_sent_time = cur_time;
  }

  while (se_read.available()) {
    char ch = se_read.read();
    //Serial.print("RECV: ");0
    //Serial.println((byte)ch);
    if (cur_buffer_length == -1) {
      cur_data_header = ch;
      switch (cur_data_header) {
        case UPDATE_PROJECT_DATA:
          expected_data_size = sizeof(ProjectData);
          cur_buffer_length = 0;
          break;
        case GET_SERVER_DATA:
          expected_data_size = sizeof(ServerData);
          cur_buffer_length = 0;
          break;
      }
    } else if (cur_buffer_length < expected_data_size) {
      buffer[cur_buffer_length++] = ch;
      if (cur_buffer_length == expected_data_size) {
        switch (cur_data_header) {
          case UPDATE_PROJECT_DATA: {
              ProjectData *project_data = (ProjectData*)buffer;
              float temp = project_data->temp;
              int32_t light_lux = project_data->light_lux;
              int32_t is_button_pressed = project_data->is_button_pressed;
              float sound = project_data->sound;
              int32_t door = project_data->door;
              POST(set_builder("exceed-temp", temp).c_str(), update_data_to_server_callback);
              POST(set_builder("exceed-light_lux", light_lux).c_str(), update_data_to_server_callback);
              POST(set_builder("exceed-is_button_pressed", is_button_pressed).c_str(), update_data_to_server_callback);
              POST(set_builder("exceed-sound", sound).c_str(), update_data_to_server_callback);
              POST(set_builder("exceed-door", door).c_str(), update_data_to_server_callback);
            }
            break;
          case GET_SERVER_DATA:
            send_to_arduino(GET_SERVER_DATA_RESULT, &server_data, sizeof(ServerData));
            break;
        }
        cur_buffer_length = -1;
      }
    }
  }
}
