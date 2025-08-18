/**
 *    VITAL
 * Make it easier to exit host subscreen
 * Add "T Menu" indicator sprites:
 *      Matrix     
               D<--M-->I
                   |
                   V
                   O
 *      Diag       
                M-->-->I        
                   |
                   O
 *      Inputs    
               D<--<--M
                   |
                   O
 *      Options
               D<--^-->I
                   |
                   M
 * change screens to better match sprites
 *      Matrix: 
              left to diag, right to inputs, down to options
              A decrements by 1, B increments by 1
              UP - increments by 10
              A+B activates sense
 *      Diag: 
              Right to matrix
              (LEFT - unused???)
              Down decreases by 1, Up increases by 1
              A also increases by 1? B also decreases by 1?
 *      Inputs: 
              up and down scroll the list, right toggles the phase 
              left brings to matrix screen
              A also scrolls left? B also scrolls right?
 *      Options: 
              down scrolls down; up to matrix screen
              A/B selects the option
              left scrolls up, right also scrolls down

 *
 *    BOTTLENECKS
 * add gap junctions variants to neural rom sets 
 * implement the compressed learning arrays and eeprom saving
 *
 *    LESS IMPORTANT
 * add sprites for the Different options?
 * use neuron names instead of IDs?
 * quick scrolling in menus?
 *
 *    STRETCH
 * add a worm movement indicator sprite to the matrix screen (left of the mood indicator)
 * add an actually accurate perceptron model in diag screen
 */

#include "sprites.h"              //import libraries
#include "neuralROM.h"
#include "bit_array.h"
#include "sized_int_array_reader.h"
#include <Arduboy2.h>

Arduboy2 arduboy;                 //create arduboy object

const uint8_t threshold = 1;      //threshold for activation function
const uint16_t maxSynapse = 65;   //max synapses a neuron can have as inputs
const uint8_t expressionX = 50;   //X position to draw the expression
const uint8_t expressionY = 25;   //Y position to draw the expression
uint16_t currentID = 0;           //interface variable to indicate which neuron is being analyzed
uint16_t numSynapses = 0;         //interface variable to indicate number of synapses connected
uint16_t preID = 0;               //interface variable to indicate which neuron is being analyzed
uint8_t sense = 0;                //interface variable to indicate which sense is activated
uint8_t option = 0;               //interface variable to indicate which option is selected
uint8_t posCount = 0;             //interface variable to indicate which sense is being looked at
uint8_t lastScreen = 0;           //interface variable to indicate last screen before button press
int8_t synWeight = 0;             //interface variable to indicate the weight of a given synapse
bool startFlag = true;            //interface flag for title screen to play
bool startInputFlag = false;      //variable to determine if an input has been selected yet
float vaRatio = 0;                //muscle ratios for the interface printout
float vbRatio = 0;
float daRatio = 0;
float dbRatio = 0;
uint8_t synCounter = 0;           //Counter to hold information on which pre-synaptic neuron is selected
uint8_t scroll = 0;               //variable to indicate which sub-screen in the inputs list user is on
uint16_t id = 0;                  //the current neuron activation function is calculating
uint16_t tick = 0;                //connectome ticks (resets to zero at 3628800)
bool sated = false;               //indicates if worm is sated
bool isEating = false;            //indicates if worm is eating
bool isAsleep = false;            //indicates if worm is sleeping
bool isTonic = false;             //indicates if an input is tonic (or phasic)
bool isChemotaxis = false;        //indicates if the worm is experiencing some kind of chemotaxis
uint8_t learningArraySize = 125;    //number of neurons in learning array
uint8_t maxLearningVal = 71;      //maximum possible value of learning array elements
uint8_t selectedOption = 0;       //for if an options screen is selected
uint8_t currentExp = 0;           //value representing the current facial expression

SizedIntArrayReader<9> NEURAL_ROM(COMPRESSED_NEURAL_ROM, 7578, -70, true);
int8_t learningArray[] = {20, 25, 60, 70, 1};
uint16_t preSynapticNeuronList[maxSynapse];  //interface array to hold all the different presynaptic neurons
//BitArray<302> <maxSynapse> learningArray;    //an array that, for each neuron, holds its firing history
BitArray<302> outputList;                    //list of neurons
BitArray<302> nextOutputList;                //buffer to solve conflicting time differentials in firing

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
  if (!(arduboy.nextFrame())) {  //verify that runs properly
    return;
  }
  
  doTitleScreen();

  activationFunction();         //do the main calculation of the connectome
  arduboy.pollButtons();        //get buttons pressed and send to function to process them
  doButtons();

  //delay(50);
  if (tick == 3628800) {                        //reset tick Counter for gradient senses when it reaches the factorial of 10 (each gradient is 5)
    tick == 0;
  } else {
    tick++;
  }
  
  //go to the proper screens based on the direction button last clicked
  if (arduboy.justPressed(UP_BUTTON) || startFlag) doMatrixScreen();
  if (arduboy.justPressed(LEFT_BUTTON)) doDiagnosticScreen();
  if (arduboy.justPressed(RIGHT_BUTTON)) doInputScreen();
  if (arduboy.justPressed(DOWN_BUTTON)) doOptions();

  //do options screens if selected
  if (selectedOption == 1) doSaveScreen();
  if (selectedOption == 2) doSerialPrintScreen();
  if (selectedOption == 3) doSerialHostScreen();
  if (selectedOption == 4) doSaveEditorScreen();

  arduboy.drawRoundRect(0, 0, 128, 64, 3);      //draw a border around the screen

  arduboy.display();
  
  startFlag = false;            //set the flag off; only do the title screen once
}

