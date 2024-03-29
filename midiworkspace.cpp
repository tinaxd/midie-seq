#include "midiworkspace.h"

#include <MidiEvent.h>
#include <cmath>
#include <boost/format.hpp>


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
    auto& first_track = (*mf)[0];

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

    m_cache = std::vector<int>(2, 0);
}

MidiWorkspace::MidiWorkspace(const std::string& path)
{
    auto mf = new smf::MidiFile;
    mf->read(path);
    m_midi.reset(mf);

    m_cache = std::vector<int>(static_cast<size_t>(m_midi->getTrackCount()), 0);
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

void
MidiWorkspace::append_event(unsigned int track, uint64_t abs_tick, smf::MidiMessage msg)
{
    auto& events = events_abs_tick_mut(track);
    const auto events_len = events.getEventCount();
    smf::MidiEvent ev;
    ev = msg;
    ev.tick = static_cast<int>(abs_tick);
    if (events_len == 0) {
        events.append(ev);
    } else if (ev.tick < events.getEvent(m_cache.at(track)).tick)
    {
        auto i = m_cache.at(track)-1;
        for (; i>0; i--) {
            // if two events have the same timestamp,
            // note_off must come before note_on.
            if (static_cast<uint64_t>(events.getEvent(i-1).tick) <= abs_tick)
                break;
        }
        events.insert(i, ev);
        m_cache.at(track)++;
    }
    else
    {
        auto i = m_cache.at(track);
        for (; i<events_len; i++) {
            // if two events have the same timestamp,
            // note_off must come before note_on.
            if (static_cast<uint64_t>(events.getEvent(i).tick) > abs_tick)
                break;
        }
        events.insert(i, ev);
    }
}

bool
MidiWorkspace::delete_event(unsigned int track, uint64_t abs_tick, const smf::MidiMessage& msg)
{
    return delete_event_if_once(track, abs_tick, [&msg](auto& m){ return m == msg; });
}

bool
MidiWorkspace::delete_event_if_once(unsigned int track, uint64_t abs_tick, std::function<bool(const smf::MidiMessage&)> pred)
{
    auto& events = events_abs_tick_mut(track);
    if (abs_tick <= static_cast<uint64_t>(events.getEvent(m_cache.at(track)).tick))
    {
        auto i = m_cache.at(track);
        for(; i>=0; i--)
        {
            const auto& event = events.getEvent(i);
            if (static_cast<uint64_t>(event.tick) == abs_tick
                    && pred(event))
            {
                m_cache.at(track) = std::max(0, m_cache.at(track));
                return events.remove(i) != -1;
            }
        }
    }
    else
    {
        auto i = m_cache.at(track);
        auto events_len = events.getEventCount();
        for (; i<events_len; i++)
        {
            const auto& event = events.getEvent(i);
            if (static_cast<uint64_t>(event.tick) == abs_tick
                    && pred(event))
            {
                return events.remove(i) != -1;
            }
        }
    }
    return false;
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
            ts.denominator = static_cast<uint8_t>(event.getP2());
            ts.numerator = static_cast<uint8_t>(event.getP3());
            change.time_signature = ts;
            change.abs_tick = static_cast<uint64_t>(event.tick);
            changes.push_back(std::move(change));
        }
    }

    return TimeSignatureInfo(changes, false);
}

std::vector<std::tuple<uint8_t, std::string>>
MidiWorkspace::track_info() const
{
    std::vector<std::tuple<uint8_t, std::string>> tracks;
    for (auto i=0; i<m_midi->getTrackCount(); i++) {
        tracks.push_back(std::make_tuple(i, (boost::format("Track %1%") % i).str()));
    }
    return tracks;
}

void
MidiWorkspace::reset_cache()
{
    for (auto& c : m_cache)
    {
        c = 0;
    }
}

}
