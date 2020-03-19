#ifndef PIANOROLLWIDGET_H
#define PIANOROLLWIDGET_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QScrollArea>

namespace midie {

using PianoRollPoint = int;
const int WHITE_KEYS = 69;

struct PianoRollConfig;
struct PianoRollViewport;
class PianoRollScroll;
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

class PianoRollWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PianoRollWidget)
public:
    PianoRollWidget(QWidget *parent = nullptr, PianoRollConfig config = PianoRollConfig());

private:
    int m_currentTrack;
    PianoRollConfig m_config;
//    PianoRollViewport m_viewport;

    void paintAll(QPainter& painter, const PianoRollViewport& viewport);
    void paintKeyboard(QPainter& painter, const PianoRollViewport& viewport);
    void paintNotes(QPainter& painter);

//    void onResize(QResizeEvent *event);

    friend PianoRollScroll;

signals:

public slots:

};

}

#endif // PIANOROLLWIDGET_H
