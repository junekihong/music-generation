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
        word = word.strip().split("|")
        delta = float(word[0])
        newtime = time + delta
        word[0] = str(newtime)
        time = newtime
        
        if word[5] == "1":
            f.write("TW" + word[0] + " V- " + "-timesig_numr:" + word[1] + "\n")
            f.write("TW" + word[0] + " V- " + "-timesig_denr:" + word[2] + "\n")
            f.write("TW" + word[0] + " V- " + "-keysigi:" + word[3] + "\n")
            if word[4] == "0":
                word[4] = "\'majora\'"
            elif word[4] == "1":
                word[4] = "\'minora\'"
            f.write("TW" + word[0] + " V- " + "-modea:" + word[4] + "\n")
        elif word[5] == "0":
            # Key
            word[1] = "K" + word[1]
            # Pitch
            word[2] = "P" + word[2]
            # Duration
            word[3] = "Q" + word[3]
            # Gate
            word[4] = "L" + word[4]
            word = word[:1] + ["V1"] + word[1:]
            
            word[0] = "TW"+word[0]
            word = " ".join(word[:5])
            f.write(word + "\n")
f.close()