/**
 * function to run the diagnostic screen; a screen that shows details of synapses
 * and allows the user to make changes
 */
void doDiagnosticScreen() {
  lastScreen = 3;

//TODO: replace currentID with the actual cell name???
  arduboy.clear();
  arduboy.setCursor(10, 10);
  arduboy.print(currentID);
  arduboy.print(" Syn: ");
  arduboy.print(numSynapses);
  arduboy.setCursor(10, 20);
  arduboy.print("Weight: ");

  if (synWeight >= 90) {
    arduboy.print("GAP-");
    arduboy.print(synWeight - 90);
  } else {
    arduboy.print(synWeight);
  }

  arduboy.setCursor(10, 30);
  arduboy.print("Pre ID: ");
  arduboy.print(preID);
  arduboy.setCursor(10, 40);
  arduboy.print("postID: ");
  arduboy.print(currentID);

//TODO: make the perceptron sprites actually show real number of synapses???
  if (outputList[currentID]) Sprites::drawOverwrite(85, 30, perceptronON, 0);
  if (!outputList[currentID]) Sprites::drawOverwrite(85, 30, perceptronOFF, 0);

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

      //draw a tonic/phasic key
      if (isTonic) {
        arduboy.drawPixel(123, 53);       //indicator pixels
        arduboy.drawPixel(124, 53);
        arduboy.drawRect(114, 50, 8, 8);  //draw a border
        arduboy.drawPixel(117, 52);       //draw a mini "T"
        arduboy.drawPixel(117, 53);
        arduboy.drawPixel(117, 54);
        arduboy.drawPixel(117, 55);
        arduboy.drawPixel(116, 52);
        arduboy.drawPixel(118, 52);
      } else {
        arduboy.drawPixel(117, 47);       //indicator pixels
        arduboy.drawPixel(117, 48);
        arduboy.drawRect(114, 50, 8, 8);  //draw a border
        arduboy.drawPixel(117, 52);       //draw a mini "P"
        arduboy.drawPixel(117, 53);
        arduboy.drawPixel(117, 54);
        arduboy.drawPixel(117, 55);
        arduboy.drawPixel(118, 52);
        arduboy.drawPixel(119, 53);
        arduboy.drawPixel(118, 54);
      }
}

/**
 * function to query which buttons are pressed, setting up proper screen transitions
 */
