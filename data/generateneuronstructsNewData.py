#author @gwyndoll
import pandas as pd	
import random
import sys
from sys import platform
import os


path = os.path.dirname(__file__)
neuronDataFilePath = (path+"/./inputdata/neuronlist.csv") #made by us from worm database
synapseDataFilePath = (path+"/./inputdata/nt+rdataset.csv") #from elegansign
gapjunctionFilePath = (path+"/./inputdata/gapjunctionlist.csv")
#outputFilePath = (path+"/./datasets/Unk_Comp_GJ.txt") #default is Unk_Comp_GJ.txt
#outputFilePath = (path+"/./datasets/No_Unk_GJ.txt")
#outputFilePath = (path+"/./datasets/No_Comp_GJ.txt")
#outputFilePath = (path+"/./datasets/Simple_GJ.txt")
outputFilePath = (path+"/./datasets/test.txt")

neuronData = pd.read_csv(neuronDataFilePath)
synapseData = pd.read_csv(synapseDataFilePath)
gjunctData = pd.read_csv(gapjunctionFilePath)
synapsedf = pd.DataFrame(synapseData, columns=['Source','Target','Sign','Edge Weight'])
neurondf = pd.DataFrame(neuronData, columns=['neuron'])
gjunctdf = pd.DataFrame(gjunctData, columns=['source', 'target','synapses'])
neurons = neurondf['neuron'].tolist()
source = synapsedf['Source'].tolist()
target = synapsedf['Target'].tolist()
sign = synapsedf['Sign'].tolist()
weight = synapsedf['Edge Weight'].tolist()
gjsource = gjunctdf['source'].tolist()
gjtarget = gjunctdf['target'].tolist()
gjweight = gjunctdf['synapses'].tolist()

# uncomment this when you want to write to file
sys.stdout = open(outputFilePath, 'w')

weightadj = 1

def printData(inputIDs, neuron, weight): #prints are formatted for neuralROM in ardunet currently  
    #print(neuron)
    #print(neurons.index(neuron)) #cellID is index of neurons listed in neuronswithzero.csv
    
    print(str(len(inputIDs)) + ', ', end = '') # input length
                
    for inputs in inputIDs:
        print(str(inputs) + ', ', end = '') # inputs
                    
    #print(len(weight)) # weight length 
                
    for widx, w in enumerate(weight):
        print(str(w) + ', ', end ='')
        
        #adjustedWeight = int(weightadj) # convert double to int ONLY while weight adjust equals 1
        #if widx < len(weight) - 1: #kinda dumb way to trim space off last line
        #    print(str(adjustedWeight) + ', ', end = '') #each iteration of loop has a comma then space for readability
        #else:
        #    print(str(adjustedWeight) + ',', end = '\n')
    print()

def generateStructs():
    #totalSynapses = 0 # for determining weight ratios
    for neuron in neurons: #iterate over list of neurons
        inputIDs=[] #list (array) of cellID's that input (source) a particular neuron (target)
        weightList = [] #list of weights for each synapse between the source and target neuron
        
        for tidx, t in enumerate(target): #iterating of target neuron list, tidx is index, t is target neuron
            if neuron == t: #if          
                sourceSynapseElement = source[tidx] #stores the source neuron for target neuron
                synapseSign = sign[tidx] #stores sign
                synapseWeight = weight[tidx] #stores weight of synapse
                if sourceSynapseElement in neurons: #compares source neurons in nt+rdataset.csv to list of neurons in neuronlist.csv and trims any that arent a part of that list
                    cellID = neurons.index(sourceSynapseElement) 
                    inputIDs.append(cellID) #adds source neuron cellID to inputs list
                    #name = sourceSynapseElement
                    #inputIDs.append(name)
                    
                    if synapseSign == '-': # adds synapse sign to weight values
                        synapseWeight = synapseWeight * -1
                        weightList.append(synapseWeight)
                        
                    elif synapseSign == 'no pred':
                        synapseWeight = synapseWeight * weightedChoice(1203,1,212,-1) #* 0 #if not included
                        weightList.append(synapseWeight)
                        
                    elif synapseSign == 'complex':
                        synapseWeight = synapseWeight * weightedChoice(377,1,94,-1) #* 0 #if not included
                        weightList.append(synapseWeight)
                        
                    else: 
                        weightList.append(synapseWeight)           
        
        #mark the gap junctions with 9 in the tens (90-99)
        #0-9 in the ones place indicates its weight (the number of synapses is the weight)
        #if a gap junctions weight is greater than 9 just set it to 9 anyways (that’s our max)
        for gidx, g in enumerate(gjtarget):
            if neuron == g:
                sourcegjElement = gjsource[gidx]
                gjWeight = gjweight[gidx]
                if sourcegjElement in neurons:
                    gjcellID = neurons.index(sourcegjElement)
                    inputIDs.append(gjcellID)
                    #names = sourcegjElement + 'gj'
                    #inputIDs.append(names)

                    if gjWeight < 10:
                        gjWeight += 90
                        weightList.append(gjWeight)
                    else:
                        gjWeight = 99
                        weightList.append(gjWeight)

        printData(inputIDs, neuron, weightList)
        
    
# Total: 3638
# Positive: 1327
# Negative: 425
# No Pred: 1415
# Complex: 471
# Desired ratios: 1203 of no pred to be postive, 212 to be negative
#        377 of complex to be postive, 94 to be negative
def checkRatios(): 
    numPos, numNeg, numNoPred, numCom = 0,0,0,0
    signIteration = [] # list to add to check sign ratios
    
    for s in sign: 
        if s == '-': 
            numNeg += 1
        elif s == 'no pred':
            numNoPred += 1
        elif s == 'complex':
            numCom += 1
        else:
            numPos += 1 
    signIteration.append(numPos)
    signIteration.append(numNeg)
    signIteration.append(numNoPred)
    signIteration.append(numCom)
    print(signIteration)  

#function by @dinoceros
def weightedChoice(odds1, outcome1, odds2, outcome2): 
        if random.randint(1, odds1 + odds2) <= odds1:
            return outcome1
        else:
           return outcome2
        
#checkRatios()
generateStructs() 
