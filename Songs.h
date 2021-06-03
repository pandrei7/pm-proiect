#ifndef SONGS_H_
#define SONGS_H_

#include "./Pitches.h"

// This module contains functions and data types for interacting with songs.

// The maximum number of notes in a song.
#define MAX_SONG_LEN 100

// All attributes of a song.
struct Song {
    int melody[MAX_SONG_LEN];
    int durations[MAX_SONG_LEN];
    int len;
};

// A song to celebrate winning the game.
const Song kHappySong = {
    { NOTE_FS5, NOTE_FS5, NOTE_D5, NOTE_B4, NOTE_B4, NOTE_E5,
      NOTE_E5, NOTE_E5, NOTE_GS5, NOTE_GS5, NOTE_A5, NOTE_B5,
      NOTE_A5, NOTE_A5, NOTE_A5, NOTE_E5, NOTE_D5, NOTE_FS5,
      NOTE_FS5, NOTE_FS5, NOTE_E5, NOTE_E5, NOTE_FS5, NOTE_E5 },
    { 8, 8, 8, 4, 4, 4,
      4, 5, 8, 8, 8, 8,
      8, 8, 8, 4, 4, 4,
      4, 5, 8, 8, 8, 8 },
    24
};

// A song used when the player loses the game.
const Song kSadSong = {
    { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 },
    { 4, 8, 8, 4, 4, 4, 4, 4 },
    8
};

// Plays a song on a given buzzer pin.
void playSong(int buzzer, const Song& song);

#endif
