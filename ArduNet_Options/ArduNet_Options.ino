#include "sprites.h"              //import libraries
#include "neuralROM.h"
#include "bit_array.h"
#include "sized_int_array.h"
#include "sized_int_array_reader.h"
#include <Arduboy2.h>

Arduboy2 arduboy;                 //create arduboy object

const uint16_t totalNeurons = 302;
const uint16_t maxSynapse = 78;   //max synapses a neuron can have as inputs
uint16_t id = 0;                   //the current neuron activation function is calculating
uint8_t option = 0;               //interface variable to indicate which option is selected
bool startFlag = true;            //interface flag for title screen to play
bool isChemotaxis = false;        //flag to know if baseline occurs or not
uint16_t tick = 0;                //connectome ticks (resets to zero at 3628800)
uint8_t selectedOption = 0;       //for if an options screen is selected
const uint16_t learnValMax = 963;
const uint8_t gapJuncMinVal = 90;
const uint8_t screenWidth = 128;
const uint8_t screenHeight = 64; 
const uint8_t maxLearningVal = 7;         //maximum possible value of learning array elements
const uint8_t minLearningVal = -8;        //minimum possible value of learning array elements
bool isForward = false;
bool isLeft = false;

SizedIntArrayReader<9> NEURAL_ROM(COMPRESSED_NEURAL_ROM, 9806, -70, true);
SizedIntArray<4, learnValMax, true> learningArray;    //an array that, for each neuron, holds its firing history
BitArray<totalNeurons> outputList;                    //list of neurons
BitArray<totalNeurons> nextOutputList;                //buffer to solve conflicting time differentials in firing

struct Neuron {
  int16_t cellID;
  int16_t inputLen;
  int16_t inputs[maxSynapse];
  int16_t weights[maxSynapse];    //global neuron struct for neuron 'n'
} n;                              

//setup function
void setup() {
  arduboy.begin();
  arduboy.setFrameRate(25);
}

//primary loop function
void loop() {
  const uint32_t maxTick = 3628800;

  if (!(arduboy.nextFrame())) {  //verify that runs properly
    return;
  }

  doTitleScreen();
  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);      //draw a border around the screen

  if (tick == maxTick) {                        //reset tick Counter for gradient senses when it reaches the factorial of 10 (each gradient is 5)
    tick == 0;
  } else {
    tick++;
  }
  
  doOptions();                                  //screen to select the options
  arduboy.pollButtons();                        //get buttons pressed and send to function to process them
  doButtons();                                  //get user input and update the selected option variable

  if (selectedOption == 1) doDeleteSave(); 
  if (selectedOption == 2) doSerialHostScreen();
  if (selectedOption == 3) doSaveEditorScreen();

  arduboy.display();
  startFlag = false;            //set the flag off; only do the title screen once
}

/**
 * Function that displays the main title screen
 */
void doTitleScreen() {
  if (startFlag) {

    //clear the screen then write app name
    arduboy.clear();
    arduboy.setCursor(15, 10);
    arduboy.print(F("-ArduNet Elegans-"));
    arduboy.setCursor(30, 50);
    arduboy.print(F("APP: OPTIONS"));
    arduboy.display();

    for (uint16_t i = 0; i < learningArray.size; i++) {
      EEPROM.get(EEPROM_STORAGE_SPACE_START + i, learningArray.compressed[i]);
    }

    delay(3000);  
  }
}

/**
 * function to query which buttons are pressed, setting up proper screen transitions
 */
void doButtons() {
  //exit to bootloader
  if (arduboy.justPressed(UP_BUTTON) && arduboy.justPressed(DOWN_BUTTON)) arduboy.exitToBootloader();

  if (arduboy.justPressed(DOWN_BUTTON) || arduboy.justPressed(RIGHT_BUTTON)) {  //increment position in options
    if (option == 2) {
      option = 0;
    } else {
      option++;
    }
  }
  
  if (arduboy.justPressed(UP_BUTTON) || arduboy.justPressed(LEFT_BUTTON)) {     //decrement position in options
    if (option == 0) {
      option = 2;
    } else {
      option--;
    }
  }
  
  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {         //select an option
    if (option == 0) selectedOption = 1;                                        //print current learning array data over serial ("PRINT SAVE")
    if (option == 1) selectedOption = 2;                                        //host a simulation over serial (receive inputs, send output info) ("HOST")
    if (option == 2) selectedOption = 3;                                        //edit learning array data ("SAVE EDIT")
  }
}

