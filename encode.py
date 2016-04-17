#!/bin/python

import sys
from pprint import pprint

filename = sys.argv[1]
f = open(filename)

lines = []
tempo = []
other = []
channels = {}


for line in f:
    #if line[0] == "#":
    #    continue
    line = line.strip()
    if not line:
        continue

    if "-tempor" in line:
        tempo.append(line)
        continue
    if "-texts:" in line or "-names" in line or "-copyrights:" in line or "-markers" in line:
        other.append(line)
        continue

    if "V" in line:
        index = line.split()[1]
        channels[index] = channels.get(index, [])
        channels[index].append(line)
    
    lines.append(line)
f.close()





#lines = sorted(lines, key=lambda line: float(line.split()[0][2:]))
#pprint(sorted(lines)[:10])




#filename = ".".join((filename.split("/")[-1]).split(".")[:-1]) + ".groprocessed"


filename = filename.split("/")[-1] +  ".encoded"
directory = "encoded/"
print filename
f = open(directory + filename,"w")


#f.write("#track 0\n")
#for line in tempo:
#    f.write(line + "\n")


#f.write("#track 1\n")

channel_meta = {}
time = 0
for line in sorted([item for sublist in channels.values() for item in sublist], key=lambda x: float(x.split()[0][2:])):
    channel = line.split()[1]
    channel_meta[channel] = channel_meta.get(channel, {})
    if "-programi" in line:
        channel_meta[channel]["-programi"] = line
        continue
    elif "-control" in line or "-bendr" in line:
        ID = line.split()[2]
        channel_meta[channel][ID] = line
        continue

    line = line.split()
    if not channel == "V-":
        line[1] = "V1"

    # Remove channel information
    line = line[:1] + line[2:]

    # Handle meta events (time signature, key signature, etc)
    if "timesig" in line[1] or "keysig" in line[1] or "mode" in line[1]:
        line = [line[1]]
    else:
        # Timestamp
        line[0] = line[0][2:]
        newtime = float(line[0])
        delta = newtime - time
        round(delta*1000000)/1000000
        line[0] = str(delta)
        time = newtime

        # Key
        line[1] = line[1][1:]
        # Pitch
        line[2] = line[2][1:]
        # Duration
        line[3] = line[3][1:]
        # Gate
        line[4] = line[4][1:]
        
        
    line = "|".join(line)
    #print line
    f.write(line + " ")

    
#f.write("#track 1\n")
#for line in channels["V1"]:
#    f.write(line + "\n")


#f.write("#track 2\n")
#for line in channels["V2"]:
#    f.write(line + "\n")


#for line in lines:
#    f.write(line + "\n")
f.close()
