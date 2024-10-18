﻿#include "stdafx.h"
#include <QAction>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>
#include <QTimer>
#include <QVBoxLayout>
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdialogs.h"
#include "qscreen.h"
#include "qkeyboardview.h"
#include "qconsoleview.h"
#include "qdebugview.h"
#include "qdisasmview.h"
#include "qmemoryview.h"
#include "qscripting.h"
#include "Emulator.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("UKNC Back to Life"));

    // Assign signals
    QSignalMapper *langMapper = new QSignalMapper(this);
    QObject::connect(ui->actionLangEnglish, SIGNAL(triggered()), langMapper, SLOT(map()));
    langMapper->setMapping(ui->actionLangEnglish, "en");
    QObject::connect(ui->actionLangRussian, SIGNAL(triggered()), langMapper, SLOT(map()));
    langMapper->setMapping(ui->actionLangRussian, "ru");
    QObject::connect(langMapper, SIGNAL(mapped(QString)), this, SLOT(selectLanguage(QString)));

    QObject::connect(ui->actionSaveStateImage, SIGNAL(triggered()), this, SLOT(saveStateImage()));
    QObject::connect(ui->actionLoadStateImage, SIGNAL(triggered()), this, SLOT(loadStateImage()));
    QObject::connect(ui->actionFileScreenshot, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
    QObject::connect(ui->actionFileScreenshotAs, SIGNAL(triggered()), this, SLOT(saveScreenshotAs()));
    QObject::connect(ui->actionFileScreenshotToClipboard, SIGNAL(triggered()), this, SLOT(screenshotToClipboard()));
    QObject::connect(ui->actionFileScreenToClipboard, SIGNAL(triggered()), this, SLOT(screenTextToClipboard()));
    QObject::connect(ui->actionScriptRun, SIGNAL(triggered()), this, SLOT(scriptRun()));
    QObject::connect(ui->actionFileExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionEmulatorRun, SIGNAL(triggered()), this, SLOT(emulatorRun()));
    QObject::connect(ui->actionEmulatorReset, SIGNAL(triggered()), this, SLOT(emulatorReset()));
    QObject::connect(ui->actionactionEmulatorAutostart, SIGNAL(triggered()), this, SLOT(emulatorAutostart()));
    QObject::connect(ui->actionDrivesFloppy0, SIGNAL(triggered()), this, SLOT(emulatorFloppy0()));
    QObject::connect(ui->actionDrivesFloppy1, SIGNAL(triggered()), this, SLOT(emulatorFloppy1()));
    QObject::connect(ui->actionDrivesFloppy2, SIGNAL(triggered()), this, SLOT(emulatorFloppy2()));
    QObject::connect(ui->actionDrivesFloppy3, SIGNAL(triggered()), this, SLOT(emulatorFloppy3()));
    QObject::connect(ui->actionDrivesCartridge1, SIGNAL(triggered()), this, SLOT(emulatorCartridge1()));
    QObject::connect(ui->actionDrivesHard1, SIGNAL(triggered()), this, SLOT(emulatorHardDrive1()));
    QObject::connect(ui->actionDrivesCartridge2, SIGNAL(triggered()), this, SLOT(emulatorCartridge2()));
    QObject::connect(ui->actionDrivesHard2, SIGNAL(triggered()), this, SLOT(emulatorHardDrive2()));
    QObject::connect(ui->actionDebugConsoleView, SIGNAL(triggered()), this, SLOT(debugConsoleView()));
    QObject::connect(ui->actionDebugDisasmView, SIGNAL(triggered()), this, SLOT(debugDisasmView()));
    QObject::connect(ui->actionDebugMemoryView, SIGNAL(triggered()), this, SLOT(debugMemoryView()));
    QObject::connect(ui->actionDebugCpuPpu, SIGNAL(triggered()), this, SLOT(debugCpuPpu()));
    QObject::connect(ui->actionDebugStepInto, SIGNAL(triggered()), this, SLOT(debugStepInto()));
    QObject::connect(ui->actionDebugStepOver, SIGNAL(triggered()), this, SLOT(debugStepOver()));
    QObject::connect(ui->actionDebugClearConsole, SIGNAL(triggered()), this, SLOT(debugClearConsole()));
    QObject::connect(ui->actionDebugRemoveAllBreakpoints, SIGNAL(triggered()), this, SLOT(debugRemoveAllBreakpoints()));
    QObject::connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
    QObject::connect(ui->actionViewKeyboard, SIGNAL(triggered()), this, SLOT(viewKeyboard()));
    QObject::connect(ui->actionViewRgbScreen, SIGNAL(triggered()), this, SLOT(viewRgbScreen()));
    QObject::connect(ui->actionViewGrbScreen, SIGNAL(triggered()), this, SLOT(viewGrbScreen()));
    QObject::connect(ui->actionViewGrayscaleScreen, SIGNAL(triggered()), this, SLOT(viewGrayscaleScreen()));
    QObject::connect(ui->actionViewSizeRegular, SIGNAL(triggered()), this, SLOT(viewSizeRegular()));
    QObject::connect(ui->actionViewSizeDoubleInterlaced, SIGNAL(triggered()), this, SLOT(viewSizeDoubleInterlaced()));
    QObject::connect(ui->actionViewSizeDouble, SIGNAL(triggered()), this, SLOT(viewSizeDouble()));
    QObject::connect(ui->actionViewSizeUpscaled, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled()));
    QObject::connect(ui->actionViewSizeUpscaled3, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled3()));
    QObject::connect(ui->actionViewSizeUpscaled4, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled4()));
    QObject::connect(ui->actionViewSizeUpscaled175, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled175()));
    QObject::connect(ui->actionViewSizeUpscaled5, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled5()));
    QObject::connect(ui->actionViewSizeUpscaled6, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled6()));
    QObject::connect(ui->actionSoundEnabled, SIGNAL(triggered()), this, SLOT(soundEnabled()));
    QObject::connect(ui->actionSoundAY, SIGNAL(triggered()), this, SLOT(emulatorSoundAY()));

    // Screen and keyboard
    m_screen = new QEmulatorScreen();
    m_keyboard = new QKeyboardView();
    m_console = new QConsoleView();
    m_debug = new QDebugView(this);
    m_disasm = new QDisasmView();
    m_memory = new QMemoryView();

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(4);
    vboxlayout->setSpacing(4);
    vboxlayout->addWidget(m_screen);
    vboxlayout->addWidget(m_keyboard);
    ui->centralWidget->setLayout(vboxlayout);
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    int maxwid = m_screen->maximumWidth() > m_keyboard->maximumWidth() ? m_screen->maximumWidth() : m_keyboard->maximumWidth();
    ui->centralWidget->setMaximumWidth(maxwid);

    m_dockDebug = new QDockWidget(tr("Debug"));
    m_dockDebug->setObjectName("dockDebug");
    m_dockDebug->setWidget(m_debug);
    m_dockDebug->setFeatures(m_dockDebug->features() & ~QDockWidget::DockWidgetClosable);
    m_dockDisasm = new QDockWidget(tr("Disassemble"));
    m_dockDisasm->setObjectName("dockDisasm");
    m_dockDisasm->setWidget(m_disasm);
    m_dockMemory = new QDockWidget(tr("Memory"));
    m_dockMemory->setObjectName("dockMemory");
    m_dockMemory->setWidget(m_memory);
    m_dockConsole = new QDockWidget(tr("Debug Console"));
    m_dockConsole->setObjectName("dockConsole");
    m_dockConsole->setWidget(m_console);
    m_dockConsole->setFeatures(m_dockConsole->features() & ~QDockWidget::DockWidgetClosable);

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDebug, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDisasm, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockMemory, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockConsole);

    m_statusLabelInfo = new QLabel(this);
    m_statusLabelFrames = new QLabel(this);
    m_statusLabelUptime = new QLabel(this);
    statusBar()->addWidget(m_statusLabelInfo, 600);
    statusBar()->addPermanentWidget(m_statusLabelFrames, 150);
    statusBar()->addPermanentWidget(m_statusLabelUptime, 150);

    this->setFocusProxy(m_screen);

    autoStartProcessed = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_screen;
    delete m_keyboard;
    delete m_console;
    delete m_debug;
    delete m_disasm;
    delete m_memory;
    delete m_dockConsole;
    delete m_dockDebug;
    delete m_dockDisasm;
    delete m_dockMemory;
    delete m_statusLabelInfo;
    delete m_statusLabelFrames;
    delete m_statusLabelUptime;
}

