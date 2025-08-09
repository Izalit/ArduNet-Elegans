import pandas as pd	
import random
import sys
from sys import platform
import os




path = os.path.dirname(__file__)
neuronDataFilePath = (path+"/./neuronlist.csv") #made by us from worm database
synapseDataFilePath = (path+"/./nt+rdataset.csv") #from elegansign
outputFilePath = (path+"/./ntrweightvalsupdatedTEST.txt") #default is ntrweightvalsupdated.txt


neuronData = pd.read_csv(neuronDataFilePath)
synapseData = pd.read_csv(synapseDataFilePath)
synapsedf = pd.DataFrame(synapseData, columns=['Source','Target','Sign','Edge Weight'])
neurondf = pd.DataFrame(neuronData, columns=['neuron'])
neurons = neurondf['neuron'].tolist()
source = synapsedf['Source'].tolist()
target = synapsedf['Target'].tolist()
sign = synapsedf['Sign'].tolist()
weight = synapsedf['Edge Weight'].tolist()


threshold = 1
rand = [1, 1, 1, 1, -1] # 4:1 postive to negative ratio
noPredRand = random.sample(range(1415), 212)
weightadj = 1

#desired ratios: 1203 of no pred to be postive, 212 to be negative
#        377 of complex to be postive, 94 to be negative

# uncomment this when you want to write to file
sys.stdout = open(outputFilePath, 'w')

def printData(inputIDs, neuron, weight): #prints are formatted for neuralROM in ardunet currently  
    #print(neuron)
    #print(neurons.index(neuron)) #cellID is index of neurons listed in neuronswithzero.csv
    #print(threshold) #threshold
    
    print(str(len(inputIDs)) + ', ', end = '') # input length
                
    for inputs in inputIDs:
        print(str(inputs) + ', ', end = '') # inputs
                    
    #print(len(weight)) # weight length 
                
    for widx, w in enumerate(weight):
        adjustedWeight = int(w/weightadj) # convert double to int ONLY while weight adjust equals 1
        if widx < len(weight) - 1: #kinda dumb way to trim space off last line
            print(str(adjustedWeight) + ', ', end = '') #each iteration of loop has a comma then space for readability
        else:
            print(str(adjustedWeight) + ',', end = '\n')

    #print(checkRatios)
    #print(0) #output value

def generateStructs():
    #totalSynapses = 0 # for determining weight ratios
    for neuron in neurons: #iterate over list of neurons
        inputIDs=[] #list (array) of cellID's that input (source) a particular neuron (target)
        weightList = [] #list of weights for each synapse between the source and target neuron
        weightNoPred = []
        weightComplex = []
        
        for tidx, t in enumerate(target): #iterating of target neuron list, tidx is index, t is target neuron
            if neuron == t: #if          
                sourceSynapseElement = source[tidx] #stores the source neuron for target neuron
                synapseSign = sign[tidx] #stores sign
                synapseWeight = weight[tidx] #stores weight of synapse
                if sourceSynapseElement in neurons: #compares source neurons in newdataset.csv to list of neurons in neuronswithzero.csv and trims any that arent a part of that list
                    cellID = neurons.index(sourceSynapseElement) 
                    inputIDs.append(cellID) #adds source neuron cellID to inputs list
                    #totalSynapses += 1 

                    if synapseSign == '-': # adds synapse sign to weight values
                        synapseWeight = synapseWeight * -1
                        weightList.append(synapseWeight)
                        
                    elif synapseSign == 'no pred':
                        synapseWeight = synapseWeight #* random.choice(rand) #no pred and complex both use random to add sign, ratio of + to - stated in rand
                        weightNoPred.append(synapseWeight)
                        
                    elif synapseSign == 'complex':
                        synapseWeight = synapseWeight #* random.choice(rand) 
                        weightComplex.append(synapseWeight)
                        
                    else: 
                        weightList.append(synapseWeight)           
        
        printData(inputIDs, neuron, weightList)
        
    
# Total: 3638
# Positive: 1327
# Negative: 425
# No Pred: 1415
# Complex: 471

def checkRatios(): 
    numPos = 0
    numNeg = 0
    numNoPred = 0
    numCom = 0
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

    #return signIteration    

#checkRatios()
generateStructs()
#print(noPredRand)
#def weighted_choice(odds1, outcome1, odds2, outcome2): #unused odds functions
#        if random.randint(1, odds1 + odds2) <= odds1:
#            return outcome1
#        else:
#           return outcome2