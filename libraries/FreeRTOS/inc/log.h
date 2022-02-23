#pragma once

#include <stdio.h>
#include <stdlib.h>

void log_init();

void LOG(LogLevel level, const char* file, const char* line, const char * fmt, ... ){

void LOG_C(LogLevel level, const char* file, const char* line, const char * fmt, ... ){
