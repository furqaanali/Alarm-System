// digital pins
const int button = 8;
// variables
int buttonState = 0;
// set up button and serial
void setup() {
  pinMode(button, INPUT);
  Serial.begin(38400);
}
void loop() {
 // send signal if high
 buttonState = digitalRead(button);
 if (buttonState == HIGH) {
   Serial.write('1'); // send signal
 }
 delay(10);
}
