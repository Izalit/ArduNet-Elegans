#include "sprites.h"              //import libraries
#include "neuralROM.h"
#include "bit_array.h"
#include "sized_int_array.h"
#include "sized_int_array_reader.h"
#include <Arduboy2.h>

Arduboy2 arduboy;                 //create arduboy object

/*
  Optimal Thresholds:
    Simple - 5??
    Simple GJ - breaks
    No Comp - 9?
    No Comp GJ - breaks
    No Unk - 10???
    No Unk GJ - breaks
   *Unk Comp - 9
    Unk Comp GJ - breaks
*/

const uint16_t totalNeurons = 302;
static uint16_t id = 0;                   //the current neuron activation function is calculating
const uint16_t maxSynapse = 78;   //max synapses a neuron can have as inputs
uint16_t currentID = 0;           //interface variable to indicate which neuron is being analyzed
uint16_t numSynapses = 0;         //interface variable to indicate number of synapses connected
uint16_t preID = 999;             //interface variable to indicate which neuron is being analyzed
uint8_t sense = 0;                //interface variable to indicate which sense is activated
uint8_t posCount = 0;             //interface variable to indicate which sense is being looked at
uint8_t lastScreen = 0;           //interface variable to indicate last screen before button press
int16_t synWeight = 0;            //interface variable to indicate the weight of a given synapse
bool startFlag = true;            //interface flag for title screen to play
bool startInputFlag = false;      //variable to determine if an input has been selected yet
uint8_t scroll = 0;               //variable to indicate which sub-screen in the inputs list user is on
uint16_t tick = 0;                //connectome ticks (resets to zero at 3628800)
bool isAsleep = false;            //indicates if worm is sleeping
bool isChemotaxis = false;        //indicates if the worm is experiencing some kind of chemotaxis
uint8_t option = 0;               //interface variable to indicate which option is selected
uint8_t selectedOption = 0;       //for if an options screen is selected
const uint16_t saveSizeMod = 10;
const uint8_t screenWidth = 128;
const uint8_t screenHeight = 64; 
const uint16_t learnValMax = 963;
const uint16_t gapJuncMinVal = 90;
const uint8_t maxLearningVal = 7;         //maximum possible value of learning array elements
const uint8_t minLearningVal = -8;        //minimum possible value of learning array elements
bool isForward = false;
bool isLeft = false;

SizedIntArrayReader<9> NEURAL_ROM(COMPRESSED_NEURAL_ROM, 9806, -70, true);
SizedIntArray<9, maxSynapse, false> preSynapticNeuronList; //interface array to hold all the different presynaptic neurons
SizedIntArray<4, learnValMax, true> learningArray;  //an array that, for each neuron, holds its firing history
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

  activationFunction();         //do the main calculation of the connectome
  arduboy.pollButtons();        //get buttons pressed and send to function to process them

  if (tick == maxTick) {                        //reset tick Counter for gradient senses when it reaches the factorial of 10 (each gradient is 5)
    tick == 0;
  } else {
    tick++;
  }

  //go to the proper screens based on the direction button last clicked
  if (startFlag || lastScreen == 1) doMatrixScreen();
  if (lastScreen == 2) doOptions();
  if (lastScreen == 3) doDiagnosticScreen();
  if (lastScreen == 4) doInputScreen();

  //do options screens if selected
  if (selectedOption == 1) doSaveScreen();
  if (selectedOption == 2) doSerialPrintScreen();
  if (selectedOption == 3) doCreditsScreen();

  doButtons();

  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);      //draw a border around the screen
  arduboy.display();
  
  startFlag = false;            //set the flag off; only do the title screen once
}

/**
 * function to run the diagnostic screen; a screen that shows details of synapses
 * and allows the user to make changes
 */
