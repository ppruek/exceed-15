#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <EspSoftwareSerial.h>

SoftwareSerial se_read(D5, D6); // write only
SoftwareSerial se_write(D0, D1); // read only

struct ProjectData {
  uint16_t temp;
  uint16_t light_lux;
};

struct ServerData {
  uint16_t temp;
};

// wifi configuration
const char SSID[] = "ase-407";
const char PASSWORD[] = "bigbaby007";

// for nodemcu communication
uint32_t last_sent_time = 0;
char expected_data_size = 0;
char cur_data_header = 0;
char buffer[256];
int8_t cur_buffer_length = -1;

void send_to_arduino(char code, void *data, char data_size) {
  char *b = (char*)data;
  char sent_size = 0;
  while (se_write.write(code) == 0) {
    delay(1);
  }
  while (sent_size < data_size) {
    sent_size += se_write.write(b, data_size);
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

void setup() {
  serial_initialization();
  wifi_initialization();

  Serial.print("ESP READY!");
}

bool GET(const char *url, void (*callback)(String const&)) {
  HTTPClient main_client;
  Serial.print("GET: ");
  Serial.println(url);
  main_client.begin(url);
  if (main_client.GET() == HTTP_CODE_OK) {
    Serial.println("GET REQUEST RESPONSE OK");
    if (callback != 0) {
      callback(main_client.getString());
    }
    return true;
  }
  Serial.println("GET REQUEST ERROR");
  return false;
}

void update_data_to_server_callback(String const &str) {
  Serial.println("update_data_to_server_callback FINISHED!");
}

const char SEND_PROJECT_DATA_TO_NODEMCU = 1; 

String set_builder(const char *key, int16_t value) {
  String str = "http://ku-exceed-backend.appspot.com/api/";
  str = str + key;
  str = str + "/set/?value=";
  str = str + value;
  return str;
}

void loop() {
  /* 1) Server polling data from server every 1500 ms
   * 2) Arduino always get local data
   */

  uint32_t cur_time = millis();
  
  if (cur_time - last_sent_time > 2000) {
    last_sent_time = cur_time;
  }
  
  // register(SEND_PROJECT_DATA_TO_NODEMCU, sizeof(ProjectData));
  while (se_read.available()) {
    char ch = se_read.read();
    
    if (cur_buffer_length == -1) {
      cur_data_header = ch;
      switch (cur_data_header) {
        case SEND_PROJECT_DATA_TO_NODEMCU:
          expected_data_size = sizeof(ProjectData);
          cur_buffer_length = 0;
          break;
      }
    } else if (cur_buffer_length < expected_data_size) {
      buffer[cur_buffer_length++] = ch;
      if (cur_buffer_length == expected_data_size) {
        switch (cur_data_header) {
          case SEND_PROJECT_DATA_TO_NODEMCU: {
            ProjectData *project_data = (ProjectData*)buffer; 
            uint16_t temp = project_data->temp;
            uint16_t light_lux = project_data->light_lux;
            
            GET(set_builder("project-data-temp", temp).c_str(), update_data_to_server_callback);
            GET(set_builder("project-data-light", light_lux).c_str(), update_data_to_server_callback);
          }
          break;
        }
        cur_buffer_length = -1;
      }
    }
  }
}
