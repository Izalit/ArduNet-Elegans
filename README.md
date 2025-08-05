
# ArduNet Elegans

This project is a little attempt of mine to try to fit a simplified neural network based on the neurons inside the model organism C. Elegans, a small nematode that lives in soils with only 302 neurons, into the Arduboy. I intend to make a little digital pet simulator too, so you can take your worm friend with you wherever you go!

. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 



DISCLAIMER! 

[so. it turns out that debugging a brain is really really hard, and while I built a tool into this that allows me to scroll through the network and verify the neurons are read properly from the neural rom (the diagnostic screen which I decided to keep because it is cool af), actually telling if its crunching numbers correctly is really hard, and so I am currently in the process of making small neural circuits with a more easily distinguishable output so I can verify that the small-world network that is the C. elegans brain is implemented with 100% accurate activation function calculation. Regardless, for small circuits it has been demonstrated to be functional so at least something works!]
## Philosophy
While 302 neurons may seem like a puny or insignificant network to many, this guy is capable of a lot, and I wanted to make a program on arduboy that could help people understand just how mindbogglingly complicated even a one millimeter worm actually is. We often abstract away just how complex even a single neuron is, and many people do not know that the computational power of just one biological neuron is hundreds times more than the digital ones we often use... So it became my goal to try to make something that allowed people to understand the complexity and depth and value of even the most seemingly insignificant thing, all while challenging myself to make a tiny brain capable of learning fit onto an Arduboy.
## Features and Use
The ardunet elegans project features, currently, very limited user interaction. It mainly functions to showcase the worm's brain being inside the arduboy and reacting, and not much else, as of right now. I plan to change this, but regardless, currently here is how it works.


STARTUP: On startup an "ardunet elegans" splash screen pops up, then it switches to the main *matrix screen*. This is only on startup.

MATRIX SCREEN: In this screen you'll find a few things. Some text in the top left indicating that the big box on the right is a matrix of neurons. This box is empty by default as the network initializes with all neurons in an OFF state, so the matrix is blank. But once you give it an input by pressing both A and B you'll see it come alive! There is also some text in the bottom talking about the direction of the worm, forward - backward - left - right; this is how the worm is moving in response to inputs. Lastly there is a number below the matrix, this is the *selected neuron*. Clicking the A and B buttons scrolls through this number. When you click left on the D-Pad you switch to the *Diagnostic Screen*. Alternatively, if you click right on the D-Pad you go to the *Input Select Screen*. 

As mentioned before, clicking both A and B simultaneously allows the user to interact with the network, activating a sense, and turning some neurons on, starting a chain reaction that activates much of the network. The matrix shows the neurons in the network in order of their ID's in the program. I will list these below; later I intend to actually print out the neurons name (usually four letters, read more about these here: https://www.wormatlas.org/neurons/Individual%20Neurons/Neuronframeset.html). 

What's really cool is that if you look at the matrix you can see some neurons firing in a pattern, and if you select different input senses from the *Input Select Screen* you can see some of these changing. As of right now, you can only see one main output-- the direction the worm is moving. I've done some math with muscle ratios so you can roughly tell how the worm is currently moving-- forward, backward, left, and right. That's it right now, but it is interesting to see the worm change in response to you.

Dianostic Screen: This screen allows you to see information about the neuron that was selected in the matrix screen. Up on top it will tell you the selected neuron and the number of synapses connected to it. For each of these synapses the selected neuron is the "post-synaptic cell", and each connected neuron is the "pre-synaptic cell". Listed under the number of presynaptic cells you will see the weight of the connection. This number was calculated based on the real worm's data. You can think of it kind of like how much oomf the pre-synaptic cell is giving to the post-synaptic cell, how much it is trying to make it fire, or, on the other hand, how much it is trying to keep it from firing! If the number is positive then the cell connected to the selected neuron is trying to get it to fire, it's an "excitatory connection. If the number is negative it's trying to keep it from firing; what we call an "inhibitory connection". 

You can scroll through each these synapses using the A and B buttons, the Pre ID will change, as this is the ID of the presynaptic neuron. The postID will stay the same, as this is selected neuron. Finally, on the bottom right it will show you a diagram of the neuron connected to the synapses-- if the neuron is filled in that means it just fired! If it is empty, then it has not.

