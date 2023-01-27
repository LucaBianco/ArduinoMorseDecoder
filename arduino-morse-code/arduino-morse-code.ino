/*
Author: Luca Bianconi
License: see included License file.
*/

//Possible states of the Lightbulb:
#define STATE_OFF 0   //The Lightbulb is off;
#define STATE_ON  1   //The Lightbulb is on.

//EVENTS:
#define EDGE_NONE       0   //Lightbulb didn't change state;
#define EDGE_RISING     1   //Lightbulb just turned on;
#define EDGE_FALLING   -1   //Lightbulb just turned off.

//SYMBOLS
#define DOT    1
#define DASH   2


//If read sensor value is above threshold, the bulb is considered on and vice versa.
int threshold = 340;
long int t; //Time counter [in tens of milliseconds]


//Initialization and code that runs at the power up or reset:
void setup() 
{
  Serial.begin(9600);
  t = millis() / 10;
}


String myWord = "";
bool started = false;

//Code that runs in a fast loop
void loop() 
{
  int val = analogRead(A0);
  int ev = event(val, threshold); //Check if an event occurred (1 = bulb just turned on, -1 = bulb just turned off. 0 = no change.)

  long int delta = 0;


  char L = 0;
  
  if (ev)     //If there is an event
  {
    long int cur = millis() / 10;
    delta = cur - t;
    t = cur;
      
    if (ev == EDGE_RISING)
    {
      if (delta < 50 && started) //This is definitely a DOT
      {
        Serial.print(".");
        L = letter(DOT, false);
      }
      else if (delta < 200 && started) //This is a DASH
      {
        Serial.print("-");
        L = letter(DASH, false);
      }
    }
    else if (ev == EDGE_FALLING)
    {
      if (delta < 50 && started) //Time between symbols
      {
        //Do nothing
      }
      else if (delta < 200 && started) //Next letter
      {
        Serial.print(" ");
        L = letter(0, true);
      }
      else if (delta < 500) //End of the word (no, not the world)
      {
        if (started)
        {
          myWord += letter(0, true); //Get last letter and reset the letter decoder
          
          Serial.println(" = " + myWord); //Print the word
          
          myWord = "";  //Reset variables for next word
          L = 0;
        }
        else started = true;
      }
    }

    if (L != 0 && started) //We decoded a letter
    {
      myWord += L;
    }
  }

  delay(10);
}

//This function finds edges in the incoming signal.
//It accepts val as an input, which is an integer between 0 and 1023
//  and also accepts thr, the threshold.
//The output is the edge data.
int event(int val, int thr)
{
  static int state = STATE_OFF;

  switch(state)
  {
  case 0:
    if (val < thr)
      {
        state = STATE_ON;
        return EDGE_RISING;
      }
    break;
    
  case 1:
    if (val > thr)
    {
      state = STATE_OFF;
      return EDGE_FALLING;
    }
    break;
  
  default:
    state = 0;
  }

  return EDGE_NONE;
}


//Decode morse. Morse numbers not supported.
  //Look-up table:
char morseTable[54] = {'E',0,0,0,0,0,0,0,0,'I',0,0,'S','H','V','U','F',0,'A',0,0,'R','L',0,'W','P','J','T',0,0,0,0,0,0,0,0,'N',0,0,'D','B','X','K','C','Y','M',0,0,'G','Z','Q','O',0,0};
  //Base 3 - "digits" are null=0, dot=1, dash=2.     Starting from 1000 (= 27) up to 2222 (=80).
  //All letters are in this range. All the zeros represent invalid combinations (there can't be null symbols in between dots and dashes. Others are just unused combinations.)

//Letter decoder function.
  //This function has memory (it's basically a state machine). It receives one symbol after the other until the end of the letter.
  //It builds a base 3 representation of the letter (letters are 1 - 4 ternary digits long).
  //It then converts the base 3 number to a decimal value, "num", and then gets the letter from the table, using "num" as index.
char letter(int symbol, bool endletter)
{
  static int num = 0;
  static int mult = 27;
  char L=0;
  
  if (endletter) //This is the ending letter
  {
    L = morseTable[num-27]; //Get value of the letter from the table.
      //The first 27 spots (0000 to 0222) on the array would be full of zeros so we don't consider them to save memory (a letter can't start with a null character).
      
    num = 0;
    mult = 27;
  }
  else
  {
    num += symbol * mult; //Next digit in our "Base conversion"
    mult /= 3;
  }
  
  return L;
}