void MainWindow::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);

    // Process Autostart/Autoboot on first show event
    if (!autoStartProcessed)
    {
        if (Settings_GetAutostart() || Option_AutoBoot > 0)
        {
            QTimer::singleShot(0, this, SLOT(emulatorRun()));
        }

        autoStartProcessed = true;
    }
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    Global_getSettings()->setValue("MainWindow/ScreenViewMode", m_screen->mode());
    Global_getSettings()->setValue("MainWindow/ScreenSizeMode", m_screen->sizeMode());

    Global_getSettings()->setValue("MainWindow/Geometry", saveGeometry());
    Global_getSettings()->setValue("MainWindow/WindowState", saveState());

    Global_getSettings()->setValue("MainWindow/OnscreenKeyboard", m_keyboard->isVisible());
    Global_getSettings()->setValue("MainWindow/ConsoleView", m_dockConsole->isVisible());
    Global_getSettings()->setValue("MainWindow/DebugView", m_dockDebug->isVisible());
    Global_getSettings()->setValue("MainWindow/DisasmView", m_dockDisasm->isVisible());
    Global_getSettings()->setValue("MainWindow/MemoryView", m_dockMemory->isVisible());

    QMainWindow::closeEvent(event);
}

void MainWindow::restoreSettings()
{
    ScreenViewMode scrViewMode = (ScreenViewMode)Global_getSettings()->value("MainWindow/ScreenViewMode").toInt();
    if (scrViewMode == 0) scrViewMode = GRBScreen;
    m_screen->setMode(scrViewMode);
    ScreenSizeMode scrSizeMode = (ScreenSizeMode)Global_getSettings()->value("MainWindow/ScreenSizeMode").toInt();
    if (scrSizeMode == 0) scrSizeMode = RegularScreen;
    m_screen->setSizeMode(scrSizeMode);

    // Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    int maxwid = m_screen->maximumWidth() > m_keyboard->maximumWidth() ? m_screen->maximumWidth() : m_keyboard->maximumWidth();
    ui->centralWidget->setMaximumWidth(maxwid);

    QByteArray geometry = Global_getSettings()->value("MainWindow/Geometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
    if (isMaximized())  //HACK for restoring maximized window, see https://bugreports.qt.io/browse/QTBUG-46620
        setGeometry(QApplication::desktop()->availableGeometry(this));
    restoreState(Global_getSettings()->value("MainWindow/WindowState").toByteArray());

    m_keyboard->setVisible(Global_getSettings()->value("MainWindow/OnscreenKeyboard", false).toBool());
    m_dockConsole->setVisible(Global_getSettings()->value("MainWindow/ConsoleView", false).toBool());
    m_dockDebug->setVisible(Global_getSettings()->value("MainWindow/DebugView", false).toBool());
    m_dockDisasm->setVisible(Global_getSettings()->value("MainWindow/DisasmView", false).toBool());
    m_dockMemory->setVisible(Global_getSettings()->value("MainWindow/MemoryView", false).toBool());

    ui->actionSoundEnabled->setChecked(Settings_GetSound());
    ui->actionSoundAY->setChecked(Settings_GetSoundAY());
    m_debug->updateWindowText();
    m_disasm->updateWindowText();
    m_memory->updateWindowText();
}

void MainWindow::updateMenu()
{
    ui->actionEmulatorRun->setChecked(g_okEmulatorRunning);
    ui->actionactionEmulatorAutostart->setChecked(Settings_GetAutostart());
    ui->actionViewRgbScreen->setChecked(m_screen->mode() == RGBScreen);
    ui->actionViewGrbScreen->setChecked(m_screen->mode() == GRBScreen);
    ui->actionViewGrayscaleScreen->setChecked(m_screen->mode() == GrayScreen);
    ui->actionViewSizeRegular->setChecked(m_screen->sizeMode() == RegularScreen);
    ui->actionViewSizeUpscaled->setChecked(m_screen->sizeMode() == UpscaledScreen);
    ui->actionViewSizeDoubleInterlaced->setChecked(m_screen->sizeMode() == DoubleInterlacedScreen);
    ui->actionViewSizeDouble->setChecked(m_screen->sizeMode() == DoubleScreen);
    ui->actionViewSizeUpscaled3->setChecked(m_screen->sizeMode() == UpscaledScreen3);
    ui->actionViewSizeUpscaled4->setChecked(m_screen->sizeMode() == UpscaledScreen4);
    ui->actionViewSizeUpscaled175->setChecked(m_screen->sizeMode() == UpscaledScreen175);
    ui->actionViewSizeUpscaled5->setChecked(m_screen->sizeMode() == UpscaledScreen5);
    ui->actionViewSizeUpscaled6->setChecked(m_screen->sizeMode() == UpscaledScreen6);

    ui->actionViewKeyboard->setChecked(m_keyboard->isVisible());

    ui->actionDrivesFloppy0->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(0) ? ":/images/iconFloppy.svg" : ":/images/iconFloppySlot.svg" ));
    ui->actionDrivesFloppy1->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(1) ? ":/images/iconFloppy.svg" : ":/images/iconFloppySlot.svg" ));
    ui->actionDrivesFloppy2->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(2) ? ":/images/iconFloppy.svg" : ":/images/iconFloppySlot.svg" ));
    ui->actionDrivesFloppy3->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(3) ? ":/images/iconFloppy.svg" : ":/images/iconFloppySlot.svg" ));

    ui->actionDrivesCartridge1->setIcon(QIcon(
            g_pBoard->IsROMCartridgeLoaded(1) ? ":/images/iconCartridge.svg" : ":/images/iconCartridgeSlot.svg" ));
    ui->actionDrivesCartridge2->setIcon(QIcon(
            g_pBoard->IsROMCartridgeLoaded(2) ? ":/images/iconCartridge.svg" : ":/images/iconCartridgeSlot.svg" ));

    ui->actionDrivesHard1->setIcon(QIcon(
            g_pBoard->IsHardImageAttached(1) ? ":/images/iconHdd.svg" : ":/images/iconHddSlot.svg" ));
    ui->actionDrivesHard2->setIcon(QIcon(
            g_pBoard->IsHardImageAttached(2) ? ":/images/iconHdd.svg" : ":/images/iconHddSlot.svg" ));

    ui->actionDebugConsoleView->setChecked(m_console->isVisible());
    ui->actionDebugDisasmView->setChecked(m_dockDisasm->isVisible());
    ui->actionDebugMemoryView->setChecked(m_dockMemory->isVisible());

    if (m_debug != nullptr)
        m_debug->updateToolbar();
}