/**
 * gets worm movements based on output from its network
 */
void doMovement() {
  //AVB: command interneuron, forward
  if (outputList[55] && outputList[56]) { 
    isForward = true;
  }
  
  //AVA: command interneuron, reversal
  if (outputList[53] && outputList[54]) {
    isForward = false;
  }

  //if dorsal > ventral, set to left turn; if ventral > dorsal, set to right turn
  //SMB: amplitude of turn, oscillates with head bends
  if ((outputList[240] && outputList[241]) || !(outputList[242] && outputList[243])) {
    isLeft = true;
  } else if (!(outputList[240] && outputList[241]) || (outputList[242] && outputList[243])) {
    isLeft = false;
  }

  //if dorsal > ventral, set to left turn; if ventral > dorsal, set to right turn
  //SMD: post-reversal amplitude of turn, oscillates with head bends
  if ((outputList[244] && outputList[245]) || !(outputList[246] && outputList[247])) {
    isLeft = true;
  } else if (!(outputList[244] && outputList[245]) || (outputList[246] && outputList[247])) {
    isLeft = false;
  }
}

/**
 * Function to display the screen that saves data and gives the user the option to 
 * export the eeprom save data
 */
void doOptions() {
  arduboy.clear();

  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);      //draw a border around the screen
  arduboy.setCursor(38, 5);
  arduboy.print(F("-OPTIONS-"));                   //print out the options
  arduboy.setCursor(10, 20);
  arduboy.print(F("DELETE SAVE"));
  arduboy.setCursor(10, 35);
  arduboy.print(F("HOST SIM"));
  arduboy.setCursor(10, 50);
  arduboy.print(F("EDIT SAVE"));
  arduboy.drawRect(83, 20, 36, 36);

  if (option == 0) {
    arduboy.drawPixel(4, 23);
    arduboy.drawPixel(5, 23);
    arduboy.drawPixel(6, 23);
    arduboy.drawPixel(5, 22);
    arduboy.drawPixel(5, 24);
    arduboy.display();
    Sprites::drawOverwrite(85, 22, erase, 0);    
  } else if (option == 1) {
    arduboy.drawPixel(4, 38);
    arduboy.drawPixel(5, 38);
    arduboy.drawPixel(6, 38);
    arduboy.drawPixel(5, 37);
    arduboy.drawPixel(5, 39);
    arduboy.display();
    Sprites::drawOverwrite(85, 22, host, 0);
  } else if (option == 2) {
    arduboy.drawPixel(4, 53);
    arduboy.drawPixel(5, 53);
    arduboy.drawPixel(6, 53);
    arduboy.drawPixel(5, 52);
    arduboy.drawPixel(5, 54);
    arduboy.display();
    Sprites::drawOverwrite(85, 22, edit, 0);
  }
}

/**
 * Function to do logic for the options serial print subscreen
 */
