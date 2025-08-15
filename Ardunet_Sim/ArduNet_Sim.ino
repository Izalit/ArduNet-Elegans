/**
* fix the compressed ROM memory issue
* implement the learning array
* add gap junctions to the neural rom (with mollys help!)
* re-compress neural rom with gap junction code

* remake doProprioception() to look at individual motor neurons in if-statements
*     For turns: if more left side muscles are active than right, or right side muscles than left, doProprioception()
*     For movement: if AVB and PVC or AVD, AVA, and AVE are active, then doProprioception()
* add collision logic for the rest of the rocks, leaves, and stick sprites
* implement "petting" when "bond" is good (check memory association between certain neurons in the learning array to make a "bond" variable)

* add numbers to the simulation demarcations to show gas level and temperature readouts, indicate both goldilocks numbers and nociceptive numbers
* create a cycle of where the leaves fall from the tree, blink after so many ticks, then disappear and drop food; when it is eaten the cycle repeats
* make the worm's movement more biologically accurate (remove the "tank-drive" abstraction from current code), make doProprioception() more bio-accurate
* create randomizers for toxin spawn position and sun/moon (if moon: black out some sun pixels, remove noxious surface heat and replace with cold, remove phototaxis)
*/

/*
    secondary list 1: sensory neurons                     (29)
PHBL, PHBR, OLQDL, OLQDR, OLQVL, OLQVR, IL1DL, IL1DR, IL1L, IL1R, IL1VL, 
IL1VR, ADLL, ADLR, CEPDL, CEPDR, CEPVL, CEPVR, ADEL, ADER, ASGL, ASGR, 
ASJL, ASJR, ASKL, ASKR, AVG, BAGL, BAGR,

    secondary list 2: behavior-specific neurons           (36)
FLPL, FLPR, PHAL, PHAR, PHCL, PHCR, PVM, SDQL, SDQR, HSNL, HSNR, SAADL, 
SAADR, SAAVL, SAAVR, VB1, VB2, VB3, VB4, VB5, VB6, VB7, VB8, VB9, VB10, 
VB11, AVBL, AVBR, PVCL, PVCR, AVEL, AVER, AWAL, AWAR, RIBL, RIBR

    primary list: sensory-interneurons and interneurons   (60)
SIADL, SIADR, SIAVL, SIAVR, AIYL, AIYR, ASIL, ASIR, ASEL, ASER, RIML, RIMR,
PDEL, PDER, AVKL, AVKR, DVA, AWCL, AWCR, RIAL, RIAR, AVAL, AVAR, AVDL, AVDR,
ALML, ALMR, AVM, PLML, PLMR, PVDL, PVDR, ASHL, ASHR, ADFL, ADFR, AIAL, AIAR, 
NSML, NSMR, AFDL, AFDR, AIZL, AIZR, AQR, PQR, URXL, URXR, AWBL, AWBR, AIBL, 
AIBR, SMDDL, SMDDR, SMDVL, SMDVR, OLLL, OLLR, RICL, RICR

-try to use the secondary, and the primary first...        total learning neurons: 96
-potentially just sensory neurons and the primary list...  total learning neurons: 89
-otherwise, if possible all three...                       total learning neurons: 125


Sources for Memory Neurons:
    Wormatlas
    https://www.frontiersin.org/journals/physiology/articles/10.3389/fphys.2013.00088/full
    https://www.eneuro.org/content/6/4/ENEURO.0014-19.2019
    https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0006019
    https://www.eneuro.org/content/9/4/ENEURO.0084-22.2022

sources for satiety circuit
    https://pmc.ncbi.nlm.nih.gov/articles/PMC3726252/

sources for definitional help
    https://www.biorxiv.org/content/10.1101/2025.07.21.665845v1.full

Credit to Nategri for the motor neuron summation idea for tank drive
*/


#include "sprites.h"              //import libraries
#include "neuralROM.h"
#include "bit_array.h"
#include "sized_int_array_reader.h"
#include <Arduboy2.h>

Arduboy2 arduboy;                 //create arduboy object

