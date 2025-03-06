#include "letter_identify.h"

#define CHECK_INTERVAL // 1000 milliseconds
#define SLOT_COUNT 5

enum slotState {
  EMPTY, // no tile in the slot, show a dim white light
  FULL, // a tile in this slot but other slots are empty, show a bright white light
  MISPLACED, // the letter is in the word, but not here, orange light
  CORRECT, // this letter is here, green light
  ABSENT, // this letter isn't in the word, red light
  WIN, // win, game over, rainbow
  INVALID, // word doesn't exist in the database, blink red lights 
}

class Slot {
  // add constructor that assigns an analog input to this instance and saves it to the pin variable
  // add a method which reads the analog signal from the pin and identifies the letter in the slot, and saves it to the letter variable
  public: 
    int pin;
    slotState state;
    string letter;
}

class Word {
  // this is a class that takes in an array of slots and concatenates the letters into a word.
  // if any of the slots is missing, the word is incomplete
  // if the word doesn't exist in the dictionary, it is invalid
  // if the word does exist in the dictionary, it is valid
}

class Dictionary {
  // this is a singleton class knows all the valid words
  // it can be asked for a random word
  // it can be fed a word to see if that word is valid
}

class Result {
  // takes in a word, evaluates for completion, validity, and correctness
  // updates slots accordingly
  switch(word.status) {
    case incomplete:
      // all slots without letters "EMPTY" and the ones with letters "FULL"
      break;
    case invalid:
      // all slots INVALID
      break;
    case valid:
      // evaluate each slot for correctness
  }
}

class Board {
  // handles lighting the slots
}

Dictionary dictionary; // initialize a global dictionary
Board board;
String targetWord;
Slot slot1(A0)
Slot slot2(A1)
Slot slots[SLOT_COUNT] = {slot1, slot2}

void setup(){
  targetWord= dictionary.randomWord()
}

void loop(){
  // run the check_word method every check interval
}

void check_word(){
  word = Word()
  result = Result(word)
  board.light()
}
