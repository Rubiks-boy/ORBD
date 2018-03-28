#include <Stepper.h>

//Number of steps in each revolution
const int STEPS_PER_REV = 2048;

//number of characters (steppers) and the number of I/O pins each uses
const int NUM_CHARS = 2;
const int NUM_PINS = 4;

//speed of each stepper (RPM)
const int SPEED = 15;

//number of characters on the left and right character wheels
const int LEFT_POSITIONS = 8, RIGHT_POSITIONS = 9;

//increment value for each wheel
const int LEFT_INC = 3, RIGHT_INC = -1;

//pins each stepper uses
const int STEPPER_PINS[NUM_CHARS][NUM_PINS] = {{46, 50, 48, 52}, {3, 5, 4, 6}};
//button pin to display a new set of characters
const int NEXT_BTN_PIN = 7;

//every character it can display, along with corresponding wheel indexes
const int NUM_DISPLAYED_CHARS = 42;
const char ALPHABET [NUM_DISPLAYED_CHARS] = {'~', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '.', ',', '?', '!', '#'};
const int POS_LEFT [NUM_DISPLAYED_CHARS] = {0, 3, 5, 3, 3, 3, 5, 5, 5, 2, 2, 4, 7, 4, 4, 4, 7, 7, 7, 6, 6, 4, 7, 2, 4, 4, 4, 3, 5, 3, 3, 3, 5, 5, 5, 2, 2, 2, 2, 6, 6, 1};
const int POS_RIGHT [NUM_DISPLAYED_CHARS] = {0, 0, 0, 3, 5, 2, 3, 5, 2, 3, 5, 0, 0, 3, 5, 2, 3, 5, 2, 3, 5, 1, 1, 7, 4, 7, 6, 0, 0, 3, 5, 2, 3, 5, 2, 3, 5, 6, 0, 1, 2, 7};

//delay for debouncing
const unsigned long BTN_DEBOUNCE = 50;

//remainder number of steps due to integer division
int remainder[NUM_CHARS] = {0};

//whether its ready to display a new set of characters
boolean readyForNext = true;

///last button state
boolean lastBtn = 0;

//last time the button toggled
unsigned long lastBtnToggle = 0;

//characters to display in braille
String charBuffer = "";

//whether the readyForNext already toggled this press
boolean alreadyToggled = false;

//declare the list of stepper motors
Stepper steppers[NUM_CHARS] = 
{
  Stepper(STEPS_PER_REV, STEPPER_PINS[0][0], STEPPER_PINS[0][1], STEPPER_PINS[0][2], STEPPER_PINS[0][3]),
  Stepper(STEPS_PER_REV, STEPPER_PINS[1][0], STEPPER_PINS[1][1], STEPPER_PINS[1][2], STEPPER_PINS[1][3])
};

//current stepper locations
int leftWheelPos[NUM_CHARS];
int rightWheelPos[NUM_CHARS];

//how many iterations are required to achieve the desired position, moving forwards
int itersReqdForward(int motorIndex, int charIndex)
{
  int newL = leftWheelPos[motorIndex];
  int newR = rightWheelPos[motorIndex];
  int iters = 0;

  while(newL != POS_LEFT[charIndex] || (newR != POS_RIGHT[charIndex] && newR - 8 != POS_RIGHT[charIndex] && newR + 8 != POS_RIGHT[charIndex]))
  {
    iters++;
    
    //increment the wheel positions
    newL += LEFT_INC;
    newR += RIGHT_INC;

    //take care of index overflows/underflows
    while(newL >= LEFT_POSITIONS)
    {
      newL -= LEFT_POSITIONS;
    }
      
    while(newR < 0)
    {
      newR += RIGHT_POSITIONS;
    }
  }

  Serial.print("Steps Required Forward: ");
  Serial.println(iters);

  return iters;
}

//how many iterations are required to achieve the desired position, moving backwards
int itersReqdBackward(int motorIndex, int charIndex)
{
  //current stepper locations
  int newL = leftWheelPos[motorIndex];
  int newR = rightWheelPos[motorIndex];
  int iters = 0;

  while(newL != POS_LEFT[charIndex] || (newR != POS_RIGHT[charIndex] && newR - 8 != POS_RIGHT[charIndex] && newR + 8 != POS_RIGHT[charIndex]))
  {
    iters++;
    
    //increment the wheel positions
    newL -= LEFT_INC;
    newR -= RIGHT_INC;

    //take care of index overflows/underflows
    while(newL < 0)
    {
      newL += LEFT_POSITIONS;
    }
      
    while(newR >= RIGHT_POSITIONS)
    {
      newR -= RIGHT_POSITIONS;
    }
  }

  Serial.print("Steps Required Backward: ");
  Serial.println(iters);
  return iters;
}

