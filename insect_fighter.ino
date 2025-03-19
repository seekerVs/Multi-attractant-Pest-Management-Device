
// Button pins
const int lightBtnPin = 10;
const int ultravioletBtnPin = 11;
const int vaporizerBtnPin = 12;
const int ultrasonicBtnPin = 13;

// Relay pins
const int whiteLightRelayPin = 4;
const int yellowLightRelayPin = 5;
const int ultravioletRelayPin = 6;
const int vaporizerRelayPin = 7;
const int grillShockRelayPin = 8;
const int ultrasonicRelayPin = 9;

// LED indicator pins
const int lightLedPin = A5;
const int ultravioletLedPin = A4;
const int vaporizerLedPin = 3;
const int ultrasonicLedPin = 2;

// Sensor pins
const int lightSensorPin = A3;

// Button states
int lightBtnState = 0;
int ultrasonicBtnState = 0;
int ultravioletBtnState = 0;
int vaporizerBtnState = 0;

bool lastLightBtnState = LOW;
bool lastUltrasonicBtnState = LOW;
bool lastUltravioletBtnState = LOW;
bool lastVaporizerBtnState = LOW;

// Interval constants
const uint16_t interval_1s = 1000;
const uint16_t interval_3s = 3000;
const uint16_t interval_10s = 10000;
const unsigned long auto_interval = 10000;
const uint16_t idle_interval = 300;

// Time delays
unsigned long currentMillis = 0;
unsigned long modeButtonEventMillis = 0;
unsigned long onVaporMillis = 0;
unsigned long offVaporMillis = 0;
unsigned long autoModeMillis = auto_interval;
unsigned long startTime = 0;
unsigned long previousMillis = 0;

int lightMode = 0;
int autoMode = 0;
int step = 0;

bool isIdle = true;
bool isAuto = false;
bool timerStarted = false;

int lightSensorValue = 0;
const int light_threshold = 600;

void setup() {
  Serial.begin(9600);

  pinMode(lightLedPin, OUTPUT);
  pinMode(ultravioletLedPin, OUTPUT);
  pinMode(vaporizerLedPin, OUTPUT);
  pinMode(ultrasonicLedPin, OUTPUT);

  pinMode(lightBtnPin, INPUT);
  pinMode(ultrasonicBtnPin, INPUT);
  pinMode(ultravioletBtnPin, INPUT);
  pinMode(vaporizerBtnPin, INPUT);

  pinMode(whiteLightRelayPin, OUTPUT);
  pinMode(yellowLightRelayPin, OUTPUT);
  pinMode(ultrasonicRelayPin, OUTPUT);
  pinMode(ultravioletRelayPin, OUTPUT);
  pinMode(vaporizerRelayPin, OUTPUT);
  pinMode(grillShockRelayPin, OUTPUT);
}

