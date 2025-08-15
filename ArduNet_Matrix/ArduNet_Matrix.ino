/**
* For *both versions*, add gap junctions variants to neural rom sets 

* For *both versions*, compress neural rom, add reader functions

* For *both versions*, implement the hebbian functions and learning arrays

* UPDATE matrix with stuff from the sim file

* For *Matrix Ver.*, add quick scrolling for the matrix screen and diag screen neuron selections

* For *Matrix Ver.*, initialize the direction the worm is in as "stationary" so it doesn't start in "forward"

* For *Matrix Ver.*, make neuron IDs associated with their irl names so you can see them in diagnostic and not have to look at this huge github list lmao

* For *Matrix Ver.*, make a more complex input screen-- scrolling list of senses, ability to choose tonic or phasic input type

* For *Matrix Ver.*, if a synapse is a gap junction indicate this on the diag screen

* For *Matrix Ver.*, add a worm movement indicator sprite 

* For *Matrix Ver.*, add an actually accurate percpetron model in diag screen

* For *Matrix Ver.*, add the cute worm expressions to the matrix screen

* For *Matrix Ver.*, add indicators for X-Y position on side of matrix to show the selected current neuron

* For *Matrix Ver.*, add an options screen by pressing down; options to save data, edit learning array, output save data over serial, output all data over serial, connect input over serial

* For *Matrix Ver.*, For options screen serial output, add a pseudo-terminal that writes text to the arduboy as well
 */

#include "sprites.h"              //import libraries
#include "neuralROM.h"
#include "bit_array.h"
#include "sized_int_array_reader.h"
#include <Arduboy2.h>
#include <stdlib.h>


Arduboy2 arduboy;                 //create arduboy object

const uint8_t threshold = 1;      //threshold for activation function
const uint16_t maxSynapse = 65;
uint16_t currentID = 0;           //interface variable to indicate which neuron is being analyzed
uint16_t numSynapses = 0;         //interface variable to indicate number of synapses connected
uint16_t preID = 0;               //interface variable to indicate which neuron is being analyzed
uint8_t sense = 0;                //interface variable to indicate which sense is activated
uint8_t posCount = 0;             //interface variable to indicate which sense is being looked at
uint8_t lastScreen = 0;           //interface variable to indicate last screen before button press
uint8_t buttonHold = 0;            //interface variable to hold number of iterations button is held
int8_t synWeight = 0;             //interface variable to indicate the weight of a given synapse
bool startFlag = true;            //interface flag for title screen to play
float vaRatio = 0;                //muscle ratios for interface printout
float vbRatio = 0;
float daRatio = 0;
float dbRatio = 0;
uint8_t counter = 0;


SizedIntArrayReader<9> NEURAL_ROM(COMPRESSED_NEURAL_ROM, 7578, -70, true);

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

  arduboy.pollButtons();        //get buttons pressed and send to function to process them
  doButtons();

  activationFunction();         //do the main calculation of the connectome
  delay(50);

  //go to the proper screens based on the direction button last clicked
  if (arduboy.justPressed(UP_BUTTON) || startFlag) doMatrixScreen();
  if (arduboy.justPressed(LEFT_BUTTON)) doDiagnosticScreen();
  if (arduboy.justPressed(RIGHT_BUTTON)) doInputScreen();
  if (arduboy.justPressed(DOWN_BUTTON)) doSaveScreen();
  arduboy.display();
  
  startFlag = false;            //set the flag off; only do the title screen once
}

/**
 * function to run the diagnostic screen; a screen that shows details of synapses
 * and allows the user to make changes
 */
void doDiagnosticScreen() {
      lastScreen = 3;

      arduboy.clear();    
      arduboy.setCursor(10, 10);
      arduboy.print(currentID);
      arduboy.print(" Syn: ");
      arduboy.print(numSynapses); 
      arduboy.setCursor(10, 20);
      arduboy.print("Weight: ");
      arduboy.print(synWeight);
      arduboy.setCursor(10, 30);
      arduboy.print("Pre ID: ");
      arduboy.print(preID);
      arduboy.setCursor(10, 40);
      arduboy.print("postID: ");
      arduboy.print(currentID);

//TODO: print the actual neuron name above the sprite
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
      
      arduboy.clear();
      arduboy.setCursor(10,0);
      arduboy.print("Select Input Type: ");
      arduboy.setCursor(10,20);
      arduboy.print("- Gentle Nose Touch");
      arduboy.setCursor(10,30);
      arduboy.print("- Aerotaxis");
      arduboy.setCursor(10,40);
      arduboy.print("- Chemoattraction");
      arduboy.setCursor(10,50);
      arduboy.print("- Phototaxis");

        if (posCount == 0) {
          arduboy.drawPixel(0, 23, WHITE);
          arduboy.drawPixel(1, 23, WHITE);
          arduboy.drawPixel(2, 23, WHITE);
          arduboy.drawPixel(1, 22, WHITE);
          arduboy.drawPixel(1, 24, WHITE);
          arduboy.display();
        } else if (posCount == 1) {
          arduboy.drawPixel(0, 33, WHITE);
          arduboy.drawPixel(1, 33, WHITE);
          arduboy.drawPixel(2, 33, WHITE);
          arduboy.drawPixel(1, 32, WHITE);
          arduboy.drawPixel(1, 34, WHITE);
          arduboy.display();
        } else if (posCount == 2) {
          arduboy.drawPixel(0, 43, WHITE);
          arduboy.drawPixel(1, 43, WHITE);
          arduboy.drawPixel(2, 43, WHITE);
          arduboy.drawPixel(1, 42, WHITE);
          arduboy.drawPixel(1, 44, WHITE);
          arduboy.display();          
        } else if (posCount == 3) {
          arduboy.drawPixel(0, 53, WHITE);
          arduboy.drawPixel(1, 53, WHITE);
          arduboy.drawPixel(2, 53, WHITE);
          arduboy.drawPixel(1, 52, WHITE);
          arduboy.drawPixel(1, 54, WHITE);
          arduboy.display();       
        }
}