//move stepper motorIndex forward one iteration
void moveStepperForward(int motorIndex)
{ 
  //increment the wheel positions
  leftWheelPos[motorIndex] += LEFT_INC;
  rightWheelPos[motorIndex] += RIGHT_INC;

  //take care of index overflows/underflows
  while(leftWheelPos[motorIndex] >= LEFT_POSITIONS)
  {
    leftWheelPos[motorIndex] -= LEFT_POSITIONS;
  }
    
  while(rightWheelPos[motorIndex] < 0)
  {
    rightWheelPos[motorIndex] += RIGHT_POSITIONS;
  }

  //move the stepper motor the necessary amount
  //and move a remainder of steps (occurs over time due to integer division)
  double dblSteps = 1.0 * STEPS_PER_REV * RIGHT_INC / RIGHT_POSITIONS;
  int numSteps = (int)dblSteps;
  remainder[motorIndex] += dblSteps - numSteps;
  if(remainder[motorIndex] > 1.0)
  {
    steppers[motorIndex].step((int)remainder);
    remainder[motorIndex] -= (int)(remainder);
  }
  steppers[motorIndex].step(numSteps);

  Serial.print("C:");
  Serial.print(motorIndex);
  Serial.print(": L:");
  Serial.print(leftWheelPos[motorIndex]);
  Serial.print(" R:");
  Serial.println(rightWheelPos[motorIndex]);
}

//move stepper motorIndex backward one iteration
void moveStepperBackward(int motorIndex)
{
  //increment the wheel positions
  leftWheelPos[motorIndex] -= LEFT_INC;
  rightWheelPos[motorIndex] -= RIGHT_INC;

  //take care of index overflows/underflows
  while(leftWheelPos[motorIndex] < 0)
  {
    leftWheelPos[motorIndex] += LEFT_POSITIONS;
  }
    
  while(rightWheelPos[motorIndex] >= RIGHT_POSITIONS)
  {
    rightWheelPos[motorIndex] -= RIGHT_POSITIONS;
  }

  //move the stepper motor the necessary amount
  //and move a remainder of steps (occurs over time due to integer division)
  double dblSteps = 1.0 * STEPS_PER_REV * RIGHT_INC / RIGHT_POSITIONS;
  int numSteps = (int)dblSteps;
  remainder[motorIndex] -= dblSteps - numSteps;
  if(remainder[motorIndex] < -1.0)
  {
    steppers[motorIndex].step((int)remainder);
    remainder[motorIndex] -= (int)(remainder);
  }
  steppers[motorIndex].step(-numSteps);

  Serial.print("Character ");
  Serial.print(motorIndex);
  Serial.print(": L:");
  Serial.print(leftWheelPos[motorIndex]);
  Serial.print(" R:");
  Serial.println(rightWheelPos[motorIndex]);
}

//moves the stepper motor motorIndex steps steps
void advanceSteps(int motorIndex, int steps)
{
  Serial.print("Advancing Character ");
  Serial.print(motorIndex);
  Serial.print(" ");
  Serial.print(steps);
  Serial.println(" steps.");
  steppers[motorIndex].step(steps);
}

//sets the position that a stepper is currently at
void setPosition(int motorIndex, int l, int r)
{
  leftWheelPos[motorIndex] = l;
  rightWheelPos[motorIndex] = r;
  Serial.print("\nC:");
  Serial.print(motorIndex);
  Serial.print(" L:");
  Serial.print(l);
  Serial.print(" R:");
  Serial.print(r);
}

void setup() 
{
  //open serial port
  Serial.begin(9600);

  //set the speed of each stepper (rpm)
  for(int i = 0; i < NUM_CHARS; i++)
  {
    steppers[i].setSpeed(SPEED);
    leftWheelPos[i] = rightWheelPos[i] = 0;
  }

  pinMode(NEXT_BTN_PIN, INPUT);
  pinMode(13, OUTPUT);
}

