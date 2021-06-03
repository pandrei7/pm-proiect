#include <FTDebouncer.h>
#include <LiquidCrystal_I2C.h>

#include "./Songs.h"

// Number of possible symbols.
#define SYMBOLS 6
// Number of symbols in a code sequence.
#define SEQ_LEN 4
// Number of attempts before the user loses.
#define TRIES 5
// Number of seconds before the user loses.
#define TIME 100

// A datatype for the state of the FSM.
enum class State {
    kPregame,
    kPreInput,
    kInput,
    kPreFeedback,
    kFeedback,
    kGameOver,
};

// The current state of the FSM.
State state = State::kPregame;

// User inputs.
const FTDebouncer pin_debouncer;
const int kButton = 8;
const int kPot = A0;

// Outputs for the user.
const int kLed0 = 7;
const int kLed1 = 6;
const int kLed2 = 5;
const int kLed3 = 4;
const int kBuzzer = 9;
const LiquidCrystal_I2C kLcd(0x27, 16, 2);

// The secret code.
char seq[SEQ_LEN + 1] = {};
// The user's latest guess.
char seq_tried[SEQ_LEN + 1] = {};
// The number of symbols in the user's guess.
int seq_tried_len = 0;

// The number of attempts left.
int tries = TRIES;
// The number of seconds left before the game ends.
int seconds_left = 0;

// The number of correctly matched symbols in the last guess.
int correct_elems = 0;
// The number of partially matched symbols in the last guess.
int partial_elems = 0;

// The current state of the button (used for debouncing).
bool button_is_pushed = false;

// A flag set when the actual game starts.
bool game_started = false;
// A flag set when the player guesses correctly.
bool player_won = false;

// Called once per second once the game starts.
ISR(TIMER1_COMPA_vect) {
    seconds_left -= 1;
}

// Seeds the random number generator with a fairly random value.
static void seedRandomly() {
    // If `A3` is unconnected, the seed will be random analog noise.
    randomSeed(analogRead(A3));

    for (int i = 0; i < 10; i += 1) {
        randomSeed(random(0, 1024) * analogRead(A0));
        randomSeed(random(0, 1024) * analogRead(A1));
        randomSeed(random(0, 1024) * analogRead(A2));
        randomSeed(random(0, 1024) * analogRead(A3));
        randomSeed(random(0, 1024) * analogRead(A4));
        randomSeed(random(0, 1024) * analogRead(A5));
    }
}

// Chooses the secret code randomly.
static void chooseRandomSeq() {
    for (int i = 0; i < SEQ_LEN; i += 1) {
        seq[i] = 'A' + random(0, SYMBOLS);
    }
}

// Converts the potentiometer reading into a symbol.
static char convertToSymbol(int pot) {
    int interval_len = 1024 / SYMBOLS;
    int interval = pot / interval_len;

    return 'A' + interval;
}

// Checks if the button is pressed.
// Only actual presses are counted, holding the button counts as one press.
static bool buttonIsPressed() {
    auto res = button_is_pushed;
    if (button_is_pushed) {
        button_is_pushed = false;
    }
    return res;
}

// Configures and starts the timer which ticks each second.
static void startTimer() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    OCR1A = 62500;
    TCCR1B |= 1 << WGM12;
    TCCR1B |= 1 << CS12;
    TIMSK1 |= 1 << OCIE1A;
    sei();
}

// The FSM behaviour before the actual game starts.
static void pregame() {
    if (buttonIsPressed()) {
        seconds_left = TIME;
        startTimer();
        game_started = true;
        state = State::kPreInput;
    }
}

// The FSM behaviour before the player can make a guess.
static void preInput() {
    kLcd.clear();
    kLcd.blink();

    kLcd.setCursor(0, 0);
    kLcd.print("Tries: " + String(tries));

    kLcd.setCursor(0, 1);
    kLcd.print(">");

    seq_tried_len = 0;

    state = State::kInput;
}

// The FSM behaviour while the user is inputting a guess.
static void input() {
    // Update the time counter, if needed.
    static int prev_seconds = 0;
    int curr_seconds = seconds_left;
    if (curr_seconds != prev_seconds) {
        kLcd.setCursor(10, 0);
        kLcd.print("T: ");
        if (curr_seconds < 10) {
            kLcd.print(" ");
        }
        kLcd.print(String(curr_seconds) + "s");
    }

    // Preview the next symbol.
    char symbol = convertToSymbol(analogRead(kPot));
    int col = 2 + seq_tried_len;
    kLcd.setCursor(col, 1);
    kLcd.print(symbol);
    kLcd.setCursor(col, 1);

    // Commit symbols when pressing the button.
    if (buttonIsPressed()) {
        seq_tried[seq_tried_len] = symbol;
        seq_tried_len += 1;
    }
    // Once the whole guess is completed, try to get feedback.
    if (seq_tried_len == SEQ_LEN) {
        tries -= 1;
        state = State::kPreFeedback;
    }
}

