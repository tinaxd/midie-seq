#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace midie { class MidiWorkspace; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

signals:
    void notifyNewSmf(std::shared_ptr<midie::MidiWorkspace> new_ws);

public slots:
    void newSmf();
    void loadSmf();
};
#endif // MAINWINDOW_H