void doDeleteSave() {  
  const uint16_t saveSizeMod = 10;

  arduboy.clear();
  
  while (!(arduboy.justPressed(A_BUTTON) && arduboy.justPressed(B_BUTTON))) {  //if user selects NO
    arduboy.setCursor(2, 5);
    arduboy.print(F("DELETE SAVE DATA?"));   //print out a screen title
    arduboy.setCursor(30, 15);
    arduboy.print(F("(L+R) - YES"));
    arduboy.setCursor(30, 25);
    arduboy.print(F("(A+B) - NO"));
    arduboy.display();

    if (arduboy.justPressed(LEFT_BUTTON) && arduboy.justPressed(RIGHT_BUTTON)) {            //if user selects YES
      arduboy.clear();
      arduboy.drawRect(2, 40, (learningArray.size/saveSizeMod) + 1, 5);  //draw a status bar
      arduboy.setCursor(5, 5);
      arduboy.print(F("Erasing Data..."));

      for (uint16_t i = 0; i < learningArray.count; i++) {  //set the learning array to zeroes
        learningArray[i] = 0;
      }

      for (uint16_t i = 0; i < learningArray.size; i++) {  //save that to eeprom
        EEPROM.write(EEPROM_STORAGE_SPACE_START + i, learningArray.compressed[i]);
  
        if (i % saveSizeMod == 0) {
          arduboy.drawPixel(3 + (i / saveSizeMod), 41, WHITE);  //top bar pixel
          arduboy.drawPixel(3 + (i / saveSizeMod), 42, WHITE);  //middle bar pixel
          arduboy.drawPixel(3 + (i / saveSizeMod), 43, WHITE);  //bottom bar pixel
          arduboy.display();
        }

        delay(10);  //delay to give time to update
      }
      
      arduboy.clear();  //print success message
      arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);
      arduboy.setCursor(2, 5);
      arduboy.print(F("SAVE DATA DELETED."));
      arduboy.display();
      delay(3000);

      selectedOption = 0;     //go back to options screen
      doOptions();

      return;
    }

    arduboy.pollButtons();
  }

  arduboy.clear();    //if the user selected NO
  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);
  arduboy.display();
  delay(1000);

  selectedOption = 0;
  doOptions();
}

/**
 * Function to do logic for the options serial host subscreen
 */