// Computes the number of correct and partial matches of the current guess.
static void evaluateGuess() {
    bool used[SEQ_LEN] = {};

    correct_elems = 0;
    for (int i = 0; i < SEQ_LEN; i += 1) {
        if (seq[i] == seq_tried[i]) {
            correct_elems += 1;
            used[i] = true;
        }
    }

    partial_elems = 0;
    for (int i = 0; i < SEQ_LEN; i += 1) {
        for (int j = 0; j < SEQ_LEN; j += 1) {
            if (!used[j] && seq_tried[i] == seq[j]) {
                partial_elems += 1;
                used[j] = true;
                break;
            }
        }
    }
}

// Prints some debugging information to the serial interface.
static void printDebugInfo() {
    Serial.println("Guessed: " + String(seq_tried));
    Serial.println(
        "Tries: " + String(tries) + "\tTime: " + String(seconds_left) + "s"
    );
}

// The FSM behaviour before the user sees their feedback.
static void preFeedback() {
    kLcd.clear();
    kLcd.noBlink();

    printDebugInfo();
    evaluateGuess();
    player_won = correct_elems >= SEQ_LEN;

    // Light up LEDs for correct matches.
    digitalWrite(kLed0, correct_elems > 0 ? HIGH : LOW);
    digitalWrite(kLed1, correct_elems > 1 ? HIGH : LOW);
    digitalWrite(kLed2, correct_elems > 2 ? HIGH : LOW);
    digitalWrite(kLed3, correct_elems > 3 ? HIGH : LOW);

    // Display the number of partial matches.
    kLcd.setCursor(0, 0);
    kLcd.print("Partial: " + String(partial_elems));

    // Display the guess for convenience.
    kLcd.setCursor(0, 1);
    kLcd.print("Guess was: " + String(seq_tried));

    state = State::kFeedback;
}

// The FSM behaviour while the user is reading their feedback.
static void feedback() {
    if (buttonIsPressed()) {
        state = State::kPreInput;
    }
}

// Checks if the game should end.
static bool gameIsOver() {
    return game_started && (player_won || tries <= 0 || seconds_left <= 0);
}

// The FSM behaviour after the game has ended.
static void gameOver() {
    kLcd.clear();
    kLcd.noBlink();

    // In some gases, this state can be reached without checking the guess.
    if (seq_tried_len >= SEQ_LEN && strncmp(seq, seq_tried, SEQ_LEN) == 0) {
        player_won = true;
    }

    if (player_won) {
        kLcd.setCursor(0, 0);
        kLcd.print("YOU WON!!!");
    } else {
        kLcd.setCursor(0, 0);
        kLcd.print("Game over!");
    }

    // Print the solution.
    kLcd.setCursor(16 - SEQ_LEN, 0);
    kLcd.print(String(seq));

    Song song = player_won ? kHappySong : kSadSong;
    while (true) {
        playSong(kBuzzer, song);
        delay(250);
    }
}

// A function required by `FTDebouncer`.
void onPinActivated(int pin) {
    button_is_pushed = true;
}

// A function required by `FTDebouncer`.
void onPinDeactivated(int pin) {
    button_is_pushed = false;
}

void setup() {
    // User inputs.
    pinMode(kButton, INPUT_PULLUP);
    pinMode(kPot, INPUT);
    pin_debouncer.addPin(kButton, HIGH, INPUT_PULLUP);
    pin_debouncer.begin();

    // Outputs.
    pinMode(kLed0, OUTPUT);
    pinMode(kLed1, OUTPUT);
    pinMode(kLed2, OUTPUT);
    pinMode(kLed3, OUTPUT);
    pinMode(kBuzzer, OUTPUT);

    kLcd.init();
    kLcd.backlight();

    // Display a welcome message.
    kLcd.clear();
    kLcd.setCursor(1, 0);
    kLcd.print("Press button");
    kLcd.setCursor(1, 1);
    kLcd.print("when ready...");

    // Choose the secret code.
    seedRandomly();
    chooseRandomSeq();

    Serial.begin(9600);
    Serial.println("Chosen code: " + String(seq));
}

void loop() {
    pin_debouncer.update();

    if (gameIsOver()) {
        state = State::kGameOver;
    }

    switch (state) {
    case State::kPregame:
        pregame();
        break;
    case State::kPreInput:
        preInput();
        break;
    case State::kInput:
        input();
        break;
    case State::kPreFeedback:
        preFeedback();
        break;
    case State::kFeedback:
        feedback();
        break;
    case State::kGameOver:
        gameOver();
        break;
    }
}
