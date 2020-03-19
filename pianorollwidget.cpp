#include "pianorollwidget.h"

#include <QPainter>
#include <QScrollArea>
#include <QResizeEvent>
#include <QScrollBar>
#include "pianorollutil.h"
#include "midiworkspace.h"

namespace midie {

PianoRollConfig::PianoRollConfig()
    : whiteHeight(30.0),
      whiteWidth(60.0),
      blackWidth(25.0),
      blackHeight(20.0),
      beatWidth(75.0)
{
    noteHeight = (whiteHeight * WHITE_KEYS) / 128;
}

PianoRollConfig::PianoRollConfig(PianoRollPoint whiteHeight, PianoRollPoint whiteWidth, PianoRollPoint blackWidth, PianoRollPoint blackHeight, PianoRollPoint noteHeight, PianoRollPoint beatWidth)
    : whiteHeight(whiteHeight),
      whiteWidth(whiteWidth),
      blackWidth(blackWidth),
      blackHeight(blackHeight),
      noteHeight(noteHeight),
      beatWidth(beatWidth)
{}

PianoRollScroll::PianoRollScroll(QWidget *parent)
    : QScrollArea(parent)
{}

QSize PianoRollScroll::scroll_viewport() const
{
    return viewport()->size();
}

//void PianoRollScroll::resizeEvent(QResizeEvent *event)
//{
//    m_widget->onResize(event);
//}

void PianoRollScroll::paintEvent(QPaintEvent *)
{
    auto w = widget();
    if (w) {
        QPainter painter(viewport());
        const auto& size = viewport()->size();

        PianoRollViewport pr_viewport;
        pr_viewport.width = size.width();
        pr_viewport.height = size.height();
        pr_viewport.maxWidth = pr_viewport.width * 2; // TODO
        pr_viewport.left_upper_x = horizontalScrollBar()->value();
        pr_viewport.left_upper_y = verticalScrollBar()->value();

        qobject_cast<PianoRollWidget*>(w)->paintAll(painter, pr_viewport);
    }
}

PianoRollWidget::PianoRollWidget(QWidget *parent, PianoRollConfig config)
    : QWidget(parent),
      m_config(config),
      m_ws(new MidiWorkspace)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setMinimumSize(10000, static_cast<int>(m_config.whiteHeight) * WHITE_KEYS);
}

void PianoRollWidget::paintAll(QPainter& painter, const PianoRollViewport& viewport)
{
    qDebug("piano roll window maxWidth: %f", viewport.maxWidth);

    painter.translate(QPointF{0, -viewport.left_upper_y});
    paintKeyboard(painter, viewport);

    painter.translate(QPointF{m_config.whiteWidth-viewport.left_upper_x, 0});
    try {
        const auto& track = m_ws->events_abs_tick(m_currentTrack);
        NoteDrawingBounds bounds;
        bounds.left = viewport.left_upper_x;
        bounds.right = viewport.left_upper_x + viewport.width;
        bounds.upper = viewport.left_upper_y;
        bounds.lower = viewport.left_upper_y + viewport.height;
        paintNotes(painter, track, bounds);
    }  catch (const std::out_of_range&) {
        // ignore
    }

    paintTimeline(painter, viewport);
}

void PianoRollWidget::paintKeyboard(QPainter& painter, const PianoRollViewport& viewport)
{
    auto last_c = 0.0;
    auto first_line = true;
    auto processing_note = 127;

    QPen keyboardBlackPen{QColor{0, 0, 0}};
    QBrush keyboardBlackBrush{QColor{0, 0, 0}};
    QBrush keyboardWhiteBrush{QColor{255, 255, 255}};

    for (auto i=0; i<(WHITE_KEYS-1); i++) {
        const auto height = m_config.whiteHeight * i;
        const auto is_c = (i - 4) % 7 == 0;

        painter.setPen(keyboardBlackPen);
        painter.setBrush(keyboardWhiteBrush);
        painter.drawRect(QRectF{0, height, m_config.whiteWidth, m_config.whiteHeight});

        if (is_c) {
            const auto index = (i - 4) / 7;
            auto string = QString("C%1").arg(9 - index);
            painter.drawText(QPointF{m_config.whiteWidth / 4 + 25, i * m_config.whiteHeight + 25}, string);

            const auto curr_c = height + m_config.whiteHeight;
            int n_keys;
            if (first_line) {
                first_line = false;
                n_keys = 8;
            } else {
                n_keys = 12;
            }
            const auto h_line_interval = (curr_c - last_c) / n_keys;
            painter.setPen(QColor{200, 200, 200});
            for (auto i=1; i<=n_keys; i++) {
                const auto y = last_c + h_line_interval * i;
                m_noteHeightCache.at(static_cast<size_t>(processing_note)) = y;
                if (i == n_keys) {
                    painter.setPen(QColor{50, 50, 50});
                }
                painter.drawLine(QPointF{static_cast<double>(m_config.whiteWidth), y}, QPointF{static_cast<double>(viewport.maxWidth), y});
                processing_note--;
            }

            last_c = height + m_config.whiteHeight;
        }
    }

    painter.setBrush(keyboardBlackBrush);
    painter.drawRect(QRectF{0, 0, m_config.blackWidth, m_config.blackHeight/2});

    auto black_index = 2;
    for (auto i=0; i<WHITE_KEYS; i++) {
        if (black_index == 3) {
            black_index += 1;
            continue;
        } else if (black_index == 6) {
            black_index = 0;
        } else {
            const auto left_up_y = m_config.whiteHeight * (i + 1) - m_config.blackHeight / 2;
            painter.drawRect(QRectF{0, left_up_y, m_config.blackWidth, m_config.blackHeight});
            black_index++;
        }
    }
}

