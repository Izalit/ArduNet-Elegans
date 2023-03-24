//TODO: fix the matrixToNeuron() function to properly parse the rom into structs
//TODO: what causes occasional reboots? Memory leak? using too much data? something else?
//TODO: to save space in neural rom, combine the weights of same pre-post connections into one single weight?
//TODO: implement button fast scrolling for large lists of neurons; if you hold for multiple iterations it increments by 10?
//TODO: save outputList[] into eeprom?


#include <Arduboy2.h>
#include "neuralROM.h"
#include "neurons.h"
#include "sprites.h"

Arduboy2 arduboy; //make an arduboy object
constexpr uint8_t frameRate = 25; // Frame rate in frames per second
const uint16_t neuronCount = 10;  //number of neurons in network
bool startFlag = true;
uint8_t sense = 0;
uint16_t currentID = 0;
uint16_t postID = 0;
uint8_t posCount = 0;
uint8_t lastScreen = 0;
bool outputList[neuronCount];
float vaRatio = 0;
float vbRatio = 0;
float daRatio = 0;
float dbRatio = 0;
  
//Struct defined to hold all elements of a neuron
struct neuron {
  int16_t cellID;
  int16_t inputLen;
  int16_t inputs[];
  int16_t weights[];
};

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(frameRate);
  arduboy.clear();
}

void loop() {
  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame())) return;

  //poll the buttons so it can detect last clicked buttons
  arduboy.pollButtons();
  doButtons();
  
  //show title screen
  doTitleScreen();
    
  //calculate next tick of connectome
  activationFunction();

  //go to the proper screens based on the direction button last clicked
  if (arduboy.justPressed(UP_BUTTON) || startFlag) doMatrixScreen();
  //if (arduboy.justPressed(LEFT_BUTTON))  doOutputScreen();
  if (arduboy.justPressed(DOWN_BUTTON)) doDiagnosticScreen();
  //if (arduboy.justPressed(RIGHT_BUTTON)) doInputScreen();

  //flag to not display the title screen again
  startFlag = false;
}

void doTitleScreen() {
  if (startFlag) {
    //clear the screen then write app name
    arduboy.clear();
    arduboy.setCursor(15, 10);
    arduboy.print("-ArduNet Elegans-");
    arduboy.setCursor(10, 50);
    arduboy.display();
    delay(2000);  
  }
}

void doOutputScreen() {
  //if pressed left draw output screen
      lastScreen = 2;
      
      //draw graphic of worm for cell output
      //draw grid of interneurons
      //draw muscle ratios
      //draw direction of movement
    
      arduboy.clear();    
      arduboy.setCursor(0, 0);
  
     //print the motor ratios for the motor cells
      uint8_t VAcount = 12;
      uint8_t VBcount = 11;
      uint8_t DAcount = 9;
      uint8_t DBcount = 7;
      uint8_t VAsum = 0;
      uint8_t VBsum = 0;
      uint8_t DAsum = 0;
      uint8_t DBsum = 0;

      //backward ventral locomotion motor neurons
      const uint16_t motorNeuronAVentral[VAcount] = {
        VA1, VA2, VA3, VA4, VA5, VA6, VA7, VA8, VA9, VA10, VA11, VA12
      };
  
      //forward ventral locomotion motor neurons
      const uint16_t motorNeuronBVentral[VBcount] = {
        VB1, VB2, VB3, VB4, VB5, VB6, VB7, VB8, VB9, VB10, VB11
      };
  
      //backward dorsal locomotion motor neurons
      const uint16_t motorNeuronADorsal[DAcount] = {
        DA1, DA2, DA3, DA4, DA5, DA6, DA7, DA8, DA9
      };
  
      //forward dorsal locomotion motor neurons
      const uint16_t motorNeuronBDorsal[DBcount] = {
        DB1, DB2, DB3, DB4, DB5, DB6, DB7
      };
  
  
      for (uint16_t i = 0; i < VAcount; i++) {
          if (outputList[i]) {
              VAsum++;
          }
      }
  
      for (uint16_t i = 0; i < VBcount; i++) {
          if (outputList[i]) {
              VBsum++;
          }
      }
  
      for (uint16_t i = 0; i < DAcount; i++) {
          if (outputList[i]) {
              DAsum++;
          }
      }
  
      for (uint16_t i = 0; i < DBcount; i++) {
          if (outputList[i]) {
              DBsum++;
          }
      }
  
  
      vaRatio = float(VAsum)/float(VAcount);
      vbRatio = float(VBsum)/float(VBcount);
      daRatio = float(DAsum)/float(DAcount);
      dbRatio = float(DBsum)/float(DBcount);
  
      arduboy.setCursor(10, 20);
      arduboy.print("VA Ratio: ");
      arduboy.print(vaRatio);
      arduboy.setCursor(10, 30);    
      arduboy.print("VB Ratio: ");
      arduboy.print(vbRatio);
      arduboy.setCursor(10, 40);
      arduboy.print("DA Ratio: ");
      arduboy.print(daRatio);
      arduboy.setCursor(10, 50);
      arduboy.print("DB Ratio: ");
      arduboy.print(dbRatio);
      
      printMovementDir(5, 10);
            
    arduboy.display();
}