void MainWindow::updateAllViews()
{
    Emulator_OnUpdate();

    if (m_debug != nullptr)
        m_debug->updateData();
    if (m_disasm != nullptr)
        m_disasm->updateData();
    if (m_memory != nullptr)
        m_memory->updateData();
    if (m_console != nullptr)
        m_console->updatePrompt();

    m_screen->repaint();
    if (m_debug != nullptr)
        m_debug->repaint();
    if (m_disasm != nullptr)
        m_disasm->repaint();
    if (m_memory != nullptr)
        m_memory->repaint();

    updateMenu();
}

void MainWindow::redrawDebugView()
{
    if (m_debug != nullptr)
        m_debug->repaint();
}
void MainWindow::redrawDisasmView()
{
    if (m_disasm != nullptr)
        m_disasm->repaint();
}

void MainWindow::updateWindowText()
{
    if (g_okEmulatorRunning)
        this->setWindowTitle(tr("UKNC Back to Life [run]"));
    else
        this->setWindowTitle(tr("UKNC Back to Life [stop]"));
}

void MainWindow::setCurrentProc(bool okProc)
{
    if (m_debug != nullptr)
        m_debug->setCurrentProc(okProc);
    if (m_disasm != nullptr)
        m_disasm->setCurrentProc(okProc);
    if (m_console != nullptr)
        m_console->setCurrentProc(okProc);
}

