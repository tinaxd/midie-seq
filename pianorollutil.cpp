#include "pianorollutil.h"

#include <map>
#include <tuple>

namespace midie
{

IndependentNoteDrawingInfo::IndependentNoteDrawingInfo
    (uint8_t note,
     uint8_t velocity,
     NoteTick start_tick,
     NoteTick end_tick)
    : note(note), start_tick(start_tick), end_tick(end_tick), velocity(velocity)
{}


std::vector<IndependentNoteDrawingInfo>
build_drawing_graph(const smf::MidiEventList& track)
{
    std::map<uint8_t, std::tuple<uint64_t, uint8_t>> start_note; // Map<note, (abs_tick, velocity)>
    std::vector<IndependentNoteDrawingInfo> note_drawing;

    const auto track_len = track.getEventCount();
}

}