void doDiagnosticScreen() {
  //if pressed down draw cell diagnostics screen
      lastScreen = 3;
      
      uint16_t preID = currentID;
    
      arduboy.clear();    
      arduboy.setCursor(10, 10);
      arduboy.print("# Syn: " + numSynapses(currentID)); 
      arduboy.setCursor(10, 20);
      arduboy.print("W: ");
      arduboy.print(outputWeight(preID, postID));
      arduboy.setCursor(10, 30);
      arduboy.print("Output: ");
      arduboy.print(outputList[preID]);  
      arduboy.setCursor(10, 40);
      arduboy.print("Pre ID: ");
      arduboy.print(preID);
      arduboy.setCursor(10, 50);
      arduboy.print("postID: ");
      arduboy.print(postID);
      arduboy.display();

      if (outputList[preID]) Sprites::drawOverwrite(85, 30, perceptronON, 0);
      if (!outputList[preID]) Sprites::drawOverwrite(85, 30, perceptronOFF, 0);


}

void doInputScreen() {
  //if pressed right draw inputs screen
      lastScreen = 4;
      
      arduboy.clear();
      arduboy.setCursor(10,0);
      arduboy.print("Select Input Type: ");
      arduboy.setCursor(10,20);
      arduboy.print("-Nose Touch");
      arduboy.setCursor(10,30);
      arduboy.print("-Chemorepulsion");
      arduboy.setCursor(10,40);
      arduboy.print("-Chemoattraction");

        if (posCount == 0) {
          arduboy.drawPixel(0, 33, WHITE);
          arduboy.drawPixel(1, 33, WHITE);
          arduboy.drawPixel(2, 33, WHITE);
          arduboy.drawPixel(1, 32, WHITE);
          arduboy.drawPixel(1, 34, WHITE);
          arduboy.display();
        } else if (posCount == 1) {
          arduboy.drawPixel(0, 43, WHITE);
          arduboy.drawPixel(1, 43, WHITE);
          arduboy.drawPixel(2, 43, WHITE);
          arduboy.drawPixel(1, 42, WHITE);
          arduboy.drawPixel(1, 44, WHITE);
          arduboy.display();
        } else if (posCount == 2) {
          arduboy.drawPixel(0, 23, WHITE);
          arduboy.drawPixel(1, 23, WHITE);
          arduboy.drawPixel(2, 23, WHITE);
          arduboy.drawPixel(1, 22, WHITE);
          arduboy.drawPixel(1, 24, WHITE);
          arduboy.display();
        }
}

void doMatrixScreen() {
    //if press up draw out grid screen
        lastScreen = 1;
        
        arduboy.clear();
        arduboy.setCursor(5, 10);
        arduboy.print("NEURAL GRID: ");
        arduboy.setCursor(5, 40);
        arduboy.print("CURRENT CELL ID: ");
        arduboy.print(currentID);
        printMovementDir(5, 50);
        
        uint8_t gridWidth = 17;
        uint8_t gridHeight = 18;
        uint16_t xPos = 100;
        uint16_t yPos = 10;
        
        //draw border on grid of cells
        arduboy.drawRect(xPos, yPos, gridWidth + 1, gridHeight + 1);

        int neuronCounter = 0;
      
        //draw grid of cells, blacking out current cell ID
        for (uint16_t x = 1; x <= gridWidth; x++) {
          for (uint16_t y = 1; y <= gridHeight; y++) {
            //for each neuron get the output state and draw a pixel accordingly               
            if (neuronCounter < neuronCount) {
              if (outputList[neuronCount]) {
                neuronCounter++;
                arduboy.drawPixel(x + xPos, y + yPos, WHITE);        
              }
            }
          }
        }
        arduboy.display();
}

