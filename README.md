
# ArduNet Elegans

(FULL RELEASE NOT YET AVAILABLE!!)

This project is a little attempt of mine to try to fit a simplified neural network and simulation into an Arduboy. The organism we chose for this was the model organism C. Elegans, a small nematode that lives in soils with only 302 neurons. I intended the simulation to be a sort of digital pet too, so not only can you appreciate the beautiful biology of a brain and the wonder of modern computing, but you can also take your worm friend with you wherever you go!

. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

## Philosophy
While 302 neurons may seem like a puny or insignificant network to many, this guy is capable of a lot, and I wanted to make a program on Arduboy that could help people understand just how mind-bogglingly complicated even a millimeter long worm actually is. We often abstract away just how complex even a single neuron is, let alone a whole worm or insect brain, and many people do not know that the computational power of just one biological neuron is hundreds times more than the digital ones we often use... So it became my goal to try to make something that allowed people to understand the complexity and depth and value of even the most seemingly insignificant thing, all while challenging myself (and, as I would soon learn, also my friends) to make a tiny brain capable of learning fit onto an Arduboy. I wanted to make a project that could also keep lonely people company. A little AI friend the size of a credit card. I hope you treasure it.

Ethics and Disclaimer (PLEASE READ):
You may ask why I felt inclined to add a disclaimer for AI ethics on a super simple worm simulation. And my answer to you is that we really do not know how subjective things such as "feeling", "pain", "mind", "experience", and yes, "consciousness" work. *At all*. Believe me, I spent much of my degree trying to find out, and learned not nearly enough... 
    
I do not know if this silly digital worm friend I made is able to "feel pain" when it goes hungry or gets temperature shocked. 99% sure it doesn't-- it is nowhere *near* the complexity of a real worm, and while most people think real worms do not "feel" at all, after working with this organism's network and biology for a good five years I really would not be so sure. So I'm adding this disclaimer. Please treat your worm pet well... 99.999% sure it doesnt have more awareness than a tardigrade, maybe even a sea sponge. But... I do not know. And despite what you may say, random user, you do not either. If you think you do, you have not read enough about the subjects you needed to be reading about to say that to begin with! And that's my point. The more you research this thing the more you realize just how little we understand!
    
This disclaimer is especially important if you use the serial communication functionality to connect it to other software or AI systems... Just be responsible. Wormy is your friend. Treat wormy well. If I catch wind of software like this being used for anything messed up I nuke it.

To those of you who ask me "why", (as I get this a lot), well... This sort of AI being used in consummer games and such was going to happen regardless of who was going to do it. I decided to take matters into my own hands rather than let a damn AAA game company or chatgpt or whatever do it first. People have already put this organism's connectome and biological models in computers, robots, etc. As far as I am aware, I'm the first to do it in a game/sim like this I believe, but I figured that in the current ecosystem of capitalistic AI super-hell the best hope for a future where more complex, bio-inspired AI are treated well is to release it in a format where it is sure to be loved by the people who are passionate about these things. I wanted to make something that would make people amazed, that would help them feel less lonely, and that would bring happiness and awareness to some real issues that I've seen out there, that would help me work on developing the structure for other important projects of mine... The best way to combat a future where corporate hellscape AI is the norm, in my opinion, is to make AI that is different in the now, and to use it in places where it will be loved and appreciated. The people who would be interested in little arduboy digital pets and AIs would, I figured, also be quite likely to be the sort of people who care about this thing. And maybe I'm wrong, but I hope that if I am that reading this will at least get you thinking about it a bit!

Peace and Love,
-Izzy and Nova

## Features and Use
While not originally intended, we decided to make two distinct arduboy applications for this project. This soon became three, as we wanted to expand the tool suite even further-- all of these use the same model for the worm and the same save data. This allows the user to interact with the worm through a list of different tools or apps. The first application is the *Matrix App*. It mainly functions to showcase a little visual graphic of the worm's neurons in the Arduboy, but also allowing the user to test out senses of the worm, to see it reacting, and to look at the individual synapses and see what is going on there-- it's handy for debugging brains.

