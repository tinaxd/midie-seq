#include "pianorollutil.h"

#include <map>
#include <tuple>
#include <stdexcept>

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
    for (auto i=0; i<track_len; i++) {
        const auto& event = track.getEvent(i);
        if (event.isNote()) {
            const auto abs_tick = event.tick;
            const auto note = static_cast<uint8_t>(event.getP0());
            const auto velocity = event.getP1();
            if (event.isNoteOn()) {
                if (velocity == 0) {
                    // note_on with velocity 0 are treated as note_off
                    try {
                        const auto& start_pair = start_note.at(note);
                        note_drawing.emplace_back(
                                    note,
                                    velocity,
                                    std::get<0>(start_pair),
                                    event.tick
                                    );
                    }  catch (const std::out_of_range&) {
                        // ignore
                    }
                } else {
                    start_note.insert_or_assign(note, std::make_tuple(abs_tick, velocity)); // FIXME: duplicates note_ons?
                }
            } else if (event.isNoteOff()) {
                try {
                    const auto& start_pair = start_note.at(note);
                    note_drawing.emplace_back(
                                note,
                                velocity,
                                std::get<0>(start_pair),
                                event.tick
                                );
                }  catch (const std::out_of_range&) {
                    // ignore
                }
            }
        }
    }

    return note_drawing;
}

}