Input Select Screen: This simple screen lets you choose between three input modalities for when you press the A and B buttons in the *Matrix Screen* simply press A to scroll through the list, and whichever sense the cursor is on is the sense that will be used when pressing the buttons in the *Matrix Screen*. From here just press left or up to go to the other screens.

...and right now that's it. I plan to do a lot more in the future. I actually want to add a whole new screen as the main screen later. A whole simulation that works kinda like a digital pet. But we'll see if that ends up happening. The update that allows the worm to learn should be coming out soon though so stay tuned!

## Neural ROM Versions
As of right now, there are a few different versions of the "Neural ROM", the data specifying the edges and weights of the neural network. Each of these versions is based on a few different approaches one could take in analyzing data that is either missing or too complex to incorporate into this simulation. In the case of data that is too complex, certain synapses require a model that is many magnitudes more complex to simulate; these synapses are not as simple as the ionotropic synapses that make up the majority of the network used in this connectome. Instead they require modeling individual neurotransmitter-receptor interactions, with timers on those receptors, modelling the gradients of neurochemicals, and factoring in spatial locations of each neuron... While these are things that we want to address in later versions of this simulation, in this early-stage Arduboy simulation it is simply not possible. 

Similarly we are not able to model each and every gap junction in the network with our limited resources on the Arduboy. So complex and electrical synapses are not accounted for, except in the cases of a few specific gap junctions. As for the missing data, there are many ionotropic synapses with unknown signs; that is, we do not know if they are excitatory or inhibitory...

In order to solve all these issues we are assembling datasets with the following attributes:

* Default [Unk_Comp_GJ]: unknown sign synapses and complex synapses have a randomized sign (with 4:1 excitatory-inhibitory ratio), relevant gap junctions approximated
* Unk_Comp: unknown sign synapses and complex synapses have a randomized sign (4:1), relevant gap junctions not included
* No_Unk_GJ: complex synapses have randomized signs (4:1), unknown sign synapses not included, relevant gap junctions approximated
* No_Unk: complex synapses have randomized signs (4:1), unknown sign synapses not included, relevant gap junctions not included
* No_Comp_GJ: unknown sign synapses have randomized signs (4:1), complex synapses not included, relevant gap junctions approximated
* No_Comp: unknown sign synapses have randomized signs (4:1), complex synapses not included, relevant gap junctions not included
* Simple_GJ: unknown signs and complex synapses are not included, relevant gap junctions approximated 
* Simple: unknown signs and complex synapses are not included, relevant gap junctions not included

## Planned Additions

I plan to do a few key things with this in the future, some crazier than others.. if I can fit it in the program. Some of these may not be possible on arduboy, given the constraints, but I'll add as much as I can with that in mind. But since space is tight given the huge arrays of neuron data and few thousand synapses... well, it'll be a challenge! Regardless the following features are things I want to try to add:

* Add multiple different data set options for Neural ROM

* Initialize the direction the worm is in as "stationary" so it doesn't start in "forward" [not yet implemented; high priority]

* Make neuron IDs associated with their irl names so you can see them in diagnostic and not have to look at this huge github list lmao [not yet implemented; high priority]

* Implement the hebbian long term potentiation and long term depression functions for learning, now that dinokaiz2 made more room for the new arrays [partial implementation; high priority]

* Saving the hebbian array to eeprom, saves on a timer? or with button? not sure yet, but want the worm to "remember" interactions and adjust certain weights based on firing frequency as mentioned above, so I need some way to retain these through powercycle. I plan to look into what other people use for arduboy game saves. [not yet implemented; high priority]

* A full blown simulation, accessed by pressing the down button... it would be a bordered screen wherein the user can interact with the nematode as a digital pet of sorts. [partial implementation; low priority]

* Silly idea, but important for the digital pet aspect. I want to give it cute expressions that are activated similarly to muscle output based on neurons that effect behaviors (ex. make it "scared" when doing a quick reversal doing a O_O face, or "happy" when doing foraging behaviors and so it does a ^_^ face). Have already chosen which expressions and neurons to use. [not yet implemented; low priority]