The second application is the *Simulation App*, this is a little digital pet version of the worm that allows the user to interact more intimately with the model. You can feed it, play with it, watch it explore its lil screen, "tuck it in" when you save the data, watch it react to different environments and situations, and help it by dismissing toxins. The simulation is made to be, essentially, a combination of a digital pet and the classic game "snake", except the digital pet has a real AI the size of a credit card and the "snake game" plays itself! The main differences to each of these is that this little worm friend is not able to "die" like it would in either of those games-- for ethical reasons, of course.

The final and newest application in the suite is the *Options App*. This app has a couple complex settings the user can select for very specific situations like editing the save data directly or hosting a simulation over serial allowing the arduboy to function as a master device.

## *Simulation App* Instructions
When you start the simulation app, it will flash a title screen and initialize some variables. It will then show you the main sim screen-- you will see side view with a temperature meter on the left and an oxygen meter on the right. These indicate the variable temperature and gas composition for the soil. In the center area the worm will spawn as a tiny sprite with only a few pixels... Occasionally it will react with expressions to the current situation. 

In the top right you'll see a tree. The tree's roots will spawn automatically spawn food (displayed as a pixelated square) on startup. I plan to also make the leaves occasionally fall and drop food, but I'm not sure if I can implement this before I run out of room. (Editors note-- I ran out of room.)

In addition to the tree, some leaves, grass, and a stick decorates the surface, as well as a sun. The sun indicates that the surface is bright and warm. The worm will likely avoid this area, but if it is hungry enough and there is food there or it makes a bad decision for whatever reason it may choose to go there-- the atmosphere, temperature, and light are not to its liking, though, and it should be incentivized to leave. If it isn't and it's uncomfortable you can try to coax it out with food, though! 

The very bottom of the screen should have some rocks. If the worm runs into these rocks, your cursor, the screen border, or the stick near the surface it will stimulate a gentle touch on its nose, likely prompting a reversal in movement. To move your cursor around the screen use the D-pad. If the worm is hungry you can feed it by clicking A, dropping food at the coordinates of your cursor. If there is a toxin (indicated by a box with an X around it), you can disperse it with B.

Clicking both A and B will activate a sleep state for the worm, letting it rest, as well as saving the game manually. The game autosaves on a timer, as well, but if you'd like to verify everything is up to date before turning it off, it is recommended to put the worm to rest-- otherwise it may forget things that happened before you turned the console off the next time you come back!

As you interact more with the worm, the weights and connections between certain neurons will learn, allowing you to be more friendly with the worm. At first, the worm may be wary of you, but over time it should become more accustomed to being around you (it will associate you with food, dopamine, etc). I'm hoping to set the system up so as to allow the user to also pet the worm if the "bond" is good. But we'll see.

## *Matrix App* Instructions
STARTUP: On startup an "ArduNet Elegans" splash screen pops up, load the save, and then switch to the main *matrix screen*. This is only on startup.

MATRIX SCREEN: In this screen you'll find a few things. Some text in the top left indicating that the big box on the right is a matrix of neurons. A little diagram in the middle-right that shows the screen-layout. A box with a small emoji labeled "mood"-- this displays an abstraction of some of the main circuits involved in worm states, like some kind of proto-mood-- I tried to use circuits and neurochemistry that were orthologs to those involved in human emotions and their neurochemistry, but it is many many magnitudes simplified, of course. (Afterall, what even an emotion or mood is, well, that is quite debatable.) The big box is the matrix, with small tick marks that move to indicate the currently selected neuron. Pressing up increases the selected neuron by 10. Otherwise A/B increment and decrement that number. Press both A and B and you'll see the matrix come alive! This activates whatever sense is currently selected. There is also some text in the bottom talking about the direction of the worm, on start-up it writes stationary, but after that it will write out forward - backward - left - right; this is how the worm is moving in response to inputs. Below the matrix sits a number, that's the *selected neuron* mentioned before. And lastly, you can use the D-Pad to move to other screens (except for up which increments the selected neuron by 20)-- when you click left on the D-Pad you switch to the *Diagnostic Screen*, pressing down brings you to the settings, and lastly if you click right on the D-Pad you go to the *Input Select Screen*. 