const uint8_t threshold = 1;      //threshold for activation function
const uint16_t maxSynapse = 65;   //maximum number of synapses a neuron can have
uint16_t tick = 0;                //connectome ticks
uint16_t autosaveTimer = 1000;    //the number to modulo the tick by to activate autosave
bool startFlag = true;            //interface flag for title screen to play
float vaRatio = 0;                //muscle ratios for interface printout
float vbRatio = 0;
float daRatio = 0;
float dbRatio = 0;
uint8_t foodEaten = 0;            //how much food has been eaten
uint8_t cursorX = 64;     //cursor
uint8_t cursorY = 27;     //cursor
uint8_t wormX = 64;       //worm
uint8_t wormY = 40;       //worm
uint8_t wormFacing = 0;   //the direction the worm is facing
uint8_t prevWormDir = 0;  //the previous direction of the worm
uint8_t foodX = 99;       //food
uint8_t foodY = 30;       //food
uint8_t obstacleX1 = 5;   //rock
uint8_t obstacleY1 = 49;  //rock
uint8_t obstacleX2 = 40;  //stick
uint8_t obstacleY2 = 13;  //stick
uint8_t obstacleX3 = 105; //rock
uint8_t obstacleY3 = 45;  //rock
uint8_t obstacleX4 = 114; //leaf
uint8_t obstacleY4 = 11;  //leaf
uint8_t obstacleX5 = 75;  //leaf
uint8_t obstacleY5 = 11;  //leaf
uint8_t repellentX = 34;  //repellent
uint8_t repellentY = 52;  //repellent
uint8_t fullTick = 0;
bool isRepel = true;      //if theres any repellents
bool isFood = true;       //if theres any food
bool isAsleep = false;    //if sleep state is active
bool sated = false;       //if the worm is not hungry
//uint16_t foodTouchCounter = 0;  //how long the food has been eaten for

//massive thanks to Dinokaiz2 for help with the bit array functionality!!!
SizedIntArrayReader<9> NEURAL_ROM(COMPRESSED_NEURAL_ROM, 7578, -70, true);

//BitArray<> learningArray;  //an array that, for each neuron that does hebbian learning, holds a form of simplified output history
BitArray<302> outputList;     //list of neurons
BitArray<302> nextOutputList; //buffer to solve conflicting time differentials in firing

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

  simulation();

  arduboy.display();
  
  startFlag = false;            //set the flag off; only do the title screen once
}

/**
 * function to query which buttons are pressed, setting up proper screen transitions
 */
void doButtons() {
  uint8_t address = EEPROM_STORAGE_SPACE_START;      //eeprom address for save function

  //if last screen was the simulation
    if (arduboy.justPressed(A_BUTTON)) {
      foodX = cursorX;
      foodY = cursorY;
      isFood = true;
    }

    if (arduboy.justPressed(B_BUTTON)) {
      if (abs(cursorX - repellentX) < 10 && abs(cursorY - repellentY) < 10) {
        Sprites::drawErase(repellentX, repellentY, toxin, 0);
        isRepel = false;
      }
    }

    if (!isAsleep && (arduboy.justPressed(A_BUTTON) && arduboy.justPressed(B_BUTTON))) {
      activateSleep();
      isAsleep = true;
//TODO: uncomment when learningArray is implemented, fix address as well
//      for (uint8_t i = 0; i < learningArraySize; i++) {
//        EEPROM.update(address + i, learningArray[i]);
//      }
    } else if (isAsleep && (arduboy.justPressed(A_BUTTON) && arduboy.justPressed(B_BUTTON))) {
      isAsleep = false;
    }

    if (cursorY >= 5 && arduboy.justPressed(UP_BUTTON)) {
      //if (!(cursorY-5 >= 49 && cursorY-5 <= 64 && cursorY-5 >= 45 && cursorY-5 <= 64)) {
        cursorY -= 5;
      //}
    }

    if (cursorX >= 5 && arduboy.justPressed(LEFT_BUTTON)) {
      //if (!(cursorX-5 <= 5 && cursorX-5 >= 20 && cursorX-5 <= 105 && cursorX-5 >= 123)) {
        cursorX -= 5;
      //}
    }

    if (cursorX <= 123 && arduboy.justPressed(RIGHT_BUTTON)) {
      //if (!(cursorX+5 <= 5 && cursorX+5 >= 20 && cursorX+5 <= 105 && cursorX+5 >= 123)) {
        cursorX += 5;
      //}
    }

    if (cursorY <= 59 && arduboy.justPressed(DOWN_BUTTON)) {
      //if (!(cursorY+5 >= 49 && cursorY+5 <= 64 && cursorY+5 >= 45 && cursorY+5 <= 64)) {
        cursorY += 5;
      //}
    }

  arduboy.display();
}

