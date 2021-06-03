#include "./Songs.h"

void playSong(int buzzer, const Song& song) {
    for (int i = 0; i < song.len; i += 1) {
        int duration = 1000 / song.durations[i];
        int pause = duration * 1.3;

        tone(buzzer, song.melody[i], duration);
        delay(pause);
        noTone(buzzer);
    }
}