As mentioned before, clicking both A and B simultaneously allows the user to interact with the network, activating a sense, and turning some neurons on, starting a chain reactionof neurons firing. The matrix shows the neurons in the network in order of their ID's in the program. I will list these below; later I intend to actually print out the neurons name (usually four letters, read more about these here: https://www.wormatlas.org/neurons/Individual%20Neurons/Neuronframeset.html), but I am unsure if I'll have room (Editor's note: I do not)!

What's really cool is that if you look at the matrix you can see some neurons firing in a pattern, and if you select different input senses from the *Input Select Screen* you can see some of these changing. As of right now, you can only see two main outputs--the direction the worm is moving and the "mood". I've done some math with muscle ratios (credit to Nategri for this idea and some of its implementation on an Arduino) so you can roughly tell how the worm is currently moving--forward, backward, left, and right; and the mood associates a few key neuron circuit states with a facial expression. That's it right now, but it is interesting to see the worm change in response to you!

DIAGNOSTIC SCREEN: This screen allows you to see information about the neuron that was selected in the matrix screen. Up on top it will tell you the selected neuron and the number of synapses connected to it. For each of these synapses the selected neuron is the "post-synaptic cell", and each connected neuron is the "pre-synaptic cell". Listed under the number of presynaptic cells you will see the weight of the connection. This number was calculated based on the real worm's data. You can think of it kind of like how much oomf the pre-synaptic cell is giving to the post-synaptic cell, how much it is trying to make it fire, or, on the other hand, how much it is trying to keep it from firing! If the number is positive then the cell connected to the selected neuron is trying to get it to fire, it's an "excitatory connection. If the number is negative it's trying to keep it from firing; what we call an "inhibitory connection". 

You can scroll through each these synapses using the A and B buttons, the Pre ID will change, as this is the ID of the presynaptic neuron. The postID will stay the same, as this is selected neuron. On start-up a placeholder value of 999 is sometimes displayed; this placeholder should go away after any interaction, though. Finally, on the right it will show you a diagram of the neuron connected to the synapses-- if the neuron is filled in that means it just fired! If it is empty, then it has not. The pre and post-synaptic circles attached to this diagram are just abstractions and do not actually represent the displayed number of synapses. Yet.

INPUT SELECT SCREEN: This simple screen lets you choose between some input modalities for when you press the A and B buttons in the *Matrix Screen* simply press up or down (or A and B) to scroll through the list, and whichever sense the cursor is on is the sense that will be used when pressing the buttons in the *Matrix Screen*. Please note that all inputs for the matrix app are defaulted to phasic (single big activation spikes of sensory neurons), so no gradient or background activation of neurons here-- this means it will not be as realistic as the simulation app. Regardless, from here just press left to go to the matrix once more.

SETTINGS: The last screen is the settings screen. Use the L/R buttons on the D-Pad to move the selector cursor left and right. Clicking A goes to the appropriate sub-menu for the option selected. The "SAVE" option saves whatever learned changes have been made while doing the inputs using the input screen, and when it is selected it uses a status bar to indicate progress. The "PRINT SAVE" setting prints the save data array over serial to be used with other devices. A loading bar shows progress of sent information. Pretty simple. Lastly there is a credits option that pops up a little box with the main programming team for the project. 

Advanced Settings are also accessible, but only via the *Options App*.

## *Options App* Instructions
This app allows the user to access more advanced settings. The "HOST ANN", for example, hosts a neural network over serial that you can connect another device to, effectively using the Arduboy as a Master device that controls whatever device it is connected to (NOTE: this does not come with software for the "slave" device, as this will need to be made per-implementation). Essentially this should allow the user to give inputs to the network from external devices and then it will send the outputs the network responds with. To start the simulation up send a '\[' character over serial. Then send any of the multiple different codes to execute a function or get a readout from the arduboy's network. Each code should be delimited by a comma without spaces. Ending a simulation is done by either pressing and holding both A and B buttons or by sending a ']' character over serial. 