/**
 * Function that displays the main title screen
 */
void doTitleScreen() {
  uint16_t address = EEPROM_STORAGE_SPACE_START;

  if (startFlag) {

    //clear the screen then write app name
    arduboy.clear();
    arduboy.setCursor(15, 10);
    arduboy.print("-ArduNet Elegans-");
    arduboy.setCursor(10, 50);
    arduboy.display();
//TODO: uncomment when learningArray is implemented, fix address as well
//    for (uint8_t i = 0; i < learningArraySize; i++) {
//      EEPROM.read(address + i, learningArray[i]);
//    }

    delay(3000);  
  }
}

/**
 * Function to show the output of muscle movement ratios to the user
 */
void simulation() {
  arduboy.clear();

  //draw repellant on screen
  if (isRepel) {
    Sprites::drawOverwrite(repellentX, repellentY, toxin, 0);
  }

  //figure out if worm is hungry or not
  doAppetite();

  //draw stipling in the dirt to indicate depth
  bool offset = false;
  uint8_t widthOffset = 1;
  for (uint8_t j = 25; j <= 64; j+=3) {
    for (uint8_t i = 0; i <= 128; i+=widthOffset) {
      if (j > 55) {
        widthOffset = 3;
      } else if (j > 45) {
        widthOffset = 6;
      } else if (j > 35) {
        widthOffset = 12;
      } else if (j > 25) {
        widthOffset = 24;
      } else if (j > 20) {
        widthOffset = 48;
      }
        
      if (offset) {
        arduboy.drawPixel(i, j, WHITE);
      } else {
        arduboy.drawPixel(i+(widthOffset/2), j, WHITE);
      }
    }
    offset = !offset;
  }

  Sprites::drawOverwrite(obstacleX1, obstacleY1, rock1, 0);
  Sprites::drawOverwrite(obstacleX2, obstacleY2, stick, 0);
  Sprites::drawOverwrite(obstacleX3, obstacleY3, rock3, 0);
  Sprites::drawOverwrite(obstacleX4, obstacleY4, leaf, 0);
  Sprites::drawOverwrite(obstacleX5, obstacleY5, leaf, 0);
  Sprites::drawOverwrite(20, 2, sun, 0);    //draw a sun on surface
  Sprites::drawOverwrite(90, 0, tree, 0);   //draw the tree in top right

  //draw grass on surface
  for (uint8_t i = 1; i < 127; i += 10) {
    arduboy.drawPixel(i, 19, WHITE);
    arduboy.drawPixel(i, 18, WHITE);
    arduboy.drawPixel(i, 17, WHITE);
    arduboy.drawPixel(i, 16, WHITE);
    
    arduboy.drawPixel(i+2, 19, WHITE);
    arduboy.drawPixel(i+2, 18, WHITE);
    arduboy.drawPixel(i+2, 17, WHITE);

    arduboy.drawPixel(i+4, 19, WHITE);
    arduboy.drawPixel(i+4, 18, WHITE);
    arduboy.drawPixel(i+4, 17, WHITE);

    arduboy.drawPixel(i+6, 19, WHITE);
    arduboy.drawPixel(i+6, 18, WHITE);
    arduboy.drawPixel(i+6, 17, WHITE);
    arduboy.drawPixel(i+6, 16, WHITE);

    arduboy.drawPixel(i+8, 19, WHITE);
    arduboy.drawPixel(i+8, 18, WHITE);
    arduboy.drawPixel(i+8, 17, WHITE);

    arduboy.drawPixel(i+10, 19, WHITE);
    arduboy.drawPixel(i+10, 18, WHITE);
    arduboy.drawPixel(i+10, 17, WHITE);
    arduboy.drawPixel(i+10, 16, WHITE);    
  }

  //draw food on screen (store this in sprites.h)
  if (isFood) {
    Sprites::drawOverwrite(foodX, foodY, food, 0);
  }

  //draw line showing surface and light exposure activation
  arduboy.drawLine(0, 20, 128, 20);
  
  calculateCollisions();
  calculateGradients();
  drawFaces();
  drawWorm();

  //draw cursor centered on cursorX, cursorY 
  arduboy.drawPixel(cursorX, cursorY - 1);
  arduboy.drawPixel(cursorX, cursorY - 2);
  arduboy.drawPixel(cursorX - 1, cursorY);
  arduboy.drawPixel(cursorX - 2, cursorY);
  arduboy.drawPixel(cursorX + 1, cursorY);
  arduboy.drawPixel(cursorX + 2, cursorY);
  arduboy.drawPixel(cursorX, cursorY + 1);
  arduboy.drawPixel(cursorX, cursorY + 2);

  wormMove();

  //draw border
  arduboy.drawRoundRect(0, 0, 128, 64, 3);
  
  //draw demarcations on left for temp gradients
  Sprites::drawOverwrite(1, 2, demarc, 0);
  arduboy.setCursor(7, 2);
  arduboy.print("C");
  Sprites::drawOverwrite(2, 2, degree, 0);

  //draw demarcations on right for O2/CO2 gradients
  Sprites::drawOverwrite(124, 2, demarc, 0);
  arduboy.setCursor(116, 2);
  arduboy.print("O");
  Sprites::drawOverwrite(122, 5, sub2, 0);

  if (isAsleep == true) {   //if asleep, keep the worm asleep until network is powered off
    //TESTING REQUIRED: unsure if this should be active every single step or a dispersed phasic firing pattern?
    //maintainSleep();
    phasic(false, 90, maintainSleep);

    arduboy.drawRoundRect(25, 23, 76, 21, 3);
    arduboy.setCursor(27, 25);
    arduboy.print("Sleep Active");    
    arduboy.setCursor(27, 35);
    arduboy.print("[DATA SAVED]");
  }

  arduboy.display();
}

