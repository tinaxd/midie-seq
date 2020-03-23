#ifndef MIDIE_EVENTLIST_H
#define MIDIE_EVENTLIST_H

#include <QTreeView>
#include <QStandardItemModel>
#include <memory>


namespace midie {

class MidiWorkspace;
class EventList;
class EventListModel;

class EventList : public QTreeView
{
public:
    EventList(QWidget *parent = nullptr);

private:
    EventListModel *model;
    unsigned int m_currentTrack;

public slots:
    void replaceWorkspace(std::shared_ptr<MidiWorkspace> ws);
    void updateList();
    void changeTrack(unsigned int track);

};

class EventListModel : public QStandardItemModel
{
public:
    EventListModel(QObject *parent = nullptr);

private:
    std::shared_ptr<MidiWorkspace> m_ws;

    static QString formatTime(uint64_t absTick);

    void setupHeader();

public slots:
    void replaceWorkspace(std::shared_ptr<MidiWorkspace> ws);
    void updateList(unsigned int track);
};

} // namespace midie

#endif // MIDIE_EVENTLIST_H
