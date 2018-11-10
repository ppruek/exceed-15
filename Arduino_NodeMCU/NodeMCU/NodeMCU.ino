#include <EspSoftwareSerial.h>
#include <ESP8266WiFi.h>

SoftwareSerial sw_serial(D5,D6); //(D5)left to (13)right arduino

enum class DataCode {
  NONE = 0,
  ARDUINO_UPDATE_PROJECT_DATA = 1,
  ARDUINO_GET_PROJECT_DATA = 2,
  NODEMCU_PROJECT_PROJECT_DATA_RESULT = 3
};

struct ProjectData {
  bool is_button_pressed;
  float temp;
  int light_ldr;
} local_storage = { false, 0, 0};

struct SerialData {
  DataCode data_code;
  ProjectData project_data;
};

char buffer[256];
int cur_buffer_index = 0;

void send_to_arduino(DataCode code, void *data, int data_size){
  char *b = (char*)data;
  Serial.print("SEND TO ARDUINO: ");
  Serial.println((byte)((char)code));
  sw_serial.write((char)code);
  for(int i = 0; i<data_size; i++){
    sw_serial.write(b[i]);
  }
}

DataCode receive_from_arduino() {
  if (sw_serial.available()) {
    cur_buffer_index = sw_serial.readBytes(buffer, sizeof(SerialData));
  }
  return cur_buffer_index > 0 ? (DataCode)buffer[0] : DataCode::NONE;
 
//  while(sw_serial.available()){
//    char b = sw_serial.read();
//    buffer[cur_buffer_index++] = b;
//  }
//  if (cur_buffer_index > 0) {
//    Serial.print("LENGHT: ");
//    Serial.println(cur_buffer_index);
//    cur_buffer_index = 0;
//    return (DataCode)buffer[0];
//  }
//  return DataCode::NONE;
}

void setup() {
  Serial.begin(115200);
  sw_serial.begin(9600);

  delay(2000);
  while (sw_serial.available()) {
    sw_serial.read();  
  }

  Serial.println();
  Serial.println("ESP READY");

}

void loop() {
  /*local_storage = {true, 123.456, 321};
  send_to_arduino(DataCode::NODEMCU_PROJECT_PROJECT_DATA_RESULT, &local_storage, sizeof(ProjectData));
  delay(1000);*/

  // 4 Bytes [ 0x11, 0x22, 0x33, 0x44 ] [ 2 Bytes : Length ] [ . . . . . . ]

  int length = sw_serial.readBytesUntil(255, buffer, sizeof(buffer));
  for (int i = 0; i < length; i++) {
    Serial.print(buffer[i]);
  }
  Serial.println();
  
  /*switch (receive_from_arduino()){
    case DataCode::ARDUINO_UPDATE_PROJECT_DATA: {
      Serial.print("UPDATE: ");
      Serial.println(cur_buffer_index);
      ProjectData *data = (ProjectData*)buffer;
      Serial.println(data->is_button_pressed);
      Serial.println(data->temp);
      Serial.println(data->light_ldr);
      //update to server; 
    }break;
    case DataCode::ARDUINO_GET_PROJECT_DATA:{
      Serial.println("GET");
      local_storage = {true, 123.456, 321};
      send_to_arduino(DataCode::NODEMCU_PROJECT_PROJECT_DATA_RESULT, &local_storage, sizeof(ProjectData));
    }break;
  }
  delay(2);*/
}
