#!/bin/python

from keras.models import Sequential
from keras.layers.core import Dense, Activation, Dropout
from keras.layers.recurrent import LSTM
from keras.utils.data_utils import get_file
from keras.models import model_from_json

import numpy as np
import random
import sys

from lstm_text_generation import read_training, reencode, sample

model_file = sys.argv[1]
weights_file = sys.argv[2]

training_file = sys.argv[3]
input_file = sys.argv[4]

output_file = "output.encoded.txt"

#name = #model_file.split("/")[-1].split(".")[0].split("_")[0]
name = training_file.split("/")[-1].split(".")[0]
name += "." + input_file.split("/")[-1].split(".")[0]
name += ".output"

model = model_from_json(open(model_file, "r").read())
model.load_weights(weights_file)

text = read_training(training_file)
#sentence = [x for x in text]
words = set(text)
#sys.stderr.write("nb sequences: " + str(len(sentences)) + "\n")
sys.stderr.write("Vectorization\n")
word_indices = dict((w, i) for i,w in enumerate(words))
indices_words = dict((i, w) for i,w in enumerate(words))


maxlen = 100
#start_index = random.randint(0, len(text) - maxlen - 1)


inputtext = read_training(input_file)
sentence = inputtext[len(inputtext)-maxlen:]

#f = open(output_file, "w")
f = open(name, "w")

#sentence = text[start_index: start_index + maxlen]
generated = " ".join([reencode(x) for x in sentence])
f.write(generated + " ")

while len(sentence) < maxlen:
    sentence = sentence + sentence
    sentence = sentence[-maxlen:]

#for i in range(1000):


for i in range(1000):
    x = np.zeros((1, len(sentence), 6))
    for t,word in enumerate(sentence):
        x[0, t] = word

    preds = model.predict(x, verbose=0)[0]

    #print preds
    
    next_index = sample(preds, 1.0)

    #print next_index

    #print len(words)
    
    next_word = indices_words[next_index]

    #generated += reencode(next_word) + " "
    sentence = sentence[1:] + [next_word]

    
    if (i + 150) % 150 == 0:
        start_index = random.randint(0, len(text) - int(maxlen) - 1)
        sentence = sentence[-1:] + text[start_index: start_index + int(maxlen) - 1]

    f.write(reencode(next_word) + " ")

    #if i % 50:
    #    f.flush()
f.close()