void doSerialHostScreen() {
  char serChar;
  static bool isSimulating = false;

  arduboy.clear();
  arduboy.setCursor(5, 5);
  arduboy.print(F("Hosting: Serial Sim"));            //print out screen title
  arduboy.setCursor(0, 50);
  arduboy.print(F("(To Quit: A+B or ']')"));
  arduboy.display();

  Serial.begin(9600);                                 //start comms for outputs
  
  while (!arduboy.justPressed(A_BUTTON) || !arduboy.justPressed(B_BUTTON)) {  //continue looping and reading over serial as long as A/B are not pressed
    serChar = Serial.read();            //read in a character from serial
    char serInput[4] = {}; 

    arduboy.setCursor(10, 20);
    arduboy.print(serInput[0]);
    arduboy.print(serInput[1]);
    arduboy.print(serInput[2]);
    arduboy.print(serInput[3]);

    if (serChar == '[' || serChar == ',') {               //simulation start character
      isSimulating = true;
      serInput[0] = Serial.read();      //read in the next four characters
      serInput[1] = Serial.read();
      serInput[2] = Serial.read();
      serInput[3] = Serial.read();
    } else if (serChar == ']') {        //simulation end character
      isSimulating = false;
      break;
    }

    if (isSimulating) {
      activationFunction(); //run simulation if the proper serial command was received
      doMovement();
    }
/*                                          Input Legend
                  .MTR = write output matrix to serial,
                  .MSC = write muscle output to serial,
                  BASE = baseline, STTY = satiety, HNGR = hunger, GUST = gustation, 
                  ACTS = activate sleep, MNTS = maintain sleep, GNTN = gentle nose touch, 
                  HSHN = harsh nose touch, TEXT = texture, ATTR = chemoattraction, 
                  REPL = chemorepulsion, COOL = cooling, HEAT = heating, HOTR = noxious heat
                  CLDR = noxious cold, PHOT = photosense, OXYS = oxygen sense, CO2S = co2 sense
                  SLTS = salt sense, PHER = pheromone sense, PROP = proprioception
*/
    if (serInput[0] == '.' && serInput[1] == 'M' && serInput[2] == 'T' && serInput[3] == 'R') {    
      Serial.println(F("["));                              //print an "output start" char
      const uint8_t gridWidth = 18;
      const uint8_t gridHeight = 17;

      //print matrix of neuron values
      for (uint16_t neuronCount = 1; neuronCount <= (gridWidth*gridHeight); neuronCount++) {   //iterate through all neurons plus some dummy ones to make a square grid
        if (neuronCount <= totalNeurons) {                      //if not a dummy neuron
          if (outputList[neuronCount]) {              //if it fired
            if (neuronCount % gridWidth == 0) {       //if its at the end of a row
              Serial.println("█");                      //print out a neuron in the list that fired at end of row
            } else {
              Serial.print("█");                        //print out a neuron in the list that fired in the row
            }
          } else {                                      
            if (neuronCount % gridWidth == 0) {
              Serial.println("░");                      //print out a neuron in the list that didnt fire at end of row
            } else {
              Serial.print("░");                        //print out a neuron in the list that didnt fire in the row
            }
          }
        } else {
          if (neuronCount % gridWidth == 0) {
            Serial.println("_");                        //print out a dummy neuron at end of row
          } else {
            Serial.print("_");                          //print out a dummy neuron in the row
          }
        }
      }

      Serial.println(F("]"));                            //print an "output end" char      
    } else if (serInput[0] == '.' && serInput[1] == 'M' && serInput[2] == 'S' && serInput[3] == 'C') {
      Serial.print(F("["));                              //print an "output start" char
      Serial.print(F("isForward:")); Serial.print(isForward);     //print comma delimeted output info over serial
      Serial.print(F(", isLeft:")); Serial.print(isLeft);
      Serial.println(F("]"));                            //print an "output end" char
    } else if (serInput[0] == 'B' && serInput[1] == 'A' && serInput[2] == 'S' && serInput[3] == 'E') {
      /*if (serInput[4] == 'T') tonic(__, __, __, doBaseline);
            if (serInput[4] == 'P') phasic(__, __, __, doBaseline);*/
      doBaseline();
    } else if (serInput[0] == 'S' && serInput[1] == 'T' && serInput[2] == 'T' && serInput[3] == 'Y') {
      doSatiety();
    } else if (serInput[0] == 'H' && serInput[1] == 'N' && serInput[2] == 'G' && serInput[3] == 'R') {
      isHungry();
    } else if (serInput[0] == 'G' && serInput[1] == 'U' && serInput[2] == 'S' && serInput[3] == 'T') {
      doGustation();
    } else if (serInput[0] == 'A' && serInput[1] == 'C' && serInput[2] == 'T' && serInput[3] == 'S') {
      activateSleep();
    } else if (serInput[0] == 'M' && serInput[1] == 'N' && serInput[2] == 'T' && serInput[3] == 'S') {
      maintainSleep();
    } else if (serInput[0] == 'G' && serInput[1] == 'N' && serInput[2] == 'T' && serInput[3] == 'N') {
      doGentleNoseTouch();
    } else if (serInput[0] == 'H' && serInput[1] == 'S' && serInput[2] == 'H' && serInput[3] == 'N') {
      doHarshNoseTouch();
    } else if (serInput[0] == 'T' && serInput[1] == 'E' && serInput[2] == 'X' && serInput[3] == 'T') {
      doTextureSense();
    } else if (serInput[0] == 'A' && serInput[1] == 'T' && serInput[2] == 'T' && serInput[3] == 'R') {
      doChemoattraction();
      isChemotaxis = true;
    } else if (serInput[0] == 'R' && serInput[1] == 'E' && serInput[2] == 'P' && serInput[3] == 'L') {
      doChemorepulsion();
      isChemotaxis = true;
    } else if (serInput[0] == 'C' && serInput[1] == 'O' && serInput[2] == 'O' && serInput[3] == 'L') {
      doCoolingResponse();
    } else if (serInput[0] == 'H' && serInput[1] == 'E' && serInput[2] == 'A' && serInput[3] == 'T') {
      doHeatingResponse();
    } else if (serInput[0] == 'H' && serInput[1] == 'O' && serInput[2] == 'T' && serInput[3] == 'R') {
      doNoxiousHeatResponse();
    } else if (serInput[0] == 'C' && serInput[1] == 'L' && serInput[2] == 'D' && serInput[3] == 'R') {
      doNoxiousColdResponse();
    } else if (serInput[0] == 'P' && serInput[1] == 'H' && serInput[2] == 'O' && serInput[3] == 'T') {
      doPhotosensation();
    } else if (serInput[0] == 'O' && serInput[1] == 'X' && serInput[2] == 'Y' && serInput[3] == 'S') {
      doOxygenSensation();
    } else if (serInput[0] == 'C' && serInput[1] == 'O' && serInput[2] == '2' && serInput[3] == 'S') {
      doCO2Sensation();
    } else if (serInput[0] == 'S' && serInput[1] == 'L' && serInput[2] == 'T' && serInput[3] == 'S') {
      doSaltSensation();
    } else if (serInput[0] == 'P' && serInput[1] == 'H' && serInput[2] == 'E' && serInput[3] == 'R') {
      sensePheromones();
    } else if (serInput[0] == 'P' && serInput[1] == 'R' && serInput[2] == 'O' && serInput[3] == 'P') {
      doProprioception();
    } else {  //handle the error in serial message
      arduboy.setCursor(20, 3);
      arduboy.print(F("SERIAL ERROR!"));
      arduboy.setCursor(5, 20);
      arduboy.print(F("unexpected serial"));
      arduboy.setCursor(5, 30);
      arduboy.print(F("code: "));
      arduboy.print(serInput[0] + serInput[1] + serInput[2] + serInput[3] + serInput[4]);
      arduboy.print(F("?"));
    }

    arduboy.pollButtons();
    delay(100);
  }

  Serial.end();                                     //end comms for outputs
  arduboy.clear();  //print out simulation end message
  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);
  arduboy.setCursor(5, 5);
  arduboy.print(F("SIMULATION ENDED!"));
  arduboy.display();
  delay(3000);

  selectedOption = 0;
}

