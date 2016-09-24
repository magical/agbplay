#include <sndfile.h>
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
#undef BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/algorithm/string/replace.hpp>
#include <chrono>

#include "SoundExporter.h"
#include "Util.h"
#include "MyException.h"
#include "Constants.h"

using namespace agbplay;
using namespace std;

/*
 * public SoundExporter
 */

SoundExporter::SoundExporter(ConsoleGUI& _con, SoundData& _sd, GameConfig& _cfg, Rom& _rom, bool _benchmarkOnly, bool seperate)
: con(_con), cfg(_cfg), sd(_sd), rom(_rom)
{
    benchmarkOnly = _benchmarkOnly;
    this->seperate = seperate;
}

SoundExporter::~SoundExporter()
{
}

void SoundExporter::Export(string outputDir, vector<SongEntry>& entries, vector<bool>& ticked)
{
    if (entries.size() != ticked.size())
        throw MyException("SoundExporter: input vectors do not match");
    vector<SongEntry> tEnts;
    for (size_t i = 0; i < entries.size(); i++) {
        if (!ticked[i])
            continue;
        tEnts.push_back(entries[i]);
    }


    boost::filesystem::path dir(outputDir);
    if (boost::filesystem::exists(dir)) {
        if (!boost::filesystem::is_directory(dir)) {
            throw MyException("Output directory exists but isn't a dir");
        }
    }
    else if (!boost::filesystem::create_directory(dir)) {
        throw MyException("Creating output directory failed");
    }

    size_t totalBlocksRendered = 0;

    auto startTime = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < tEnts.size(); i++)
    {
        string fname = tEnts[i].name;
        boost::replace_all(fname, "/", "_");
        uilock.lock();
        con.WriteLn(FormatString("%3d %% - Rendering to file: \"%s\"", (i + 1) * 100 / tEnts.size(), fname));
        uilock.unlock();
        size_t rblocks = exportSong(FormatString("%s/%03d - %s", outputDir, i + 1, fname), tEnts[i].name, tEnts[i].GetUID());
        totalBlocksRendered += rblocks;
    }

    auto endTime = chrono::high_resolution_clock::now();

    if (chrono::duration_cast<chrono::seconds>(endTime - startTime).count() == 0) {
        con.WriteLn(FormatString("Successfully wrote %d files", tEnts.size()));
    } else {
        con.WriteLn(FormatString("Successfully wrote %d files at %d blocks per second", 
                    tEnts.size(), 
                    int(totalBlocksRendered / (size_t)chrono::duration_cast<chrono::seconds>(endTime - startTime).count())));
    }
}

/*
 * private SoundExporter
 */

size_t SoundExporter::exportSong(string fileName, string title, uint16_t uid)
{
    // setup our generators
    Sequence seq(sd.sTable->GetPosOfSong(uid), cfg.GetTrackLimit(), rom);
    StreamGenerator sg(seq, EnginePars(cfg.GetPCMVol(), cfg.GetEngineRev(), cfg.GetEngineFreq()), 1, 1.0f, cfg.GetRevType());
    size_t blocksRendered = 0;
    size_t nBlocks = sg.GetBufferUnitCount();
    size_t nTracks = seq.tracks.size();
    // libsndfile setup
    if (!benchmarkOnly) 
    {
        if (seperate)
        {
            vector<SNDFILE *> ofiles(nTracks, nullptr);
            vector<SF_INFO> oinfos(nTracks);

            for (size_t i = 0; i < nTracks; i++)
            {
                memset(&oinfos[i], 0, sizeof(oinfos[i]));
                oinfos[i].samplerate = STREAM_SAMPLERATE;
                oinfos[i].channels = N_CHANNELS;
                oinfos[i].format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
                ofiles[i] = sf_open(FormatString("%s.%02d.flac", fileName, i).c_str(), SFM_WRITE, &oinfos[i]);
                if (ofiles[i] == NULL)
                {
                    uilock.lock();
                    con.WriteLn(FormatString("Error: %s", sf_strerror(NULL)));
                    uilock.unlock();
                }
                sf_set_string(ofiles[i], SF_STR_TITLE, FormatString("%s track #%d", title, i).c_str());
            }

            while (true)
            {
                vector<vector<float>>& rbuffers = sg.ProcessAndGetAudio();
                if (sg.HasStreamEnded())
                    break;

                assert(rbuffers.size() == nTracks);

                for (size_t i = 0; i < nTracks; i++) 
                {
                    // do not write to invalid files
                    if (ofiles[i] == NULL)
                        continue;
                    sf_count_t processed = 0;
                    do {
                        processed += sf_write_float(ofiles[i], rbuffers[i].data() + processed, sf_count_t(nBlocks * N_CHANNELS) - processed);
                    } while (processed < sf_count_t(nBlocks * N_CHANNELS));
                }
                blocksRendered += nBlocks;
            }

            for (SNDFILE *& i : ofiles)
            {
                int err = sf_close(i);
                if (err != 0)
                {
                    uilock.lock();
                    con.WriteLn(FormatString("Error: %s", sf_error_number(err)));
                    uilock.unlock();
                }
            }
        }
        else
        {
            SF_INFO oinfo;
            memset(&oinfo, 0, sizeof(oinfo));
            oinfo.samplerate = STREAM_SAMPLERATE;
            oinfo.channels = N_CHANNELS;
            oinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
            SNDFILE *ofile = sf_open((fileName + ".flac").c_str(), SFM_WRITE, &oinfo);
            if (ofile == NULL) {
                uilock.lock();
                con.WriteLn(FormatString("Error: %s", sf_strerror(NULL)));
                uilock.unlock();
                return 0;
            }
            // set title
            sf_set_string(ofile, SF_STR_TITLE, title.c_str());

            // do rendering and write
            vector<float> renderedData(nBlocks * N_CHANNELS);

            while (true) 
            {
                vector<vector<float>>& rbuffers = sg.ProcessAndGetAudio();
                if (sg.HasStreamEnded())
                    break;
                // mix streams to one master
                assert(rbuffers.size() == nTracks);
                // clear mixing buffer
                fill(renderedData.begin(), renderedData.end(), 0.0f);
                // mix all tracks to buffer
                for (vector<float>& b : rbuffers)
                {
                    assert(b.size() == renderedData.size());
                    for (size_t i = 0; i < b.size(); i++)
                    {
                        renderedData[i] += b[i];
                    }
                }
                sf_count_t processed = 0;
                do {
                    processed += sf_write_float(ofile, renderedData.data() + processed, sf_count_t(nBlocks * N_CHANNELS) - processed);
                } while (processed < sf_count_t(nBlocks * N_CHANNELS));
                blocksRendered += nBlocks;
            }

            int err;
            if ((err = sf_close(ofile)) != 0) {
                uilock.lock();
                con.WriteLn(FormatString("Error: %s", sf_error_number(err)));
                uilock.unlock();
            }
        }
    } 
    // if benchmark only
    else {
        while (true)
        {
            sg.ProcessAndGetAudio();
            blocksRendered += nBlocks;
            if (sg.HasStreamEnded())
                break;
        }
    }
    return blocksRendered;
}
