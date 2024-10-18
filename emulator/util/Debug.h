// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <cstdint>

//////////////////////////////////////////////////////////////////////
// DebugPrint

typedef void (*TLoggerProc)(const char*);

void DebugSetup(TLoggerProc printer, TLoggerProc logger);
void DebugPrint(const char* pszFormat, ...);
void DebugLog(const char* pszFormat, ...);

void PrintOctalValue(char* buffer, uint16_t value);
