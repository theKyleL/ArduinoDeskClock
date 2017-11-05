/*
* Arduino Desk Clock
* Written by Kyle Latino
*
* This clock incorporates a 4-digit Common Cathode 7-segment display.
* Anodes will be grounded using a hex inverter.
*
*
*
*Three switches will be used to set the time. (Hour and Minute incrementing. Second Clearing)
*
* Still needs functions for reading set switch inputs.
*
*
*/

//Global Variables

int seconds = 0; // will not be displayed on clock, therefore will be represented as a single variable.
int min1 = 0; // Ones place minute digit.
int min10 = 0; // Tens place minute digit.
int hour1 = 0; // Ones place hour digit.
int hour10 = 0; // Tens place hour digit.

bool secFlag = false; // interrupt flag to remain false between 1hz counts

unsigned long previousTime = 0;
unsigned long currentTime = 0;
int setDelay = 500;

// function adds one to the miutes and clears the seconds
void setMinute(void){
  minCount();
}

// function adds one to the hours and clears the seconds
void setHour(void){
  hourCount();
}

// function to increment seconds and initiate minute incrementing.
void secondCount(void){
  seconds += 1;
  if (seconds > 59){
    seconds = 0;
    minCount(); // increment minutes if seconds are greater than 59.
  }

  // set pin 13 HIGH on even seconds to illuminate the colon.
  if ((seconds % 2) == 0){
    digitalWrite(13, HIGH);
  }
  else {
    digitalWrite(13, LOW);
  }
}

// function to increment minute values
void minCount(void){
  min1 += 1;
  if (min1 > 9){
    min1 = 0;
    min10 += 1;
    if (min10 > 5){
      hourCount(); // increment hours if minutes are greater than 59.
      min10 = 0;
    }
  }
}

// function to increment hour values
void hourCount(void){
  hour1 += 1;
  if (hour10 > 1 && hour1 > 3){ // if hours are greater than 23, clear them.
    hour1 = 0;
    hour10 = 0;
  }
  else if (hour1 > 9){ // carry the one.
    hour1 = 0;
    hour10 += 1;
  }
}

// function for decoding value to 7-segment form.
void displayDigit(int num){
  // Truth table for 7 seg decoder/driver.
  //Will be replaced with DEC to BCD when I receive the decoders (IOT save pins)
  switch (num) {
    case 0: // displays the case value in 7-segment format.
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    break;

    case 1:
    digitalWrite(0, LOW);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    break;

    case 2:
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    break;

    case 3:
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    break;

    case 4:
    digitalWrite(0, LOW);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    break;

    case 5:
    digitalWrite(0, HIGH);
    digitalWrite(1, LOW);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    break;

    case 6:
    digitalWrite(0, HIGH);
    digitalWrite(1, LOW);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    break;

    case 7:
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    break;

    case 8:
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    break;

    case 9:
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    break;

    default:
    break;
  }
}

//interrupt service routine
ISR(TIMER1_COMPA_vect){
  secFlag = true;// sets flag to increment seconds
}

void setup(){

  //stop interrupts
  cli();

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  for (int x = 0; x < 11; x++){ // set pins 0-10 as outputs. 0-6 control individual segments. 7-10 control inverter to ground display anodes.
    pinMode(x, OUTPUT);
    digitalWrite(x, LOW); // turn all digits off.
  }
  for (int x = 11; x < 13; x++){ // set pins 11-12 as inputs. Will be used to set clock hours, minutes, and seconds.
    pinMode(x, INPUT);
  }
  pinMode(13, OUTPUT);// Will be the driving pin for the colon symbol.

  //allow interrupts
  sei();

}//end setup

void loop(){

  //check interrupt flag
  if (secFlag){
    secFlag = false;
    secondCount();
  }

  //set current time
  currentTime = millis();

  //check for setMinute switch
  if (digitalRead(11) == HIGH){
    if ((currentTime - previousTime) > setDelay){
      previousTime = currentTime;
      setMinute();
    }
  }

  //check for setHour switch
  if (digitalRead(12) == HIGH){
    if ((currentTime - previousTime) > setDelay){
      previousTime = currentTime;
      setHour();
    }
  }

  //set segments for min1 digit
  displayDigit(min1);

  //turn on min 1 digit
  digitalWrite(7, HIGH);
  delay(1);

  //turn off min 1 digit
  digitalWrite(7, LOW);

  //set segments for min10 digit
  displayDigit(min10);

  //turn on min10 digit
  digitalWrite(8, HIGH);
  delay(1);

  //turn off min10 digit
  digitalWrite(8, LOW);

  //set segments for hour1 digit
  displayDigit(hour1);

  //turn on hour1 digit
  digitalWrite(9, HIGH);
  delay(1);

  //turn off hour1 digit
  digitalWrite(9, LOW);

  if (hour10 > 0){// only display leading digit when it is necessary
    //set segments for hour10 digit
    displayDigit(hour10);

    //turn on hour10 digit
    digitalWrite(10, HIGH);
    delay(1);

    //turn off hour10 digit
    digitalWrite(10, LOW);
  }
}