/***********************************SIM FUNCTIONS*************************************/
/**
 * Autosave the current learning array by using the 
 * connectome's tick variable
 */
void autosave() {
  if (tick % autosaveTimer == 0) {
//TODO: save learning array to eeprom
  }
}

/**
 * Function to use the isHungry() and doSatiety() functions
 * at the proper rate by using the tick variable
 */
void doAppetite() {
  //logic to regulate hunger
  if (foodEaten >= 5 && (tick % 100 == 0)) {                  //if the worm has eaten enough food, and given time to digest, then it is full
    sated = true; 
    fullTick = tick;
  }     
  if (sated) phasic(false, 20, doSatiety);                    //if the worm is full then activate the satiety neurons in a phasic pattern
  if (!sated && (tick % 100 == 0)) isHungry();                //if the worm is hungry then activate neurons to inhibit satiety every little while
  if (sated && (tick - fullTick >= 1000)) {                   //if the worm is full and it has been enough time to be hungry, then it is hungry again
    sated = false;             
    foodEaten = 0;
  }
  //TESTING REQUIRED: unsure if params in this satiety code needs to be tuned
}
/**
 * Function to do ASE neuron baseline chemotaxis
 */
void doBaseline() {
  if (!isFood && !isRepel) {  //activate baseline ASER chemotaxis
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
 * Function to draw the worm's body to the simulation screen
 */
void drawWorm() {
  //draw worm body on screen (Put this in a helper function)
  if (wormFacing == 0) {                              //NORTH
    if (prevWormDir == 0) { //if previously faced N       T
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY + 1);
      arduboy.drawPixel(wormX, wormY + 2);
    }

    if (prevWormDir == 1) { //if previously facing E      _T
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY + 1);
      arduboy.drawPixel(wormX - 1, wormY + 1);
    }

    if (prevWormDir == 2) { //if previously facing S     |   
      arduboy.drawPixel(wormX, wormY - 1);
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY + 1);
    }

    if (prevWormDir == 3) { //if previously facing W      T_
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY + 1);
      arduboy.drawPixel(wormX + 1, wormY + 1);
    }

  } else if (wormFacing == 1) {                       //EAST
    if (prevWormDir == 0) { //if previously faced N     |_.       
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX - 1, wormY);
      arduboy.drawPixel(wormX - 1, wormY - 1);
    }

    if (prevWormDir == 1) { //if previously facing E    __.
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX - 1, wormY);
      arduboy.drawPixel(wormX - 2, wormY);
    }

    if (prevWormDir == 2) { //if previously facing S    |''       
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX - 1, wormY);
      arduboy.drawPixel(wormX - 1, wormY + 1);
    }

    if (prevWormDir == 3) { //if previously facing W    _._
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX + 1, wormY);
      arduboy.drawPixel(wormX - 1, wormY);
    }
  } else if (wormFacing == 2) {                       //SOUTH
    if (prevWormDir == 0) { //if previously faced N     |      
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY - 1);
      arduboy.drawPixel(wormX, wormY + 1);
    }

    if (prevWormDir == 1) { //if previously facing E    |''
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY - 1);
      arduboy.drawPixel(wormX + 1, wormY - 1);
    }

    if (prevWormDir == 2) { //if previously facing S    |       
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY + 1);
      arduboy.drawPixel(wormX, wormY + 2);
    }

    if (prevWormDir == 3) { //if previously facing W    ''|
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX, wormY - 1);
      arduboy.drawPixel(wormX - 1, wormY - 1);
    }
  } else if (wormFacing == 3) {                       //WEST
    if (prevWormDir == 0) { //if previously faced N     _|     
      arduboy.drawPixel(wormX + 1, wormY - 1);
      arduboy.drawPixel(wormX + 1, wormY);
      arduboy.drawPixel(wormX, wormY);
    }

    if (prevWormDir == 1) { //if previously facing E    __
      arduboy.drawPixel(wormX - 1, wormY);
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX + 1, wormY);
    }

    if (prevWormDir == 2) { //if previously facing S    ''|      
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX + 1, wormY);
      arduboy.drawPixel(wormX + 1, wormY + 1);
    }

    if (prevWormDir == 3) { //if previously facing W    __
      arduboy.drawPixel(wormX, wormY);
      arduboy.drawPixel(wormX + 1, wormY);
      arduboy.drawPixel(wormX + 2, wormY);
    }
  }
}