void doButtons() {
  //matrix screen
  if (lastScreen == 1) {
    if (arduboy.justPressed(A_BUTTON) && arduboy.justPressed(B_BUTTON)) {
      startInputFlag = true;

//TEST: test these to make sure the values for tonic and phasic frequencies are good
      if (sense == 0) {
        if (isTonic) {
          tonic(false, 1, 0, doGentleNoseTouch);    //sense gentle nose touch
        } else {
          phasic(false, 90, doGentleNoseTouch);
        }
      }
      if (sense == 1) {
        isChemotaxis = true;
        if (isTonic) {
          tonic(false, 1, 0, doChemoattraction);    //sense attractant chemicals
        } else {
          phasic(false, 90, doChemoattraction);
        }
      }
      if (sense == 2) {
        isChemotaxis = true;
        if (isTonic) {
          tonic(false, 1, 0, doChemorepulsion);     //sense avoidant chemicals
        } else {
          phasic(false, 90, doChemoattraction);
        }
      }
      if (sense == 3) {
        if (isTonic) {
          tonic(false, 1, 0, doCoolingResponse);            //sense cooling
        } else {
          phasic(false, 90, doCoolingResponse);
        }
      }
      if (sense == 4) {
        if (isTonic) {
          tonic(false, 1, 0, doHeatingResponse);            //sense warmth
        } else {
          phasic(false, 90, doHeatingResponse);
        }
      }
      if (sense == 5) {
        if (isTonic) {
          tonic(false, 1, 0, doPhotosensation);     //sense light
        } else {
          phasic(false, 90, doPhotosensation);
        }
      }
      if (sense == 6) {
        if (isTonic) {
          tonic(false, 1, 0, doOxygenSensation);    //sense oxygen
        } else {
          phasic(false, 90, doPhotosensation);
        }
      }
      if (sense == 7) {
        isChemotaxis = true;
        if (isTonic) {
          tonic(false, 1, 0, sensePheromones);      //sense pheromones
        } else {
          phasic(false, 90, sensePheromones);
        }
      }
      if (sense == 8) {
        isChemotaxis = false;
        if (isTonic) {
          tonic(false, 1, 0, doBaseline);     //activtate baseline state
        } else {
          phasic(false, 90, doBaseline);
        }
      }
      if (sense == 9) {                 //toggle fullness state
        sated = false;
        if (isTonic) {
          tonic(false, 1, 0, isHungry);
        } else {
          phasic(false, 90, isHungry);
        }
      }
      if (sense == 10) {                //toggle fullness state
        sated = true;
        if (isTonic) {
          tonic(false, 1, 0, doSatiety);
        } else {
          phasic(false, 90, doSatiety);
        }
      }
      if (sense == 11 && !sated) {                //toggle eating state
        isChemotaxis = true;
        isEating = true;
        if (isTonic) {
          tonic(false, 1, 0, doGustation);
        } else {
          phasic(false, 90, doGustation);
        }
      } else if (sense == 11 && sated) {
        isChemotaxis = true;
        isEating = false;
        arduboy.setCursor(70,40);
        arduboy.print("(SATED)");
        delay(50);
      }
      if (sense == 12 && isAsleep) {    //toggle sleep state
        tonic(false, 1, 0, maintainSleep);
      } else if (sense == 12 && !isAsleep) {
        isAsleep = true;
        phasic(false, 90, activateSleep);
        tonic(false, 1, 0, maintainSleep);
      }
    }

//TODO: implement quick scrolling for A???
    if (arduboy.justPressed(A_BUTTON) && !arduboy.justPressed(B_BUTTON)) {
      if (currentID < 302 - 1) {
        currentID++;
      } else {
        currentID = 0;
      }
    }

//TODO: implement decrement for B???
    if (arduboy.justPressed(B_BUTTON) && !arduboy.justPressed(A_BUTTON)) {
      if (currentID < 302 - 1) {
        if (currentID + 10 > 302 - 1) {
          currentID = 301;
        } else {
          currentID += 10;
        }
      } else {
        currentID = 0;
      }
    }

    doMatrixScreen();
  }

  //options screen
  if (lastScreen == 2) {
    if (arduboy.justPressed(A_BUTTON)) {    //scroll through options
      option++;
      if (option > 3) option = 0;
      doOptions();
    }
    
    if (arduboy.justPressed(B_BUTTON)) {               //select an option
      if (option == 0) {                                  //save current learning array data ("SAVE")
        selectedOption = 1;
      } else if (option == 1) {                           //print current learning array data over serial ("PRINT SAVE") 
        selectedOption = 2;
      } else if (option == 2) {                           //host a simulation over serial (receive inputs, send output info) ("HOST")
        selectedOption = 3;
      } else if (option == 3) {                           //edit learning array data ("SAVE EDIT")
        selectedOption = 4;
      }
    } else {
      doOptions();
    }
  }

  //diagnostic screen
  if (lastScreen == 3) {
    if (arduboy.justPressed(A_BUTTON)) {
      if (synCounter < numSynapses - 1) {
        synCounter++;
      } else {
        synCounter = 0;
      }
      //synWeight = 0;
      preID = preSynapticNeuronList[synCounter];
    }

    if (arduboy.justPressed(B_BUTTON)) {
      if (synCounter > 0) {
        synCounter--;   
      } else {
        synCounter = n.inputLen;
      }
      //synWeight = 0;
      preID = preSynapticNeuronList[synCounter];
    }

    doDiagnosticScreen();
  }
  
  //input select screen
  if (lastScreen == 4) {
    if (arduboy.justPressed(B_BUTTON)) {
      isTonic = !isTonic;
    }
    if (arduboy.justPressed(A_BUTTON)) {
       posCount++;
       if (posCount > 12) posCount = 0;
       if (posCount >= 0 && posCount <= 3) scroll = 0;
       if (posCount >= 4 && posCount <= 7) scroll = 1;
       if (posCount >= 8 && posCount <= 11) scroll = 2;
       if (posCount == 12) scroll = 3;
  
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

    doInputScreen();
  }  

  arduboy.display();
}

/**
 * Function that displays the main title screen
 */
void doTitleScreen() {
  if (startFlag) {
    uint16_t address = EEPROM_STORAGE_SPACE_START;

    //clear the screen then write app name
    arduboy.clear();
    arduboy.setCursor(15, 10);
    arduboy.print(F("-ArduNet Elegans-"));
    arduboy.setCursor(10, 50);
    arduboy.display();
//TODO: uncomment when learningArray is implemented, fix address as well
/*    for (uint8_t i = 0; i < learningArraySize; i++) {
      EEPROM.read(address + i, learningArray[i]);
    }*/

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
  arduboy.print("NEURAL GRID: ");

  arduboy.setCursor(25, 20);
  arduboy.print("Mood: ");

//TODO: print the actual cell name instead of ID???
  arduboy.setCursor(5, 45);
  arduboy.print("CURRENT CELL ID: ");
  arduboy.print(currentID);

  if (!startInputFlag) {
    arduboy.setCursor(5, 55);
    arduboy.print("STATIONARY");
  } else {
    printMovementDir(5, 55);
  }

  drawFaces();

  uint8_t gridWidth = 17;
  uint8_t gridHeight = 18;
  uint16_t xPos = 80;
  uint16_t yPos = 5;
        
  //draw border on grid of cells
  arduboy.drawRect(xPos, yPos, (gridWidth + 1)*2, (gridHeight + 1)*2);

  int neuronCounter = 0;
      
  //draw grid of cells, blacking out current cell ID
  for (uint16_t x = 1; x <= (gridWidth*2); x++) {
    for (uint16_t y = 1; y <= (gridHeight*2); y++) {
      //for each neuron get the output state and draw a pixel accordingly               
      if (neuronCounter < 302) {
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
//TODO: add sprites for the options on the right side of screen
  lastScreen = 2;
  arduboy.clear();

  //print out the options
  arduboy.setCursor(38, 3);
  arduboy.print("-OPTIONS-");
  arduboy.setCursor(10, 18);
  arduboy.print("SAVE DATA");
  arduboy.setCursor(10, 28);
  arduboy.print("PRINT SAVE");
  arduboy.setCursor(10, 38);
  arduboy.print("HOST ANN");
  arduboy.setCursor(10, 48);
  arduboy.print("EDIT SAVE");

  //draw the selector pixels for the current option
  if (option == 0) {          
    arduboy.drawPixel(4, 21, WHITE);
    arduboy.drawPixel(5, 21, WHITE);
    arduboy.drawPixel(6, 21, WHITE);
    arduboy.drawPixel(5, 20, WHITE);
    arduboy.drawPixel(5, 22, WHITE);
    arduboy.display();
  } else if (option == 1) {
    arduboy.drawPixel(4, 31, WHITE);
    arduboy.drawPixel(5, 31, WHITE);
    arduboy.drawPixel(6, 31, WHITE);
    arduboy.drawPixel(5, 30, WHITE);
    arduboy.drawPixel(5, 32, WHITE);
    arduboy.display();
  } else if (option == 2) {
    arduboy.drawPixel(4, 41, WHITE);
    arduboy.drawPixel(5, 41, WHITE);
    arduboy.drawPixel(6, 41, WHITE);
    arduboy.drawPixel(5, 40, WHITE);
    arduboy.drawPixel(5, 42, WHITE);
    arduboy.display();
  } else if (option == 3) {
    arduboy.drawPixel(4, 51, WHITE);
    arduboy.drawPixel(5, 51, WHITE);
    arduboy.drawPixel(6, 51, WHITE);
    arduboy.drawPixel(5, 50, WHITE);
    arduboy.drawPixel(5, 52, WHITE);
    arduboy.display();
  }
}

/** 
 * Function to do logic for the options save subscreen
 */
void doSaveScreen() {
  arduboy.clear();
  arduboy.drawRect(10, 30, learningArraySize + 1, 5);  //draw a status bar

  arduboy.setCursor(5, 5);
  arduboy.print("Saving Data...");

  for (uint8_t i = 0; i < learningArraySize; i++) {  //save each element of the learning array save data
//TODO: add learning array here
    //EEPROM.write(address + i, learningArray[i]);
    arduboy.drawPixel(11 + i, 31, WHITE);  //top bar pixel
    arduboy.drawPixel(11 + i, 32, WHITE);  //middle bar pixel
    arduboy.drawPixel(11 + i, 33, WHITE);  //bottom bar pixel

    arduboy.display();
    delay(100);  //delay to give time to update
  }

  arduboy.clear();  //print out success message
  arduboy.drawRoundRect(0, 0, 128, 64, 3);
  arduboy.setCursor(5, 5);
  arduboy.print("DATA SAVED!");
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
  arduboy.print("Print: Save -> Serial");   //print out a screen title
  arduboy.drawRect(10, 30, learningArraySize + 1, 5);  //draw a status bar

  for (uint8_t i = 0; i < learningArraySize; i++) {  //print each element of the learning array save data
//TODO: add learning array here
    Serial.begin(9600);
    //Serial.print(EEPROM.read(address + i));
    Serial.end();

    arduboy.drawPixel(11 + i, 31, WHITE);  //top bar pixel
    arduboy.drawPixel(11 + i, 32, WHITE);  //middle bar pixel
    arduboy.drawPixel(11 + i, 33, WHITE);  //bottom bar pixel
    arduboy.display();
    delay(100);  //delay to give time to update
  }

  arduboy.clear();  //print out success message
  arduboy.drawRoundRect(0, 0, 128, 64, 3);
  arduboy.setCursor(5, 5);
  arduboy.print("SERIAL DATA SENT!");
  arduboy.display();
  delay(3000);  //delay before going to normal screen again

  selectedOption = 0;
  doOptions();
}

/**
 * Function to do logic for the options serial host subscreen
 */
void doSerialHostScreen() {
  arduboy.clear();
  arduboy.setCursor(5, 5);
  arduboy.print("Hosting: Serial Sim");            //print out screen title
  arduboy.display();

  while (!arduboy.justPressed(A_BUTTON) || !arduboy.justPressed(B_BUTTON)) {  //continue over serial as long as A/B are not pressed
    arduboy.pollButtons();
    delay(100);

    /*arduboy.setCursor(5, 20);                         //print output readouts to arduboy
    arduboy.print("VA: "); arduboy.print(vaRatio);
    arduboy.setCursor(65, 20);
    arduboy.print("VB: "); arduboy.print(vbRatio);
    arduboy.setCursor(5, 30);
    arduboy.print("DA: "); arduboy.print(daRatio);
    arduboy.setCursor(65, 30);
    arduboy.print("DB: "); arduboy.print(dbRatio);
    arduboy.setCursor(5, 40);
    arduboy.print("EXP: "); arduboy.print(currentExp);
    arduboy.display();*/

    Serial.begin(9600);                               //start comms for outputs
    Serial.print("VA:"); Serial.println(vaRatio);     //print output info over serial, delimited by new lines
    Serial.print("VB:"); Serial.println(vbRatio);
    Serial.print("DA:"); Serial.println(daRatio);
    Serial.print("DB:"); Serial.println(dbRatio);
    Serial.print("EXP:"); Serial.println(currentExp);
    Serial.end();                                     //end comms for outputs

//TEST: adjust serial protocols as needed
    // Input: {BAS0/1, HGR-0/1, GUS-0/1, ASL-0/1, MSL-0/1, GNT-0/1, HNT-0/1, TEX-0/1, ATR-0/1, REP-0/1, COL-0/1, HEA-0/1, HOT-0/1, CLD-0/1, PHO-0/1, OXY-0/1, CO2-0/1, SLT-0/1, PHE-0/1, PRO-0/1}
    for (uint8_t i = 0; i < 20; i++) {
      char serInput[4];
      for(uint8_t j = 0; serInput[j] = ','; j++) {
        serInput[j] = Serial.read();
      }

      if (serInput[0] == 'B' && serInput[1] == 'A' && serInput[2] == 'S' && serInput[3] == 1) doBaseline();
      if (serInput[0] == 'S' && serInput[1] == 'A' && serInput[2] == 'T' && serInput[3] == 1) doSatiety();
      if (serInput[0] == 'H' && serInput[1] == 'G' && serInput[2] == 'R' && serInput[3] == 1) isHungry();
      if (serInput[0] == 'G' && serInput[1] == 'U' && serInput[2] == 'S' && serInput[3] == 1) doGustation();
      if (serInput[0] == 'A' && serInput[1] == 'S' && serInput[2] == 'L' && serInput[3] == 1) activateSleep();
      if (serInput[0] == 'M' && serInput[1] == 'S' && serInput[2] == 'L' && serInput[3] == 1) maintainSleep();
      if (serInput[0] == 'G' && serInput[1] == 'N' && serInput[2] == 'T' && serInput[3] == 1) doGentleNoseTouch();
      if (serInput[0] == 'H' && serInput[1] == 'N' && serInput[2] == 'T' && serInput[3] == 1) doHarshNoseTouch();
      if (serInput[0] == 'T' && serInput[1] == 'E' && serInput[2] == 'X' && serInput[3] == 1) doTextureSense();
      if (serInput[0] == 'A' && serInput[1] == 'T' && serInput[2] == 'R' && serInput[3] == 1) doChemoattraction();
      if (serInput[0] == 'R' && serInput[1] == 'E' && serInput[2] == 'P' && serInput[3] == 1) doChemorepulsion();
      if (serInput[0] == 'C' && serInput[1] == 'O' && serInput[2] == 'L' && serInput[3] == 1) doCoolingResponse();
      if (serInput[0] == 'H' && serInput[1] == 'E' && serInput[2] == 'A' && serInput[3] == 1) doHeatingResponse();
      if (serInput[0] == 'H' && serInput[1] == 'O' && serInput[2] == 'T' && serInput[3] == 1) doNoxiousHeatResponse();
      if (serInput[0] == 'C' && serInput[1] == 'L' && serInput[2] == 'D' && serInput[3] == 1) doNoxiousColdResponse();
      if (serInput[0] == 'P' && serInput[1] == 'H' && serInput[2] == 'O' && serInput[3] == 1) doPhotosensation();
      if (serInput[0] == 'O' && serInput[1] == 'X' && serInput[2] == 'Y' && serInput[3] == 1) doOxygenSensation();
      if (serInput[0] == 'C' && serInput[1] == 'O' && serInput[2] == '2' && serInput[3] == 1) doCO2Sensation();
      if (serInput[0] == 'S' && serInput[1] == 'L' && serInput[2] == 'T' && serInput[3] == 1) doSaltSensation();
      if (serInput[0] == 'P' && serInput[1] == 'H' && serInput[2] == 'E' && serInput[3] == 1) sensePheromones();
      if (serInput[0] == 'P' && serInput[1] == 'R' && serInput[2] == 'O' && serInput[3] == 1) doProprioception();

    }
  }

  arduboy.clear();  //print out simulation end message
  arduboy.drawRoundRect(0, 0, 128, 64, 3);
  arduboy.setCursor(5, 5);
  arduboy.print("SIMULATION ENDED!");
  arduboy.display();
  delay(3000);

  selectedOption = 0;
  doOptions();
}

/**
 * Function to do logic for the options save editor subscreen
 */
void doSaveEditorScreen() {
  uint16_t savePos = 0;
  int8_t tempArray[learningArraySize];

  for (uint8_t i = 0; i < learningArraySize; i++) {
    tempArray[i] = learningArray[i];
  }

  while (!arduboy.justPressed(A_BUTTON)) {  //if A press; exit without saving
    arduboy.clear();
    arduboy.setCursor(27, 5);
    arduboy.print("-Save Editor-");         //print out screen title
    arduboy.setCursor(8, 45);
    arduboy.print("(B)-----------SAVE");
    arduboy.setCursor(8, 55);
    arduboy.print("(A)--EXIT (NO SAVE)");  //print out instructions for save data editor
    arduboy.pollButtons();
    arduboy.display();

//TODO: add learning arrays here
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
      arduboy.print("[");
      arduboy.print(tempArray[savePos-1]);  //arduboy.print(EEPROM.read(address + savePos - 1);
      arduboy.setCursor(36, 25);
      arduboy.print("]");
      arduboy.setCursor(43, 25);
      arduboy.print("-");
    } else {
      arduboy.setCursor(43, 25);
      arduboy.print(">");
    }

    //draw MIDDLE TOP arrow
    arduboy.drawPixel(64, 19);
    arduboy.drawPixel(64, 20);
    arduboy.drawPixel(64, 21);
    arduboy.drawPixel(64, 22);
    arduboy.drawPixel(63, 21);
    arduboy.drawPixel(65, 21);

    //print current element
    arduboy.setCursor(49, 25);
    arduboy.print("[");
    arduboy.print(tempArray[savePos]); //arduboy.print(EEPROM.read(address + savePos);
    arduboy.setCursor(72, 25);
    arduboy.print("]");

    //draw MIDDLE BOTTOM arrow
    arduboy.drawPixel(64, 34);
    arduboy.drawPixel(64, 35);
    arduboy.drawPixel(64, 36);
    arduboy.drawPixel(64, 37);
    arduboy.drawPixel(63, 35);
    arduboy.drawPixel(65, 35);

    //print next element (if not last)
    if (savePos < learningArraySize) {
      arduboy.setCursor(79, 25);
      arduboy.print("-");
      arduboy.setCursor(85, 25);
      arduboy.print("[");
      arduboy.print(tempArray[savePos+1]); //arduboy.print(EEPROM.read(address + savePos + 1));
      arduboy.setCursor(108, 25);
      arduboy.print("]");

      //draw RIGHT arrow
      arduboy.drawPixel(117, 27);
      arduboy.drawPixel(115, 28);
      arduboy.drawPixel(116, 28);
      arduboy.drawPixel(117, 28);
      arduboy.drawPixel(118, 28);
      arduboy.drawPixel(117, 29);
    } else {
      arduboy.setCursor(78, 25);
      arduboy.print("<");
    }

    arduboy.display();

//TODO: add fast scrolling to the learning array save data editor???
    if (arduboy.justPressed(RIGHT_BUTTON)) {        //if R press; move to next position in array
      if (savePos != learningArraySize) {
        savePos++;
      } else {
        savePos = 0;
      }
    } else if (arduboy.justPressed(LEFT_BUTTON)) {  //if L press; move to prev position in array
      if (savePos != 0) {
        savePos--;
      } else {
        savePos = learningArraySize;
      }      
    } else if (arduboy.justPressed(UP_BUTTON)) {    //if Up press; change value in current position
      if (tempArray[savePos] > maxLearningVal) {
        tempArray[savePos] = -70;       //EEPROM.write(address + i, 0);                     //set to zero if past the max val
      } else {
        tempArray[savePos]++;           //EEPROM.write(address + i, learningArray[i] + 1);  //increment value
      }
    } else if (arduboy.justPressed(DOWN_BUTTON)) {
      if (tempArray[savePos] < -70) {
        tempArray[savePos] = maxLearningVal;         //EEPROM.write(address + i, 0);        //set to max if past the min val
      } else {
        tempArray[savePos]--;           //EEPROM.write(address + i, learningArray[i] + 1);  //increment value
      }
    }


    if (arduboy.justPressed(B_BUTTON)) {            //if B press, flush buffer
      for (uint8_t i = 0; i < learningArraySize; i++) {
        learningArray[i] = tempArray[i];
      }

      arduboy.clear();  //print out success message
      arduboy.drawRoundRect(0, 0, 128, 64, 3);
      arduboy.setCursor(5, 5);
      arduboy.print("EDITS SAVED!");
      arduboy.display();
      delay(3000);

      selectedOption = 0;

      return;
    }
  }

  arduboy.clear();  //exit subscreen, do not print any message
  arduboy.drawRoundRect(0, 0, 128, 64, 3);
  arduboy.display();
  delay(100);
  
  selectedOption = 0;
}

/***********************************SIM FUNCTIONS*************************************/
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
  if (outputList[39] && !outputList[40]) { //attractive chemotaxis; contented
    currentExp = 0;
    Sprites::drawOverwrite(expressionX + 9, expressionY - 4, content, 0);           //^-^
    arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);  
  } //ASEL is on and ASER is off indicates attractive chemotaxis
        
  for (uint16_t i = 0; i < 302; i++) {
    if (outputList[i]) {
      if (i == 53 || i == 54 || i == 57 || i == 58 || i == 59 || i == 60) {  //escape behavior; fear
        currentExp = 1;
        Sprites::drawOverwrite(expressionX + 9, expressionY - 4, fear, 0);          //O_O
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } //AVA, AVD, AVE

      if (i == 149 || i == 150 || (i == 167 || i == 147)) { //foraging behavior; happy
        currentExp = 2;
        Sprites::drawOverwrite(expressionX + 8, expressionY - 4, happy, 0);         //^v^
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } //NSM, MC

      if (i == 43 && i == 44 && (i == 6 || i == 6 || i == 49 || i == 50)) {  //repulsive chemotaxis; disgust
        currentExp = 3;
        Sprites::drawOverwrite(expressionX + 9, expressionY - 4, disgust, 0);       //>n<
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);              
      } //ASH, ADL, ASK

      if (i == 174 || i == 175 || (i == 8 || i == 9 || i == 113 || i == 114 || i == 165 || i == 166)) { //hot or cold; discomfort
        currentExp = 4;
        Sprites::drawOverwrite(expressionX + 9, expressionY - 4, discomfort, 0);    //@_@
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } //PVD, AFD, FLP, PHC

      if (i == 199 || i == 200) {   //sleep
        currentExp = 5;
        Sprites::drawOverwrite(expressionX + 7, expressionY - 4, sleepy, 0);        //UwU
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);
      } //RIM

      if (i == 43 || i == 44) { //generalized nociception; pain
        currentExp = 6;
        Sprites::drawOverwrite(expressionX + 8, expressionY - 4, pain, 0);          //;_;
        arduboy.drawRoundRect(expressionX + 5, expressionY - 6, 17, 8, 2, WHITE);         
      } //ASH
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
    if (tick % mod == 0) senseFunction();
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

  //calculate next output for all neurons using the current output list
  for (id; id < 302; id++) {
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
    
    //matrixToNeuron(id);                               //fill the neuron struct with the information of the ith neuron
    int32_t sum = 0;
    bool hebFlag = false;
    uint8_t offset = 2;                              //value to adjust how much "charge" a gap junction sends to next neuron

    for (uint8_t j = 0; j < 66; j++) {               //check to see if current ID is in the hebbian-capable neuron  list
      if (HEBBIAN_NEURONS[j] == id) {
        //sum += learningArray[j];
        hebFlag = true;
      }
    }

    for (uint8_t j = 0; j < n.inputLen; j++) {                          //loop over every presynaptic neuron
      if (id == currentID) {                                             //if the current neuron is the one selected in matrix screen
        numSynapses = n.inputLen;                                       //get the input length
        preSynapticNeuronList[j] = n.inputs[j];                         //get the pre-synaptic neuron (inputs) list
        if (n.inputs[j] == preID) synWeight = n.weights[j];             //if the current pre-synaptic neuron (input) is the one selected in matrix screen, get its weight
      }

      if (n.weights[j] >= 90) {                                         //if the weight has a gap junction indicator (9_)
        int8_t gapWeight = n.weights[j] - 90;                           //gap junctions are indicated by weights at 90-99
        uint8_t gapOutput = 0;                                          //gap junctions presynapse outputs are adjusted, as they are not binary
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

//TODO: add learning array calls here
    for (uint8_t j = 0; j < n.inputLen; j++) {                          //loop over every presynaptic neuron
      if (hebFlag) {                                                    //if current neuron is in list
        if (outputList[n.inputs[j]] && nextOutputList[id]) {            //if the pre and postsynaptic neuron both fire
          //the specific synapse gets an increased hebbian value
        } else if (!outputList[n.inputs[j]] && !nextOutputList[id]) {   //if the pre and postsynaptic neuron both do NOT fire
          //the specific synapse gets a decreased hebbian value
        }
      }
    }
  }

  //flush the buffer
    for (int16_t i = 0; i < 302; i++) {
      outputList[i] = nextOutputList[i];
      id = 0;
    }
}

/*************************************HELPER FUNCTIONS***************************************/
/**
 * Function to draw the contents of the input screen
 */
void drawInputs(uint8_t scroll, char option1[], char option2[], char option3[], char option4[]) {
  arduboy.clear();                              //scroll 0

  if (scroll == 0 || scroll == 1 || scroll == 2) {
    arduboy.setCursor(10,3);
    arduboy.print("Select Input Type: ");
    arduboy.setCursor(10,23);
    arduboy.print("- ");
    arduboy.print(option1);
    arduboy.setCursor(10,33);      
    arduboy.print("- ");
    arduboy.print(option2);
    arduboy.setCursor(10,43);
    arduboy.print("- ");
    arduboy.print(option3);
    arduboy.setCursor(10,53);      
    arduboy.print("- ");
    arduboy.print(option4);
  } else if (scroll == 3) {            //if its the last of the input screens just print the one option
    arduboy.setCursor(10,3);
    arduboy.print("Select Input Type: ");
    arduboy.setCursor(10,23);
    arduboy.print("- ");
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
    if (isTonic) {                      //if its a tonic selection draw a pixel to the right of the indicator
      arduboy.drawPixel(6, 26, WHITE);
    } else {                            //if its a phasic selection draw a pixel to the left
      arduboy.drawPixel(3, 23, WHITE);
    }
    arduboy.drawPixel(2, 26, WHITE);    //draw the rest of the pixel
    arduboy.drawPixel(3, 26, WHITE);
    arduboy.drawPixel(4, 26, WHITE);
    arduboy.drawPixel(3, 25, WHITE);
    arduboy.drawPixel(3, 27, WHITE);
    arduboy.display();
  } else if (posCount == 1 || posCount == 5 || posCount == 9) {
    if (isTonic) {
      arduboy.drawPixel(6, 36, WHITE);
    } else {
      arduboy.drawPixel(3, 33, WHITE);
    }
    arduboy.drawPixel(2, 36, WHITE);
    arduboy.drawPixel(3, 36, WHITE);
    arduboy.drawPixel(4, 36, WHITE);
    arduboy.drawPixel(3, 35, WHITE);
    arduboy.drawPixel(3, 37, WHITE);
    arduboy.display();
  } else if (posCount == 2 || posCount == 6 || posCount == 10) {
    if (isTonic) {
      arduboy.drawPixel(6, 46, WHITE);
    } else {
      arduboy.drawPixel(3, 43, WHITE);
    }
    arduboy.drawPixel(2, 46, WHITE);
    arduboy.drawPixel(3, 46, WHITE);
    arduboy.drawPixel(4, 46, WHITE);
    arduboy.drawPixel(3, 45, WHITE);
    arduboy.drawPixel(3, 47, WHITE);
    arduboy.display();
  } else if (posCount == 3 || posCount == 7 || posCount == 11) {
    if (isTonic) {
      arduboy.drawPixel(6, 56, WHITE);
    } else {
      arduboy.drawPixel(3, 53, WHITE);
    }
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
 * A small function to draw out the muscle ratio information to the screen.
 * Used by the UI to make information more readable for the user.
 */
void printMovementDir(uint16_t xpos, uint16_t ypos) {
  if (vaRatio + daRatio > vbRatio + dbRatio) {
    arduboy.setCursor(xpos, ypos);
    arduboy.print("BACKWARD-");
  } else {
    arduboy.setCursor(xpos, ypos);      
    arduboy.print("FORWARD-");
  }
  
  if (vaRatio + vbRatio > daRatio + dbRatio) {
    arduboy.print("L/VENTRAL");
  } else {
    arduboy.print("R/DORSAL");
  }
}
