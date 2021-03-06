#pragma once

#include <cstdint>
#include <vector>

#include "Types.h"

#define AGB_FPS 60
#define N_CHANNELS 2

namespace agbplay
{
    class ReverbEffect
    {
        public:
            ReverbEffect(ReverbType rtype, uint8_t intesity, uint32_t streamRate, uint8_t numAgbBuffers);
            ~ReverbEffect();
            void ProcessData(float *buffer, uint32_t nBlocks);
        private:
            uint32_t processNormal(float *buffer, uint32_t nBlocks);
            uint32_t processGS1(float *buffer, uint32_t nBlocks);
            uint32_t processGS2(float *buffer, uint32_t nBlocks);
            uint32_t getBlocksPerBuffer();
            ReverbType rtype;
            float intensity;
            //uint32_t streamRate;
            std::vector<float> reverbBuffer;
            uint32_t bufferPos;
            uint32_t bufferPos2;
            uint32_t bufferPos3;
            float delay1HPcarryL;
            float delay1HPcarryR;
            float delay2HPcarryL;
            float delay2HPcarryR;
            float delay1HPprevL;
            float delay1HPprevR;
            float delay2HPprevL;
            float delay2HPprevR;
    };
}
