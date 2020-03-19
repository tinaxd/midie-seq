#include "midiworkspace.h"

#include <MidiEvent.h>
#include <cmath>


namespace midie
{

bool
operator==(const TimeSignature& ts1, const TimeSignature& ts2)
{
    return ts1.numerator == ts2.numerator && ts1.denominator == ts2.denominator;
}


bool
operator==(const TimeSignatureChange& c1, const TimeSignatureChange& c2)
{
    return c1.abs_tick == c2.abs_tick && c1.time_signature == c2.time_signature;
}


TimeSignatureInfo::TimeSignatureInfo(std::vector<TimeSignatureChange> changes, bool need_sort)
{
    if (need_sort) {
        std::stable_sort(changes.begin(), changes.end(), [](auto c1, auto c2){return c1.abs_tick < c2.abs_tick;});
    }
    m_changes = changes;
}

void
TimeSignatureInfo::append(TimeSignatureChange change)
{
   m_changes.push_back(change);
   std::stable_sort(m_changes.begin(), m_changes.end(), [](auto c1, auto c2){return c1.abs_tick < c2.abs_tick;});
}

void
TimeSignatureInfo::deleteChange(TimeSignatureChange deleted)
{
    size_t i = 0;
    while (i != m_changes.size()) {
        if (m_changes.at(i) == deleted) {
            m_changes.erase(m_changes.begin() + static_cast<long>(i));
        } else {
            i++;
        }
    }
}

std::optional<TimeSignature>
TimeSignatureInfo::time_signature(uint64_t abs_tick) const
{
    // m_changes is always sorted.
    std::optional<TimeSignature> last_change;
    for (auto it=m_changes.cbegin(); it!=m_changes.cend(); it++)
    {
        if (it->abs_tick <= abs_tick) {
            last_change = std::make_optional(it->time_signature);
        } else {
            break;
        }
    }
    return last_change;
}


bool
operator==(const TempoChange& c1, const TempoChange& c2)
{
    return c1.abs_tick == c2.abs_tick && c1.bpm == c2.bpm;
}


TempoInfo::TempoInfo(std::vector<TempoChange> changes, bool need_sort)
{
    if (need_sort) {
        std::stable_sort(changes.begin(), changes.end(), [](auto c1, auto c2){return c1.abs_tick < c2.abs_tick;});
    }
    m_changes = changes;
}

void
TempoInfo::append(TempoChange change)
{
    m_changes.push_back(change);
    std::stable_sort(m_changes.begin(), m_changes.end(), [](auto c1, auto c2){return c1.abs_tick < c2.abs_tick;});
}

void
TempoInfo::deleteChange(TempoChange deleted)
{
    size_t i = 0;
    while (i != m_changes.size()) {
        if (m_changes.at(i) == deleted) {
            m_changes.erase(m_changes.begin() + static_cast<long>(i));
        } else {
            i++;
        }
    }
}

std::optional<uint16_t>
TempoInfo::tempo(uint64_t abs_tick) const
{
    // m_changes is always sorted.
    std::optional<uint16_t> last_tempo;
    for (auto it=m_changes.cbegin(); it!=m_changes.cend(); it++)
    {
        if (it->abs_tick <= abs_tick) {
            last_tempo = std::make_optional(it->bpm);
        } else {
            break;
        }
    }
    return last_tempo;
}


MidiWorkspace::MidiWorkspace()
{
    auto mf = new smf::MidiFile;
    const auto first_track_idx = mf->addTrack();
    auto& first_track = (*mf)[first_track_idx];

    smf::MidiMessage time_signature;
    time_signature.makeTimeSignature(4, 4);
    smf::MidiEvent time_signature_event;
    time_signature_event = time_signature; // midifile library bug?
    first_track.append(time_signature_event);

    smf::MidiMessage tempo;
    tempo.makeTempo(120.0);
    smf::MidiEvent tempo_event;
    tempo_event = tempo; // midifile library bug?
    first_track.append(tempo_event);

    // need EoT for track 0?

    mf->addTrack();

    // need EoT for track 1?

    mf->setTicksPerQuarterNote(480);

    m_midi.reset(mf);
}

MidiWorkspace::MidiWorkspace(const std::string& path)
{
    auto mf = new smf::MidiFile;
    mf->read(path);
    m_midi.reset(mf);
}

MidiWorkspace::MidiWorkspace(const QString& path)
    : MidiWorkspace(path.toStdString())
{}

unsigned int
MidiWorkspace::track_count() const
{
    return static_cast<unsigned int>(m_midi->getTrackCount());
}

const smf::MidiEventList&
MidiWorkspace::events_abs_tick(unsigned int track) const
{
    if (track >= static_cast<unsigned int>(m_midi->getTrackCount()))
        throw std::out_of_range("track index out of range");
    return (*m_midi)[static_cast<int>(track)];
}

smf::MidiEventList&
MidiWorkspace::events_abs_tick_mut(unsigned int track) const
{
    if (track >= static_cast<unsigned int>(m_midi->getTrackCount()))
        throw std::out_of_range("track index out of range");
    return (*m_midi)[static_cast<int>(track)];
}

TempoInfo
MidiWorkspace::create_tempo_info(unsigned int track) const
{
    auto& events = events_abs_tick(track);
    auto events_len = events.getEventCount();
    std::vector<TempoChange> changes;

    for (auto i=0; i<events_len; i++)
    {
        auto& event = events.getEvent(i);
        if (event.isMeta() && event.isTempo()) {
            TempoChange change;
            change.bpm = static_cast<uint16_t>(event.getTempoBPM());
            change.abs_tick = static_cast<uint64_t>(event.tick);
            changes.push_back(std::move(change));
        }
    }

    return TempoInfo(changes, false);
}

TimeSignatureInfo
MidiWorkspace::create_time_signature_info(unsigned int track) const
{
    auto& events = events_abs_tick(track);
    auto events_len = events.getEventCount();
    std::vector<TimeSignatureChange> changes;

    for (auto i=0; i<events_len; i++)
    {
        auto& event = events.getEvent(i);
        if (event.isMeta() && event.isTimeSignature()) {
            TimeSignatureChange change;
            TimeSignature ts;
            ts.numerator = static_cast<uint8_t>(event.getP0());
            ts.denominator = static_cast<uint8_t>(std::pow(2, event.getP1()));
            change.time_signature = ts;
            change.abs_tick = static_cast<uint64_t>(event.tick);
            changes.push_back(std::move(change));
        }
    }

    return TimeSignatureInfo(changes, false);
}

}
