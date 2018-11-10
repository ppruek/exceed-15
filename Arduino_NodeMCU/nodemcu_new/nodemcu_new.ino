#include <EspSoftwareSerial.h>
#include <ESP8266WiFi.h>

SoftwareSerial sw_serial(D5,D6); //(D5)left to (13)right arduino
//SoftwareSerial node_to_ar(D0,D1);

enum class DataCode {
  NONE = 0,
  ARDUINO_UPDATE_PROJECT_DATA = 1,
  ARDUINO_GET_PROJECT_DATA = 2,
  NODEMCU_PROJECT_PROJECT_DATA_RESULT = 3
};

struct ProjectData {
  int32_t is_button_pressed;
  float temp;
  int32_t light_ldr;
} local_storage = { 1, 23.4, 200};

const char HEADER[] = { 0x1F, 0x2F, 0x3F, 0x4F, 0x5F };
const char STATE_FINDING_HEADER = 0;
const char STATE_RECEIVE_DATA_CODE = 1;
const char STATE_RECEIVE_PAYLOAD = 2;
const char STATE_RECEIVE_END = 3;

DataCode receive_data_code = DataCode::NONE;
char cur_header_index = 0;
char receive_state = STATE_FINDING_HEADER;
char expected_payload_length = 0;
char buffer[255];
char cur_buffer_index = 0;

void send_to_arduino(DataCode code, void *data, int data_size){
  sw_serial.write(HEADER, sizeof(HEADER));
  char *b = (char*)data;
  sw_serial.write((char)code);
  for(int i = 0; i<data_size; i++){
    sw_serial.write(b[i]);
  }
}

void setup() {
  Serial.begin(115200);
  sw_serial.begin(115200);
  //node_to_ar.begin(9600);
  Serial.println();
  Serial.println("ESP READY");
}

uint32_t last_send = 0;

void loop() {
  /*uint32_t cur_time = millis();
  if (cur_time - last_send > 1000) {
    Serial.print("SEND: ");
    Serial.println(cur_time);
    send_to_arduino(DataCode::NODEMCU_PROJECT_PROJECT_DATA_RESULT, &local_storage, sizeof(ProjectData));
    last_send = cur_time;
  }*/
  while (sw_serial.available()) {
    char ch = sw_serial.read();
    switch (receive_state) {
      case STATE_FINDING_HEADER:
        Serial.println("STATE: FINDING_HEADER");
        if (ch == HEADER[cur_header_index]) {
          cur_header_index++;
          if (cur_header_index >= sizeof(HEADER)) {
            receive_state = STATE_RECEIVE_DATA_CODE;
            //Serial.println("NEXT: RECEIVE_DATA_CODE");
          }
        } else {
          cur_header_index = 0;
        }
        break;
      case STATE_RECEIVE_DATA_CODE:
        Serial.print("STATE: STATE_RECEIVE_DATA_CODE: ");
        Serial.println((int)ch);
        receive_data_code = (DataCode)ch;
        switch (receive_data_code) {
          case DataCode::ARDUINO_UPDATE_PROJECT_DATA:
            expected_payload_length = sizeof(ProjectData);
            receive_state = STATE_RECEIVE_PAYLOAD;
          break;
          case DataCode::ARDUINO_GET_PROJECT_DATA:
            expected_payload_length = sizeof(ProjectData);
            receive_state = STATE_RECEIVE_PAYLOAD;
          break;
          default:
            expected_payload_length = 0;
            receive_state = STATE_FINDING_HEADER;
            cur_header_index = 0;
          break;
        }
        break;
      case STATE_RECEIVE_PAYLOAD:
        Serial.println("STATE: RECEIVE PAYLOAD");
        if (cur_buffer_index < expected_payload_length) {
          buffer[cur_buffer_index++] = ch;
          Serial.print((int)ch);
          Serial.print(" ");
        } else {
          cur_buffer_index = 0;
          receive_state = STATE_RECEIVE_END;
          Serial.println();
        }
        break;
      case STATE_RECEIVE_END:
        switch (receive_data_code) {
          case DataCode::ARDUINO_UPDATE_PROJECT_DATA: { // { } use for declaration because switch case havn't stack
            ProjectData *data = (ProjectData*)buffer;
            Serial.println("ARDUINO_UPDATE_PROJECT_DATA");
            Serial.print("is_button_pressed: ");
            Serial.println(data->is_button_pressed);
            Serial.print("temperature: ");
            Serial.println(data->temp);
            Serial.print("light_ldr: ");
            Serial.println(data->light_ldr);
          } break;
          case DataCode::ARDUINO_GET_PROJECT_DATA: {
            while (sw_serial.available()){
              sw_serial.read();
            }
            send_to_arduino(DataCode::NODEMCU_PROJECT_PROJECT_DATA_RESULT, &local_storage, sizeof(ProjectData));
          } break;
        }
        receive_state = STATE_FINDING_HEADER;
        cur_header_index = 0;
        break;
    }
  }
}
