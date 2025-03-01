#include <AccelStepper.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD globally
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper Motors
AccelStepper stepper1(AccelStepper::DRIVER, 2, 5);
AccelStepper stepper2(AccelStepper::DRIVER, 3, 6);
AccelStepper stepper3(AccelStepper::DRIVER, 4, 7);

// Rotation Data
int rotations[2] = {800, 1600}; 
int degrees[4] = {0, 200, 400, 600};
int direction[2] = {-1, 1};

const char* names[32] = {
    "Cornelius: The Keeper of Bees", "Boris: Executed", "Nigel: STAY AWAY", "Fergus: Freaky", "Jebediah: Chili Man", "Mortimer: Magician", "Vinny: French", "Rupert: leader of men",
    "Otis: Wizard", "Edgar: Echo Collector", "Reginald: Bug Man", "Waldo: Inquierer", "Bartholomew: Stabber", "Eustace: Meddler of Fate", "Gerald: Can Soup", "Winston: Destroyer of Man",
    "Percival: Dead", "Alfredo: Red Sauce", "Cletus: Cool", "Rufus: The Watcher", "Thaddeus: Mead", "Ulysses: Sphinx", "Clarence: Riddle Consultant", "Orville: Ballon Expert",
    "Marmaduke: Has A Pet Dog", "Cecil: Stirs Stews", "Eugene: Womanizer", "Horatio: Nelson", "Barnaby: Lover", "Seymour: Says Less", "Norbert: Weasle Wrangler", "Herbert: Owns A book"
};

//different dash frames

byte c0[8] = { B00000, B00001, B00010, B00100, B01000, B01000, B10000, B10000 };
byte c1[8] = { B11111, B00000, B00000, B00000, B00000, B00000, B00000, B00000 };
byte c2[8] = { B00000, B10000, B01000, B00100, B00010, B00010, B00001, B00001 };
byte c5[8] = { B10000, B10000, B01000, B01000, B00100, B00010, B00001, B00000 };
byte c4[8] = { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B11111 };
byte c3[8] = { B00001, B00001, B00010, B00010, B00100, B01000, B10000, B00000 };

// Stepper Parameters
const float maxSpeed = 500;
const float acceleration = 300;

// 2x3 Grid Positions for LCD Animation
int positions[6][2] = { {0, 0}, {0, 1}, {0, 2}, {1, 2}, {1, 1}, {1, 0} };

// Wrap-around function
int wrapIndex(int index, int length) {
    return (index % length + length) % length;
}

// Generates random step count for each motor
int cycles() {
    int randomCycles = random(0, 2); 
    int randomDegrees = random(0, 4); 
    int directions = random(0,2);
    return (rotations[randomCycles] + degrees[randomDegrees]) * direction[directions];
}

//  Runs the LCD animation for `twoSeconds`
void animated(int twoSeconds) {
    int startingIndex1 = 0; 
    int startingIndex2 = 1;
    int startingIndex3 = 2;

    unsigned long animationStartTime = millis(); // Reset start time each call
    unsigned long lastLCDUpdate = millis();      //Keep LCD update time

    const unsigned long lcdUpdateInterval = 200; // Animation frame interval

    //  Keep animating for `twoSeconds`
    while (millis() - animationStartTime < twoSeconds * 1000) {  
        
        
        stepper1.run();
        stepper2.run();
        stepper3.run();

        if (millis() - lastLCDUpdate > lcdUpdateInterval) {  //  Only update LCD at intervals
            lastLCDUpdate = millis();

            //  Update LCD positions
            lcd.setCursor(positions[startingIndex1][1] + 5, positions[startingIndex1][0]);
            lcd.write(byte(startingIndex1));

            lcd.setCursor(positions[startingIndex2][1] + 9, positions[startingIndex2][0]);
            lcd.write(byte(startingIndex2));

            lcd.setCursor(positions[startingIndex3][1] + 13, positions[startingIndex3][0]);
            lcd.write(byte(startingIndex3));

            // Erase position from three cycles ago
            int previousIndex1 = wrapIndex(startingIndex1 - 3, 6);
            int previousIndex2 = wrapIndex(startingIndex2 - 3, 6);
            int previousIndex3 = wrapIndex(startingIndex3 - 3, 6);

            lcd.setCursor(positions[previousIndex1][1] + 5, positions[previousIndex1][0]);
            lcd.print(" ");

            lcd.setCursor(positions[previousIndex2][1] + 9, positions[previousIndex2][0]);
            lcd.print(" ");

            lcd.setCursor(positions[previousIndex3][1] + 13, positions[previousIndex3][0]);
            lcd.print(" ");

            // Always stays in bound
            startingIndex1 = (startingIndex1 + 1) % 6;
            startingIndex2 = (startingIndex2 + 1) % 6;
            startingIndex3 = (startingIndex3 + 1) % 6;
        }
    } 
}