/**
 * Function to calculate worm's movement based on its muscle outputs
 */
void wormMove() {
  prevWormDir = wormFacing;                       //update previous worm direction

  if (vaRatio + daRatio > vbRatio + dbRatio) {    //worm is moving backward
    if (wormFacing == 0 && wormY <= 64 && wormY >= 0) {   
      if (!(wormY-1 >= 49  && wormY-1 <= 64 
         && wormY-1 >= 45  && wormY-1 <= 64)
         && wormY   >   1  && wormY   <  63) {
        phasic(false, 90, doProprioception);
        wormY--;
      }
    } else if (wormFacing == 1 && wormX <= 128 && wormX >= 0) {
      if (!(wormX-1 >= 5   && wormX-1 <= 23 
         && wormX-1 >= 105 && wormX-1 <= 123) 
         && wormX   >  1   && wormX   <  127) {
        phasic(false, 90, doProprioception);
        wormX--;
      }
    } else if (wormFacing == 2 && wormY <= 64 && wormY >= 0) {
      if (!(wormY+1 >= 49  && wormY+1 <= 64 
         && wormY+1 >= 45  && wormY+1 <= 64)
         && wormY   >   1  && wormY   <  63) {
        phasic(false, 90, doProprioception);
        wormY++;
      }
    } else if (wormFacing == 3 && wormX <= 128 && wormX >= 0) {
      if (!(wormX+1 >= 5   && wormX+1 <= 23 
         && wormX+1 >= 105 && wormX+1 <= 123) 
         && wormX   >  1   && wormX   <  127) {
        phasic(false, 90, doProprioception);
        wormX++;
      }
    }
  } else {                                        //worm is moving forward
    if (wormFacing == 0 && wormY <= 64 && wormY >= 0) {
      if (!(wormY+1 >= 49  && wormY+1 <= 64 
         && wormY+1 >= 45  && wormY+1 <= 64)
         && wormY   >   1  && wormY   <  63) {
        phasic(false, 90, doProprioception);
        wormY++;
      }
    } else if (wormFacing == 1 && wormX <= 128 && wormX >= 0) {
      if (!(wormX+1 >= 5   && wormX+1 <= 23 
         && wormX+1 >= 105 && wormX+1 <= 123) 
         && wormX   >  1   && wormX   <  127) {
        phasic(false, 90, doProprioception);
        wormX++;
      }
    } else if (wormFacing == 2 && wormY <= 64 && wormY >= 0) {
      if (!(wormY-1 >= 49  && wormY-1 <= 64 
         && wormY-1 >= 45  && wormY-1 <= 64)
         && wormY   >   1  && wormY   <  63) {
        phasic(false, 90, doProprioception);
        wormY--;
      }
    } else if (wormFacing == 3 && wormX <= 128 && wormX >= 0) {
      if (!(wormX-1 >= 5   && wormX-1 <= 23 
         && wormX-1 >= 105 && wormX-1 <= 123) 
         && wormX   >  1   && wormX   <  127) {
        phasic(false, 90, doProprioception);
        wormX--;
      }
    }
  }
  
  if (vaRatio + vbRatio > daRatio + dbRatio) {    //worm is moving left
    if (wormFacing == 0) {
      wormFacing = 3;
    } else if (wormFacing == 1) {
      wormFacing = 0;
    } else if (wormFacing == 2) {
      wormFacing = 1;
    } else if (wormFacing == 3) {
      wormFacing = 2;
    }
  } else {                                        //worm is moving right
    if (wormFacing == 0) {
      wormFacing = 1;
    } else if (wormFacing == 1) {
      wormFacing = 2;
    } else if (wormFacing == 2) {
      wormFacing = 3;
    } else if (wormFacing == 3) {
      wormFacing = 0;
    }
  }  
}

