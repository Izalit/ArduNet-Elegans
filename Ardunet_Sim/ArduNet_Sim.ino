/**
 *      Simulation Version TODOs
 * Make room for the ascii faces, title screen, satiety, and drawWorm
 *
 * Implement satiety
 *    create a counter for food eaten
 *    use tick variable to check every so many ticks how much food has been eaten
 * Phasic and Tonic Firing Types
 *    Implement phasic activation (current system for gradient activation is tonic, non-gradient is a single call)
 *    Implement proper activation (phasic or tonic) for the senses seperated via height (light, temp, aerotaxis)
 * Add collisions for full obstacle (1, 2, 3, 4, 5) sprite hit-boxes
 * Fix the randomizers
 *    multiple food; spawn points are under the stick, inside of the tree roots, and the user placed food
 *    multiple toxins; spawn points are near the bottom rocks, top left of surface
 *    leaves on surface in random spots
 *    Randomize to moon deactivate phototaxis when doing so
 *    smaller rocks in random spots?
 * Make motor neurons --> worm movement more biologically accurate
 *    move away from tank drive and instead move to a proper sinusoid
 * Make DVA activation (proprioception) more accurate
 *    activate when doing turns and rapid forward oscillations
 * Leaves periodically fall from tree and blink after so many ticks, then dissappear and drop food direclty under the surface
 * Randomized or timed Weather, Day-Night cycle, etc
 *
 * BottleNeck: Waiting on K to finish
 *   Goal: Implement the hebbian long term potentiation and long term depression functions
 *     Save the hebbian array to eeprom
 *     Gap junctions: manually add synapses marked with a weight of "75" - the largest is 71, smallest is -70
 *        add an "if" statement in the activation function which, detects a gap junction and adds a large + or - weight depending on presynaptic neurons output state
 *     Using remaining space, add as many necessary gap junction synapses into neural ROM as possible
 *

sensory neurons (35)
FLPL, FLPR, PHAL, PHAR, PHBL, PHBR, OLQDL, OLQDR, OLQVL, OLQVR, IL1DL, 
IL1DR, IL1L, IL1R, IL1VL, IL1VR, ADLL, ADLR, CEPDL, CEPDR, CEPVL, CEPVR, 
ADEL, ADER, ASGL, ASGR, ASJL, ASJR, ASKL, ASKR, PHCL, PHCR, AVG, BAGL, BAGR,

sensory-interneurons and interneurons(62)
SIADL, SIADR, SIAVL, SIAVR, AIYL, AIYR, ASIL, ASIR, ASEL, ASER, RIML, RIMR,
PDEL, PDER, AVKL, AVKR, DVA, AWCL, AWCR, RIAL, RIAR, AVAL, AVAR, AVDL, AVDR,
ALML, ALMR, AVM, PLML, PLMR, PVDL, PVDR, ASHL, ASHR, ADFL, ADFR, AIAL, AIAR, 
NSML, NSMR, AFDL, AFDR, AIZL, AIZR, AQR, PQR, URXL, URXR, AWBL, 
AWBR, AIBL, AIBR, SMDDL, SMDDR, SMDVL, SMDVR, OLLL, OLLR, RICL, RICR

sensory neurons to DVA: AQR, FLPL/FLPR, PDEL/PDER, PHAL/PHAR, PHCL/PHCR, PLML/PLMR, PVDL/PVDR, PVM, SDQL/SDQR
motor neurons to RIM: HSNL/HSNR, SAADL/SAADR/SAAVL/SAAVR, VB1/VB2/VB3/VB4/VB5/VB6/VB7/VB8/VB9/VB10/VB11, AS1-11?
neurons to add according to below sources: AVBL/AVBR, PVCL/PVCR, PVM, AVEL/AVER, AWAL/AWAR, RIBL/RIBR

Sources for Memory Neurons (other than wormatlas):
    https://www.frontiersin.org/journals/physiology/articles/10.3389/fphys.2013.00088/full
    https://www.eneuro.org/content/6/4/ENEURO.0014-19.2019
    https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0006019
    https://www.eneuro.org/content/9/4/ENEURO.0084-22.2022
 */

