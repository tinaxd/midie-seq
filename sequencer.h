#ifndef MIDIE_SEQUENCER_H
#define MIDIE_SEQUENCER_H

#include <memory>


namespace midie {

class MidiWorkspace;

class Sequencer
{
public:
    Sequencer();

private:
    std::shared_ptr<MidiWorkspace> m_ws;
};

} // namespace midie

#endif // MIDIE_SEQUENCER_H