void doDiagnosticScreen() {
  const char species[] = "elegans";             //write the name of the species here

  lastScreen = 3;

  arduboy.clear();
  arduboy.drawRoundRect(4, 3, 75, 52, 2);       //draw a borders around text
  arduboy.drawRoundRect(80, 3, 46, 22, 2);
  arduboy.setCursor(83, 6);                     //write out text
  arduboy.print(F("NAME:"));
  arduboy.setCursor(83, 15);
  arduboy.print(species);
  arduboy.setCursor(10, 10);
  arduboy.print(F("NEURON: "));
  arduboy.setCursor(55, 10);
  arduboy.print(currentID); 
  arduboy.setCursor(10, 20);
  arduboy.print(F("SYN CT: "));
  arduboy.setCursor(55, 20);
  arduboy.print(numSynapses);

  arduboy.setCursor(10, 30);                    //write remaining text
  arduboy.print(F("INPUTS: "));
  arduboy.setCursor(55, 30);
  arduboy.print(preID);
  arduboy.setCursor(10, 40);
  arduboy.print(F("WEIGHT: "));  
  arduboy.setCursor(55, 40);

  if (synWeight >= gapJuncMinVal) {             //handle gap junction text
    arduboy.print(F("GAP"));
    arduboy.print(synWeight - gapJuncMinVal);
  } else {
    arduboy.print(synWeight);
  }


  arduboy.drawRoundRect(86, 28, 35, 27, 2);     //draw a border around the perceptron model
  if (outputList[currentID]) Sprites::drawOverwrite(88, 30, perceptronON, 0);       //draw the perceptron image
  if (!outputList[currentID]) Sprites::drawOverwrite(88, 30, perceptronOFF, 0);

  arduboy.display();
}

/**
 * Function to activate the input screen where the user can choose a sensory
 * input modality to interface with
 */
void doInputScreen() {
  //if pressed right draw inputs screen
      lastScreen = 4;

      if (scroll == 0) {
        drawInputs(0, "Gentle Nose Touch", "Chemoattraction", "Chemorepulsion", "Cooling");
      }

      if (scroll == 1) {
        drawInputs(1, "Heating", "Phototaxis", "Aerotaxis (O2)", "Pheromone Sense");
      }

      if (scroll == 2) {
        drawInputs(2, "State:Baseline", "State:Hungry", "State:Sated", "State:Eating");
      }
      
      if (scroll == 3) {
        drawInputs(3, "State:Sleeping", " ", " ", " ");
      }
}

/**
 * function to query which buttons are pressed, setting up proper screen transitions
 */
