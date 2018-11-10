#include <SoftwareSerial.h>

SoftwareSerial se_read(12, 13); // write only
SoftwareSerial se_write(10, 11); // read only

struct ProjectData {
  uint16_t temp;
  uint16_t light_lux;
} project_data = { 11   , 69 };

struct ServerData {
  uint16_t temp;
};

void send_to_nodemcu(char code, void *data, char data_size) {
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

void setup() {
  // put your setup code   here, to run once:
  Serial.begin(115200);
  se_read.begin(38400);
  se_write.begin(38400);

  while (!se_read.isListening()) {
    se_read.listen();
  }
  
  Serial.println("ARDUINO READY!");
}

uint32_t last_sent_time = 0;
boolean is_data_header = false;
char expected_data_size = 0;
char cur_data_header = 0;
char buffer[256];
int8_t cur_buffer_length = -1;
const char SEND_PROJECT_DATA_TO_NODEMCU = 1;

void loop() {
  uint32_t cur_time = millis();
  if (cur_time - last_sent_time > 2000) {
    send_to_nodemcu(SEND_PROJECT_DATA_TO_NODEMCU, &project_data, sizeof(ProjectData));
    last_sent_time = cur_time;
  }

  while (se_read.available()) {
    char ch = se_read.read();
    if (cur_buffer_length == -1) {
      cur_data_header = ch;
      switch (cur_data_header) {
        default:
          break;
      }
    } else if (cur_buffer_length < expected_data_size) {
      buffer[cur_buffer_length++] = ch;
      if (cur_buffer_length == expected_data_size) {
        switch (cur_data_header) {
          default:
            break;
        }
        cur_buffer_length = -1;
      }
    }
  }
}

