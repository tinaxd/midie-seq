#ifndef PIANOROLLUTIL_H
#define PIANOROLLUTIL_H

#include <cstdint>
#include <MidiEventList.h>

namespace midie
{

using NoteTick = uint64_t;

struct IndependentNoteDrawingInfo
{
    IndependentNoteDrawingInfo(uint8_t note, uint8_t velocity, NoteTick start_tick, NoteTick end_tick);

    uint8_t note;
    NoteTick start_tick;
    NoteTick end_tick;
    uint8_t velocity;
};

std::vector<IndependentNoteDrawingInfo> build_drawing_graph(const smf::MidiEventList& track);

}

#endif // PIANOROLLUTIL_H