/*
Random Notes:
  Consider adding petting! User can "pet" the worm, activating ALM/AVM/PLM/PVM
    signal relayed to command interneurons
    causes a reversal or forward acceleration
    pair the activation with CEP/PDE/ADE (dopaminergic reward-like neurons)
    behavioral changes to increased exploration, mild energetic arousal-like state
    will need to implement learning in the relevent neurons to allow this
*/

#include "neuralROM.h"            //import libraries
#include "sprites.h"
#include "bit_array.h"
#include <Arduboy2.h>
//#include <stdlib.h>

Arduboy2 arduboy;                 //create arduboy object

const uint8_t threshold = 1;      //threshold for activation function
const uint16_t maxSynapse = 65;   //maximum number of synapses a neuron can have
bool startFlag = true;            //interface flag for title screen to play
float vaRatio = 0;                //muscle ratios for interface printout
float vbRatio = 0;
float daRatio = 0;
float dbRatio = 0;
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
uint16_t tick = 0;        //connectome ticks
uint8_t repellentX = 34;  //repellent
uint8_t repellentY = 52;  //repellent
bool isRepel = true;      //if theres any repellents
bool isFood = true;       //if theres any food
bool isAsleep = false;    //if sleep state is active
//uint16_t foodTouchCounter = 0;  //how long the food has been eaten for

