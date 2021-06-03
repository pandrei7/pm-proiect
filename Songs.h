#ifndef SONGS_H_
#define SONGS_H_

#include "./Pitches.h"

#define MAX_SONG_LEN 100

struct Song {
    int melody[MAX_SONG_LEN];
    int durations[MAX_SONG_LEN];
    int len;
};

const Song kHappySong = {
    { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 },
    { 4, 8, 8, 4, 4, 4, 4, 4 },
    8
};

const Song kSadSong = {
    { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 },
    { 4, 8, 8, 4, 4, 4, 4, 4 },
    8
};

void playSong(int buzzer, const Song& song);

#endif