/**
 * Function to draw the worms facial expressions
 */
void drawFaces() {
  if (outputList[39] && !outputList[40]) { //attractive chemotaxis; contented
    Sprites::drawOverwrite(wormX + 9, wormY - 4, content, 0);           //^-^
    arduboy.drawRoundRect(wormX + 5, wormY - 6, 17, 8, 2, WHITE);  
  } //ASEL is on and ASER is off indicates attractive chemotaxis
        
  for (uint16_t i = 0; i < 302; i++) {
    if (outputList[i]) {
      if (i == 53 || i == 54 || i == 57 || i == 58 || i == 59 || i == 60) {  //escape behavior; fear
        Sprites::drawOverwrite(wormX + 9, wormY - 4, fear, 0);          //O_O
        arduboy.drawRoundRect(wormX + 5, wormY - 6, 17, 8, 2, WHITE);
      } //AVA, AVD, AVE

      if (i == 149 || i == 150 || (i == 167 || i == 147)) { //foraging behavior; happy
        Sprites::drawOverwrite(wormX + 8, wormY - 4, happy, 0);         //^v^
        arduboy.drawRoundRect(wormX + 5, wormY - 6, 17, 8, 2, WHITE);
      } //NSM, MC

      if (i == 43 && i == 44 && (i == 6 || i == 6 || i == 49 || i == 50)) {  //repulsive chemotaxis; disgust
        Sprites::drawOverwrite(wormX + 9, wormY - 4, disgust, 0);       //>n<
        arduboy.drawRoundRect(wormX + 5, wormY - 6, 17, 8, 2, WHITE);              
      } //ASH, ADL, ASK

      if (i == 174 || i == 175 || (i == 8 || i == 9 || i == 113 || i == 114 || i == 165 || i == 166)) { //hot or cold; discomfort
        Sprites::drawOverwrite(wormX + 9, wormY - 4, discomfort, 0);    //@_@
        arduboy.drawRoundRect(wormX + 5, wormY - 6, 17, 8, 2, WHITE);
      } //PVD, AFD, FLP, PHC

      if (i == 199 || i == 200) {   //sleep
        Sprites::drawOverwrite(wormX + 7, wormY - 4, sleepy, 0);        //UwU
        arduboy.drawRoundRect(wormX + 5, wormY - 6, 17, 8, 2, WHITE);
      } //RIM

      if (i == 43 || i == 44) { //generalized nociception; pain
        Sprites::drawOverwrite(wormX + 8, wormY - 4, pain, 0);          //;_;
        arduboy.drawRoundRect(wormX + 5, wormY - 6, 17, 8, 2, WHITE);         
      } //ASH
    }
  }
}

/**
 * Function to make stepwise gradients on the screen
 */