/**
 * Function to do logic for the options save editor subscreen
 */
void doSaveEditorScreen() {
  int8_t data = 0;
  uint16_t savePos = 0;

  while (!arduboy.justPressed(A_BUTTON)) {  //if A press; exit without saving
    arduboy.clear();
    arduboy.setCursor(27, 5);
    arduboy.print(F("-Save Editor-"));         //print out screen title
    arduboy.setCursor(8, 45);
    arduboy.print(F("(B)-----------SAVE"));
    arduboy.setCursor(8, 55);
    arduboy.print(F("(A)--EXIT (NO SAVE)"));  //print out instructions for save data editor
    arduboy.pollButtons();
    arduboy.display();

    //print previous element (if not first)
    if (savePos != 0) {
      //draw LEFT arrow
      arduboy.drawPixel(9, 27);
      arduboy.drawPixel(8, 28); 
      arduboy.drawPixel(9, 28); 
      arduboy.drawPixel(10, 28); 
      arduboy.drawPixel(11, 28);
      arduboy.drawPixel(9, 29);

      arduboy.setCursor(13, 25);
      arduboy.print(F("["));
      data = learningArray[savePos-1];
      arduboy.print(data);
      arduboy.setCursor(36, 25);
      arduboy.print(F("]"));
      arduboy.setCursor(43, 25);
      arduboy.print(F("-"));
    } else {
      arduboy.setCursor(43, 25);
      arduboy.print(F(">"));
    }

    //draw MIDDLE TOP arrow
    arduboy.drawPixel(64, 19);
    arduboy.drawPixel(64, 20);
    arduboy.drawPixel(64, 21);
    arduboy.drawPixel(64, 22);
    arduboy.drawPixel(63, 21);
    arduboy.drawPixel(65, 21);

    //draw the current elements number
    arduboy.setCursor(62, 39);
    arduboy.print(savePos);

    //print current element
    arduboy.setCursor(49, 25);
    arduboy.print(F("["));
    data = learningArray[savePos];
    arduboy.print(data);
    arduboy.setCursor(72, 25);
    arduboy.print(F("]"));

    //draw MIDDLE BOTTOM arrow
    arduboy.drawPixel(64, 34);
    arduboy.drawPixel(64, 35);
    arduboy.drawPixel(64, 36);
    arduboy.drawPixel(64, 37);
    arduboy.drawPixel(63, 35);
    arduboy.drawPixel(65, 35);

    //print next element (if not last)
    if (savePos != learningArray.count - 1) {
      arduboy.setCursor(79, 25);
      arduboy.print(F("-"));
      arduboy.setCursor(85, 25);
      arduboy.print(F("["));
      data = learningArray[savePos+1];
      arduboy.print(data);
      arduboy.setCursor(108, 25);
      arduboy.print(F("]"));

      //draw RIGHT arrow
      arduboy.drawPixel(117, 27);
      arduboy.drawPixel(115, 28);
      arduboy.drawPixel(116, 28);
      arduboy.drawPixel(117, 28);
      arduboy.drawPixel(118, 28);
      arduboy.drawPixel(117, 29);
    } else {
      arduboy.setCursor(78, 25);
      arduboy.print(F("<"));
    }

    arduboy.display();

    int8_t saveData = learningArray[savePos];

    if (arduboy.justPressed(RIGHT_BUTTON)) {        //if R press; move to next position in array
      if (savePos != learningArray.count - 1) {
        savePos++;
      } else {
        savePos = 0;
      }
    } else if (arduboy.justPressed(LEFT_BUTTON)) {  //if L press; move to prev position in array
      if (savePos != 0) {
        savePos--;
      } else {
        savePos = learningArray.count - 1;
      }      
    } else if (arduboy.justPressed(UP_BUTTON)) {    //if Up press; change value in current position
      if (saveData >= maxLearningVal) {
        learningArray[savePos] = minLearningVal;       //set to zero if past the max val
      } else {
        learningArray[savePos] = learningArray[savePos] + 1;           //increment value
      }
    } else if (arduboy.justPressed(DOWN_BUTTON)) {
      if (saveData <= minLearningVal) {
        learningArray[savePos] = maxLearningVal;         //set to max if past the min val
      } else {
        learningArray[savePos] = learningArray[savePos] - 1;           //decrement value
      }
    }

    if (arduboy.justPressed(B_BUTTON)) {            //if B press, write to eeprom
      for (uint16_t i = 0; i < learningArray.size; i++) {
        EEPROM.write(EEPROM_STORAGE_SPACE_START + i, learningArray.compressed[i]);
      }

      arduboy.clear();  //print out success message
      arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);
      arduboy.setCursor(5, 5);
      arduboy.print(F("EDITS SAVED!"));
      arduboy.display();
      arduboy.pollButtons();
      delay(3000);

      selectedOption = 0;
      doOptions();

      return;
    }
  }

  arduboy.clear();  //exit subscreen, do not print any message
  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);
  arduboy.display();
  arduboy.pollButtons();
  delay(100);

  selectedOption = 0;
}