void loop() {
  // Serial.println("Working!!");
  currentMillis = millis();

  lightBtnState = digitalRead(lightBtnPin);
  ultrasonicBtnState = digitalRead(ultrasonicBtnPin);
  ultravioletBtnState = digitalRead(ultravioletBtnPin);
  vaporizerBtnState = digitalRead(vaporizerBtnPin);
  lightSensorValue = analogRead(lightSensorPin);
  
  // idle mode
  if (isIdle) {
    // Serial.print("Analog reading (0-1023): ");
    // Serial.println(lightSensorValue);
    if (lightSensorValue >= light_threshold) {
      
      if (!timerStarted) {
          startTime = millis(); // Start the timer
          timerStarted = true;
      }

      if (millis() - startTime >= interval_10s) {
          // Condition met for 10 seconds
          digitalWrite(grillShockRelayPin, HIGH);
          update_led_state();
          auto_mode();
          isAuto = true;
      }
    } else {
        // Reset the timer if the condition is not met
        timerStarted = false;
    // }
    
    // if (lightSensorValue >= 600) {
    //   // Serial.println("1");
    //   // Serial.print("Analog reading (0-1023): ");
    //   // Serial.println(lightSensorValue);
    //   digitalWrite(grillShockRelayPin, HIGH);
    //   update_led_state();

    //   auto_mode();
    //   isAuto = true;
    // } else {

      // Serial.println("3");
      digitalWrite(grillShockRelayPin, LOW);

      if (currentMillis - previousMillis >= idle_interval) {
        previousMillis = currentMillis;

        switch (step) {
            case 0:
                digitalWrite(lightLedPin, LOW);
                digitalWrite(ultravioletLedPin, HIGH);
                break;
            case 1:
                digitalWrite(ultravioletLedPin, LOW);
                digitalWrite(vaporizerLedPin, HIGH);
                break;
            case 2:
                digitalWrite(vaporizerLedPin, LOW);
                digitalWrite(ultrasonicLedPin, HIGH);
                break;
            case 3:
                digitalWrite(ultrasonicLedPin, LOW);
                digitalWrite(lightLedPin, HIGH);
                break;
        }
        step = (step + 1) % 4;
      }
    }
  } else {
    update_led_state();
  }

  if (lightBtnState == HIGH) {
    if (currentMillis - modeButtonEventMillis > 500) {
      Serial.println("Light Button Pressed");

      lightMode++;
      if (lightMode > 2) {
          lightMode = 0;
      }

      Serial.print("Mode: ");
      Serial.println(lightMode);

      if (lightMode == 0) {
        light_off();
      } else if (lightMode == 1) {
        yellow_light_mode();
      } else if (lightMode == 2) {
        white_light_mode();
      }
      check_mode();      
    }
    modeButtonEventMillis = currentMillis;
  } 
  else if (ultrasonicBtnState == HIGH) {
    if (currentMillis - modeButtonEventMillis > 500) {
      Serial.println("Ultrasonic Button Pressed");
      // isIdle = true;
      
      ultrasonic_mode();
      check_mode();
    }
    modeButtonEventMillis = currentMillis;
  }
  else if (ultravioletBtnState == HIGH) {
    if (currentMillis - modeButtonEventMillis > 500) {
      Serial.println("Ultraviolet Button Pressed");
      
      reset_idle();
      ultraviolet_mode();
      check_mode();
    }
    modeButtonEventMillis = currentMillis;
  }
  else if (vaporizerBtnState == HIGH) {
    if (currentMillis - modeButtonEventMillis > 500) {
      Serial.println("Vaporizer Button Pressed");
      
      vaporizer_mode();
      check_mode();
    }
    modeButtonEventMillis = currentMillis;
  } else {
    if (lastVaporizerBtnState == HIGH) {
      if (onVaporMillis == 0) {
        onVaporMillis = currentMillis;
      }
      else if (currentMillis - onVaporMillis >= interval_10s) {
        digitalWrite(vaporizerRelayPin, HIGH);
        Serial.println("Vaporizer ON");
        if (offVaporMillis == 0) {
          offVaporMillis = currentMillis;
        }
        else if (currentMillis - offVaporMillis >= interval_3s) {
          digitalWrite(vaporizerRelayPin, LOW);
          Serial.println("Vaporizer OFF");

          onVaporMillis = currentMillis;
          offVaporMillis = 0;
        }
      }
    }
  }
}

void check_mode() {
  digitalWrite(grillShockRelayPin, HIGH);
  if(lightMode == 0 && lastUltrasonicBtnState != HIGH && lastUltravioletBtnState != HIGH && lastVaporizerBtnState != HIGH) {
    isIdle = true;
    reset_auto();
  } else {
    isIdle = false;
    reset_auto();
    reset_idle();
  }
}

void reset_idle() {
  digitalWrite(vaporizerLedPin, LOW);
  digitalWrite(ultrasonicLedPin, LOW);
  digitalWrite(lightLedPin, LOW);
  digitalWrite(ultravioletLedPin, LOW);
  delay(500);
}

void reset_auto() {
  autoModeMillis = auto_interval;
  autoMode = 0;
  
  if (isAuto) {
    if (lightBtnState == HIGH) {
      // digitalWrite(whiteLightRelayPin, LOW);
      // digitalWrite(yellowLightRelayPin, LOW);
      // digitalWrite(lightLedPin, LOW);

      lastUltrasonicBtnState = LOW;
      digitalWrite(ultrasonicRelayPin, lastUltrasonicBtnState);
      // digitalWrite(ultrasonicLedPin, lastUltrasonicBtnState);

      lastUltravioletBtnState = LOW;
      digitalWrite(ultravioletRelayPin, lastUltravioletBtnState);
      // digitalWrite(ultravioletLedPin, lastUltravioletBtnState);

      lastVaporizerBtnState = LOW;
      digitalWrite(vaporizerRelayPin, lastVaporizerBtnState);
      // digitalWrite(vaporizerLedPin, lastVaporizerBtnState);
    }
    
    if (ultrasonicBtnState == HIGH) {
      lightMode = 0;
      digitalWrite(whiteLightRelayPin, LOW);
      digitalWrite(yellowLightRelayPin, LOW);
      digitalWrite(lightLedPin, LOW);

      lastUltravioletBtnState = LOW;
      digitalWrite(ultravioletRelayPin, lastUltravioletBtnState);
      // digitalWrite(ultravioletLedPin, lastUltravioletBtnState);

      lastVaporizerBtnState = LOW;
      digitalWrite(vaporizerRelayPin, lastVaporizerBtnState);
      // digitalWrite(vaporizerLedPin, lastVaporizerBtnState);
    }

    if (ultravioletBtnState == HIGH) {
      lightMode = 0;
      digitalWrite(whiteLightRelayPin, LOW);
      digitalWrite(yellowLightRelayPin, LOW);
      digitalWrite(lightLedPin, LOW);

      lastUltrasonicBtnState = LOW;
      digitalWrite(ultrasonicRelayPin, lastUltrasonicBtnState);
      // digitalWrite(ultrasonicLedPin, lastUltrasonicBtnState);

      lastVaporizerBtnState = LOW;
      digitalWrite(vaporizerRelayPin, lastVaporizerBtnState);
      // digitalWrite(vaporizerLedPin, lastVaporizerBtnState);
    }

    if (vaporizerBtnState == HIGH) {
      lightMode = 0;
      digitalWrite(whiteLightRelayPin, LOW);
      digitalWrite(yellowLightRelayPin, LOW);
      digitalWrite(lightLedPin, LOW);

      lastUltrasonicBtnState = LOW;
      digitalWrite(ultrasonicRelayPin, lastUltrasonicBtnState);
      // digitalWrite(ultrasonicLedPin, lastUltrasonicBtnState);

      lastUltravioletBtnState = LOW;
      digitalWrite(ultravioletRelayPin, lastUltravioletBtnState);
      // digitalWrite(ultravioletLedPin, lastUltravioletBtnState);
    }
  }

  isAuto = false;
}

