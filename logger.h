//logger.h
/*
 * Uso:
 * LOG_PRINT("Hello World ");
 * LOG_PRINT("Zing is back !!! %s %d",s,x++);
 */
#ifndef DATOS_H
#include "datos.h"
#endif

#include "poller.h"

#define LOG_PRINT(...) log_print(__FILE__, __LINE__, __VA_ARGS__ )
