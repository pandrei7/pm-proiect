const int kButton = 8;
const int kPot = A0;

void setup() {
    pinMode(kButton, INPUT_PULLUP);
    pinMode(kPot, INPUT);

    Serial.begin(9600);
}

void loop() {
    int button = digitalRead(kButton);
    int pot = analogRead(kPot);
    Serial.println("But: " + String(button) + "    Pot: " + String(pot));
}
