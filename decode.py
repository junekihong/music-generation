#!/bin/python

import sys
from pprint import pprint

filename = sys.argv[1]
f = open(filename)

lines = []
for line in f:
    lines.append(line)
f.close()

    
filename = filename.split("/")[-1]
filename = ".".join(filename.split(".")[:-1]) + ".decoded"
directory = "decoded/"
print filename
f = open(directory + filename, "w")

f.write("#track 1\n")

time = 0
for line in lines:
    for word in line.split():
        word = word.strip()
        if "timesig" in word or "keysig" in word or "mode" in word:
            word = "TW0 V- " + word
        else:
            word = word.split("|")

            # Key
            word[1] = "K" + word[1]
            # Pitch
            word[2] = "P" + word[2]
            # Duration
            word[3] = "Q" + word[3]
            # Gate
            word[4] = "L" + word[4]
            
            word = word[:1] + ["V1"] + word[1:]
            
            delta = float(word[0])
            newtime = time + delta
            word[0] = str(newtime)
            time = newtime
            
            
            word[0] = "TW"+word[0]
            word = " ".join(word)
        
            
            
        f.write(word + "\n")
f.close()
