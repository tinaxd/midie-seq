#include "pianorollwidget.h"

#include <QPainter>
#include <QScrollArea>
#include <QResizeEvent>
#include <QScrollBar>

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
      m_config(config)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setMinimumSize(10000, m_config.whiteHeight * WHITE_KEYS);
}

void PianoRollWidget::paintAll(QPainter& painter, const PianoRollViewport& viewport)
{
    qDebug("piano roll window maxWidth: %d", viewport.maxWidth);

    painter.translate(QPoint{0, -viewport.left_upper_y});
    paintKeyboard(painter, viewport);
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
        painter.drawRect(QRect{0, height, m_config.whiteWidth, m_config.whiteHeight});

        if (is_c) {
            const auto index = (i - 4) / 7;
            auto string = QString("C%1").arg(9 - index);
            painter.drawText(QPoint{m_config.whiteWidth / 4 + 25, i * m_config.whiteHeight + 25}, string);

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
                // cache[processing_note] = y;
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
    painter.drawRect(QRect{0, 0, m_config.blackWidth, m_config.blackHeight/2});

    auto black_index = 2;
    for (auto i=0; i<WHITE_KEYS; i++) {
        if (black_index == 3) {
            black_index += 1;
            continue;
        } else if (black_index == 6) {
            black_index = 0;
        } else {
            const auto left_up_y = m_config.whiteHeight * (i + 1) - m_config.blackHeight / 2;
            painter.drawRect(QRect{0, left_up_y, m_config.blackWidth, m_config.blackHeight});
            black_index++;
        }
    }
}

//void PianoRollWidget::onResize(QResizeEvent *event)
//{
//    m_viewport.maxWidth = event->size().width();
//}

}