void doButtons() {
  const uint8_t longSkip = 20;
  static uint8_t synCounter = 0;           //Counter to hold information on which pre-synaptic neuron is selected
  static bool sated = false;               //indicates if worm is sated
  static bool isEating = false;            //indicates if worm is eating

  //exit to bootloader
  if (arduboy.justPressed(UP_BUTTON) && arduboy.justPressed(DOWN_BUTTON)) arduboy.exitToBootloader();

  //MATRIX SCREEN
  if (lastScreen == 1) {
    if (arduboy.justPressed(A_BUTTON) && arduboy.justPressed(B_BUTTON)) {     //activate sense with A+B
      startInputFlag = true;
      if (sense == 0) {
          phasic(false, 0, doGentleNoseTouch);
      }
      if (sense == 1) {
        isChemotaxis = true;
          phasic(false, 0, doChemoattraction);
      }
      if (sense == 2) {
        isChemotaxis = true;
          phasic(false, 0, doChemoattraction);
      }
      if (sense == 3) {
          phasic(false, 0, doCoolingResponse);
      }
      if (sense == 4) {
          phasic(false, 0, doHeatingResponse);
      }
      if (sense == 5) {
          phasic(false, 0, doPhotosensation);
      }
      if (sense == 6) {
          phasic(false, 0, doPhotosensation);
      }
      if (sense == 7) {
        isChemotaxis = true;
          phasic(false, 0, sensePheromones);
      }
      if (sense == 8) {
        isChemotaxis = false;
          phasic(false, 0, doBaseline);
      }
      if (sense == 9) {                 //toggle fullness state
        sated = false;
          phasic(false, 0, isHungry);
      }
      if (sense == 10) {                //toggle fullness state
        sated = true;
          phasic(false, 0, doSatiety);
      }
      if (sense == 11 && !sated) {                //toggle eating state
        isChemotaxis = true;
        isEating = true;
          phasic(false, 0, doGustation);
      } else if (sense == 11 && sated) {
        isChemotaxis = true;
        isEating = false;
        arduboy.setCursor(70,40);
        arduboy.print(F("(SATED)"));
        delay(50);
      }
      if (sense == 12 && isAsleep) {    //toggle sleep state
        phasic(false, 0, maintainSleep);
      } else if (sense == 12 && !isAsleep) {
        isAsleep = true;
        phasic(false, 0, activateSleep);
        phasic(false, 0, maintainSleep);
      }
    }

    if (arduboy.justPressed(A_BUTTON) && !arduboy.justPressed(B_BUTTON)) {    //just A decrements by 1
      if (currentID == 0) {
        currentID = totalNeurons - 1;
      } else {
        currentID--;
      }
    }

    if (arduboy.justPressed(B_BUTTON) && !arduboy.justPressed(A_BUTTON)) {    //just B increments by 1
      if (currentID == totalNeurons - 1) {
          currentID = 0;
      } else {
        currentID++;
      }
    }

    if (arduboy.justPressed(UP_BUTTON)) {                                     //UP increments by 20 instead of 1
      if (currentID < totalNeurons - 1) {
        if (currentID + longSkip > totalNeurons - 1) {
          currentID = totalNeurons - 1;
        } else {
          currentID += longSkip;
        }
      } else {
        currentID = 0;
      }
    } else if (arduboy.justPressed(DOWN_BUTTON)) {
      lastScreen == 2;
      doOptions();
    } else if (arduboy.justPressed(LEFT_BUTTON)) {
      lastScreen == 3;
      doDiagnosticScreen();
    } else if (arduboy.justPressed(RIGHT_BUTTON)) {
      lastScreen == 4;
      doInputScreen();
    }
  }

  //OPTIONS SCREEN
  if (lastScreen == 2) {
    if (arduboy.justPressed(UP_BUTTON)) {    //go to matrix screen
      doMatrixScreen();
    } else if (arduboy.justPressed(RIGHT_BUTTON)) {              //scroll through options
      if (option == 2) {
        option = 0;
      } else {
        option++;
      }
    } else if (arduboy.justPressed(LEFT_BUTTON)) {
      if (option == 0) {
        option = 2;
      } else {
        option--;
      }
    }
    
    if (arduboy.justPressed(B_BUTTON)) {         //select an option
      if (option == 0) {                                  //save current learning array data ("SAVE")
        selectedOption = 1;
      } else if (option == 1) {                           //print current learning array data over serial ("PRINT SAVE") 
        selectedOption = 2;
      } else if (option == 2) {                           //show the credits ("CREDITS")
        selectedOption = 3;
      }
    }
  }//DOWN and A BUTTON unused

  //DIAGNOSTIC SCREEN
  if (lastScreen == 3) {
    if (arduboy.justPressed(RIGHT_BUTTON)) {                                             //right to matrix
      doMatrixScreen();
      preID = preSynapticNeuronList[synCounter];
    } else if (arduboy.justPressed(B_BUTTON)) {      //B increases n.inputs array by one
      if (synCounter == numSynapses) {
        synCounter = 0;
      } else {
        synCounter++;
      }
      preID = preSynapticNeuronList[synCounter];
    } else if (arduboy.justPressed(A_BUTTON)) {      //A decreases n.inputs array by one
      if (synCounter == 0) {
        synCounter = numSynapses;
      } else {
        synCounter--;   
      }
      preID = preSynapticNeuronList[synCounter];
    } else if (arduboy.justPressed(UP_BUTTON)) {     //UP increases current neuron ID
      if (currentID == 0) {
        currentID = totalNeurons - 1;
      } else {
        currentID--;
      }
    } else if (arduboy.justPressed(DOWN_BUTTON)) {   //DOWN decreases current neuron ID
      if (currentID == totalNeurons - 1) {
          currentID = 0;
      } else {
        currentID++;
      }      
    }
  }// LEFT unused
  
  //INPUT SELECT SCREEN
  if (lastScreen == 4) {
    if (arduboy.justPressed(B_BUTTON) || arduboy.justPressed(DOWN_BUTTON)) {      //B and DOWN incrementally scroll
      if (posCount == 12) {
        posCount = 0;
      } else {
        posCount++;
      }
      doPos();
    } else if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(UP_BUTTON)) {        //A and UP decrementally scroll
      if (posCount == 0) {
        posCount = 12;
      } else {
        posCount--;
      }
      doPos();
    } else if (arduboy.justPressed(LEFT_BUTTON)) {         //left goes to matrix screen
      doMatrixScreen();
    }
  }  

  arduboy.display();
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
    arduboy.print(F("APP: MATRIX"));
    arduboy.display();

    for (uint16_t i = 0; i < learningArray.size; i++) {
      EEPROM.get(EEPROM_STORAGE_SPACE_START + i, learningArray.compressed[i]);
    }

    delay(3000);  
  }
}

