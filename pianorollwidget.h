#ifndef PIANOROLLWIDGET_H
#define PIANOROLLWIDGET_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QScrollArea>
#include <MidiEventList.h>
#include <memory>
#include <array>
#include <variant>


namespace midie {

using PianoRollPoint = double;
const int WHITE_KEYS = 69;

class MidiWorkspace;

struct PianoRollConfig;
struct PianoRollViewport;
class PianoRollScroll;
struct EditingState;
class PianoRollWidget;


struct PianoRollConfig
{
    explicit PianoRollConfig();
    PianoRollConfig(PianoRollPoint whiteHeight, PianoRollPoint whiteWidth, PianoRollPoint blackWidth, PianoRollPoint blackHeight, PianoRollPoint noteHeight, PianoRollPoint beatWidth);

    PianoRollPoint whiteHeight;
    PianoRollPoint whiteWidth;
    PianoRollPoint blackWidth;
    PianoRollPoint blackHeight;
    PianoRollPoint noteHeight;
    PianoRollPoint beatWidth;
};


struct PianoRollViewport
{
    PianoRollPoint maxWidth;
    PianoRollPoint width;
    PianoRollPoint height;
    PianoRollPoint left_upper_x;
    PianoRollPoint left_upper_y;
};


struct NoteDrawingBounds
{
    PianoRollPoint left, right, upper, lower;
};


class PianoRollScroll : public QScrollArea
{
    Q_OBJECT
    Q_DISABLE_COPY(PianoRollScroll)

public:
    explicit PianoRollScroll(QWidget *parent = nullptr);

    QSize scroll_viewport() const;

private:
//    PianoRollWidget *m_widget;

protected:
//    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

};


struct EditingState
{
    struct Clicked
    {
        Clicked(double x, double y) : x(x), y(y) {}
        double x, y;
    };

    struct SubClicked
    {
        SubClicked(double x, double y) : x(x), y(y) {}
        double x, y;
    };

    using Released = std::monostate;
    using ClickState = std::variant<Released, Clicked, SubClicked>;
    uint64_t quantize_unit = 480 / 4;
    ClickState click_state = Released{};
};


class PianoRollWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PianoRollWidget)
public:
    PianoRollWidget(QWidget *parent = nullptr, PianoRollConfig config = PianoRollConfig());

private:
    unsigned int m_currentTrack = 0;
    PianoRollConfig m_config;
//    PianoRollViewport m_viewport;
    std::array<PianoRollPoint, 128> m_noteHeightCache;

    EditingState m_editingState;
    std::shared_ptr<MidiWorkspace> m_ws;

    void paintAll(QPainter& painter, const PianoRollViewport& viewport);
    void paintKeyboard(QPainter& painter, const PianoRollViewport& viewport);
    void paintNotes(QPainter& painter, const smf::MidiEventList& track, const NoteDrawingBounds& bounds);
    void paintTimeline(QPainter& painter, const PianoRollViewport& viewport);

    PianoRollPoint calculateNoteVCord(uint8_t note) const;
    PianoRollPoint calculateNoteHCord(uint64_t abs_tick) const;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    bool deleteNoteTickNote(uint64_t tick, uint8_t note);

    uint64_t quantizeTime(uint64_t absTick);

    std::optional<std::tuple<uint64_t, uint8_t>> parseClickPosition(const std::tuple<PianoRollPoint, PianoRollPoint>& pos) const;

//    void onResize(QResizeEvent *event);

    friend PianoRollScroll;

signals:

public slots:
    void replaceWorkspace(std::shared_ptr<MidiWorkspace> new_ws);
    void changeCurrentTrack(unsigned int track);

};

}

#endif // PIANOROLLWIDGET_H
