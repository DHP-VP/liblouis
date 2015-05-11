/* liblouis Braille Translation and Back-Translation  Library

   Copyright (C) 2014 ViewPlus Technologies, Inc. www.viewplus.com
   and the liblouis team. http://liblouis.org
   All rights reserved

   This file is free software; you can redistribute it and/or modify it
   under the terms of the Lesser or Library GNU General Public License 
   as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
   Library GNU General Public License for more details.

   You should have received a copy of the Library GNU General Public 
   License along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
   */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "louis.h"

void logWidecharBuf(logLevels level, const char *msg, const widechar *wbuf, int wlen)
{
  /* When calculating output size:
   * Each wdiechar is represented in hex, thus needing two bytes for each
   * byte in the widechar (sizeof(widechar) * 2)
   * Allow space for the "0x%X " formatting (+ 3)
   * Number of characters in widechar buffer (wlen * )
   * Give space for additional message (+ strlen(msg))
   * Remember the null terminator (+ 1)
   */
  int logBufSize = (wlen * ((sizeof(widechar) * 3) + 3)) + 3 + strlen(msg);
  char *logMsg = xmalloc(logBufSize, __FILE__, __FUNCTION__, __LINE__);
  char *p = logMsg;
  char *formatString;
  int i = 0;
  if (sizeof(widechar) == 2)
    formatString = "0x%04X ";
  else
    formatString = "0x%08X ";
  for (i = 0; i < strlen(msg); i++)
    logMsg[i] = msg[i];
  p += strlen(msg);
  for (i = 0; i < wlen; i++)
    {
      p += sprintf(p, formatString, wbuf[i]);
    }
	*p = '~';
	p++;
	*p = ' ';
	p++;
	for(i = 0; i < wlen; i++)
	{
		*p = (char)wbuf[i];
		p++;
	}	
  *p = '\0';
  logMessage(level, logMsg);
  xfree(logMsg, __FILE__, __FUNCTION__, __LINE__);
}

static void defaultLogCallback(int level, const char *message)
{
  lou_logPrint("%s", message); // lou_logPrint takes formatting, protect against % in message
}

static logcallback logCallbackFunction = defaultLogCallback;
void EXPORT_CALL lou_registerLogCallback(logcallback callback)
{
  if (callback == NULL)
    logCallbackFunction = defaultLogCallback;
  else
    logCallbackFunction = callback;
}

static logLevels logLevel = LOG_INFO;
void EXPORT_CALL lou_setLogLevel(logLevels level)
{
  logLevel = level;
}

void logMessage(logLevels level, const char *format, ...)
{
  if (format == NULL)
      return;
  if (level < logLevel)
      return;
  if (logCallbackFunction != NULL)
    {
#ifdef _WIN32
      float f = 2.3; // Needed to force VC++ runtime floating point support
#endif
      char *s;
      size_t len;
      va_list argp;
      va_start(argp, format);
      len = vsnprintf(0, 0, format, argp);
      va_end(argp);
      if ((s = xmalloc(len+1, __FILE__, __FUNCTION__, __LINE__)) != 0)
        {
          va_start(argp, format);
          vsnprintf(s, len+1, format, argp);
          va_end(argp);
          logCallbackFunction(level, s);
          xfree(s, __FILE__, __FUNCTION__, __LINE__);
        }
    }
}


static FILE *logFile = NULL;
static char initialLogFileName[256];

void EXPORT_CALL
lou_logFile (const char *fileName)
{
	if(logFile)
	{
		fclose(logFile);
		logFile = NULL;
	}
  if (fileName == NULL || fileName[0] == 0)
    return;
  if (initialLogFileName[0] == 0)
    strcpy (initialLogFileName, fileName);
  logFile = fopen (fileName, "a");
  if (logFile == NULL && initialLogFileName[0] != 0)
    logFile = fopen (initialLogFileName, "a");
  if (logFile == NULL)
    {
      fprintf (stderr, "Cannot open log file %s\n", fileName);
      logFile = stderr;
    }
}

void EXPORT_CALL
lou_logPrint (const char *format, ...)
{
#ifndef __SYMBIAN32__
  va_list argp;
  if (format == NULL)
    return;
  if (logFile == NULL)
    logFile = fopen (initialLogFileName, "a");
  if (logFile == NULL)
    logFile = stderr;
  va_start (argp, format);
  vfprintf (logFile, format, argp);
  fprintf (logFile, "\n");
  fflush(logFile);
  va_end (argp);
#endif
}

void EXPORT_CALL
lou_logEnd ()
{
  if (logFile != NULL)
    fclose (logFile);
  logFile = NULL;
}

void closeLogFile()
{
  if (logFile != NULL)
    fclose (logFile);
}