/**
 * A function that displays the main "matrix" screen to the user, allows them to activate
 * a chosen sensory modality, and to view network activation patterns in the matrix
 */
void doMatrixScreen() {      
  lastScreen = 1;
  
  doBaseline();

  arduboy.clear();
  arduboy.setCursor(5, 5);
  arduboy.print(F("NEURAL GRID: "));

  arduboy.setCursor(45, 20);
  arduboy.print(F("Mood: "));

  arduboy.setCursor(5, 45);
  arduboy.print(F("CELL ID: "));
  arduboy.print(currentID);

  if (!startInputFlag) {
    arduboy.setCursor(5, 55);
    arduboy.print(F("STATIONARY"));
  } else {
    printMovementDir(5, 55);
  }

  drawFaces();
  Sprites::drawOverwrite(5, 20, tMatrix, 0);

  uint8_t gridWidth = 17;
  uint8_t gridHeight = 18;
  uint16_t xPos = 85;
  uint16_t yPos = 10;
        
  arduboy.drawRoundRect(xPos - 5, yPos - 5, (gridWidth*2) + 15, (gridHeight*2) + 12, 3);      //draw a border around the matrix
  arduboy.drawRoundRect(5, 18, 34, 23, 2);      //draw a border around the legend
  arduboy.drawRect(xPos, yPos, (gridWidth + 1)*2, (gridHeight + 1)*2);  //draw border on grid of cells

  int neuronCounter = 0;
      
  //draw grid of cells, blacking out current cell ID
  for (uint16_t x = 1; x <= (gridWidth*2); x++) {
    for (uint16_t y = 1; y <= (gridHeight*2); y++) {
      //for each neuron get the output state and draw a pixel accordingly               
      if (neuronCounter < totalNeurons) {
        if (outputList[neuronCounter]) {
          arduboy.drawPixel(x + xPos, y + yPos, WHITE);
          arduboy.drawPixel(x + xPos + 1, y + yPos, WHITE);
          arduboy.drawPixel(x + xPos, y + yPos + 1, WHITE);
          arduboy.drawPixel(x + xPos + 1, y + yPos + 1, WHITE);        
        }
      }
      
      if (neuronCounter == currentID) {
        arduboy.drawPixel(xPos - 2, yPos + y);  //draw Y indicator for the selected cell
        arduboy.drawPixel(xPos - 2, yPos + y + 1);
        arduboy.drawPixel(xPos - 3, yPos + y);
        arduboy.drawPixel(xPos - 3, yPos + y + 1);
        arduboy.drawPixel(xPos + x, yPos - 2);  //draw X indicator for the selected cell
        arduboy.drawPixel(xPos + x + 1, yPos - 2);
        arduboy.drawPixel(xPos + x, yPos - 3);
        arduboy.drawPixel(xPos + x + 1, yPos - 3);
      }

      neuronCounter++;

      y++;
    }

    x++;
  }
}

/**
 * Function to display the screen that saves data and gives the user the option to 
 * export the eeprom save data
 */
void doOptions() {
  lastScreen = 2;
  arduboy.clear();

  arduboy.setCursor(38, 5);
  arduboy.print(F("-OPTIONS-"));     //print out the options
  arduboy.setCursor(10, 20);
  arduboy.print(F("SAVE DATA"));
  arduboy.setCursor(10, 35);
  arduboy.print(F("PRINT SAVE"));
  arduboy.setCursor(10, 50);
  arduboy.print(F("CREDITS"));

  if (option == 0) {
    arduboy.drawPixel(4, 23);
    arduboy.drawPixel(5, 23);
    arduboy.drawPixel(6, 23);
    arduboy.drawPixel(5, 22);
    arduboy.drawPixel(5, 24);
    arduboy.display();
  } else if (option == 1) {
    arduboy.drawPixel(4, 38);
    arduboy.drawPixel(5, 38);
    arduboy.drawPixel(6, 38);
    arduboy.drawPixel(5, 37);
    arduboy.drawPixel(5, 39);
    arduboy.display();
  } else if (option == 2) {
    arduboy.drawPixel(4, 53);
    arduboy.drawPixel(5, 53);
    arduboy.drawPixel(6, 53);
    arduboy.drawPixel(5, 52);
    arduboy.drawPixel(5, 54);
    arduboy.display();
  }
}

