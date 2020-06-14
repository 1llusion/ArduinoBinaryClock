// Credit to Elegoo 74HC595 tutorial
// connect to the ST_CP of 74HC595 (pin 3,latch pin)
int latchPin = 3;
// connect to the SH_CP of 74HC595 (pin 4, clock pin)
int clockPin = 4;
// connect to the DS of 74HC595 (pin 2)
int dataPin = 2;

// Speed adjustment pin
int inputPin = 0;

// Holds values of Seconds, Minutes, Hours
int clock_arr[] = {0,0,0};

// Offset minute time
int minute_time;

// Offset hour time
int hour_time = 0;

void setup() {
  Serial.begin(9600);
  // Set latchPin, clockPin, dataPin as output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // Setting the first 9 pins as output
  for(int i = 5; i <= 13; i++){
    pinMode(i,OUTPUT);
  }

  // Speed input
  pinMode(inputPin, INPUT);

  // Reseting the 74HC595
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, B00000000);
  digitalWrite(latchPin, HIGH);
}


// Turns on/off seconds segment and first 3 bits of minutes
void write_pin(int time_in, int pin){
  if (time_in & 1){
      digitalWrite(pin, HIGH);
   } else {
      digitalWrite(pin,LOW); 
   }
}

// Sets the correct values for last 3 bits of minutes + hours
void write_multi(){
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, hour_time | minute_time); // Flipping bits to preserve both hour time and minute time
   digitalWrite(latchPin, HIGH);
}

void update_seconds(int time_in){
  int pin = 14;
  while(pin-->=7){
    write_pin(time_in, pin);
    time_in = time_in >> 1;
  }  
}
void update_minutes(int time_in){
  /*
   * First 3 bits are pins 7 -> 4
   * Last 3 bits are multiplexed
   */
   int pin = 8; 
   minute_time = time_in >> 3; // Adding minutes offset so time aligns on the last 3 bits
   while(pin-->=0){
    if(pin >4){
      write_pin(time_in, pin);
      time_in = time_in >> 1;
     } else {
      write_multi();
     }
   }
}

void update_hours(int time_in){
  if(time_in >= 1){
    // Shifting the time to create space for minute values
    hour_time = time_in << 3;
    write_multi();
  }
}

void tick(int time_in, int pos){
  switch(pos){
    // Seconds
    case 0:
      update_seconds(time_in);
    break;
    // Minutes
    case 1:
    update_minutes(time_in);
    break;
    // Hours
    case 2:
    update_hours(time_in);
    break;
  }
 }

void loop() {
    // Reading the speed adjustment
    int spd = 0;
    spd = analogRead(inputPin);

    // Seconds should be precise, so adding a bit of wiggle room
    if(spd >= 950){
      spd = 1000;
    } 
    int time_int = 0;
    for(int i = 0; i < 3; i++){
      if(i < 3){
        if(clock_arr[i] >= 60){
          clock_arr[i] = 0;
          clock_arr[i+1] += 1;
        }
       } else {
        // Resetting the clock past 24 hours
        if(clock_arr[i] >= 24){
          memset(clock_arr, 0, sizeof(clock_arr));
        }
       }

       // Getting time for segment (seconds, minutes, hours)
       time_int = clock_arr[i];
       tick(time_int, i);
    }
    // Moving one second ahead
    clock_arr[0] += 1;
    delay(spd);
}
