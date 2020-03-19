#ifndef TRACKCHOOSER_H
#define TRACKCHOOSER_H

#include <QWidget>
#include <QComboBox>
#include <memory>
#include <tuple>
#include <vector>


namespace midie
{

class MidiWorkspace;

class TrackChooser : public QComboBox
{
    Q_OBJECT
    Q_DISABLE_COPY(TrackChooser)

public:
    TrackChooser(QWidget *parent = nullptr);

private:
    void updateItem(const std::vector<std::tuple<uint8_t, std::string>>& tracks);

signals:
    void trackChange(unsigned int track_number);

public slots:
    void replaceWorkspace(std::shared_ptr<MidiWorkspace> new_ws);
    void onIndexChange(int index);
};

}

#endif // TRACKCHOOSER_H