//massive thanks to Dinokaiz2 for help with the bit array functionality!!!
//uint8_t preSynapticNeuronList[maxSynapse];  //interface array to hold all the different presynaptic neurons
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
  
  //doTitleScreen();

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
//TODO: save data to eeprom here
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
  if (startFlag) {

    //clear the screen then write app name
    arduboy.clear();
    arduboy.setCursor(15, 10);
    arduboy.print("-ArduNet Elegans-");
    arduboy.setCursor(10, 50);
    arduboy.display();
    delay(5000);  
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
  //drawFaces();
  //drawWorm();

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
    maintainSleep();

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
 * Function to create a satiety response
 */
void doSatiety() { //tonic
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
  bool isPropTick = tick == 4 || tick == 8 || tick == 12;

  if (vaRatio + daRatio > vbRatio + dbRatio) {    //worm is moving backward
    if (wormFacing == 0 && wormY <= 64 && wormY >= 0) {
      if (!(wormY-1 >= 44 && wormY-1 <= 62 && wormY-1 >= 38 && wormY-1 <= 54 && wormY-1 >= 38 && wormY-1 <= 57)) {
        if(isPropTick) doProprioception();
        wormY--;
      }
    } else if (wormFacing == 1 && wormX <= 128 && wormX >= 0) {
      if (!(wormX-1 <= 8 && wormX-1 >= 23 && wormX-1 <= 57 && wormX-1 >= 75 && wormX-1 <= 104 && wormX-1 >= 122)) {
        if(isPropTick) doProprioception();
        wormX--;
      }
    } else if (wormFacing == 2 && wormY <= 64 && wormY >= 0) {
      if (!(wormY+1 >= 44 && wormY+5 <= 62 && wormY+1 >= 38 && wormY+5 <= 54 && wormY+1 >= 38 && wormY+5 <= 57)) {
        if(isPropTick) doProprioception();
        wormY++;
      }
    } else if (wormFacing == 3 && wormX <= 128 && wormX >= 0) {
      if (!(wormX+1 <= 8 && wormX+1 >= 23 && wormX+1 <= 57 && wormX+1 >= 75 && wormX+1 <= 104 && wormX+1 >= 122)) {
        if(isPropTick) doProprioception();
        wormX++;
      }
    }
  } else {                                        //worm is moving forward
    if (wormFacing == 0 && wormY <= 64 && wormY >= 0) {
      if (!(wormY+1 >= 44 && wormY+1 <= 62 && wormY+1 >= 38 && wormY+1 <= 54 && wormY+1 >= 38 && wormY+1 <= 57)) {
        if(isPropTick) doProprioception();
        wormY++;
      }
    } else if (wormFacing == 1 && wormX <= 128 && wormX >= 0) {
      if (!(wormX+1 <= 8 && wormX+1 >= 23 && wormX+1 <= 57 && wormX+1 >= 75 && wormX+1 <= 104 && wormX+1 >= 122)) {
        if(isPropTick) doProprioception();
        wormX++;
      }
    } else if (wormFacing == 2 && wormY <= 64 && wormY >= 0) {
      if (!(wormY-1 >= 44 && wormY-1 <= 62 && wormY-1 >= 38 && wormY-1 <= 54 && wormY-1 >= 38 && wormY-1 <= 57)) {
        if(isPropTick) doProprioception();
        wormY--;
      }
    } else if (wormFacing == 3 && wormX <= 128 && wormX >= 0) {
      if (!(wormX-1 <= 8 && wormX-1 >= 23 && wormX-1 <= 57 && wormX-1 >= 75 && wormX-1 <= 104 && wormX-1 >= 122)) {
        if(isPropTick) doProprioception();
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
void makeGradients(bool isFullRadius, uint8_t compX, uint8_t compY, void (*senseFunction)(), bool isStimulant) {
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

  uint8_t distance = abs(wormX-compX) + abs(wormY - compY);     //diamond-like distance calculation

  if (isFullRadius && distance < 50) {                          //for full radius gradients, activate sensory function within a radius of 50
    distance = (distance / 5) + 1;                              //modify distance; 5 is the precision of the gradient, the step of the gradient
    if (tick % distance == 0) senseFunction();
  }
  
  if (!isFullRadius && distance < 25) {                         //if smaller radius is desired, activate sensory function within radius of 25
    distance = (distance / 2) + 1;                              //gradient step set to two as the diamon shape is smaller, a smaller radius
    if (tick % distance == 0) senseFunction();
  }
  

  /*if (isFullRadius) {
    if (isStimulant) {
      if (abs(wormX - compX) < 50 && abs(wormX - compY) < 50 && (sensoryNeuronCounter == 8)) {
        senseFunction();
        if (abs(wormX - compX) < 40 && abs(wormX - compY) < 40 && (sensoryNeuronCounter == 6 || sensoryNeuronCounter == 10)) {
          senseFunction();
          if (abs(wormX - compX) < 30 && abs(wormX - compY) < 30 && (sensoryNeuronCounter == 4 || sensoryNeuronCounter == 8 || sensoryNeuronCounter == 12)) {
            senseFunction();
            if (abs(wormX - compX) < 20 && abs(wormX - compY) < 20 && (sensoryNeuronCounter == 3 || sensoryNeuronCounter == 5 || sensoryNeuronCounter == 7 || sensoryNeuronCounter == 9 || sensoryNeuronCounter == 11 || sensoryNeuronCounter == 14)) {
              senseFunction();
              if (abs(wormX - compX) < 10 && abs(wormX - compY) < 10 && (sensoryNeuronCounter == 2 || sensoryNeuronCounter == 4 || sensoryNeuronCounter == 6 || sensoryNeuronCounter == 8 || sensoryNeuronCounter == 10 || sensoryNeuronCounter == 12 || sensoryNeuronCounter == 14)) {
                senseFunction();
                if (abs(wormX - compX) < 5 && abs(wormX - compY) < 5) {
                  senseFunction();
                }
              }
            }
          }
        }
      }
    }
  } else {
  if (abs(wormX - compX) < 20 && abs(wormX - compY) < 20 && (sensoryNeuronCounter == 3 || sensoryNeuronCounter == 5 || sensoryNeuronCounter == 7 || sensoryNeuronCounter == 9 || sensoryNeuronCounter == 11 || sensoryNeuronCounter == 14)) {
    senseFunction();
      if (abs(wormX - compX) < 10 && abs(wormX - compY) < 10 && (sensoryNeuronCounter == 2 || sensoryNeuronCounter == 4 || sensoryNeuronCounter == 6 || sensoryNeuronCounter == 8 || sensoryNeuronCounter == 10 || sensoryNeuronCounter == 12 || sensoryNeuronCounter == 14)) {
        senseFunction();
        if (abs(wormX - compX) < 5 && abs(wormX - compY) < 5) {
          senseFunction();
        }
      }
    }
  }*/
}

/**
 * Function to calculate the chemical gradients for the worm's senses
 */
void calculateGradients() {
//photosensation is phasic
//oxygen sensation is either
//noxious temp is phasic
//cooling and heating is tonic
//CO2 sensation is tonic
  if (55 <= wormY && wormY <= 64) {           //temp frigid; oxygen too low
    doNoxiousColdResponse();
    doCO2Sensation();
  } else if (45 <= wormY && wormY < 55) {          //temp cold; oxygen low
    doCoolingResponse();             
    doCO2Sensation();                             
  } else if (35 <= wormY && wormY < 45) {        //temp ideal; oxygen ideal
    doCO2Sensation();               
  } else if (25 <= wormY && wormY < 35) {    //temp warm; oxygen high
    doHeatingResponse();        
    doOxygenSensation();        
  } else if (wormY < 25) {                 //temp hot; oxygen too high
    doNoxiousHeatResponse();  
    doOxygenSensation();      
  } else if (wormY < 20) {               //surface; light
    doPhotosensation();
  }

  if (!isFood && !isRepel) {  //activate baseline ASER chemotaxis
    outputList[40] = true;
  } else {
    outputList[40] = false;   //activates in response to increases in NaCl
  }

  tick++;
  makeGradients(false, cursorX, cursorY, sensePheromones, true);            //pheromone sense is tonic
  makeGradients(false, foodX, foodY, doTextureSense, isFood);               //texture sense is tonic
  makeGradients(true, foodX, foodY, doChemoattraction, isFood);             //chemoattraction is tonic  
  makeGradients(true, repellentX, repellentY, doChemorepulsion, isRepel);   //chemorepulsion is phasic -->  
}

/**
 * Function to calculate collisions of simulation objects
 */
void calculateCollisions() {
//TODO: add collisions for other obstacles: 4 (leaf) and 5 (leaf)
  //if worm hits obstacles 1 (rock), 2 (stick), or 3 (rock), activate harsh touch
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

//TODO: fix the number of pixels... should be four      
  //activate gustatory neurons if worm is touching food
  if ((wormX == foodX && wormY == foodY) || (wormX == foodX + 1 && wormY == foodY + 1)) {
    //uint16_t foodSize = 5;                      //amount of food to spawn (measured in ticks till gone)

    doGustation();
    //foodTouchCounter++;

    //if (foodTouchCounter >= foodSize) {   //if worm has been eating long enough, food is gone
      isFood = false;
      Sprites::drawErase(foodX, foodY, food, 0);
    //}
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
    int16_t skipLen = pgm_read_word(&NEURAL_ROM[index]);     //read the value of the first neuron's input Len
    index += 1 + skipLen + skipLen;       //add double that value plus one, to skip the entire neuron entry
  }

  n.inputLen = pgm_read_word(&NEURAL_ROM[index]);
  index++;

  // Read neuron inputs
  for (uint8_t i = 0; i < n.inputLen; i++) {
    n.inputs[i] = pgm_read_word(&NEURAL_ROM[index++]);
  }

  // Read neuron weights
  for (uint8_t i = 0; i < n.inputLen; i++) {
    n.weights[i] = pgm_read_word(&NEURAL_ROM[index++]);
  }
}

/**
 *The activation function is the main simulation, it calculates all the next ticks of the connectome
 * and then sets the next tick to the current one when each has been individually calculated
 */
void activationFunction() {  
  //calculate next output for all neurons using the current output list
  for (uint16_t i = 0; i < 302; i++) {
    matrixToNeuron(i);                                  //fill the neuron struct with the information of the ith neuron
    int32_t sum = 0;
    bool hebFlag = false;
    uint16_t hebPos = 0;

    for (hebPos; hebPos < 66; hebPos++) {               //check to see if current ID is in the hebbian-capable neuron  list
      if (HEBBIAN_NEURONS[hebPos] == i) {
        //sum += learningArray[hebPos];
        hebFlag = true;
      }
    }

    for (uint8_t j = 0; j < n.inputLen; j++) {          //loop over every presynaptic neuron
      sum += n.weights[j] * outputList[n.inputs[j]];    //do the summation calculation on the neuron
    }

    if (sum >= threshold) {                             //note it as being true or false
      nextOutputList[i] = true;
    } else {
      nextOutputList[i] = false;
    }

    for (uint8_t j = 0; j < n.inputLen; j++) {                //loop over every presynaptic neuron
      if (hebFlag) {                                          //if current neuron is in list
        if (outputList[n.inputs[j]] && nextOutputList[i]) {            //if the pre and postsynaptic neuron both fire
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