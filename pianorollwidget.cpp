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
        pr_viewport.maxWidth = 10000; // TODO
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
    qDebug("piano roll window width: %f, maxWidth: %f", viewport.width, viewport.maxWidth);

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
    const auto beat_width = m_config.beatWidth;
    const auto beat_tick = m_ws->resolution();
    return static_cast<PianoRollPoint>(beat_width * (static_cast<double>(abs_tick) / beat_tick));
}

void
PianoRollWidget::mousePressEvent(QMouseEvent *event)
{
    const auto& pos = event->pos();
    const auto& button = event->button();

    switch (button)
    {
    case Qt::MouseButton::LeftButton:
    {
        m_editingState.click_state = EditingState::Clicked{
                static_cast<double>(pos.x()), static_cast<double>(pos.y())};
        // preview sound here
        break;
    }
    case Qt::MouseButton::RightButton:
    {
        m_editingState.click_state = EditingState::SubClicked{
                static_cast<double>(pos.x()), static_cast<double>(pos.y())};
        break;
    }
    default: {}
    }
}

void
PianoRollWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (std::holds_alternative<EditingState::Clicked>(m_editingState.click_state))
    {
        const auto& clicked_pos = std::get<EditingState::Clicked>(m_editingState.click_state);
        const auto& release_pos = event->pos();
        const auto& clicked_pos_parsed = parseClickPosition(std::make_tuple(clicked_pos.x, clicked_pos.y));
        const auto& release_pos_parsed = parseClickPosition(std::make_tuple(release_pos.x(), release_pos.y()));
        if (clicked_pos_parsed && release_pos_parsed)
        {
            auto start_tick = std::get<0>(clicked_pos_parsed.value());
            const auto note = std::get<1>(clicked_pos_parsed.value());
            auto end_tick = std::get<0>(release_pos_parsed.value());
            start_tick = quantizeTime(start_tick);
            end_tick = quantizeTime(end_tick);
            if (start_tick >= end_tick) {
                // invalid range
                return;
            } else {
                // add note to m_currentTrack
                smf::MidiMessage noteon;
                noteon.makeNoteOn(static_cast<int>(m_currentTrack), note, 100); // TODO: not always m_currentTrack == channel
                smf::MidiMessage noteoff;
                noteoff.makeNoteOff(static_cast<int>(m_currentTrack), note, 0); // TODO:

                m_ws->append_event(m_currentTrack, start_tick, noteon);
                m_ws->append_event(m_currentTrack, end_tick, noteoff);

                // request redraw
                update();
            }
        }
    } else if (std::holds_alternative<EditingState::SubClicked>(m_editingState.click_state))
    {
        const auto& clicked_pos = std::get<EditingState::SubClicked>(m_editingState.click_state);
        const auto& clicked_pos_parsed = parseClickPosition(std::make_tuple(clicked_pos.x, clicked_pos.y));
        if (clicked_pos_parsed)
        {
            uint64_t tick;
            uint8_t note;
            std::tie(tick, note) = clicked_pos_parsed.value();
            if (deleteNoteTickNote(tick, note)) {
                // request redraw
                update();
                qDebug("deleted tick %ld note %d", tick, note);
            } else {
                qDebug("failed to delete note: note not found tick %ld note %d", tick, note);
            }
        }
    }
}

bool
PianoRollWidget::deleteNoteTickNote(uint64_t tick, uint8_t note)
{
    const auto& track = m_ws->events_abs_tick(m_currentTrack);
    const auto& note_info = build_drawing_graph(track);
    auto noteon_deleted = false;
    auto noteoff_deleted = false;
    for (const auto& info : note_info)
    {
        if (info.note == note && info.start_tick <= tick && tick <= info.end_tick)
        {
            noteon_deleted = m_ws->delete_event_if_once(m_currentTrack, info.start_tick, [note](auto& m) {
               if (m.isNoteOn()) {
                   return m.getKeyNumber() == static_cast<int>(note);
               }
               return false;
            });
            noteoff_deleted = m_ws->delete_event_if_once(m_currentTrack, info.end_tick, [note](auto& m) {
                if (m.isNoteOff()) {
                    return m.getKeyNumber() == static_cast<int>(note);
                }
                return false;
            });
            break;
        }
    }
    return noteon_deleted && noteoff_deleted;
}

uint64_t
PianoRollWidget::quantizeTime(uint64_t absTick)
{
    const auto diff = absTick % m_editingState.quantize_unit;
    if (diff > (m_editingState.quantize_unit / 2)) {
        return absTick + m_editingState.quantize_unit - diff;
    } else {
        return absTick - diff;
    }
}

std::optional<std::tuple<uint64_t, uint8_t>>
PianoRollWidget::parseClickPosition(const std::tuple<PianoRollPoint, PianoRollPoint>& pos) const
{
    if (std::get<0>(pos) < m_config.whiteWidth) {
        return std::optional<std::tuple<uint64_t, uint8_t>>();
    }

    std::optional<PianoRollPoint> note;
    for (auto i=12; i<128; i++)
    {
        const auto v = m_noteHeightCache.at(static_cast<size_t>(i));
        if (std::get<1>(pos) > v) {
            note = std::make_optional(i - 1);
            break;
        }
    }
    if (note)
    {
        const auto abs_x = std::get<0>(pos) - m_config.whiteWidth;
        const auto abs_tick = static_cast<uint64_t>((abs_x * m_ws->resolution()) / m_config.beatWidth);
        return std::optional(std::tuple(abs_tick, note.value()));
    }
    return std::optional<std::tuple<PianoRollPoint, PianoRollPoint>>();
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