/** 
 * Function to do logic for the options save subscreen
 */
void doSaveScreen() {
  arduboy.clear();
  arduboy.drawRect(2, 30, (learningArray.size/saveSizeMod) + 1, 5);  //draw a status bar
  arduboy.setCursor(5, 5);
  arduboy.print(F("Saving Data..."));

  for (uint16_t i = 0; i < learningArray.size; i++) {  //save each element of the learning array save data
    EEPROM.write(EEPROM_STORAGE_SPACE_START + i, learningArray.compressed[i]);

    if (i % saveSizeMod == 0) {
      arduboy.drawPixel(3 + i, 31, WHITE);  //top bar pixel
      arduboy.drawPixel(3 + i, 32, WHITE);  //middle bar pixel
      arduboy.drawPixel(3 + i, 33, WHITE);  //bottom bar pixel
      arduboy.display();
    }

    delay(10);  //delay to give time to update
  }

  arduboy.clear();  //print out success message
  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);
  arduboy.setCursor(5, 5);
  arduboy.print(F("DATA SAVED!"));
  arduboy.display();
  delay(3000);  //delay before going to normal screen again

  selectedOption = 0;
  doOptions();
}

/**
 * Function to do logic for the options serial print subscreen
 */
void doSerialPrintScreen() {
  arduboy.clear();
  arduboy.setCursor(2, 5);
  arduboy.print(F("Print: Save -> Serial"));   //print out a screen title
  uint16_t arraySize = learningArray.count;
  arduboy.drawRect(2, 30, (arraySize/saveSizeMod) + 1, 5);  //draw a status bar
    
  Serial.begin(9600);

  for (uint16_t i = 0; i < arraySize; i++) {  //print each element of the learning array save data
    int8_t val = learningArray[i];
    Serial.println(val);

    if (i % saveSizeMod == 0) {
      arduboy.drawPixel(3 + (i / saveSizeMod), 31, WHITE);  //top bar pixel
      arduboy.drawPixel(3 + (i / saveSizeMod), 32, WHITE);  //middle bar pixel
      arduboy.drawPixel(3 + (i / saveSizeMod), 33, WHITE);  //bottom bar pixel
      arduboy.display();
    }
    delay(10);  //delay to give time to update
  }

  Serial.end();

  arduboy.clear();  //print out success message
  arduboy.drawRoundRect(0, 0, screenWidth, screenHeight, 3);
  arduboy.setCursor(5, 5);
  arduboy.print(F("SERIAL DATA SENT!"));
  arduboy.display();
  delay(3000);  //delay before going to normal screen again

  selectedOption = 0;
  doOptions();
}

/**
 * Function to do logic for and print out the credits screen
 */
void doCreditsScreen() {
  arduboy.clear();

  arduboy.setCursor(35, 5);
  arduboy.print(F("-CREDITS-"));
  arduboy.setCursor(3, 20);
  arduboy.print(F("Bio/Program:"));
  arduboy.setCursor(80, 20);
  arduboy.print(F("Izalith"));
  arduboy.setCursor(3, 30);
  arduboy.print(F("C Comp Prog:"));
  arduboy.setCursor(80, 30);
  arduboy.print(F("K Akhtar"));
  arduboy.setCursor(3, 40);
  arduboy.print(F("Python Prog:"));
  arduboy.setCursor(80, 40);
  arduboy.print(F("Molly C"));
  arduboy.setCursor(2, 50);
  arduboy.print(F("Inspiration:"));    
  arduboy.setCursor(80, 50);
  arduboy.print(F("Nova"));
  arduboy.display();

  delay(2000);
  selectedOption = 0;
  doOptions();
}

/***********************************SIM FUNCTIONS*************************************/
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

/**
 * Function to draw the worms facial expressions
 */