/**
 * function to query which buttons are pressed, setting up proper screen transitions
 */
void doButtons() {
  //if last screen was the matrix screen
  if (lastScreen == 1) {
    if (arduboy.justPressed(A_BUTTON) && arduboy.justPressed(B_BUTTON)) {
      if (sense == 0) doGentleNoseTouch(); 
      if (sense == 1) doOxygenSensation();
      if (sense == 2) doChemoattraction();
      if (sense == 3) doPhotosensation();
    }

//TODO: implement quick scrolling for A
    if (arduboy.justPressed(A_BUTTON)) {
      if (currentID < 302 - 1) {
        currentID++;
      } else {
        currentID = 0;
      }
    }

//TODO: implement decrement for B
    if (arduboy.justPressed(B_BUTTON)) {
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

  //if last screen was the serial output screen
  if (lastScreen == 2) {
    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
      Serial.begin(9600);
//TODO: print eeprom data over serial out
      //PRINT SERIAL HERE
      arduboy.drawRect(10, 30, 54, 5);
      //arduboy.drawPixel(11 + , 30, WHITE);
      //arduboy.drawPixel(11 + , 30, WHITE);
      //arduboy.drawPixel(11 + , 30, WHITE);
      Serial.end();

//TODO: print out on screen that the data is now done being sent
      delay(5000);
    }
  }

  //if last screen was the diagnostic screen
  if (lastScreen == 3) {
    if (arduboy.justPressed(A_BUTTON)) {
      if (counter < numSynapses - 1) {
        counter++;
      } else {
        counter = 0;
      }
      //synWeight = 0;
      preID = preSynapticNeuronList[counter];
    }

    if (arduboy.justPressed(B_BUTTON)) {
      if (counter > 0) {
        counter--;   
      } else {
        counter = n.inputLen;
      }
      //synWeight = 0;
      preID = preSynapticNeuronList[counter];
    }

    doDiagnosticScreen();
  }
  
  //if last screen was the input select screen
  if (lastScreen == 4) {
    if (arduboy.justPressed(A_BUTTON)) {
       posCount++;
       if (posCount > 3) posCount = 0;
  
       if (posCount == 0) {
         sense = 0;
       }
            
       if (posCount == 1) {
         sense = 1;
       }
          
       if (posCount == 2) {
         sense = 2;
       }  

       if (posCount == 3) {
         sense = 3;
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
    //clear the screen then write app name
    arduboy.clear();
    arduboy.setCursor(15, 10);
    arduboy.print(F("-ArduNet Elegans-"));
    arduboy.setCursor(10, 50);
    arduboy.display();
    delay(5000);  
  }
}

/**
 * A function that displays the main "matrix" screen to the user, allows them to activate
 * a chosen sensory modality, and to view network activation patterns in the matrix
 */
void doMatrixScreen() {      
  lastScreen = 1;
  
  arduboy.clear();
  arduboy.setCursor(5, 5);
  arduboy.print("NEURAL GRID: ");
  arduboy.setCursor(5, 40);
  arduboy.print("CURRENT CELL ID: ");
  arduboy.print(currentID);
  printMovementDir(5, 50);
        
  uint8_t gridWidth = 17;
  uint8_t gridHeight = 18;
  uint16_t xPos = 80;
  uint16_t yPos = 1;
        
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
void doSaveScreen() {
  lastScreen = 2;

  arduboy.clear();
  arduboy.drawRoundRect(0, 0, 128, 64, 3);
  arduboy.setCursor(5, 5);
  arduboy.print("Save Data? (A) / (B)");
  //arduboy.setCursor(5, 25);
  //arduboy.print("Save --> Serial");
}
/******************************************SENSES********************************************/
void doGentleNoseTouch() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 153 || id == 154 || id == 155 || id == 156 ||
                id == 125 || id == 126 || id == 127 || id == 128 || 
                id == 129 || id == 130 || id == 113 || id == 114) {
                outputList[id] = true;
            }
        }
}       //OLQ, IL1, FLP

void doHarshNoseTouch() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 113 || id == 114 || id == 43 || id == 44) {
                outputList[id] = true;
            }
        }
}       //FLP, ASH

