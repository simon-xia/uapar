#include "up_common.h"

Log_t *global_log;

int up_log_init(Log_t** new_log, char *filename, unsigned level)
{
	(*new_log) = (Log_t*)malloc(sizeof(Log_t));
	assert((*new_log) != NULL);
	
	if (filename) {
		(*new_log) -> fp = fopen(filename, "w");
		if (!(*new_log)->fp) 
			return UP_ERR;
		/*
		(*new_log) -> buf = (char*)malloc(sizeof(char)*LOG_BUF_SIZE);
		assert((*new_log) -> buf != NULL);	
		(*new_log) -> buf_size = 0;
		*/
	}
	else {
		(*new_log) -> fp = stderr;
	//	(*new_log) -> buf = NULL;
	//	(*new_log) -> buf_size = 0;
	}

	//(*new_log) -> pos = 0;
	(*new_log) -> level = level;

	return UP_SUCC;
}

int up_log_global_init(char *filename, unsigned level)
{
	return up_log_init(&global_log, filename, level);
}

void up_log_uninit(Log_t *log)
{
	if (log -> fp != stderr)
		fclose(log -> fp);
	//free(log -> buf);
	free(log);
}

void up_log_global_uninit()
{
	return up_log_uninit(global_log);
}

/*
void up_log_flush_buf(Log_t *log)
{
	if (log -> fp != stderr) {
		fwrite(log -> buf, sizeof(char), log -> pos, log -> fp);
		log -> pos = 0;
	}
}

void up_log_write(Log_t *log, unsigned level, char *fmt, ...)
{
	if (level < log -> level)
		return ;

	if (log -> pos)
}

*/

#ifdef UNIT_TEST_LOG

int main()
{
	up_log_global_init(NULL, LOG_TRACE);
	WARNING("haha\n");
	ERROR("haha\n");
	DEBUG("haha: %d\n", 9);
	STATE("here is log test: %s\n", "success");
	up_log_global_uninit(global_log);
	return 0;
}

#endif