void doButtons() {
  if (lastScreen == 1) {
    if (arduboy.justPressed(A_BUTTON) && arduboy.justPressed(B_BUTTON)) {
      if (sense == 0) doNoseTouch(); 
      if (sense == 1) doChemorepulsion();
      if (sense == 2) doChemoattraction();
    }

    if (arduboy.justPressed(A_BUTTON)) {
      if (currentID < neuronCount - 1) {
        currentID++;
      } else {
        currentID = 0;
      }
    }


    if (arduboy.justPressed(B_BUTTON)) {
      if (currentID > 0) {
        currentID--;
      } else {
        currentID = neuronCount - 1;
      }
    }

    doMatrixScreen();
  }

  if (lastScreen == 2) {
     //do nothing! doesnt use buttons for output screen
  }

  if (lastScreen == 3) {
    if (arduboy.justPressed(A_BUTTON)) {
      if (postID < neuronCount - 1) {
        postID++;
      } else {
        postID = 0;
      }
    }
    if (arduboy.justPressed(B_BUTTON)) {
      if (postID > 0) {
        postID--;   
      } else {
        postID = neuronCount - 1;
      }
    }

    doDiagnosticScreen();
  }
  
  if (lastScreen == 4) {    
    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
       posCount++;
       if (posCount > 2) posCount = 0;
  
       if (posCount == 0) {
         sense = 0;
       }
            
       if (posCount == 1) {
         sense = 1;
       }
          
       if (posCount == 2) {
         sense = 2;
       }            
    }

    doInputScreen();
  }  
  
  arduboy.display();
}


/******************************************SENSES********************************************/
/**
 * Do nose touch sense
 */
void doNoseTouch() {
    for (uint16_t j = 0; j < neuronCount; j++) {
            uint16_t id = matrixToNeuron(j).cellID;
            //if nosetouch neuron is in cellular matrix then set output to true
            if (id == FLPL || id == FLPR) {
                outputList[j] = true;
            }
        }
}

/**
 * Do chemorepulsive sense
 */
void doChemorepulsion() {
    for (uint16_t j = 0; j < neuronCount; j++) {
            uint16_t id = matrixToNeuron(j).cellID;
            //if nosetouch neuron is in cellular matrix then set output to true
            if (id == PHAL || id == PHAR || id == PHBL || id == PHBR) {
                outputList[j] = true;
            }
        }
}

/**
 * Do chemoattractive sense
 */
void doChemoattraction() {
    for (uint16_t j = 0; j < neuronCount; j++) {
            uint16_t id = matrixToNeuron(j).cellID;
            //if nosetouch neuron is in cellular matrix then set output to true
            if (id == ASEL || id == ASER) {
                outputList[j] = true;
            }
        }
}


/*******************************HELPER FUNCTIONS************************************/
/**
 * function to convert the given neuron data from the matrix into a neuron struct object
 */
neuron matrixToNeuron(int cellID) {
  neuron n;
  const uint16_t maxSynapse = 65;
  const uint16_t matrixSize = 7578;

  n.cellID = cellID;
  n.inputLen = 0;
  n.inputs[maxSynapse] = {};
  n.weights[maxSynapse] = {};

  uint16_t counter = 0;
  uint16_t i = 0;

  for (i; i < matrixSize; i++) {
    n.inputLen = NEURAL_ROM[i];
    
    for (uint16_t j = 0; j < n.inputLen; j++) {
      n.inputs[j] = NEURAL_ROM[i + j + 1];
    }

    i += n.inputLen;

    for (uint16_t k = 0; k < n.inputLen; k++) {
      n.weights[k] = NEURAL_ROM[i + k + 1];
    }

    i += n.inputLen;
    
    if (counter == cellID) {
      return n;
    } else {
      counter++;
    }
  }
  return n;
}

/**
 * Function to get the number of synapses to a postsynaptic cell
 */
int numSynapses(uint16_t cellID) {
  neuron n = matrixToNeuron(cellID);
  return n.inputLen;
}

/**
 * Function to get the output value of a given neuron
 */
bool outputBool(uint16_t cellID) {
  outputList[cellID];
}

/**
 * Function to get the output weight of a given pre synaptic connection to a post-synaptic neuron
 */
float outputWeight(uint16_t preID, uint16_t postID) {
  neuron pre = matrixToNeuron(preID);
  neuron post = matrixToNeuron(postID);

  bool containsPost = false;
  
  for (uint16_t i = 0; i < pre.inputLen; i++) {
    if (pre.inputs[i] == postID) {
      containsPost = true; 
    }
    
    if (containsPost) {
      return pre.weights[i];
    }
  }

  return 0;
}


/**
 * Function to calculate next tick of hexToNeuron outputs
 */
void activationFunction() {
  const uint8_t threshold = 1;
  
  for (uint16_t i = 0; i < neuronCount; i++) {
    float productMatrix[matrixToNeuron(i).inputLen];
    int sum = 0;

    for (uint16_t j = 0; j < matrixToNeuron(i).inputLen; j++) {
       productMatrix[j] = matrixToNeuron(i).weights[j] * matrixToNeuron(i).inputs[j];
       sum += productMatrix[j];
    }

    if (sum >= threshold) {
      outputList[i] = true;
    } else {
      outputList[i] = false;
    }
  }
}

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
