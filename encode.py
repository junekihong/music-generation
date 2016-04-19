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
    if "-instruments:" in line:
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
timesigN,timesigD,keysig,mode = None,None,None,None

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

    # Timestamp
    line[0] = line[0][2:]
    newtime = float(line[0])
    delta = newtime - time
    round(delta*1000000)/1000000
    line[0] = str(delta)
    time = newtime
    
    # Handle meta events (time signature, key signature, etc)
    if "timesig" in line[1] or "keysig" in line[1] or "mode" in line[1]:
        #line = [line[1]]
        value = line[1].split(":")[1].strip()
        if "timesig_numr" in line[1]:
            timesigN = value
        elif "timesig_denr" in line[1]:
            timesigD = value
        elif "keysig" in line[1]:
            keysig = value
        elif "mode" in line[1]:
            if value == "\'majora\'":
                mode = "0"
            elif value == "\'minora\'":
                mode = "1"
            else:
                assert False, "unknown -modea value: " + str(value) + "\n"
            #mode = value
            
        if not timesigN is None and not timesigD is None and not keysig is None and not mode is None:

            first_word = line[0] + "|" + timesigN + "|" + timesigD + "|" + keysig + "|" + mode + "|" + "1" + " "
            

            f.write(first_word)
            
    else:
        # Key
        line[1] = line[1][1:]
        # Pitch
        line[2] = line[2][1:]
        # Duration
        line[3] = line[3][1:]
        # Gate
        line[4] = line[4][1:]

        # Meta Events
        line += ["0"]
        
        line = "|".join(line)
        f.write(line + " ")

        
    
#f.write("#track 1\n")
#for line in channels["V1"]:
#    f.write(line + "\n")


#f.write("#track 2\n")
#for line in channels["V2"]:
#    f.write(line + "\n")


#for line in lines:
#    f.write(line + "\n")
f.write("\n")
f.close()