void drawFaces() {   
  const uint8_t expressionX = 40;   //X position to draw the expression 
  const uint8_t expressionY = 35;   //Y position to draw the expression     
  static uint8_t currentExp = 0;           //value representing the current facial expression

  for (uint16_t i = 0; i < totalNeurons; i++) {
    if (outputList[i]) {
      if (outputList[39] && !outputList[40]) { //attractive chemotaxis; contented
        currentExp = 0;           //ASEL is on and ASER is off indicates attractive chemotaxis
        arduboy.fillRect(expressionX + 7, expressionY - 4, 10, 10, BLACK);
        Sprites::drawOverwrite(expressionX + 9, expressionY - 4, content, 0);       //^-^
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);  
      } else if (i == 53 || i == 54 || i == 57 || i == 58 || i == 59 || i == 60) {  //escape behavior; fear
        currentExp = 1;           //AVA, AVD, AVE; command interneurons for reversals
        arduboy.fillRect(expressionX + 7, expressionY - 4, 10, 10, BLACK);
        Sprites::drawOverwrite(expressionX + 9, expressionY - 4, fear, 0);          //O_O
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } else if (i == 149 || i == 150 || (i == 167 || i == 147)) { //foraging behavior; happy
        currentExp = 2;           //NSM, MC; activates during gustation and satiety
        arduboy.fillRect(expressionX + 7, expressionY - 4, 10, 10, BLACK);
        Sprites::drawOverwrite(expressionX + 8, expressionY - 4, happy, 0);         //^v^
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } else if (i == 43 && i == 44 && (i == 6 || i == 6 || i == 49 || i == 50)) {  //repulsive chemotaxis; disgust
        currentExp = 3;           //ASH, ADL, ASK; repulsive chemosensors
        arduboy.fillRect(expressionX + 7, expressionY - 4, 10, 10, BLACK);
        Sprites::drawOverwrite(expressionX + 9, expressionY - 4, disgust, 0);       //>n<
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);   
      } else if (i == 174 || i == 175 || i == 8 || i == 9 || i == 113 || i == 114 || i == 165 || i == 166) { //hot or cold; discomfort
        currentExp = 4;           //PVD, AFD, FLP, PHC; thermosensors for cold (PVD) and hot (the rest) 
        arduboy.fillRect(expressionX + 7, expressionY - 4, 10, 10, BLACK);
        Sprites::drawOverwrite(expressionX + 9, expressionY - 4, discomfort, 0);    //@_@
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } else if (i == 204) {   //sleep
        currentExp = 5;           //RIS; sleep state neuron 
        arduboy.fillRect(expressionX + 7, expressionY - 4, 10, 10, BLACK);
        Sprites::drawOverwrite(expressionX + 7, expressionY - 4, sleepy, 0);        //UwU
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } else if (i == 43 || i == 44) { //generalized nociception; pain
        currentExp = 6;           //ASH
        arduboy.fillRect(expressionX + 7, expressionY - 4, 10, 10, BLACK);
        Sprites::drawOverwrite(expressionX + 8, expressionY - 4, pain, 0);          //;_;
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);         
      }
    }
  }
}

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
/*void tonic(bool useGradient, uint8_t mod, uint8_t gradientStep, void (*senseFunction)()) {
  if (useGradient) {
    mod = (mod / gradientStep) + 1;                 //modify distance or rate adjustment; 5 is the precision of the gradient, the step of the gradient
    if (tick % mod == 0) senseFunction();
  } else {
    if (tick % mod == 0) senseFunction();
  }
}*/

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

/*************************************HELPER FUNCTIONS***************************************/
/**
 * Function to draw the contents of the input screen
 */