void loop() 
{
  //read the button & set up next character if wanted
  boolean currBtn = digitalRead(NEXT_BTN_PIN);
  
  if(currBtn != lastBtn)
  {
    lastBtnToggle = millis();
    alreadyToggled = false;
  }

  if(millis() - lastBtnToggle > BTN_DEBOUNCE && !alreadyToggled)
  {
    Serial.println("Ready to display next");
    digitalWrite(13, HIGH);
    readyForNext = true;
    alreadyToggled = true;
  }

  if(millis() - lastBtnToggle > BTN_DEBOUNCE + 250)
  {
    digitalWrite(13, LOW);
  }

  //check if anything is available to read
  if(Serial.available() > 0)
  {
    //read the character inputted
    char inChar = Serial.read();

    //advance n motor x steps
    if(inChar == '`')
    {
      int mIndex = Serial.parseInt();
      int numSteps = Serial.parseInt();
      advanceSteps(mIndex, numSteps);
    }

    //set n motor's current position (wheel indexes)
    else if(inChar == '@')
    {
      int mIndex = Serial.parseInt();
      int lPos = Serial.parseInt();
      int rPos = Serial.parseInt();
      setPosition(mIndex, lPos, rPos);
    }

    //zero all the characters
    else if(inChar == '~')
    {
      //for each motor index
      for(int m = 0; m < NUM_CHARS; m++)
      {
        //set the position to 0 for both left and right wheels
        
        Serial.print("\nZeroing Character ");
        Serial.println(m);

        //see if front or back is more efficient
        int fIters = itersReqdForward(m, 0);
        int bIters = itersReqdBackward(m, 0);

        //move the motors
        if(fIters == 0 && bIters == 0) {}
        else if(fIters <= bIters)
          for(int i = 0; i < fIters; i++)
          {
            Serial.print("Iter:");
            Serial.print(i);
            Serial.print(" ");
   
            moveStepperForward(m);
          }
        else 
          for(int i = 0; i < bIters; i++)
          {
            Serial.print("Iter:");
            Serial.print(i);
            Serial.print(" ");
   
            moveStepperBackward(m);
          }

        //remove current from stepper
        for(int i = 0; i < NUM_PINS; i++)
          digitalWrite(STEPPER_PINS[m][i], LOW);

        charBuffer = "";
      }
    }

    //manually move forward or back a number of iterations
    else if(inChar == '+')
    {
      int mIndex = Serial.parseInt();
      int numIters = Serial.parseInt();

      for(int i = 0; i < numIters; i++)
        moveStepperForward(mIndex);

      for(int i = 0; i < NUM_PINS; i++)
          digitalWrite(STEPPER_PINS[mIndex][i], LOW);
    }
    else if(inChar == '-')
    {
      int mIndex = Serial.parseInt();
      int numIters = Serial.parseInt();

      for(int i = 0; i < numIters; i++)
         moveStepperBackward(mIndex);

      for(int i = 0; i < NUM_PINS; i++)
          digitalWrite(STEPPER_PINS[mIndex][i], LOW);
    }

    //add character to the buffer to be displayed
    else
    {
      Serial.print("\nAdded to buffer: ");
      Serial.print(inChar);
      Serial.println();

      charBuffer += inChar;

      while(Serial.available() > 0)
      {
        inChar = Serial.read();
        Serial.print("\nAdded to buffer: ");
        Serial.print(inChar);
        Serial.println();
  
        charBuffer += inChar;
      }
    }
  }

  //ready to display the next character(s)
  if(readyForNext)
  {
    boolean ran = false;
    //run until we run out of motors or run out of characters to print
    for(int motorIndex = 0; motorIndex < NUM_CHARS && charBuffer != ""; motorIndex++)
    {
      ran = true;
      
      //find the current character
      char curr = charBuffer.charAt(0);
    
      Serial.print("\nDisplaying: ");
      Serial.print(curr);
      Serial.println();
  
      //find the character's index
      int i = 0;
      for(; i < NUM_DISPLAYED_CHARS; i++)
      {
        if(curr == ALPHABET[i])
          break;
      }
  
      //see if the character can't be displayed
      if(i == NUM_DISPLAYED_CHARS)
      {
        Serial.println("Invalid Character");
      }
      else
      {
        //see if going forwards or backwards is more efficient
        int fIters = itersReqdForward(motorIndex, i);
        int bIters = itersReqdBackward(motorIndex, i);
        Serial.print(fIters);
        Serial.print(" ");
        Serial.println(bIters);

        //move the more efficient route
        if(fIters == 0 && bIters == 0) {}
        else if(fIters <= bIters)
        {
          Serial.println("Moving Forward");
          for(int i = 0; i < fIters; i++)
          {
            Serial.print("Iter:");
            Serial.print(i);
            Serial.print(" ");
            moveStepperForward(motorIndex);
          }
        }
        else 
        {
          Serial.println("Moving Backwards");
          for(int i = 0; i < bIters; i++)
          {
            Serial.print("Iter:");
            Serial.print(i);
            Serial.print(" ");
            moveStepperBackward(motorIndex);
          }
        }

        //remove current from motor
        for(int i = 0; i < NUM_PINS; i++)
          digitalWrite(STEPPER_PINS[motorIndex][i], LOW);

        //remove first character from buffer
        charBuffer = charBuffer.substring(1);
      }
    }

    //completed this loop, wait until user presses button for the next
    if(ran)
      readyForNext = false;
  }

  //delay between iterations of the loop() function
  delay(10);
}
