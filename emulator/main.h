#ifndef MAIN_H
#define MAIN_H

//////////////////////////////////////////////////////////////////////

#include <QtGlobal>

class QApplication;
class MainWindow;
class QString;
class QSettings;


QApplication* Global_getApplication();
MainWindow* Global_getMainWindow();
QSettings* Global_getSettings();
void Global_UpdateAllViews();
void Global_UpdateMenu();
void Global_RedrawDebugView();
void Global_RedrawDisasmView();
void Global_SetCurrentProc(bool);
void Global_showUptime(int uptimeMillisec);
void Global_showFps(double framesPerSecond);
void Global_loadTranslation(const QString &filename);


//////////////////////////////////////////////////////////////////////
// Settings

class Settings {
private:
    QSettings *m_pSettings;

public:
    Settings();
    ~Settings();

    void SetStringValue(const char *name, const char *value);
    const char *GetStringValue(const char *name);

    void SetEnumStringValue(const char *name, unsigned int num, const char *value);
    const char *GetEnumStringValue(const char *name, unsigned int num);
};

void Settings_SetFloppyFilePath(int slot, const QString &sFilePath);
QString Settings_GetFloppyFilePath(int slot);
void Settings_SetCartridgeFilePath(int slot, const QString &sFilePath);
QString Settings_GetCartridgeFilePath(int slot);
void Settings_SetHardFilePath(int slot, const QString &sFilePath);
QString Settings_GetHardFilePath(int slot);
void Settings_SetAutostart(bool flag);
bool Settings_GetAutostart();
void Settings_SetSound(bool flag);
bool Settings_GetSound();
void Settings_SetSoundAY(bool flag);
bool Settings_GetSoundAY();
bool Settings_GetDebugCpuPpu();
void Settings_SetDebugCpuPpu(bool flag);
void Settings_SetDebugMemoryMode(quint16 mode);
quint16 Settings_GetDebugMemoryMode();
void Settings_SetDebugMemoryAddress(quint16 address);
quint16 Settings_GetDebugMemoryAddress();
bool Settings_GetDebugMemoryByte();
void Settings_SetDebugMemoryByte(bool flag);
void Settings_SetDebugMemoryNumeral(quint16 mode);
quint16 Settings_GetDebugMemoryNumeral();


//////////////////////////////////////////////////////////////////////
// Options

extern bool Option_ShowHelp;
extern int Option_AutoBoot;


//////////////////////////////////////////////////////////////////////
#endif // MAIN_H