void drawInputs(uint8_t scroll, char option1[], char option2[], char option3[], char option4[]) {
  arduboy.clear();                              //scroll 0

  if (scroll == 0 || scroll == 1 || scroll == 2) {
    arduboy.setCursor(10,3);
    arduboy.print(F("Select Input Type: "));
    arduboy.setCursor(10,23);
    arduboy.print(F("- "));
    arduboy.print(option1);
    arduboy.setCursor(10,33);      
    arduboy.print(F("- "));
    arduboy.print(option2);
    arduboy.setCursor(10,43);
    arduboy.print(F("- "));
    arduboy.print(option3);
    arduboy.setCursor(10,53);      
    arduboy.print(F("- "));
    arduboy.print(option4);
  } else if (scroll == 3) {            //if its the last of the input screens just print the one option
    arduboy.setCursor(10,3);
    arduboy.print(F("Select Input Type: "));
    arduboy.setCursor(10,23);
    arduboy.print(F("- "));
    arduboy.print(option1);
  }
  
//             [Scroll == 0]             [Scroll == 1]           [Scroll == 2]            [Scroll == 3]
// 
//        x=2
//         |
//        |''''''''''''''''''''''| |''''''''''''''''''''''| |''''''''''''''''''''''| |''''''''''''''''''''''|
// y=23---|* pc0  gentle nose    | |* pc4  heating        | |* pc8 baseline        | |* pc12 sleeping       |
// y=33---|* pc1  chemoattr      | |* pc5  phototaxis     | |* pc9 hungry          | |                      |
// y=43---|* pc2  chemorep       | |* pc6  aerotaxis      | |* pc10 sated          | |                      |
// y=53---|* pc3  cooling        | |* pc7  pheromone      | |* pc11 eating         | |                      |
//        |......................| |......................| |......................| |......................|

  //draw the input select indicator on the left
  if (posCount == 0 || posCount == 4 || posCount == 8 || posCount == 12) {
    arduboy.drawPixel(2, 26, WHITE);    //draw the rest of the pixel
    arduboy.drawPixel(3, 26, WHITE);
    arduboy.drawPixel(4, 26, WHITE);
    arduboy.drawPixel(3, 25, WHITE);
    arduboy.drawPixel(3, 27, WHITE);
    arduboy.display();
  } else if (posCount == 1 || posCount == 5 || posCount == 9) {
    arduboy.drawPixel(2, 36, WHITE);
    arduboy.drawPixel(3, 36, WHITE);
    arduboy.drawPixel(4, 36, WHITE);
    arduboy.drawPixel(3, 35, WHITE);
    arduboy.drawPixel(3, 37, WHITE);
    arduboy.display();
  } else if (posCount == 2 || posCount == 6 || posCount == 10) {
    arduboy.drawPixel(2, 46, WHITE);
    arduboy.drawPixel(3, 46, WHITE);
    arduboy.drawPixel(4, 46, WHITE);
    arduboy.drawPixel(3, 45, WHITE);
    arduboy.drawPixel(3, 47, WHITE);
    arduboy.display();
  } else if (posCount == 3 || posCount == 7 || posCount == 11) {
    arduboy.drawPixel(2, 56, WHITE);
    arduboy.drawPixel(3, 56, WHITE);
    arduboy.drawPixel(4, 56, WHITE);
    arduboy.drawPixel(3, 55, WHITE);
    arduboy.drawPixel(3, 57, WHITE);
    arduboy.display();
  }
}

/**
 * Function to get the output value of a given neuron.
 * Used primarily by the interfacing functions to show
 * the user network information and the UI
 */
bool outputBool(uint16_t cellID) {
  return outputList[cellID];
}

/**
 * Helper function to figure out which position the marker is on the input screen
 */
void doPos() {
  if (posCount >= 0 && posCount <= 3) scroll = 0;
  if (posCount >= 4 && posCount <= 7) scroll = 1;
  if (posCount >= 8 && posCount <= 11) scroll = 2;
  if (posCount >= 12) scroll = 3;

  if (posCount == 0) {
    isAsleep = false;
    sense = 0;
  }

  if (posCount == 1) {
    isChemotaxis = true;
    isAsleep = false;
    sense = 1;
  }

  if (posCount == 2) {
    isChemotaxis = true;
    isAsleep = false;
    sense = 2;
  }

  if (posCount == 3) {
    isAsleep = false;
    sense = 3;
  }

  if (posCount == 4) {
    isAsleep = false;
    sense = 4;
  }

  if (posCount == 5) {
    isAsleep = false;
    sense = 5;
  }

  if (posCount == 6) {
    isAsleep = false;
    sense = 6;
  }

  if (posCount == 7) {
    isChemotaxis = true;
    isAsleep = false;
    sense = 7;
  }

  if (posCount == 8) {
    sense = 8;
  }

  if (posCount == 9) {
    isAsleep = false;
    sense = 9;
  }

  if (posCount == 10) {
    isAsleep = false;
    sense = 10;
  }

  if (posCount == 11) {
    isChemotaxis = true;
    isAsleep = false;
    sense = 11;
  }

  if (posCount == 12) {
    isAsleep = true;
    sense = 12;
  }
}

