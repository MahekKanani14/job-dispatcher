#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "../include/job.h"

// Opens logs/results.csv, writing the header if the file is new/empty.
void logger_init(const std::string& path);
void logger_close();

void log_dispatch(const Job& job, int worker_pid, const char* algo);
void log_completion(const JobResult& result, const char* algo);

#endif // LOGGER_H
