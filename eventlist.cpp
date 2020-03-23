#include "eventlist.h"
#include "midiworkspace.h"

#include <QList>
#include <QStandardItem>
#include <QString>

namespace midie {

EventList::EventList(QWidget *parent) : QTreeView(parent)
{
    model = new EventListModel(this);
    setModel(model);
}

void
EventList::replaceWorkspace(std::shared_ptr<MidiWorkspace> ws)
{
    model->replaceWorkspace(std::move(ws));
    this->updateList();
}

void
EventList::updateList()
{
    model->updateList(m_currentTrack);
}

void
EventList::changeTrack(unsigned int track)
{
    m_currentTrack = track;
    this->updateList();
}


EventListModel::EventListModel(QObject *parent) : QStandardItemModel(parent)
{
    setupHeader();
}

QString
EventListModel::formatTime(uint64_t absTick)
{
    // TODO: format in measure:tick style
    return QString("%1").arg(absTick);
}

void
EventListModel::replaceWorkspace(std::shared_ptr<MidiWorkspace> ws)
{
    m_ws = ws;
}

void
EventListModel::setupHeader()
{
    QStringList labels {
        QString("Type"), QString("Start"), QString("Length"), QString("Data")
    };
    setHorizontalHeaderLabels(labels);
}

void
EventListModel::updateList(unsigned int track)
{
    clear();
    setupHeader();
    if (!m_ws) return;
    auto& events = m_ws->events_abs_tick_mut(track);
    events.linkNotePairs();
    const auto events_len = events.getEventCount();

    //auto debug_vector = [](const aut)

    for (auto i=0; i<events_len; i++)
    {
        const auto& event = events.getEvent(i);
        QList<QStandardItem *> list;
#define ADD_ABSTICK list.append(new QStandardItem(QString("%1").arg(event.tick)))
#define ADD_LENGTH list.append(new QStandardItem(QString("NA")))
        if (event.isMeta())
        {
            list.append(new QStandardItem(QString("meta")));
            ADD_ABSTICK;
            ADD_LENGTH;
            list.append(new QStandardItem(QString("unknown")));
        }
        else
        {
            if (event.isNoteOn())
            {
                list.append(new QStandardItem(QString("note")));
                ADD_ABSTICK;
                list.append(new QStandardItem(QString("%1").arg(event.getTickDuration())));
                list.append(new QStandardItem(QString("%1 %2").arg(event.getKeyNumber()).arg(event.getVelocity())));
            }
//            else if (event.isNoteOff())
//            {
//                list.append(new QStandardItem(QString("note off")));
//                ADD_ABSTICK;
//                ADD_LENGTH;
//                list.append(new QStandardItem(QString("%1").arg(event.getKeyNumber())));
//            }
            else
            {
                list.append(new QStandardItem(QString("midi message")));
                ADD_ABSTICK;
                ADD_LENGTH;
                list.append(new QStandardItem(QString("unknown")));
            }
        }
        appendRow(std::move(list));
    }
#undef ADD_LENGTH
#undef ADD_ABSTICK
}

} // namespace midie
