#include "PlayerInterface.h"

using namespace std;
using namespace agbplay;

/*
 * public PlayerInterface
 */

PlayerInterface::PlayerInterface(Rom& rrom, TrackviewGUI *trackUI, long initSongPos, EnginePars pars
        ) : seq(Sequence(initSongPos, 0, &rrom)), rom(rrom)
{
    this->trackUI = trackUI;
    this->dSoundVol = pars.vol + 1;
    this->dSoundRev = pars.rev;
    this->dSoundFreq = (pars.freq == 0 || pars.freq > 12) ? freqLut[4] : freqLut[pars.freq - 1];
}

PlayerInterface::~PlayerInterface() 
{
}

void PlayerInterface::LoadSong(long songPos, uint8_t trackLimit)
{
    seq = Sequence(songPos, trackLimit, &rom);
    trackUI->SetState(seq.GetUpdatedDisp());
}

/*
 * private PlayerInterface
 */

/*
 * public EnginePars
 */

EnginePars::EnginePars(uint8_t vol, uint8_t rev, uint8_t freq)
{
    this->vol = vol;
    this->rev = rev;
    this->freq = freq;
}