#!/usr/bin/env python

from __future__ import print_function

import argparse
import sys
import librosa
import json


def main():
    args = process_arguments(sys.argv[1:])
    dFeatures = estimate_features(args)

    dFeatures["file"] = args["input_file"]
    
    if args["print"]:
        print(dFeatures) 
    
    if args["round"]:
        round_features(dFeatures)

    if args["save"]:
        saveJson(dFeatures)
    
def process_arguments(args):
    parser = argparse.ArgumentParser(description='Feature Extractor')

    parser.add_argument("-r", "--round",
                        action = "store_true",
                        help='round to 4 decimals')

    parser.add_argument("-p", "--print",
                        action = "store_true",
                        help='print result to console')
    
    parser.add_argument("-s", "--save",
                        action = "store_true",
                        help='save json')

    parser.add_argument("input_file",
                        action='store',
                        help='path to the input file')

    parser.add_argument("-ws", "--window_size",
                        action='store',
                        default=4096,
                        help='window size (default: 4096)')

    parser.add_argument("-hl", "--hop_length",
                        action = 'store',
                        default = 1024,
                        help='hop length (default: 1024)')
    

    return vars(parser.parse_args())

def estimate_features(args):
    print("Processing " + args["input_file"])

    wS = args["window_size"]
    hL = args["hop_length"]
    y, sr = librosa.load(args["input_file"], sr=None)
    S, phase = librosa.magphase(librosa.stft(y=y, n_fft=wS))
    
    
    d = {
        'flatness' : librosa.feature.spectral_flatness(S=S, n_fft=wS, hop_length=hL)[0].tolist(),
        'rms' : librosa.feature.rmse(S=S, frame_length=wS, hop_length=hL)[0].tolist(),
        'centroid' : librosa.feature.spectral_centroid(S=S, sr=sr, n_fft=wS, hop_length=hL)[0].tolist(),
        'rollOff' : librosa.feature.spectral_rolloff(S=S, sr=sr, n_fft=wS, hop_length=hL)[0].tolist(),
        'bandWidth': librosa.feature.spectral_bandwidth(S=S, sr=sr, n_fft=wS, hop_length=hL)[0].tolist()
        }
    
    length = wS
    lLength = []
    lPosition = []
       
    for i in range(len(d['flatness'])):
        lLength.append(length)
        lPosition.append(hL * i)

    d["length"] = lLength
    d["time"] = lPosition
    
    return d

def round_features(dFeatures):
    for key in dFeatures:
        if type(dFeatures[key]) == list:
            for i, s in enumerate(dFeatures[key]):
                dFeatures[key][i] = round(dFeatures[key][i], 5)
          
def saveJson(dFeatures):
    numSamples = len(dFeatures['flatness'])
    jArray = []
    
    for i in range(numSamples):
        jObject = {
            'file' : dFeatures["file"],
            }
        
        for key in dFeatures:
            if type(dFeatures[key]) == list:
                jObject[key] = dFeatures[key][i]
            else:
                jObject[key] = dFeatures[key]
        jArray.append(jObject)

    fileName = dFeatures["file"].split('.')[0]+'.json'

    features = {'samples' : jArray}
    
    with open(fileName, 'w') as outfile:
        j = json.dump(features, outfile)
        
    print("saved to " + fileName)
    
main()
