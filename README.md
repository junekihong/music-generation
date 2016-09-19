Juneki Hong (juneki@cs.cmu.edu)
April, 2016

Computer Music Generation

# Set up

* run the script

  * ./compile_allegro.sh

  * to compile the allegro codebase (which this project relies on)

  * This should compile the allegro directory and produce two binaries: midi2gro, gro2midi

## Prepare Training Data

* Prepare midi files as training data (1st step)

  * ./processMIDIfiles2GRO.sh /path/to/midi/data/*.midi

* Prepare midi files as training data (2nd step)

  * ./processGROfiles2ENCODED.sh gro/*.gro

* Prepare midi files as training data (3rd step)

  * cat encoded/*.encoded > train/train.encoded.dat

# Running the Project

* To train a model:

  * python keras/lstm_gro_generation.py train/train.encoded.dat

* To run the model to produce a sample song:

  * python keras/decode_random.py train_model_arch.json train_model_weights.h5 train/train.encoded.dat


* To decode the sample song out to a midi file:

  * python decode.py output.encoded.txt

  * ./gro2midi decoded/output.encoded.decoded output/

  * timidity output/output.encoded.mid
