const uint8_t PROGMEM perceptronOFF[] = {
30, 21,
0x0e, 0x11, 0x11, 0x11, 0x1e, 0x10, 0x20, 0x20, 0x20, 0x60, 0xe0, 0x40, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x0e, 0x11, 0x11, 0x11, 0x0e, 0x04, 0x84, 0x84, 0x8e, 0xdf, 0xe0, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x20, 0x1f, 0x04, 0x04, 0x04, 0x0e, 0x0e, 0x11, 0x11, 0x11, 0x0e, 
0x0e, 0x11, 0x11, 0x11, 0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const uint8_t PROGMEM perceptronON[] = {
30, 21,
0x0e, 0x11, 0x11, 0x11, 0x1e, 0x10, 0x20, 0x20, 0x20, 0x60, 0xe0, 0xc0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x0e, 0x11, 0x11, 0x11, 0x0e, 0x04, 0x84, 0x84, 0x8e, 0xdf, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x1f, 0x04, 0x04, 0x04, 0x0e, 0x0e, 0x11, 0x11, 0x11, 0x0e, 
0x0e, 0x11, 0x11, 0x11, 0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

/*************************NOTES*****************************/

/**
 *                         References
 * http://ims.dse.ibaraki.ac.jp/ccep-tool/
 * https://www.sciencedirect.com/science/article/pii/S0960982205009401
 * https://pubmed.ncbi.nlm.nih.gov/40666838/
 * https://www.nature.com/articles/s42003-021-02561-9
 * https://www.wormatlas.org/hermaphrodite/nervous/Neuroframeset.html
 * https://www.wormatlas.org/neurons/Individual%20Neurons/ASIframeset.html
 * https://www.ncbi.nlm.nih.gov/books/NBK19787/
 * https://www.science.org/doi/10.1126/science.aam6851
 * https://www.cell.com/current-biology/fulltext/S0960-9822(14)01501-2
 */

 //escape behavior - fear (O_O)
 //slow reversal - indecisive (-_n)
 //foraging behavior - happy (^v^)
 //attractive chemosensation - content (^-^)
 //repulsive chemosensation - disgust? (-x-)
 //noxious temp, harsh touch, bad air - pain/discomfort (>.<)
 //temperature shock response - (@_@)
 //lethargy, social sleeping - sleepy (UwU)
 //social feeding - ???
 //solitary feeding - ???
 //egg laying - ???

/**
 *                     SENSORY INFO
 *
 * Gentle Nose Touch (suppress lateral foraging, head withdrawal, escape behavior)- OLQ, IL1, FLP
 * Gentle Anterior Body Touch (causes backwards movement) - ALM, AVM
 * Gentle Posterior Body Touch (causes forward movement) - PLM
 * Harsh Body Touch (causes reversal) - PVD, potentially also ALM
 * Harsh Head/Nose Touch - FLP, ASH, potentially also OLQ
 * Osmomtic Pressure - potentially ASH and ADL
 * Texture Sensation (sense small round objects, food-induced slowing foraging behavior) - CEP, ADE, PDE
 * Proprioception (stretch and tension during movements) - DVA, potentially PHC and PVD
 * Nociception - ASH
 * Attractive Chemosensation - ADF, ASE (primary sensor), ASG, ASI, ASJ, ASK
 * Repulsive Chemosensation - ASH (primary nociceptor), ADL, ASK, ASE
 * ??? chemo - AWA, AWB, AWB
 * Thermosensation - AFD (primary sensor - seeks cooler temp), AWC (seeks warmer temp), ASI
 * Noxious Cold Thermosensation - PVD (acute cold-shock)
 * Noxious Heat Thermosensation (prolonged 30*C causes heat-shock response/thermal avoidance) - AFD (head), FLP (head), and PHC (tail)
 * Photosensation (photoboic movement reversal response) - potentially VNC but largely unknown
 * Oxygen sensation (nociceptive) - AQR, PQR, URX, possibly SDQ, ALN, BDU, ADF, and ASH
 * Carbon Dioxide sensation (nociceptive, particularly when well-fed) - FD, BAG, and AE
 */