void auto_mode() {
  if (currentMillis - autoModeMillis >= auto_interval) {
    Serial.println("Auto Mode Changed!");

    autoMode++;
    if (autoMode > 5) {
      // Turn off the last activated mode
      ultrasonic_mode();

      // Reset the loop
      autoMode = 1;
    }

    Serial.print("Auto Mode: ");
    Serial.println(autoMode);

    if (autoMode == 1) {
      light_off();
      delay(interval_1s);
      white_light_mode();
      lightMode = 1;
    } else if (autoMode == 2) {
      light_off();
      delay(interval_1s);
      yellow_light_mode();
      lightMode = 2;
    } else if (autoMode == 3) {
      lightMode = 0;
      light_off();
      delay(interval_1s);
      ultraviolet_mode();
    } else if (autoMode == 4) {
      ultraviolet_mode();
      delay(interval_1s);
      vaporizer_mode();
    } else if (autoMode == 5) {
      vaporizer_mode();
      delay(interval_1s);
      ultrasonic_mode();
    }

    autoModeMillis = currentMillis;
  }
}

void white_light_mode() {
  digitalWrite(lightLedPin, LOW);
  digitalWrite(whiteLightRelayPin, HIGH);
  digitalWrite(yellowLightRelayPin, LOW);
  delay(200);
  digitalWrite(lightLedPin, HIGH);
}

void yellow_light_mode() {
  digitalWrite(lightLedPin, LOW);
  digitalWrite(whiteLightRelayPin, LOW);
  digitalWrite(yellowLightRelayPin, HIGH);
  delay(200);
  digitalWrite(lightLedPin, HIGH);
}

void light_off() {
  digitalWrite(whiteLightRelayPin, LOW);
  digitalWrite(yellowLightRelayPin, LOW);
  delay(200);
  digitalWrite(lightLedPin, LOW);
}

void ultrasonic_mode() {
  digitalWrite(ultrasonicLedPin, lastUltrasonicBtnState);
  lastUltrasonicBtnState = !lastUltrasonicBtnState;
  digitalWrite(ultrasonicRelayPin, lastUltrasonicBtnState);
  delay(200);
  digitalWrite(ultrasonicLedPin, lastUltrasonicBtnState);
}

void ultraviolet_mode() {
  digitalWrite(ultravioletLedPin, lastUltravioletBtnState);
  lastUltravioletBtnState = !lastUltravioletBtnState;
  digitalWrite(ultravioletRelayPin, lastUltravioletBtnState);
  delay(200);
  digitalWrite(ultravioletLedPin, lastUltravioletBtnState);
}

void vaporizer_mode() {
  digitalWrite(vaporizerLedPin, lastVaporizerBtnState);
  lastVaporizerBtnState = !lastVaporizerBtnState;
  digitalWrite(vaporizerRelayPin, lastVaporizerBtnState);
  delay(200);
  digitalWrite(vaporizerLedPin, lastVaporizerBtnState);
}

void update_led_state() {
  digitalWrite(ultravioletLedPin, lastUltravioletBtnState);
  digitalWrite(vaporizerLedPin, lastVaporizerBtnState);
  digitalWrite(ultrasonicLedPin, lastUltrasonicBtnState);

  bool lightState = lightMode != 0;
  digitalWrite(lightLedPin, lightState);
}