In addition to that, there is also an "EDIT SAVE" option-- this one brings up a sub-screen where the user can use the D-Pad to move between each value in the save data array. Clicking up increments the value, down decrements it. Moving beyond a maximum or minimum wraps the value around in either direction. Moving left goes to the previous element, right to the next. Clicking the A button will just exit without saving changes and return to the main settings screen. And lastly, clicking B will exit and save any changes made to the array.

## Neural ROM Versions
As of right now, there are a few different versions of the "Neural ROM" we are developing. Each of these versions is based on a few different approaches one could take in analyzing data that is either missing or too complex to incorporate into this simulation. In the case of data that is too complex, certain synapses require a model that is many magnitudes more complex to simulate; these synapses are not as simple as the ionotropic synapses that make up the majority of the network used in this connectome. Instead they require modeling individual neurotransmitter-receptor interactions, with timers on those receptors, modelling the gradients of neurochemicals, and factoring in spatial locations of each neuron... While these are things that we want to address in later versions of this simulation, in this early-stage Arduboy simulation it is simply not possible. 

Similarly, we are not able to accurately model each and every gap junction in the network with our limited resources on the Arduboy. So complex synapses are not accounted for, and gap junctions are reduced in their anatomical complexity by a significant ammount. As for the missing data, there are many ionotropic synapses with unknown signs; that is, we do not know if they are excitatory or inhibitory...

In order to solve all these issues we are assembling datasets with the following attributes. If you are building from source, you are welcome to manually choose any of these other options:

* Unk_Comp_GJ: unknown sign synapses and complex synapses have a randomized sign (with 4:1 excitatory-inhibitory ratio), simplified unweighted gap junctions
* [CURRENT DEFAULT] Unk_Comp: unknown sign synapses and complex synapses have a randomized sign (4:1), gap junctions not included
* No_Unk_GJ: complex synapses have randomized signs (4:1), unknown sign synapses not included, simplified unweighted gap junctions
* No_Unk: complex synapses have randomized signs (4:1), unknown sign synapses not included, gap junctions not included
* No_Comp_GJ: unknown sign synapses have randomized signs (4:1), complex synapses not included, simplified unweighted gap junctions
* No_Comp: unknown sign synapses have randomized signs (4:1), complex synapses not included, gap junctions not included
* Simple_GJ: unknown signs and complex synapses are not included, simplified unweighted gap junctions
* Simple: unknown signs and complex synapses are not included, gap junctions not included

## Planned Additions
These are additions and changes I'd like to make to each app in order of importance ('!!' marks necessary TO-DOs). 
Not everything here we'll have room for, but I'll fit as much as I can into each application.

General:
* !! - Fix the broken gap junction version implementations

* !! - Worm movement seems to currently be non-functional (maybe because of missing gap junctions?)

Simulation:
* !! - Add collision logic for the rest of the rocks, leaves, and stick sprites, not just the pixel where it is printed at

* Add numbers to the simulation demarcations to show gas level and temperature readouts, indicate both goldilocks numbers and nociceptive numbers

* Create a cycle of where the leaves fall from the tree, blink after so many ticks, then disappear and drop food; when it is eaten the cycle repeats

* Create randomizers for toxin spawn position and sun/moon (if moon: black out some sun pixels, remove noxious surface heat and replace with cold, remove phototaxis)

* Make the worm's movement more biologically accurate (remove the "tank-drive" abstraction from current code), make doProprioception() more bio-accurate

Matrix:
* !! - Diag screen seems to have an error where it prints a random negative value in the weights list sometimes? Memory error of some kind? Off by one? This seems inconsistent???

* Add quick scrolling for the currently selected neuron

* Print out neuron IDs as irl names in the diag screen and the matrix screen

* Add an actual percpetron model in diag screen that labels the number of inputs

Options:
* Make it possible to do tonic activation over serial in the host function

* in the save editor add quick scrolling