/**************************************NETWORK STATES****************************************/
/**
 * Function to do ASE neuron baseline chemotaxis
 */
void doBaseline() {
  if (!isChemotaxis) {  //activate baseline ASER chemotaxis
    outputList[40] = true;
  } else {
    outputList[40] = false;   //activates in response to increases in NaCl
  }
}

/**
 * Function to create a hunger response
 */
void isHungry() {  //phasic?
  outputList[45] = false;
  outputList[46] = false;
  outputList[149] = false;
  outputList[150] = false;
}

/**
 * Function to create a satiety response
 */
void doSatiety() { //phasic?
  outputList[45] = true;
  outputList[46] = true;
  outputList[149] = true;
  outputList[150] = true;
}       //ASI, NSM

/**
 * Activate the worm's gustation neurons for sensing food at nose
 */
void doGustation() { //tonic
    outputList[149] = true;
    outputList[150] = true;
}       //NSML

/**
 * activates the gate neuron used to trigger entry to sleep
 */
void activateSleep() {
  outputList[22] = true;
}       //ALA

/**
 * activates the neuron used to sustain a sleep state
 */
void maintainSleep() { //tonic
  outputList[204] = true;
}       //RIS

/************************************INPUT MODALITIES***************************************/
/**
 * Function to calculate phasic activation of a neuron
 */
