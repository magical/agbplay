#pragma once

#include <cstdint>
#include <vector>

#include "SampleStructs.h"

namespace agbplay
{
    class CGBChannel
    {
        public: 
            CGBChannel(CGBType t);
            ~CGBChannel();
            void Init(void *owner, CGBDef def, Note note, ADSR env);
            void *GetOwner();
            float GetFreq();
            void SetVol(uint8_t leftVol, uint8_t rightVol);
            ChnVol GetVol();
            CGBDef GetDef();
            uint8_t GetMidiKey();
            int8_t GetNoteLength();
            void Release();
            void SetPitch(int16_t pitch);
            bool TickNote(); // returns true if note remains active
            EnvState GetState();
            void StepEnvelope();
            void UpdateVolFade();
            const float *GetPat();
            uint32_t pos;
            float interPos;
        private:
            std::vector<float> waveBuffer;
            void *owner;
            const float *pat;
            float freq;
            ADSR env;
            Note note;
            CGBDef def;
            CGBType cType;
            EnvState eState;
            uint8_t envInterStep;
            uint8_t leftVol;
            uint8_t rightVol;
            uint8_t envLevel;
            // these values are always 1 frame behind in order to provide a smooth transition
            uint8_t fromLeftVol;
            uint8_t fromRightVol;
            uint8_t fromEnvLevel;
    };
}
