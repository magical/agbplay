#pragma once

#include "Rom.h"
#include "DisplayContainer.h"

#define UNKNOWN_TABLE -1
#define MIN_SONG_NUM 16
#define PROG_UNDEFINED 0xFF

namespace agbplay 
{
    class Sequence 
    {
        public:
            Sequence(long songHeader, Rom *rom);
            ~Sequence();

            DisplayContainer& GetUpdatedDisp();
        private:
            struct Track 
            {
                Track(long pos);
                ~Track();

                long pos;
                long retStack[3];
                long patBegin;
                uint8_t retStackPos;
                uint8_t reptCount;
                uint8_t delay;
                uint8_t prog;
                uint8_t vol;
                uint8_t mod;
                uint8_t bendr;
                int8_t pan;
                int8_t bend;
                int8_t tune;
                int8_t keyShift;

                bool muted;
            };
            std::vector<Track> tracks;
            DisplayContainer dcont;
            Rom *rom;
            long songHeader;
            long soundBank;
            uint8_t blocks;
            uint8_t prio;
            uint8_t reverb;
    };

    class SongTable 
    {
        public:
            SongTable(Rom& rrom, long songTable);
            ~SongTable();

            long GetSongTablePos();
            long GetPosOfSong(uint16_t uid);
            unsigned short GetNumSongs();
        private:
            long locateSongTable();
            bool validateTableEntry(long pos, bool strongCheck);
            bool validateSong(agbptr_t checkPtr, bool strongCheck);
            unsigned short determineNumSongs();

            Rom& rom;
            long songTable;
            unsigned short numSongs;
    };

    struct SoundData 
    {
        SoundData(Rom& rrom);
        ~SoundData();

        SongTable *sTable;
    };
}