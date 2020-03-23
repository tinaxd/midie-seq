#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "pianorollwidget.h"
#include "midiworkspace.h"
#include "trackchooser.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionNewSmf, &QAction::triggered, this, &MainWindow::newSmf);
    connect(ui->actionOpenSmf, &QAction::triggered, this, &MainWindow::loadSmf);

    connect(this, &MainWindow::notifyNewSmf, ui->scrollAreaWidgetContents, &midie::PianoRollWidget::replaceWorkspace);
    connect(this, &MainWindow::notifyNewSmf, ui->trackComboBox, &midie::TrackChooser::replaceWorkspace);
    connect(this, &MainWindow::notifyNewSmf, ui->treeView, &midie::EventList::replaceWorkspace);

    connect(ui->trackComboBox, &midie::TrackChooser::trackChange, ui->scrollAreaWidgetContents, &midie::PianoRollWidget::changeCurrentTrack);
    connect(ui->trackComboBox, &midie::TrackChooser::trackChange, ui->treeView, &midie::EventList::changeTrack);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newSmf()
{
    notifyNewSmf(std::make_shared<midie::MidiWorkspace>());
}

void MainWindow::loadSmf()
{
    const auto& filename = QFileDialog::getOpenFileName(this, tr("Open SMF"), "", tr("Standard MIDI File (*.mid)"));
    notifyNewSmf(std::make_shared<midie::MidiWorkspace>(filename));
}