void MainWindow::showUptime(int uptimeMillisec)
{
    int seconds = (int) (uptimeMillisec % 60);
    int minutes = (int) (uptimeMillisec / 60 % 60);
    int hours   = (int) (uptimeMillisec / 3600 % 60);

    char buffer[20];
    _snprintf(buffer, 20, "%02d:%02d:%02d", hours, minutes, seconds);
    m_statusLabelUptime->setText(tr("Uptime: %1").arg(buffer));
}
void MainWindow::showFps(double framesPerSecond)
{
    if (framesPerSecond <= 0)
    {
        m_statusLabelFrames->setText("");
    }
    else
    {
        double speed = framesPerSecond / 25.0 * 100.0;
        char buffer[16];
        _snprintf(buffer, 16, "%03.f%%", speed);
        m_statusLabelFrames->setText(buffer);
    }
}

void MainWindow::selectLanguage(const QString& lang)
{
    Global_getSettings()->setValue("Language", lang);

    AlertInfo(tr("Restart the application to apply the language selection."));
}

void MainWindow::saveStateImage()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("UKNC state images (*.uknc)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveStateImage(strFileName);
}
void MainWindow::saveStateImage(const QString& strFileName)
{
    const char * sFileName = qPrintable(strFileName);
    Emulator_SaveImage(sFileName);
}
void MainWindow::loadStateImage()
{
    QFileDialog dlg;
    dlg.setNameFilter(tr("UKNC state images (*.uknc)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    loadStateImage(strFileName);
}
void MainWindow::loadStateImage(const QString& strFileName)
{
    const char * sFileName = qPrintable(strFileName);
    Emulator_LoadImage(sFileName);

    updateAllViews();
}

void MainWindow::saveScreenshot()
{
    QString strFileName = QString("%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));

    saveScreenshot(strFileName);
}
void MainWindow::saveScreenshotAs()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("PNG images (*.png)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveScreenshot(strFileName);
}
void MainWindow::saveScreenshot(const QString& strFileName)
{
    QImage image = m_screen->getScreenshot();
    image.save(strFileName, "PNG", -1);
}

void MainWindow::screenshotToClipboard()
{
    QImage image = m_screen->getScreenshot();

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->clear();
    clipboard->setImage(image);
}

void MainWindow::screenTextToClipboard()
{
    uint8_t buffer[81 * 26 + 1];
    memset(buffer, 0, sizeof(buffer));

    if (!m_screen->getScreenText(buffer))
    {
        AlertWarning(tr("Failed to prepare text clipboard from screen."));
        return;
    }

    // Prepare Unicode text
    QString text;
    for (size_t i = 0; i < sizeof(buffer) - 1; i++)
    {
        text.append(Translate_KOI8R(buffer[i]));
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->clear();
    clipboard->setText(text);
}

void MainWindow::helpAbout()
{
    QAboutDialog dialog(nullptr);
    dialog.exec();
}

void MainWindow::viewKeyboard()
{
    m_keyboard->setVisible(!m_keyboard->isVisible());
    updateMenu();
}

void MainWindow::viewRgbScreen()
{
    m_screen->setMode(RGBScreen);
    updateMenu();
}
void MainWindow::viewGrbScreen()
{
    m_screen->setMode(GRBScreen);
    updateMenu();
}
void MainWindow::viewGrayscaleScreen()
{
    m_screen->setMode(GrayScreen);
    updateMenu();
}

void MainWindow::updateCentralWidgetSize()
{
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewSizeRegular()
{
    m_screen->setSizeMode(RegularScreen);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeUpscaled()
{
    m_screen->setSizeMode(UpscaledScreen);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeDoubleInterlaced()
{
    m_screen->setSizeMode(DoubleInterlacedScreen);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeDouble()
{
    m_screen->setSizeMode(DoubleScreen);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeUpscaled3()
{
    m_screen->setSizeMode(UpscaledScreen3);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeUpscaled4()
{
    m_screen->setSizeMode(UpscaledScreen4);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeUpscaled175()
{
    m_screen->setSizeMode(UpscaledScreen175);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeUpscaled5()
{
    m_screen->setSizeMode(UpscaledScreen5);
    updateMenu();
    updateCentralWidgetSize();
}
void MainWindow::viewSizeUpscaled6()
{
    m_screen->setSizeMode(UpscaledScreen6);
    updateMenu();
    updateCentralWidgetSize();
}

void MainWindow::emulatorFrame()
{
    if (!g_okEmulatorRunning)
        return;
    if (!isActiveWindow())
        return;

    if (!Emulator_SystemFrame())
        Emulator_Stop();  // Breakpoint hit

    m_screen->repaint();
}

void MainWindow::emulatorRun()
{
    if (g_okEmulatorRunning)
        Emulator_Stop();
    else
        Emulator_Start();
    updateWindowText();
    updateMenu();
}

void MainWindow::emulatorReset()
{
    Emulator_Reset();

    m_screen->repaint();
}

void MainWindow::emulatorAutostart()
{
    Settings_SetAutostart(!Settings_GetAutostart());
    updateMenu();
}

void MainWindow::soundEnabled()
{
    bool sound = ui->actionSoundEnabled->isChecked();
    Emulator_SetSound(sound);
    Settings_SetSound(sound);
}

void MainWindow::emulatorSoundAY()
{
    bool sound = ui->actionSoundAY->isChecked();
    Emulator_SetSoundAY(sound);
    Settings_SetSoundAY(sound);
}

void MainWindow::emulatorCartridge1() { emulatorCartridge(1); }
void MainWindow::emulatorCartridge2() { emulatorCartridge(2); }
void MainWindow::emulatorCartridge(int slot)
{
    if (g_pBoard->IsROMCartridgeLoaded(slot))
    {
        detachCartridge(slot);
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter(tr("UKNC ROM cartridge images (*.bin)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);
        if (!attachCartridge(slot, strFileName))
            return;
    }
}
bool MainWindow::attachCartridge(int slot, const QString & strFileName)
{
    QFileInfo fi(strFileName);
    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

    LPCTSTR sFileName = qPrintable(strFullName);
    Emulator_LoadROMCartridge(slot, sFileName);
    //TODO: Check result

    Settings_SetCartridgeFilePath(slot, strFullName);

    updateMenu();

    return true;
}
void MainWindow::detachCartridge(int slot)
{
    g_pBoard->UnloadROMCartridge(slot);

    Settings_SetCartridgeFilePath(slot, nullptr);

    updateMenu();
}

void MainWindow::emulatorFloppy0() { emulatorFloppy(0); }
void MainWindow::emulatorFloppy1() { emulatorFloppy(1); }
void MainWindow::emulatorFloppy2() { emulatorFloppy(2); }
void MainWindow::emulatorFloppy3() { emulatorFloppy(3); }
void MainWindow::emulatorFloppy(int slot)
{
    if (g_pBoard->IsFloppyImageAttached(slot))
    {
        detachFloppy(slot);
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter(tr("UKNC floppy images (*.dsk *.rtd)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);

        if (! attachFloppy(slot, strFileName))
        {
            AlertWarning(tr("Failed to attach floppy image."));
            return;
        }
    }
}
bool MainWindow::attachFloppy(int slot, const QString & strFileName)
{
    QFileInfo fi(strFileName);
    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

    QByteArray utf8FullName = strFullName.toUtf8();
    const char *sFileName = utf8FullName.constData();
    if (! g_pBoard->AttachFloppyImage(slot, sFileName))
        return false;

    Settings_SetFloppyFilePath(slot, strFullName);

    updateMenu();

    return true;
}
void MainWindow::detachFloppy(int slot)
{
    g_pBoard->DetachFloppyImage(slot);

    Settings_SetFloppyFilePath(slot, nullptr);

    updateMenu();
}

void MainWindow::emulatorHardDrive1() { emulatorHardDrive(1); }
void MainWindow::emulatorHardDrive2() { emulatorHardDrive(2); }
void MainWindow::emulatorHardDrive(int slot)
{
    if (g_pBoard->IsHardImageAttached(slot))
    {
        detachHardDrive(slot);
    }
    else
    {
        // Check if cartridge (HDD ROM image) already selected
        bool okCartLoaded = g_pBoard->IsROMCartridgeLoaded(slot);
        if (!okCartLoaded)
        {
            AlertWarning(tr("Please select HDD ROM image as cartridge first."));
            return;
        }

        // Select HDD disk image
        QFileDialog dlg;
        dlg.setNameFilter(tr("UKNC HDD images (*.img)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);
        if (! attachHardDrive(slot, strFileName))
        {
            AlertWarning(tr("Failed to attach hard drive image."));
            return;
        }
    }
}
bool MainWindow::attachHardDrive(int slot, const QString & strFileName)
{
    if (!g_pBoard->IsROMCartridgeLoaded(slot))
        return false;

    QFileInfo fi(strFileName);
    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

    LPCTSTR sFileName = qPrintable(strFullName);
    if (!g_pBoard->AttachHardImage(slot, sFileName))
        return false;

    Settings_SetHardFilePath(slot, strFullName);

    updateMenu();

    return true;
}
void MainWindow::detachHardDrive(int slot)
{
    g_pBoard->DetachHardImage(slot);
    Settings_SetHardFilePath(slot, nullptr);
}

void MainWindow::debugConsoleView()
{
    bool okShow = !m_dockConsole->isVisible();
    m_dockConsole->setVisible(okShow);
    m_dockDebug->setVisible(okShow);
    m_dockDisasm->setVisible(okShow);
    m_dockMemory->setVisible(okShow);

    if (!okShow)
    {
        if (this->isMaximized())
            this->showNormal();
        this->adjustSize();
    }

    updateMenu();
}
void MainWindow::debugDisasmView()
{
    m_dockDisasm->setVisible(!m_dockDisasm->isVisible());
    updateMenu();
}
void MainWindow::debugMemoryView()
{
    m_dockMemory->setVisible(!m_dockMemory->isVisible());
    updateMenu();
}

void MainWindow::debugCpuPpu()
{
    if (!g_okEmulatorRunning)
        m_debug->switchCpuPpu();
}

void MainWindow::debugStepInto()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("s");
}
void MainWindow::debugStepOver()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("so");
}

void MainWindow::debugClearConsole()
{
    m_console->clear();
}

void MainWindow::debugRemoveAllBreakpoints()
{
    m_console->execConsoleCommand("bc");
}

void MainWindow::scriptRun()
{
    if (g_okEmulatorRunning)
        emulatorRun();  // Stop the emulator

    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setNameFilter(tr("Script files (*.js)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);
    QFile file(strFileName);
    file.open(QIODevice::ReadOnly);
    QString strScript = file.readAll();

    QScriptWindow window(this);
    window.runScript(strScript);
}

void MainWindow::consolePrint(const QString &message)
{
    m_console->printLine(message);
}