/**
 * A small function to draw out the muscle ratio information to the screen.
 * Used by the UI to make information more readable for the user.
 */
void printMovementDir(uint16_t xpos, uint16_t ypos) {
  doMovement();

  if (!isForward) {
    arduboy.setCursor(xpos, ypos);
    arduboy.print(F("BACKWARD-"));
  } else {
    arduboy.setCursor(xpos, ypos);      
    arduboy.print(F("FORWARD-"));
  }
  
  if (isLeft) {
    arduboy.print(F("L/VENTRAL"));
  } else {
    arduboy.print(F("R/DORSAL"));
  }
}

/*************************************SIMULATION FUNCTIONS***************************************/
/**
 *The activation function is the main simulation, it calculates all the next ticks of the connectome
 * and then sets the next tick to the current one when each has been individually calculated
 */
void activationFunction() {  
  uint16_t index = 0;                       //position in the unpacked neural ROM
  const uint8_t threshold = 15;             //threshold for activation function
  const uint8_t hebbianConstant = 1;        //constant representing the amount the learning array affects a given synapse
  const uint8_t offset = 2;                 //value to adjust how much "charge" a gap junction sends to next neuron
  const uint16_t totalLearningNeurons = 50; //number of neurons with hebbian learning ability, the size of hebbian_neurons[] array

  //calculate next output for all neurons using the current output list
  for (id; id < totalNeurons; id++) {
    // Read input length (Number of synapses) from ROM
    n.inputLen = NEURAL_ROM[index]; 
    index++;

    // Read neuron inputs from ROM
    for (uint8_t i = 0; i < n.inputLen; i++) {
      n.inputs[i] = NEURAL_ROM[index++];
    }

    // Read neuron weights from ROM
    for (uint8_t i = 0; i < n.inputLen; i++) {
      n.weights[i] = NEURAL_ROM[index++];
    }
    
    //static variable for the position in the learning array; (functions as "global" var) 
    static uint16_t learningPos = 0;
    uint16_t sum = 0;                         //variable to store a running sum for the neuron
    bool hebFlag = false;                     //boolean flag that indicates hebbian learning done at the current neuron

    for (uint8_t hebIndex = 0; hebIndex < totalLearningNeurons; hebIndex++) {             //check to see if current ID is in the hebbian-capable neuron  list
      if (HEBBIAN_NEURONS[hebIndex] == id) {                                              //if the current neuron being read in is in the hebbian neuron array
        for (uint8_t hebInput = 0; hebInput < n.inputLen; hebInput++) {                   //for every presynapse to the current neuron
          n.weights[hebInput] += hebbianConstant * learningArray[learningPos + hebInput]; //adjust its weight based on the learning array
        }

        hebFlag = true;                                                 //flag to mark the neuron's synapse as doing hebbian learning
      }
    }

    for (uint8_t j = 0; j < n.inputLen; j++) {                          //loop over every presynaptic neuron
      if (id == currentID) {                                            //if the current neuron is the one selected in matrix screen
        numSynapses = n.inputLen;                                       //get the input length
        preSynapticNeuronList[j] = n.inputs[j];                         //get the pre-synaptic neuron (inputs) list
        if (n.inputs[j] == preID) synWeight = n.weights[j];             //if the current pre-synaptic neuron (input) is the one selected in matrix screen, get its weight
      }

      if (n.weights[j] >= gapJuncMinVal) {                              //if the weight has a gap junction indicator (9_)
        int16_t gapWeight = n.weights[j] - gapJuncMinVal;               //gap junctions are indicated by weights at 90-99
        int8_t gapOutput = 0;                                           //gap junctions presynapse outputs are adjusted, as they are not binary
        if (outputList[n.inputs[j]]) gapOutput = (1 + offset);          //outputList value is adjusted for non-binary activation
        if (!outputList[n.inputs[j]]) gapOutput = -1 * (1 + offset);
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

