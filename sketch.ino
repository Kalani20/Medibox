// Include libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <DHTesp.h>

// Define OLED parameters
#define SCREEN_WIDTH 128 // OLED display width,in pixels
#define SCREEN_HEIGHT 64 //OLED display height,in pixels
#define OLED_RESET -1 //Reset pin #
#define SCREEN_ADDRESS 0x3C

#define BUZZER 5
#define LED_1 15
#define PB_CANCEL 34
#define PB_DOWN 35
#define PB_OK 32
#define PB_UP 33
#define DHTPIN 12

#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 0
int UTC_OFFSET_DST = 3600;
// Declare objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

//Global variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

unsigned long time_now = 0;
unsigned long time_last = 0;

bool alarm_enabled = true;
int no_alarms = 3;
int alarm_hours[] = {0, 1, 2};
int alarm_minutes[] = {1, 10, 20};
bool alarm_triggered[] = {false, false, false};
int no_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};
int current_mode = 0;
int max_modes = 5;
String modes [] = {"Set Timezone", "Set Alarm 1", "Set Alarm 2", "Set Alarm 3", "Disable Alarms"};



void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_DOWN, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  dhtSensor.setup(DHTPIN, DHTesp::DHT22);


  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(200); //delay for 2 seconds

  // Connecting to WiFi
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.clearDisplay();
    print_line("Connecting-WiFi", 2, 0, 0);
  }
  display.clearDisplay();
  print_line("WiFi-connected", 2, 0, 0);


  // Getting the time
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  //update_time();



  // clear the buffer
  display.clearDisplay();

  print_line("Welcome", 2, 0, 0);
  print_line("to", 2, 20, 20);
  print_line("Medibox!", 2, 40, 50);
  delay(20);
  display.clearDisplay();

}
void loop() {
  update_time_with_check_alarm();
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }
  check_temp();
  display.display();

  
}
void print_line (String text, int textsize, int column, int row) {
  display.setTextSize(textsize); // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(column, row); // Start at (row,column)
  display.println(text);
  display.display();

} 

// Inorder to avoid blinking in the text we use this function.
void print_line_1 (String text, int textsize, int column, int row) {
  display.setTextSize(textsize); // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(column, row); // Start at (row,column)
  display.println(text);

} 

void print_time_now(void) {
  display.clearDisplay();
  print_line_1(String(days), 2, 0, 0);
  print_line_1(":", 2, 20, 0);
  print_line_1(String(hours), 2, 30, 0);
  print_line_1(":", 2, 50, 0);
  print_line_1(String(minutes), 2, 60, 0);
  print_line_1(":", 2, 80, 0);
  print_line_1(String(seconds), 2, 90, 0);
}

void update_time() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error occured in obtaining time.");
    return;
  }
  char timeH[3];
  strftime(timeH, 3, "%H", &timeinfo);
  hours = atoi(timeH);

  char timeMin[3];
  strftime(timeMin, 3, "%M", &timeinfo);
  minutes = atoi(timeMin);

  char timeSec[3];
  strftime(timeSec, 3, "%S", &timeinfo);
  seconds = atoi(timeSec);

  char timeDay[3];
  strftime(timeDay, 3, "%d", &timeinfo);
  days = atoi(timeDay);
}

void ring_alarm() {
  display.clearDisplay();
  print_line("MEDICINE-TIME", 2, 0, 0);
  digitalWrite(LED_1, HIGH);
  bool break_happened = false;
  // Ring the BUZZER
  while (break_happened == false && digitalRead(PB_CANCEL) == HIGH) {
    for (int i = 0; i < no_notes; i++) {
      if (digitalRead(PB_CANCEL) == LOW) {
        delay(20);
        break_happened = true;
        break;
      }
      tone(BUZZER, notes[i], 200);
      noTone(BUZZER);
      delay(2);
    }
  }
  digitalWrite(LED_1, LOW);
  display.clearDisplay();
}
void update_time_with_check_alarm(void) {
  display.clearDisplay();
  update_time();
  print_time_now();
 
  if (alarm_enabled == true) {
    for (int i = 0; i < no_alarms; i++) {
      if ( alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes) {
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}
// Wait and return which button is pressed
int wait_for_buttonpress() {
  while (true) {
    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    }
    else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    }
    else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    }
    else if (digitalRead(PB_CANCEL) == LOW) {
      delay(200);
      return PB_CANCEL;
    }
    update_time();
  }

}
void go_to_menu() {
  // To check if the CANCEL button is not pressed
  while (digitalRead(PB_CANCEL) == HIGH) {
    display.clearDisplay();
    print_line(modes[current_mode], 2, 0, 0);
    int pressed = wait_for_buttonpress();
    if (pressed == PB_UP) {
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      current_mode -= 1;
      if (current_mode < 0) {
        current_mode = max_modes - 1;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      //Serial.println(current_mode);
      run_mode(current_mode);
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }

}

void set_timezone() {
  float offset = 0;
  while (true) {
    display.clearDisplay();
    print_line("Select-timezone:" + String(offset),2, 0, 0);

    int pressed = wait_for_buttonpress();
    if (pressed == PB_UP) {
      delay(200);
      offset += 0.5;
      if (offset > 12){
        offset = -11.5;
      }
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      offset -= 0.5;
      if (offset < -11.5) {
        offset = 12;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      UTC_OFFSET_DST = offset * 3600;
      break;
    }

    else if (pressed = PB_CANCEL) {
      delay(200);
      break;
    }
  }
  
  display.clearDisplay();
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  print_line("Your Timezone is set to"+ String(offset),1, 0, 0);
  delay(1000);
}

void set_alarm(int alarm) {
  alarm_triggered[0] = false;
  alarm_triggered[1] = false;
  alarm_triggered[2] = false;

  int temp_hour = alarm_hours[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter hour:" + String(temp_hour), 2, 0, 0);

    int pressed = wait_for_buttonpress();
    if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }

    else if (pressed = PB_CANCEL) {
      delay(200);
      break;
    }

  }
  int temp_minute = alarm_minutes[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter minute:" + String(temp_minute), 2, 0, 0);

    int pressed = wait_for_buttonpress();
    if (pressed == PB_UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }

    else if (pressed = PB_CANCEL) {
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Alarm is set",2, 0, 0);
  /*// This is the way when all alarms are enabled, we cannot ring
  // alarm again. To ring alarm again we do this procedure.
  if (alarm_enabled== false){
    alarm_enabled = true;
  }*/

  delay(1000);
}
void run_mode(int mode) {
  if (mode == 0) {
    set_timezone();
  }
  else if (mode == 1 || mode == 2 || mode == 3) {
    set_alarm(mode - 1);
  }
  else if (mode == 4) {
    alarm_enabled = false;
  }
}
void check_temp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if (data.temperature > 32 ) {
    print_line_1("Temp-High ", 1.75 , 20, 20);
  }

  else if (data.temperature < 26 ) {
    print_line_1("Temp-Low", 1.75 , 20, 20);
  }

  if (data.humidity > 80 ) {
    print_line_1("Humidity-High", 1.75 , 20, 50);
  }

  if (data.humidity < 60 ) {
    print_line_1("Humidity-Low", 1.75 , 20, 50);
  }

}