// Moves steppers & runs animation before movement
void moveSteppers(long target1, long target2, long target3) {
    stepper1.setCurrentPosition(0);
    stepper2.setCurrentPosition(0);
    stepper3.setCurrentPosition(0);
    delay(500);

    stepper1.moveTo(target1);
    stepper2.moveTo(target2);
    stepper3.moveTo(target3);

    float time1 = abs(target1) / maxSpeed;
    float time2 = abs(target2) / maxSpeed;
    float time3 = abs(target3) / maxSpeed;
    float maxTime = max(time1, max(time2, time3));

    if (time1 > 0) stepper1.setMaxSpeed(abs(target1) / maxTime);
    if (time2 > 0) stepper2.setMaxSpeed(abs(target2) / maxTime);
    if (time3 > 0) stepper3.setMaxSpeed(abs(target3) / maxTime);

    while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0 || stepper3.distanceToGo() != 0) {
        stepper1.run();
        stepper2.run();
        stepper3.run();
    }
}


void setup() {
    lcd.init();
    lcd.backlight();

    Serial.begin(9600);

    // Initialize Random Seed (Prevents Repetitive Results)
    randomSeed(analogRead(0));

    // Enable CNC shield
    pinMode(8, OUTPUT);
    digitalWrite(8, LOW);

    // Set acceleration for smooth movement
    stepper1.setAcceleration(acceleration);
    stepper2.setAcceleration(acceleration);
    stepper3.setAcceleration(acceleration);

    lcd.createChar(0, c0);
    lcd.createChar(1, c1);
    lcd.createChar(2, c2);
    lcd.createChar(3, c3);
    lcd.createChar(4, c4);
    lcd.createChar(5, c5);
    
}

void loop() {
    lcd.setCursor(0, 0);
    lcd.print("Find");
    lcd.setCursor(0, 1);
    lcd.print("Face");
    animated(3); // Run animation for 3 seconds before moving
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Generating....");

    moveSteppers(cycles(), cycles(), cycles());
    int randomIndex = random(0, 32);
    const char* fullText = names[randomIndex];
    lcd.clear();
    // Find the colon (":") in the string
    const char* colonPos = strchr(fullText, ':');

    if (colonPos != nullptr) {
        
        int nameLength = colonPos - fullText; // Length of name before ":"
        char namePart[17]; // Max 16 chars + null terminator
        strncpy(namePart, fullText, nameLength);
        namePart[nameLength] = '\0'; // Null-terminate the string

        // Extract the description (after ":")
        const char* descPart = colonPos + 2; // Skip ": " (colon and space)

        // Display on LCD
        lcd.setCursor(0, 0);
        lcd.print(namePart);
        lcd.setCursor(0, 1);
        lcd.print(descPart);
    } else {
        // If no colon found, print the full string on the first line
        lcd.setCursor(0, 0);
        lcd.print(fullText);
    }

    delay(3000);
    lcd.clear();
     // Move steppers with animation
}
