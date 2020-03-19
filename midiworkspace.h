#ifndef MIDIWORKSPACE_H
#define MIDIWORKSPACE_H

#include <memory>
#include <MidiFile.h>
#include <MidiEventList.h>
#include <QString>
#include <vector>
#include <optional>


namespace midie
{

struct TimeSignature;
struct TimeSignatureChange;
class TimeSignatureInfo;
class MidiWorkspace;
struct TempoChange;
class TempoInfo;


struct TimeSignature
{
    uint8_t numerator;
    uint8_t denominator;
};
bool operator==(const TimeSignature& ts1, const TimeSignature& ts2);


struct TimeSignatureChange
{
    uint64_t abs_tick;
    TimeSignature time_signature;
};
bool operator==(const TimeSignatureChange& c1, const TimeSignatureChange& c2);


class TimeSignatureInfo
{
public:
    TimeSignatureInfo(std::vector<TimeSignatureChange> changes, bool need_sort);

    void append(TimeSignatureChange change);
    void deleteChange(TimeSignatureChange deleted);
    std::optional<TimeSignature> time_signature(uint64_t abs_tick) const;

    bool empty() const { return m_changes.empty(); }

private:
    std::vector<TimeSignatureChange> m_changes;
};


struct TempoChange
{
    uint64_t abs_tick;
    uint16_t bpm;
};
bool operator==(const TempoChange& c1, const TempoChange& c2);


class TempoInfo
{
public:
    TempoInfo(std::vector<TempoChange> changes, bool need_sort);

    void append(TempoChange change);
    void deleteChange(TempoChange deleted);
    std::optional<uint16_t> tempo(uint64_t abs_tick) const;

    bool empty() const { return m_changes.empty(); }

private:
    std::vector<TempoChange> m_changes;
};


class MidiWorkspace
{
public:
    MidiWorkspace(const std::string& path);
    MidiWorkspace(const QString& path);
    MidiWorkspace();

    unsigned int track_count() const;

    const smf::MidiEventList& events_abs_tick(unsigned int track) const;
    smf::MidiEventList& events_abs_tick_mut(unsigned int track) const;

    TempoInfo create_tempo_info(unsigned int track) const;
    TimeSignatureInfo create_time_signature_info(unsigned int track) const;

    std::vector<std::string> track_info() const;

    int resolution() const { return m_midi->getTicksPerQuarterNote(); }

private:
    std::unique_ptr<smf::MidiFile> m_midi;

    void finalize();
};

}

#endif // MIDIWORKSPACE_H