* More complex and complete list of sensory modalities, specifically for use in the simulation, but also potentially just in the *Input Screen* selections [partial implementation; medium priority]

* Simulation gradients. Temperature, soil oxygen and CO2 composition, all which change depending on the position the worm occupies on the screen, and which would be inputs into the network. [not yet implemented; low priority]

* Simulation screen borders and randomly generated leaves or sticks or debris that the worm needs to navigate around. When colliding with the nematode it would activate the mechanosensory neurons corresponding to the area on the worm body it collides with [not yet implemented; low priority]

* A cursor for the user in the simulation, thinking of making it have its own gradient centered on the cursor. The gradient may activate chemical markers the worm would use to perceive another worm, so it would treat you as a worm? Alternatively might make you salty, like a real human, which I think it would be more social around given its behavior, not sure... Regardless I want the user to be able to place food down and "nap" so you can see its social behaviors. [not yet implemented; low priority]

## Installation
* (Reccommended Method) uploading to your arduboy using the hex file. Theres a few tools out there for this, but I recommend using like the official arduboy flashers like this https://community.arduboy.com/t/arduboy-web-flasher/11633.

* To any developers I'm sure you probably know how to do this, but if not you are welcome to try but I wouldn't do it for personal use... But to install this from source, simply put the ArduNet folder in your arduino sketches folder, usually found in your documents, but you may have installed it elsewhere. You'll need the arduboy2 library and will also need to have the arduboy board added to your board manager, but other than that you should be able to upload it just like that. With some editing you could also probably get this to work on other arduino devices. I plan on uploading some code for that eventually, might make a generalized library too, we'll see.
## Neuron IDs
Each neuron in the network is based on one of the actual C. elegans' neurons. When you scroll through the neurons in the *Matrix Screen* and interact with the synapses in the *Diagnostic Screen* it gives you a neuron's ID.

I plan to make it so you can see which neuron this is soon, but for right now here is the list:

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

https://github.com/Dinokaiz2/: Dinokaiz2, who made edits to make the code more efficient and functional, as well as for their assistance when I consulted them in logic and debugging the activation function. They've been an incredible friend and support while I've worked on this project over the years and I really cannot thank them enough.

EleganSign: A webtool by Fenyves et al. that allows the user to choose from multiple different data sets (https://elegansign.linkgroup.hu/#!Overview). We used their NT+R dataset, which used expression of neurotransmitters and receptors to predict synaptic weights. This was extremely useful, and alongside the other tools here was used to generate a list of data for our neural rom. Their datasets were an invaluable contribution!

Gwyndoll: (https://github.com/gwyndoll) who helped with building datasets and debugging. You're a great friend, thanks so so much!

K. Oshio et al.: who made another great tool for looking at wiring, CCEP-Tool (http://ims.dse.ibaraki.ac.jp/ccep-tool/) which served useful as well during this project.

https://github.com/nategri/nematoduino: The neural rom design and idea of putting C. Elegans was hugely inspired by nategri's nematoduino, so thank you for the cool project, it served as massive inspiration for putting a brain on arduboy!

Nikhil Bhatla: as their work on wormweb (http://wormweb.org/neuralnet#c=BAG&m=1) was used for finding connections between neurons and for debugging. I love their webtool it is amazingly rad.

Open Worm Project: Thanks goes to the people at open worm project, who make these curated data sets (https://openworm.org/ConnectomeToolbox/Witvliet_2021/) availabe and open source so a random programmer like me could make this.

Worm Atlas: who has a wonderful website (https://www.wormatlas.org/index.html) with tons of references for each individual neuron! I love all that they've done, and highly reccomend checking it out.

. . .

I would also like to personally thank my partner, Molly, for all her help with debugging, formatting large data files, and generally just keeping me sane while working on this. You're an incredible partner, parent, and programmer. Love ya molls.

Lastly, to my friends Molly, Kaiz, and Nova, all of you, thanks so much for your help with this crazy ambitious project, with debugging and discussing the logic and talking it out, simply just being rubber ducky was immensely helpful, but so has all that you've done for me while I've been struggling with my illness these past few years. Your friendship and your wonderful brains are more valuable than gold!
