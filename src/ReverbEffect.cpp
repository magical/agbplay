#include <algorithm>

#include "ReverbEffect.h"
#include "Debug.h"
#include "Util.h"

using namespace agbplay;
using namespace std;

/*
 * public ReverbEffect
 */

ReverbEffect::ReverbEffect(ReverbType rtype, uint8_t intensity, uint32_t streamRate, uint8_t numAgbBuffers)
    : reverbBuffer((streamRate / AGB_FPS) * N_CHANNELS * numAgbBuffers)
{
    this->intensity = (float)intensity / 128.0f;
    this->rtype = rtype;
    uint32_t bufferLen = streamRate / AGB_FPS;
    fill(reverbBuffer.begin(), reverbBuffer.end(), 0.0f);
    if (rtype == ReverbType::GS1) {
        bufferPos = 0;              // 5 frames delay
        bufferPos2 = bufferLen * 2; // 3 frames delay
        bufferPos3 = bufferLen * 3; // 2 frames delay
        reverbBuffer.resize((streamRate / AGB_FPS) * 5); // 5 frames buffering
    } else {
        bufferPos = 0;
        bufferPos2 = bufferLen;
    }

    delay1HPcarryL = 0.0f;
    delay1HPcarryR = 0.0f;
    delay2HPcarryL = 0.0f;
    delay2HPcarryR = 0.0f;

    delay1HPprevL = 0.0f;
    delay1HPprevR = 0.0f;
    delay2HPprevL = 0.0f;
    delay2HPprevR = 0.0f;
}

ReverbEffect::~ReverbEffect()
{
}

void ReverbEffect::ProcessData(float *buffer, uint32_t nBlocks)
{
    switch (rtype) {
        case ReverbType::NORMAL:
            while (nBlocks > 0) {
                uint32_t left = processNormal(buffer, nBlocks);
                buffer += (nBlocks - left) * N_CHANNELS;
                nBlocks = left;
            }
            break;
        case ReverbType::GS1:
            while (nBlocks > 0) {
                uint32_t left = processGS1(buffer, nBlocks);
                buffer += (nBlocks - left) * N_CHANNELS;
                nBlocks = left;
            }
            break;
        case ReverbType::GS2:
            while (nBlocks > 0) {
                uint32_t left = processGS2(buffer, nBlocks);
                buffer += (nBlocks - left) * N_CHANNELS;
                nBlocks = left;
            }
    }
}

uint32_t ReverbEffect::processNormal(float *buffer, uint32_t nBlocks)
{
    vector<float>& rbuf = reverbBuffer;
    uint32_t count;
    bool reset = false, reset2 = false;
    if (getBlocksPerBuffer() - bufferPos2 <= nBlocks) {
        count = getBlocksPerBuffer() - bufferPos2;
        reset2 = true;
    } else if (getBlocksPerBuffer() - bufferPos <= nBlocks) {
        count = getBlocksPerBuffer() - bufferPos;
        reset = true;
    } else {
        count = nBlocks;
    }
    for (uint32_t c = count; c > 0; c--) 
    {
        float rev = (rbuf[bufferPos * 2] + rbuf[bufferPos * 2 + 1] + 
                rbuf[bufferPos2 * 2] + rbuf[bufferPos2 * 2 + 1]) * intensity * (1.0f / 4.0f);
        rbuf[bufferPos * 2] = *buffer++ += rev;
        rbuf[bufferPos * 2 + 1] = *buffer++ += rev;
        bufferPos++;
        bufferPos2++;
    }
    if (reset2) bufferPos2 = 0;
    else if (reset) bufferPos = 0;
    return nBlocks - count;
}