void makeGradients(bool isFullRadius, uint8_t compX, uint8_t compY, void (*senseFunction)(), bool isStimulant, bool isPhasic, bool onlyVertical) {
  if (!isStimulant) return;
  //Credit again to Dinokaiz2, math whiz who helped with the math for this beautiful tonic activation system
  /**    
  * On the left, y axis, each gradient step is five. The x axis is the ticks of the connectome up to 3628800.
  * 3628800 is the factorial of 10; which is a multiple of 1 - 10, which allows each of the first 
  * ten gradient steps to have a regular rhythm. The result is tonic activation of the neurons in the
  * sense function that is called.
  *
  *           Sensory Neuron Firing Frequency For Gradients
  *       0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  ... 3628799 
  * 5     *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       *
  * 10    *       *       *       *       *       *       *       *       *       *       *       
  * 15    *           *           *           *           *           *           *               
  * 20    *               *               *               *               *               *       
  * 25    *                   *                   *                   *                   *       
  * 30    *                       *                       *                       *               
  * 35    *                           *                           *                               
  * 40    *                               *                               *                       
  * 45    *                                   *                                   *               
  * 50    *                                       *                                       *       
  */
  
  if (tick == 3628800) {                        //reset tick counter for gradient senses when it reaches the factorial of 10 (each gradient is 5)
    tick == 0;
  }
  
  if (onlyVertical) {                                               //if only vertical gradient (no x coordinate, only smaller 25 pixel distance/radius used)
    uint8_t distance = abs(wormY - compY);                          //distance calculation
    if (!isPhasic) {  //if tonic
      if (distance < 25) {
        tonic(true, distance, 2, senseFunction);
      }
    } else {          //if phasic
      phasic(true, distance, senseFunction);
    }
  } else {                                                          //diamond-shaped gradient
    uint8_t distance = abs(wormX-compX) + abs(wormY - compY);       //if diamond-shaped gradient (circle too computationally expensive)
    if (!isPhasic) {  //if tonic
      if (isFullRadius && distance < 50) {                          //for full radius gradients, activate sensory function within a radius of 50
        tonic(true, distance, 5, senseFunction);
      }
      
      if (!isFullRadius && distance < 25) {                         //if smaller radius is desired, activate sensory function within radius of 25
        tonic(true, distance, 2, senseFunction);
      }
    } else {          //if phasic
      phasic(true, distance, senseFunction);
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

/**
 * Function to calculate the chemical gradients for the worm's senses
 */
void calculateGradients() {
  tick++;

  makeGradients(false, 64, 40, doBaseline, true, false, true);                            //baseline ASE activation is tonic; Y (to center); small radius
  makeGradients(false, cursorX, cursorY, sensePheromones, true, false, false);            //pheromone sense is tonic; X/Y; small radius
  makeGradients(false, foodX, foodY, doTextureSense, isFood, false, false);               //texture sense is unknown... will try tonic for now; X/Y; small radius
  makeGradients(true, foodX, foodY, doChemoattraction, isFood, false, false);             //chemoattraction is tonic; X/Y; large radius
  makeGradients(false, 64, 5, doOxygenSensation, true, false, true);                      //oxygen is tonic; Y; small radius
  makeGradients(false, 64, 15, doHeatingResponse, true, false, true);                     //heating is tonic; Y; small radius
  makeGradients(false, 64, 45, doCoolingResponse, true, false, true);                     //cooling sensation is tonic; Y; small radius
  makeGradients(false, 64, 45, doCO2Sensation, true, false, true);                        //CO2 sensation is likely tonic???; Y; small radius
  makeGradients(true, repellentX, repellentY, doChemorepulsion, isRepel, true, false);    //chemorepulsion is phasic- responding to gradient level changes; X/Y; large radius
  makeGradients(false, 64, 5, doNoxiousHeatResponse, true, true, true);                   //noxious heat is phasic- responding to temp level changes; Y; small radius
  makeGradients(false, 64, 55, doNoxiousColdResponse, true, true, true);                  //noxious cold is phasic- responding to temp level changes; Y; small radius
  makeGradients(false, 64, 0, doPhotosensation, true, true, true);                        //phototaxis is phasic- responding to light level changes; Y; small radius

  /*      NATURAL SIM GRADIENTS
  0                               LIGHT
          O2               HOT      |
  10      |                 |       |
          |       heat      |       |
  20      |        |        |     LIGHT
          O2       |       HOT
  30               |
                  heat
  40          [GOLDILOCKS]
               cool  CO2
  50            |     |
                |     |    COLD
  60            |     |     |
               cool  CO2    |
  */
}

/**
 * Function to calculate collisions of simulation objects
 */
void calculateCollisions() {
//TODO: add collisions for other obstacles: 4 (leaf) and 5 (leaf)
  //if worm hits obstacles 1 (rock), 2 (stick), or 3 (rock), activate touch
  if ((wormX == obstacleX1 && wormY == obstacleY1) || (wormX == obstacleX2 && wormY == obstacleY2) || (wormX == obstacleX3 && wormY == obstacleY3)) {
    //activate harsh touch
    doGentleNoseTouch();
  }

  //if worm hits border, activate soft touch
  if (wormX == 0 || wormX == 128 || wormY == 0 || wormY == 64) {
    //activate soft touch
    doGentleNoseTouch();
  }

  //if worm hits cursor, activate soft touch
  if (wormX == cursorX && wormY == cursorY) { 
    //activate soft touch
    doGentleNoseTouch();
  }
  //activate gustatory neurons if worm is touching food
  if ((wormX == foodX     && wormY == foodY) || (wormX == foodX + 1 && wormY     == foodY + 1) 
   || (wormX == foodX + 1 && wormY == foodY) || (wormX == foodX     && wormY + 1 == foodY)) {
    doGustation();
    
    isFood = false;
    Sprites::drawErase(foodX, foodY, food, 0);
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

/*void doOsmoticResponse() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 43 || id == 44 || id == 6 || id == 7) {
                outputList[id] = true;
            }
        }
 }       //Educated guess based on protein presence on neurons: ASH, ADL

 void doGentlePosBodyTouch() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 167 || id == 168) {
                outputList[id] = true;
            }
        }
 }       //PLM

 void doGentleAntBodyTouch() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 23 || id == 24 || id == 71) {
                outputList[id] = true;
            }
        }
 }       //ALM, AVM

 void doHarshBodyTouch() {
      for (uint16_t id = 0; id < 302; id++) {
            if (id == 174 || id == 174) {
                outputList[id] = true;
            }
        }
 }       //PVD*/

/*************************************CONNECTOME FUNCTIONS***************************************/
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
    matrixToNeuron(i);                               //fill the neuron struct with the information of the ith neuron
    int32_t sum = 0;
    bool hebFlag = false;
    uint8_t offset = 2;                              //value to adjust how much "charge" a gap junction sends to next neuron

    for (uint8_t j = 0; j < 66; j++) {               //check to see if current ID is in the hebbian-capable neuron  list
      if (HEBBIAN_NEURONS[j] == i) {
        //sum += learningArray[j];
        hebFlag = true;
      }
    }

    for (uint8_t j = 0; j < n.inputLen; j++) {                          //loop over every presynaptic neuron
      if (n.weights[j] >= 90) {                                         //if the weight has a gap junction indicator (9_)
        int8_t gapWeight = n.weights[j] - 90;                           //gap junctions are indicated by weights at 90-99
        uint8_t gapOutput = 0;                                          //gap junctions presynapse outputs are adjusted, as they are not binary
        if (outputList[n.inputs[j]]) gapOutput = (1 + offset);         //outputList value is adjusted for non-binary activation
        if (!outputList[n.inputs[j]]) gapOutput = -(1 + offset);
        sum += gapWeight * gapOutput;                                  //do the summation calculation for a gap junction synapse
      } else {
        sum += n.weights[j] * outputList[n.inputs[j]];                  //do the summation calculation on current synapse
      }
    }

    if (sum >= threshold) {                                             //check if activation function outputs a true or false
      nextOutputList[i] = true;                                         //store the output in a buffer
    } else {
      nextOutputList[i] = false;
    }

//TODO: add learning array calls here
    for (uint8_t j = 0; j < n.inputLen; j++) {                          //loop over every presynaptic neuron
      if (hebFlag) {                                                    //if current neuron is in list
        if (outputList[n.inputs[j]] && nextOutputList[i]) {             //if the pre and postsynaptic neuron both fire
          //the specific synapse gets an increased hebbian value
        } else if (!outputList[n.inputs[j]] && !nextOutputList[i]) {    //if the pre and postsynaptic neuron both do NOT fire
          //the specific synapse gets a decreased hebbian value
        }
      }
    }
  }

  //flush the buffer
  for (int16_t i = 0; i < 302; i++) {
    outputList[i] = nextOutputList[i];
  }
}
