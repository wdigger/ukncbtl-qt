#include "Debug.h"

#include <cstdarg>
#include <cstdio>

TLoggerProc g_Printer = nullptr;
TLoggerProc g_Logger = nullptr;

void DebugSetup(TLoggerProc printer, TLoggerProc logger)
{
    g_Printer = printer;
    g_Logger = logger;
}

void DebugPrint(const char* pszFormat, ...)
{
    if (g_Printer == nullptr)
        return;
    
    char buffer[512];
    
    va_list ptr;
    va_start(ptr, pszFormat);
    vsnprintf(buffer, 512, pszFormat, ptr);
    va_end(ptr);
    
    g_Printer(buffer);
}

void DebugLog(const char* pszFormat, ...)
{
    if (g_Logger == nullptr)
        return;
    
    char buffer[512];
    
    va_list ptr;
    va_start(ptr, pszFormat);
    vsnprintf(buffer, 512, pszFormat, ptr);
    va_end(ptr);
    
    g_Logger(buffer);
}

