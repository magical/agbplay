#pragma once

#include <cstdint>
#include <vector>
#include <boost/thread.hpp>
#include <portaudio.h>

#include "Rom.h"
#include "TrackviewGUI.h"
#include "DisplayContainer.h"
#include "StreamGenerator.h"
#include "Constants.h"

namespace agbplay
{
    class PlayerInterface 
    {
        public:
            PlayerInterface(Rom& rom, TrackviewGUI *trackUI, long initSongPos, EnginePars pars);
            ~PlayerInterface();
            
            void LoadSong(long songPos, uint8_t trackLimit);
            void Play();
            void Pause();
            void Stop();
            void SpeedDouble();
            void SpeedHalve();
            bool IsPlaying();
            void UpdateView();
        private:
            void threadWorker();

            PaStream *audioStream;
            EnginePars pars;
            uint32_t speedFactor; // 64 = normal
            volatile enum class State : int { RESTART, PLAYING, PAUSED, TERMINATED, SHUTDOWN, THREAD_DELETED } playerState;
            Rom& rom;
            Sequence seq;
            StreamGenerator *sg;
            TrackviewGUI *trackUI;

            boost::thread *playerThread;
    };
}