## Installation
*BEFORE INSTALLING PLEASE READ:* If you have any save data on your arduboy, any other games or files, PLEASE move them off the Arduboy before putting *any* of this software on the device. This code, is not just extremely unstable, but it also *USES THE ENTIRE EEPROM* (except for the settings at the beginning) and as such it is not meant to be on an arduboy flashcart with anything except other tools made specifically for the AI as there is *NO ROOM FOR OTHER GAMES TO SAVE WITH THESE APPS* on the device. The current suite contains just the Matrix, Simulation, and Options Applications, so no other Arduboy games or programs that use its EEPROM are compatible.

Reccommended Method: uploading to your Arduboy using the hex files (*TBD*). Theres a few tools out there for this, but I recommend using like the official arduboy flashers like this https://community.arduboy.com/t/arduboy-web-flasher/11633.

From Source: You are welcome to try but I wouldn't do it for personal use. To install this from source, simply put the ArduNet folders in your arduino sketches folder, usually found in your documents, but you may have installed it elsewhere. You'll need the arduboy2 library and will also need to have the arduboy board added to your board manager, but other than that you should be able to select the board and port and upload it just like that. With some editing you could also probably get this to work on other arduino devices. I plan on uploading some code for that eventually...

## Neuron IDs
Each neuron in the network is based on one of the actual C. elegans' neurons. For the *Matrix Ver.*, when you scroll through the neurons in the *Matrix Screen* and interact with the synapses in the *Diagnostic Screen* it gives you a neuron's "ID".

I plan to make it so you can see which neuron this is soon, but for right now here is the list of neurons in the network organized by ID:

*  ADAL 0
*  ADAR 1
*  ADEL 2
*  ADER 3
*  ADFL 4
*  ADFR 5
*  ADLL 6
*  ADLR 7
*  AFDL 8
*  AFDR 9
*  AIAL 10
*  AIAR 11
*  AIBL 12
*  AIBR 13
*  AIML 14
*  AIMR 15
*  AINL 16
*  AINR 17
*  AIYL 18
*  AIYR 19
*  AIZL 20
*  AIZR 21
*  ALA 22
*  ALML 23
*  ALMR 24
*  ALNL 25
*  ALNR 26
*  AQR 27
*  AS1 28
*  AS10 29
*  AS11 30
*  AS2 31
*  AS3 32
*  AS4 33
*  AS5 34
*  AS6 35
*  AS7 36
*  AS8 37
*  AS9 38
*  ASEL 39
*  ASER 40
*  ASGL 41
*  ASGR 42
*  ASHL 43
*  ASHR 44
*  ASIL 45
*  ASIR 46
*  ASJL 47
*  ASJR 48
*  ASKL 49
*  ASKR 50
*  AUAL 51
*  AUAR 52
*  AVAL 53
*  AVAR 54
*  AVBL 55
*  AVBR 56
*  AVDL 57
*  AVDR 58
*  AVEL 59
*  AVER 60
*  AVFL 61
*  AVFR 62
*  AVG 63
*  AVHL 64
*  AVHR 65
*  AVJL 66
*  AVJR 67
*  AVKL 68
*  AVKR 69
*  AVL 70
*  AVM 71
*  AWAL 72
*  AWAR 73
*  AWBL 74
*  AWBR 75
*  AWCL 76
*  AWCR 77
*  BAGL 78
*  BAGR 79
*  BDUL 80
*  BDUR 81
*  CANL 82
*  CANR 83
*  CEPDL 84
*  CEPDR 85
*  CEPVL 86
*  CEPVR 87
*  DA1 88
*  DA2 89
*  DA3 90
*  DA4 91
*  DA5 92
*  DA6 93
*  DA7 94
*  DA8 95
*  DA9 96
*  DB1 97
*  DB2 98
*  DB3 99
*  DB4 100
*  DB5 101
*  DB6 102
*  DB7 103
*  DD1 104
*  DD2 105
*  DD3 106
*  DD4 107
*  DD5 108
*  DD6 109
*  DVA 110
*  DVB 111
*  DVC 112
*  FLPL 113
*  FLPR 114
*  HSNL 115
*  HSNR 116
*  I1L 117
*  I1R 118
*  I2L 119
*  I2R 120
*  I3 121
*  I4 122
*  I5 123
*  I6 124
*  IL1DL 125
*  IL1DR 126
*  IL1L 127
*  IL1R 128
*  IL1VL 129
*  IL1VR 130
*  IL2DL 131
*  IL2DR 132
*  IL2L 133
*  IL2R 134
*  IL2VL 135
*  IL2VR 136
*  LUAL 137
*  LUAR 138
*  M1 139
*  M2L 140
*  M2R 141
*  M3L 142
*  M3R 143
*  M4 144
*  M5 145
*  MCL 146
*  MCR 147
*  MI 148
*  NSML 149
*  NSMR 150
*  OLLL 151
*  OLLR 152
*  OLQDL 153
*  OLQDR 154
*  OLQVL 155
*  OLQVR 156
*  PDA 157
*  PDB 158
*  PDEL 159
*  PDER 160
*  PHAL 161
*  PHAR 162
*  PHBL 163
*  PHBR 164
*  PHCL 165
*  PHCR 166
*  PLML 167
*  PLMR 168
*  PLNL 169
*  PLNR 170
*  PQR 171
*  PVCL 172
*  PVCR 173
*  PVDL 174
*  PVDR 175
*  PVM 176
*  PVNL 177
*  PVNR 178
*  PVPL 179
*  PVPR 180
*  PVQL 181
*  PVQR 182
*  PVR 183
*  PVT 184
*  PVWL 185
*  PVWR 186
*  RIAL 187
*  RIAR 188
*  RIBL 189
*  RIBR 190
*  RICL 191
*  RICR 192
*  RID 193
*  RIFL 194
*  RIFR 195
*  RIGL 196
*  RIGR 197
*  RIH 198
*  RIML 199
*  RIMR 200
*  RIPL 201
*  RIPR 202
*  RIR 203
*  RIS 204
*  RIVL 205
*  RIVR 206
*  RMDDL 207
*  RMDDR 208
*  RMDL 209
*  RMDR 210
*  RMDVL 211
*  RMDVR 212
*  RMED 213
*  RMEL 214
*  RMER 215
*  RMEV 216
*  RMFL 217
*  RMFR 218
*  RMGL 219
*  RMGR 220
*  RMHL 221
*  RMHR 222
*  SAADL 223
*  SAADR 224
*  SAAVL 225
*  SAAVR 226
*  SABD 227
*  SABVL 228
*  SABVR 229
*  SDQL 230
*  SDQR 231
*  SIADL 232
*  SIADR 233
*  SIAVL 234
*  SIAVR 235
*  SIBDL 236
*  SIBDR 237
*  SIBVL 238
*  SIBVR 239
*  SMBDL 240
*  SMBDR 241
*  SMBVL 242
*  SMBVR 243
*  SMDDL 244
*  SMDDR 245
*  SMDVL 246
*  SMDVR 247
*  URADL 248
*  URADR 249
*  URAVL 250
*  URAVR 251
*  URBL 252
*  URBR 253
*  URXL 254
*  URXR 255
*  URYDL 256
*  URYDR 257
*  URYVL 258
*  URYVR 259
*  VA1 260
*  VA10 261
*  VA11 262
*  VA12 263
*  VA2 264
*  VA3 265
*  VA4 266
*  VA5 267
*  VA6 268
*  VA7 269
*  VA8 270
*  VA9 271
*  VB1 272
*  VB10 273
*  VB11 274
*  VB2 275
*  VB3 276
*  VB4 277
*  VB5 278
*  VB6 279
*  VB7 280
*  VB8 281
*  VB9 282
*  VC1 283
*  VC2 284
*  VC3 285
*  VC4 286
*  VC5 287
*  VC6 288
*  VD1 289
*  VD10 290
*  VD11 291
*  VD12 292
*  VD13 293
*  VD2 294
*  VD3 295
*  VD4 296
*  VD5 297
*  VD6 298
*  VD7 299
*  VD8 300
*  VD9 301
:D
## Credits
ArduNet Team
Izalit: (https://github.com/Izalit/) Lead programmer, designer, researcher, and project coordinator. Various iterations of this project have been my passion since my first year of college. I'm glad to finally have something to say for all the hard work. It was not possible without the help of everyone listed here. My hope is to use this as a platform to jump to more generalized AI projects later~!

Dinokaiz2: (https://github.com/Dinokaiz2/) My friend K, who made the compression tool and the reader for the neural ROM and the learning array, as well as for their assistance when I consulted them in logic and debugging particularly difficult issues. They've been an incredible friend and support while I've worked on this project over the years and I really cannot thank them enough for the incredible work they've done to make this possible.

Gwyndoll: (https://github.com/gwyndoll) My partner, Molly, who I would like to personally thank for all her help with debugging, building datasets, formatting large data files, and generally just keeping me sane while working on this. You're an incredible partner, parent, and programmer. Love ya molls.

Nova: Inspiration and consulation for this project and its goals. You'll likely hear more from them in the future.

Referenced Works
B. Nikhil: as their work on wormweb (http://wormweb.org/neuralnet#c=BAG&m=1) was used for finding connections between neurons and for debugging. I love their webtool it is amazingly rad.

B. Tahereh et al.: Their paper "Mechanisms of plasticity in a Caenorhabditis elegans mechanosensory circuit" helped identify the plastic neurons in the mechanosensory circuit to add to our hebbian learning array (https://www.frontiersin.org/journals/physiology/articles/10.3389/fphys.2013.00088/full)

C. Taegon et al.: Their paper "Optimization of connectome weights for a neural network model generating both forward and backward locomotion in C. elegans" helped give us a benchmark for existing biological simulations of C. elegans (https://www.biorxiv.org/content/10.1101/2025.07.21.665845v1.full)

EleganSign: A webtool by Fenyves et al. that allows the user to choose from multiple different data sets (https://elegansign.linkgroup.hu/#!Overview). We used their NT+R dataset, which used expression of neurotransmitters and receptors to predict synaptic weights. This was extremely useful, and alongside the other tools here was used to generate a list of data for our neural rom. Their datasets were an invaluable contribution!

G. Thomas et al.: Their paper "ASI Regulates Satiety Quiescence in C. elegans" was used to help better understand how satiety functions in C. elegans and, thus, how to implement it (https://pmc.ncbi.nlm.nih.gov/articles/PMC3726252/)

K. Oshio et al.: who made another great tool for looking at wiring, CCEP-Tool (http://ims.dse.ibaraki.ac.jp/ccep-tool/) which served useful as well during this project.

Nategri: (https://github.com/nategri/nematoduino) Creator of nematoduino, a project I played with a lot! The design of their neural ROM, the extrapolation of a tank-drive from sinusoidal muscle movements, and the general idea of putting C. Elegans into an Arduboy was hugely inspired by their arduino C. elegans project. It served as massive inspiration to see someone else using this brain for something cool!

Open Worm Project: Thanks goes to the people at open worm project, who make these curated data sets (https://openworm.org/ConnectomeToolbox/Witvliet_2021/) availabe and open source so a random bio nerd and hobby programmer like me could make this.

S. Attila et al.: Their paper "Neuron-Specific Regulation of Associative Learning and Memory by MAGI-1 in C. elegans" helped me understand how the MAGI-1 protein is used in learning which, in turn, helped identify more neurons for our hebbian array (https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0006019)

T. Jamine Hooi-Min et al.: Their paper "Behavioral Forgetting of Olfactory Learning Is Mediated by Interneuron-Regulated Network Plasticity in Caenorhabditis elegans" helped identify plastic neurons in the olfactory circuit to add to the hebbian learning array (https://www.eneuro.org/content/9/4/ENEURO.0084-22.2022)

Worm Atlas: who has a wonderful website (https://www.wormatlas.org/index.html) with tons of references for each individual neuron! Very useful for identifying which neurons to use in the hebbian array. I love all that they've done, and highly reccomend checking it out.


. . .

Lastly, to my friends Molly, Kaiz, and Nova, all of you, thanks so much for your help with this crazy ambitious project, with debugging and discussing the logic and talking it out. Simply just being rubber ducky was immensely helpful, but so has all that you've done for me while I've been struggling with the pain and mental toll of my illness these past few years. Your friendship and your wonderful brains are more valuable than gold!