void doTextureSense() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 84 || id == 85 || id == 86 || id == 87 || 
                id == 2 || id == 3 || id == 159 || id == 160) {
                outputList[id] = true;
            }
        }
}       //CEP, ADE, PDE

void doChemoattraction() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 39 || id == 40 || id == 4 || id == 5 || 
                id == 41 || id == 42 || id == 45 || id == 46 || 
                id == 47 || id == 48 || id == 49 || id == 50) {
                outputList[id] = true;
            }
        }
}       //ASE (primary), ADF, ASG, ASI, ASJ, ASK

void doChemorepulsion() {
    for (uint16_t id = 0; id < 302; id++) {
            if (id == 43 || id == 44 || id == 6 || id == 7 || 
                id == 49 || id == 50 || id == 39 || id == 40) {
                outputList[id] = true;
            }
        }
}       //ASH (primary), ADL, ASK, ASE

void doCoolingResponse() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 76 || id == 77) {
                outputList[id] = true;
            }
        }
}       //AWC

void doHeatingResponse() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 8 || id == 9) {
                outputList[id] = true;
            }
        }
}       //AFD

void doNoxiousHeatResponse() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 8 || id == 9 || id == 113 || id == 114 || 
                id == 165 || id == 166) {
                outputList[id] = true;
            }
        }
}       //AFD (head), FLP (head), and PHC (tail)

void doNoxiousColdResponse() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 174 || id == 175) {
                outputList[id] = true;
            }
        }
}       //PVD

void doPhotosensation() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 63) {
                outputList[id] = true;
            }
        }
}       //Educated guess based on protein presence on neurons: AVG

void doOxygenSensation() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 27 || id == 171 || id == 254 || id == 255 ||
                id == 43 || id == 44) {
                outputList[id] = true;
            }
        }
}       //AQR, PQR, URX, ASH (very likely based on protein presence); senses O2 increase

void doCO2Sensation() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 8 || id == 9 || id == 78 || id == 79 || 
                id == 39 || id == 40) {
                outputList[id] = true;
            }
        }
}       //AFD, BAG, and ASE; senses O2 decrease, CO2 increase

/*************************************SIMULATION FUNCTIONS***************************************/
/**
 * The function that reads in the neural rom into a format that is able to be read 
 * by the activation function and rest of the program
 */
void matrixToNeuron(uint16_t cellID) {
  uint16_t index = 0;

  // Skip to the correct neuron's data
  for (uint16_t i = 0; i < cellID; i++) {
    int16_t skipLen = NEURAL_ROM[index];     //read the value of the first neuron's input Len
    index += 1 + skipLen + skipLen;       //add double that value plus one, to skip the entire neuron entry
  }

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
}

/**
 *The activation function is the main simulation, it calculates all the next ticks of the connectome
 * and then sets the next tick to the current one when each has been individually calculated
 */
void activationFunction() {  
  //calculate next output for all neurons using the current output list
  for (uint16_t i = 0; i < 302; i++) {
    matrixToNeuron(i);      //fill the neuron struct with the information of the ith neuron
    int32_t sum = 0;

    for (uint8_t j = 0; j < n.inputLen; j++) {        //loop over every presynaptic neuron
      if (i == currentID) {
        numSynapses = n.inputLen;   //store some more info about these synapses
        preSynapticNeuronList[j] = n.inputs[j];
        
        if (n.inputs[j] == preID) {
          synWeight = n.weights[j];
        }
      }

      sum += n.weights[j] * outputList[n.inputs[j]];  //do the summation calculation on the neuron
      //sum += learningArray[i][j];                     //add hebbian learning and LTD
    }

    if (sum >= threshold) {                           //note it as being true or false
      nextOutputList[i] = true;

/*      for (uint8_t j = 0; j < n.inputLen; j++) {
        if (learningArray[i][j] < 0) {                //hebbian learning
          learningArray[i][j] = 0;
        } else if (learningArray[i][j] < 10) {
          learningArray[i][j]++;
        }
      }*/
    } else {
      nextOutputList[i] = false;
      
/*      for (uint8_t j = 0; j < n.inputLen; j++) {
        if (learningArray[i][j] > 0) {                //hebbian learning
          learningArray[i][j] = 0;
        } else if (learningArray[i][j] > -10) {
          learningArray[i][j]--;
        }
      }*/
    }
  }

  for (int16_t i = 0; i < 302; i++) {         //flush buffer
    outputList[i] = nextOutputList[i];
  }
}

/*************************************HELPER FUNCTIONS***************************************/
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
