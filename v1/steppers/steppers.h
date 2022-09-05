/*
Ver 0.1
Stepper control testing
*/

#define RA_DIR_P 2
#define RA_STEP_P 3
#define DEC_DIR_P 4
#define DEC_STEP_P 5
#define STEPS_REVOLUTION 200

bool RA_dir = 0;
bool DEC_dir = 0;

void setup_motors(void) {
    pinMode(RA_DIR_P, OUTPUT);
    pinMode(RA_STEP_P, OUTPUT);
    pinMode(DEC_DIR_P, OUTPUT)
    pinMode(DEC_STEP_P, OUTPUT);

    digitalWrite(DEC_DIR_P, DEC_dir);
    digitalWrite(RA_DIR_P, RA_dir);
}

void step(byte step_pin) {
    unsigned long int time_now = millis();
    digitalWrite(step_pin, HIGH);
    delay(10);
    digitalWrite(step_pin, LOW);
}
