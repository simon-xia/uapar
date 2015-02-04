#ifndef _UP_LOG_H
#define _UP_LOG_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOG_BUF_SIZE	4096

typedef enum log_level {
	LOG_TRACE,	
	LOG_DEBUG,	
	LOG_WARNING,	
	LOG_ERROR,
	LOG_STATE	
}Log_level;

typedef struct log_t {
	FILE *fp;
	Log_level level;
	/*
	char *buf;
	unsigned	buf_size;
	unsigned	pos;
	*/
}Log_t;

extern Log_t* global_log;

#define TRACE(s, arg...) \
	do { \
		if (LOG_TRACE >= global_log -> level) \
			fprintf(global_log -> fp, "[%s:%d] %s:\t"s, __FILE__, __LINE__, __func__, ##arg); \
	}while(0)

#define DEBUG(s, arg...) \
	do { \
		if (LOG_DEBUG >= global_log -> level) \
			fprintf(global_log -> fp, "[%s:%d] %s:\t"s, __FILE__, __LINE__, __func__, ##arg); \
	}while(0)

#define WARNING(s, arg...) \
	do { \
		if (LOG_WARNING >= global_log -> level) \
			fprintf(global_log -> fp, "\033[1;33m == warning == \033[0m [%s:%d] %s:\t"s, __FILE__, __LINE__, __func__, ##arg); \
	}while(0)

#define ERROR(s, arg...) \
	do { \
		if (LOG_ERROR >= global_log -> level) \
			fprintf(global_log -> fp, "\033[1;31m ==  error  == \033[0m [%s:%d] %s:\t"s, __FILE__, __LINE__, __func__, ##arg); \
	}while(0)

#define STATE(s, arg...) \
	do { \
		time_t tv;	\
		char tmp[30] = {0}; \
		time(&tv);	\
		sprintf(tmp, "%s", ctime(&tv)); \
		tmp[strlen(tmp) - 1] = 0x00; \
		if (LOG_STATE >= global_log -> level) \
			fprintf(global_log -> fp, "\033[1;32m %s: \033[0m \t"s, tmp, ##arg); \
	}while(0)



void up_log_init(Log_t**, char *, unsigned);
void up_log_global_init(char *, unsigned);
void up_log_uninit(Log_t *);
void up_log_global_uninit();

#endif
