#include <ExceedInternal.h>
#include <ESP8266WiFi.h>

struct ProjectData {
  uint16_t temp;
  uint16_t light_lux;
} project_data = { 11   , 69 };

struct ServerData {
  uint16_t temp;
};

const char ARDUINO_GET_SERVER_DATA = 0;
const char SERVER_DATA_RESULT = 1;
const char ARDUINO_UPDATE_PROJECT_DATA = 1;

void setup() {
  // put your setup code   here, to run once:
  Serial.begin(115200);
  ExceedLib.begin(D5, D6, D0, D1, arduino_data_callback);
  ExceedLib.registerDataCode(ARDUINO_GET_SERVER_DATA, sizeof(ServerData));
  ExceedLib.registerDataCode(SERVER_DATA_RESULT, sizeof(ServerData));
  ExceedLib.registerDataCode(ARDUINO_UPDATE_PROJECT_DATA, sizeof(ProjectData));
  Serial.println("ARDUINO READY!");
}

void arduino_data_callback(char code, void *data) {
  switch (code) {
    case ARDUINO_GET_SERVER_DATA: {
      
    } break;
    case ARDUINO_UPDATE_PROJECT_DATA:
      ProjectData *data = (ProjectData*)data;
      break;
  }
}

uint32_t last_sent_time = 0;
void loop() {
  uint32_t cur_time = millis();
  if (cur_time - last_sent_time > 2000) {
    last_sent_time = cur_time;
  }

  ExceedLib.readSerial();
}