void phasic(bool useGradient, uint8_t mod, void (*senseFunction)()) {
  if (useGradient) {
    mod = round(mod / 10) * 10;                     //round the distance or the rate adjustment to the nearest tens
    if (tick % (100 - mod) == 0) senseFunction();   //modulo the tick by the rounded distance/rate adjustment
  } else {
    senseFunction();
  }
}

/**
 * Function to calculate tonic activation of a neuron
 */
void tonic(bool useGradient, uint8_t mod, uint8_t gradientStep, void (*senseFunction)()) {
  if (useGradient) {
    mod = (mod / gradientStep) + 1;                 //modify distance or rate adjustment; 5 is the precision of the gradient, the step of the gradient
    if (tick % mod == 0) senseFunction();
  } else {
    if (tick % mod == 0) senseFunction();
  }
}

/******************************************SENSES********************************************/
void doGentleNoseTouch() { //tonic
  outputList[153] = true;
  outputList[154] = true;
  outputList[155] = true;
  outputList[156] = true;
  outputList[125] = true;
  outputList[126] = true;
  outputList[127] = true;
  outputList[128] = true;
  outputList[129] = true;
  outputList[130] = true;
  outputList[113] = true;
  outputList[114] = true;
}       //OLQ, IL1, FLP

void doHarshNoseTouch() { //phasic
  outputList[113] = true;
  outputList[114] = true;
  outputList[43] = true;
  outputList[44] = true;
}       //FLP, ASH

void doTextureSense() { //tonic
  outputList[84] = true;
  outputList[85] = true;
  outputList[86] = true;
  outputList[87] = true;
  outputList[2] = true;
  outputList[3] = true;
  outputList[159] = true;
  outputList[160] = true;
}       //CEP, ADE, PDE

void doChemoattraction() { //tonic
  outputList[40] = true;
  outputList[4] = true;
  outputList[5] = true;
  outputList[45] = true;
  outputList[46] = true;
  outputList[72] = true;
  outputList[73] = true;
  outputList[76] = true;
  outputList[77] = true;
}       //ASER (primary), ADF, ASI, AWA, AWC

void doChemorepulsion() { //phasic
  outputList[43] = true;
  outputList[44] = true;
  outputList[6] = true;
  outputList[7] = true;
}       //ASH (primary), ADL

void doCoolingResponse() {//tonic
  outputList[76] = true;
  outputList[77] = true;
}       //AWC

void doHeatingResponse() { //tonic
  outputList[8] = true;
  outputList[9] = true;
}       //AFD

void doNoxiousHeatResponse() { //phasic
  outputList[8] = true;
  outputList[9] = true;
  outputList[113] = true;
  outputList[114] = true;
  outputList[165] = true;
  outputList[43] = true;
  outputList[44] = true;
}       //AFD (head), FLP (head), and PHC (tail), ASH

void doNoxiousColdResponse() { //phasic
  outputList[174] = true;
  outputList[175] = true;
}       //PVD

void doPhotosensation() { //phasic
  outputList[47] = true;
  outputList[48] = true;
  outputList[49] = true;
  outputList[50] = true;
}       //ASJ, ASK, ASH

void doOxygenSensation() { //can be tonic or phasic
  outputList[27] = true;
  outputList[171] = true;
  outputList[254] = true;
  outputList[255] = true;
}       //AQR, PQR, URX

void doCO2Sensation() { //tonic
  outputList[8] = true;
  outputList[9] = true;
  outputList[78] = true;
  outputList[79] = true;
  outputList[39] = true;
  outputList[40] = true;
}       //AFD, BAG, and ASE; senses O2 decrease, CO2 increase

void doSaltSensation() { //tonic
  outputList[39] = true;
  outputList[40] = false;
}       //ASEL/ASER

void sensePheromones() { //tonic (will act as chemorepulsant if phasic)
  outputList[49] = true;
  outputList[50] = true;
}       //ASK

void doProprioception() { //phasic, unless held then it becomes tonic
  outputList[110] = true;
}       //DVA

