#ifndef TRACKCHOOSER_H
#define TRACKCHOOSER_H

#include <QWidget>
#include <QComboBox>
#include <memory>


namespace midie
{

class MidiWorkspace;

class TrackChooser : public QComboBox
{
public:
    TrackChooser();

private:
    std::shared_ptr<MidiWorkspace> m_ws;
};

}

#endif // TRACKCHOOSER_H
