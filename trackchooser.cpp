#include "trackchooser.h"

#include <QVariant>
#include "midiworkspace.h"


namespace midie
{

TrackChooser::TrackChooser(QWidget *parent)
    : QComboBox(parent)
{
    setInsertPolicy(QComboBox::InsertAtBottom);

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TrackChooser::onIndexChange);
}

void
TrackChooser::updateItem(const std::vector<std::tuple<uint8_t, std::string>>& tracks)
{
    clear();
    for (auto track=tracks.cbegin(); track!=tracks.cend(); track++) {
        unsigned int track_number;
        std::string track_desc;
        std::tie(track_number, track_desc) = *track;
        addItem(QString(track_desc.c_str()), QVariant(track_number));
    }
}

void
TrackChooser::replaceWorkspace(std::shared_ptr<MidiWorkspace> new_ws)
{
    updateItem(new_ws->track_info());
}

void
TrackChooser::onIndexChange(int index)
{
    if (index == -1) return;
    bool ok;
    unsigned int track_number = currentData().toUInt(&ok);
    if (ok) {
        trackChange(track_number);
    }
}

}