void
PianoRollWidget::paintNotes(QPainter& painter, const smf::MidiEventList& track, const NoteDrawingBounds& bounds)
{
    const auto& notes = build_drawing_graph(track);

    auto _note_drawn = 0;
    QPen pen(QColor{0, 0, 0});
    QBrush brush(QColor{255, 0, 0});
    painter.setPen(pen);
    painter.setBrush(brush);

    for (auto note=notes.cbegin(); note!=notes.cend(); note++)
    {
        const auto end_cord = calculateNoteHCord(note->end_tick);
        if (end_cord < bounds.left) {
            continue;
        }
        const auto start_cord = calculateNoteHCord(note->start_tick);
        if (start_cord > bounds.right) {
            break;
        }
        const auto note_height = calculateNoteVCord(note->note);
        if (note_height < bounds.upper || note_height > bounds.lower) {
            continue;
        }
        painter.drawRect(QRectF{start_cord, note_height, end_cord - start_cord, m_config.noteHeight});
        _note_drawn++;
    }
    qDebug("Redrew %d notes", _note_drawn);
}

void
PianoRollWidget::paintTimeline(QPainter& painter, const PianoRollViewport& viewport)
{
    const auto height = WHITE_KEYS * m_config.whiteHeight;
    const auto width = viewport.maxWidth;
    const auto beat_width = m_config.beatWidth;
    const auto ts_info = m_ws->create_time_signature_info(0);
    if (ts_info.empty()) return;
    const auto tick_per_beat = m_ws->resolution();

    auto i_ = 0;
    auto measure = 0;
    auto abs_tick = static_cast<uint64_t>(0);
    auto last_x = 0.0;

    while (true)
    {
        const auto time_signature = ts_info.time_signature(abs_tick).value();
        const auto curr_nn = time_signature.numerator;
        const auto curr_dd = time_signature.denominator;
        const auto interval = beat_width * (4.0 / static_cast<double>(curr_dd));
        const auto interval_tick = static_cast<uint64_t>(tick_per_beat * (4.0 / static_cast<double>(curr_dd)));

        for (auto beat=0; beat<curr_nn; beat++)
        {
            const auto x = last_x + interval;
            if (x > width) goto FINISH;

            if (beat == curr_nn - 1) {
                painter.setPen(QColor{120, 120, 120});
            } else {
                painter.setPen(QColor{200, 200, 200, 200});
            }
            painter.drawLine(QPointF{x, 0.0}, QPointF{x, static_cast<double>(height)});

            if (beat == 0) {
                painter.setPen(QColor{0, 0, 0});
                painter.drawText(
                            QPointF{static_cast<double>(last_x),
                                    static_cast<double>(viewport.left_upper_y + m_config.whiteHeight)},
                            QString("%1").arg(++measure));
            }

            last_x = x;
            i_++;
            abs_tick += interval_tick;
        }
    }

FINISH:
    return;
}

PianoRollPoint
PianoRollWidget::calculateNoteVCord(uint8_t note) const
{
    return m_noteHeightCache.at(1 + note);
}

PianoRollPoint
PianoRollWidget::calculateNoteHCord(uint64_t abs_tick) const
{
    const auto beat_width = static_cast<unsigned long>(m_config.beatWidth);
    const auto beat_tick = static_cast<unsigned long>(m_ws->resolution());
    return static_cast<PianoRollPoint>(beat_width * (abs_tick / beat_tick));
}

void
PianoRollWidget::replaceWorkspace(std::shared_ptr<MidiWorkspace> new_ws)
{
    m_ws = new_ws;
}

void
PianoRollWidget::changeCurrentTrack(unsigned int track)
{
    m_currentTrack = track;
}

//void PianoRollWidget::onResize(QResizeEvent *event)
//{
//    m_viewport.maxWidth = event->size().width();
//}

}