uint32_t ReverbEffect::processGS1(float *buffer, uint32_t nBlocks)
{
    // FIXME experimental, just to mess around and not to implement the actual alogrithm
    vector<float>& rbuf = reverbBuffer;
    uint32_t count;
    bool reset = false, reset2 = false, reset3 = false;
    if (reverbBuffer.size() - bufferPos3 <= nBlocks) {
        count = uint32_t(reverbBuffer.size()) - bufferPos3;
        reset3 = true;
    } else if (reverbBuffer.size() - bufferPos2 <= nBlocks) {
        count = uint32_t(reverbBuffer.size()) - bufferPos2;
        reset2 = true;
    } else if (reverbBuffer.size() - bufferPos <= nBlocks) {
        count = uint32_t(reverbBuffer.size()) - bufferPos;
        reset = true;
    } else {
        count = nBlocks;
    }
    for (uint32_t c = count; c > 0; c--)
    {
        float in_delay_1 = rbuf[bufferPos];
        //float in_delay_2 = rbuf[bufferPos2];
        //float in_delay_3 = rbuf[bufferPos3];

        //float r_left = in_delay_1 * (4.0f / 8.0f) + in_delay_2 * (4.2f / 8.0f) - in_delay_3 * (3.9f / 8.0f);
        //float r_right = -in_delay_1 * (3.7f / 8.0f) - in_delay_2 * (3.8f / 8.0f) + in_delay_3 * (4.3f / 8.0f);
        float in = (*buffer + *(buffer+1)) / 2.0f;
        float r = in_delay_1 - in;

        rbuf[bufferPos] = r + in;

        *buffer++ = r;
        *buffer++ = r;

        /*delay1HPcarryL = (r_left + delay1HPcarryL - delay1HPprevL) * 0.95f;
        delay1HPcarryR = (r_right + delay1HPcarryR - delay1HPprevR) * 0.95f;

        delay1HPprevL = r_left;
        delay1HPprevR = r_right;

        r_left = delay1HPcarryL;
        r_right = delay1HPcarryR;

        float resl = *buffer++ += r_left;
        float resr = *buffer++ += r_right;

        //__print_debug(FormatString("l=%f, r=%f", r_left, r_right));

        rbuf[bufferPos] = (resl);
        */

        bufferPos++;
        bufferPos2++;
        bufferPos3++;
    }

    if (reset3) bufferPos3 = 0;
    else if (reset2) bufferPos2 = 0;
    else if (reset) bufferPos = 0;
    return nBlocks - count;
}

uint32_t ReverbEffect::processGS2(float *buffer, uint32_t nBlocks)
{
    vector<float>& rbuf = reverbBuffer;
    uint32_t count;
    bool reset = false, reset2 = false;
    if (getBlocksPerBuffer() - bufferPos2 <= nBlocks) {
        count = getBlocksPerBuffer() - bufferPos2;
        reset2 = true;
    } else if (getBlocksPerBuffer() - bufferPos <= nBlocks) {
        count = getBlocksPerBuffer() - bufferPos;
        reset = true;
    } else {
        count = nBlocks;
    }
    for (uint32_t c = count; c > 0; c--) 
    {
        float in_delay_1_l = rbuf[bufferPos * 2], in_delay_1_r = rbuf[bufferPos * 2 + 1];
        float in_delay_2_l = rbuf[bufferPos2 * 2], in_delay_2_r = rbuf[bufferPos2 * 2 + 1];

        delay1HPcarryL = (in_delay_1_l + delay1HPcarryL - delay1HPprevL) * 0.95f;
        delay1HPcarryR = (in_delay_1_r + delay1HPcarryR - delay1HPprevR) * 0.95f;
        delay2HPcarryL = (in_delay_2_l + delay2HPcarryL - delay2HPprevL) * 0.95f;
        delay2HPcarryR = (in_delay_2_r + delay2HPcarryR - delay2HPprevR) * 0.95f;

        delay1HPprevL = in_delay_1_l;
        delay1HPprevR = in_delay_1_r;
        delay2HPprevL = in_delay_2_l;
        delay2HPprevR = in_delay_2_r;

        in_delay_1_l = delay1HPcarryL;
        in_delay_1_r = delay1HPcarryR;
        in_delay_2_l = delay2HPcarryL;
        in_delay_2_r = delay2HPcarryR;

        float r_left = in_delay_1_r * (4.0f / 8.0f) + in_delay_2_l * (4.0f / 8.0f);
        float r_right = -in_delay_1_l * (4.0f / 8.0f) - in_delay_2_r * (4.0f / 8.0f);

        rbuf[bufferPos * 2] = *buffer++ += r_left;
        rbuf[bufferPos * 2 + 1] = *buffer++ += r_right;
        
        /*float i_l = *buffer;
        float i_r = *(buffer+1);
        float r_left = in_delay_1_l - (i_l * 0.8f);
        float r_right = in_delay_1_r - (i_r * 0.8f);

        rbuf[bufferPos * 2] = r_left * 0.8f + i_l;
        rbuf[bufferPos * 2 + 1] = r_right * 0.8f + i_r;

        *buffer++ = r_left;
        *buffer++ = r_right;
        */

        bufferPos++;
        bufferPos2++;
    }
    if (reset2) bufferPos2 = 0;
    else if (reset) bufferPos = 0;
    return nBlocks - count;
}

uint32_t ReverbEffect::getBlocksPerBuffer()
{
    return uint32_t(reverbBuffer.size() / N_CHANNELS);
}