/*************************************SIMULATION FUNCTIONS***************************************/
/**
 *The activation function is the main simulation, it calculates all the next ticks of the connectome
 * and then sets the next tick to the current one when each has been individually calculated
 */
void activationFunction() {  
  uint16_t index = 0;
  const float hebbianConstant = 1;               //constant representing the amount the learning array affects a given synapse
  const uint8_t offset = 2;                              //value to adjust how much "charge" a gap junction sends to next neuron
  const uint8_t threshold = 10;             //threshold for activation function
  const uint16_t totalLearningNeurons = 50; //number of neurons with hebbian learning ability, the size of hebbian_neurons[] array

  //calculate next output for all neurons using the current output list
  for (id; id < totalNeurons; id++) {
    n.inputLen = NEURAL_ROM[index];
    index++;

    // Read neuron inputs
    for (uint8_t i = 0; i < n.inputLen; i++) {
      n.inputs[i] = NEURAL_ROM[index++];
    }

    // Read neuron weights
    for (uint8_t i = 0; i < n.inputLen; i++) {
      n.weights[i] = NEURAL_ROM[index++];
    }
    
    static uint16_t learningPos = 0;                 //static variable for the position in the learning array; (functions as "global" var) 
    int32_t sum = 0;                                 //fill the neuron struct with the information of the ith neuron
    bool hebFlag = false;

    for (uint8_t hebIndex = 0; hebIndex < totalLearningNeurons; hebIndex++) {             //check to see if current ID is in the hebbian-capable neuron  list
      if (HEBBIAN_NEURONS[hebIndex] == id) {                            //if the current neuron being read in is in the hebbian neuron array
        for (uint8_t hebInput = 0; hebInput < n.inputLen; hebInput++) {             //for every presynapse to the current neuron
          n.weights[hebInput] += hebbianConstant * learningArray[learningPos + hebInput];            //adjust its weight based on the learning array
        }

        hebFlag = true;                                                 //flag to mark the neuron's synapse as doing hebbian learning
      }
    }

    for (uint8_t j = 0; j < n.inputLen; j++) {                          //loop over every presynaptic neuron
      if (n.weights[j] >= gapJuncMinVal) {                                         //if the weight has a gap junction indicator (9_)
        int8_t gapWeight = n.weights[j] - gapJuncMinVal;                           //gap junctions are indicated by weights at 90-99
        int8_t gapOutput = 0;                                          //gap junctions presynapse outputs are adjusted, as they are not binary
        if (outputList[n.inputs[j]]) gapOutput = (1 + offset);          //outputList value is adjusted for non-binary activation
        if (!outputList[n.inputs[j]]) gapOutput = -(1 + offset);
        sum += gapWeight * gapOutput;                                   //do the summation calculation for a gap junction synapse
      } else {
        sum += n.weights[j] * outputList[n.inputs[j]];                  //do the summation calculation on current synapse
      }
    }

    if (sum >= threshold) {                                             //check if activation function outputs a true or false
      nextOutputList[id] = true;                                        //store the output in a buffer
    } else {
      nextOutputList[id] = false;
    }

    for (uint8_t j = 0; j < n.inputLen; j++) {                          //loop over every presynaptic neuron
      if (hebFlag) {                                                    //if current neuron is in list
        if (outputList[n.inputs[j]] && nextOutputList[id]) {            //if the pre and postsynaptic neuron both fire
          //the specific synapse gets an increased hebbian value if its below the max possible value
          if (learningArray[learningPos] < maxLearningVal) learningArray[learningPos] = learningArray[learningPos] + 1;
        } else if (!outputList[n.inputs[j]] && !nextOutputList[id]) {   //if the pre and postsynaptic neuron both do NOT fire
          //the specific synapse gets a decreased hebbian value if its above the min possible value
          if (learningArray[learningPos] > minLearningVal) learningArray[learningPos] = learningArray[learningPos] - 1;
        }

        learningPos++;                                                  //increment the counter for the learning array position
      }
    }
  }

  //flush the buffer
    for (int16_t i = 0; i < totalNeurons; i++) {
      outputList[i] = nextOutputList[i];
      id = 0;
    }
}