#include "logger.h"
#include <cstdio>
#include <chrono>

static FILE* csv_file = nullptr;

static long long now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void logger_init(const std::string& path) {
    // "a" (append) keeps the run history in one file.
    bool is_new_file = (fopen(path.c_str(), "r") == nullptr);
    csv_file = fopen(path.c_str(), "a");
    if (!csv_file) {
        perror("fopen results.csv");
        return;
    }
    if (is_new_file) {
        fprintf(csv_file, "timestamp_ms,event,job_id,worker_pid,algorithm,duration_ms\n");
        fflush(csv_file);
    }
}

void logger_close() {
    if (csv_file) {
        fclose(csv_file);
        csv_file = nullptr;
    }
}

void log_dispatch(const Job& job, int worker_pid, const char* algo) {
    printf("[master] job %-3d -> worker %-6d  (%s)\n", job.id, worker_pid, algo);
    if (csv_file) {
        fprintf(csv_file, "%lld,dispatch,%d,%d,%s,%d\n",
                now_ms(), job.id, worker_pid, algo, job.duration_ms);
        fflush(csv_file);
    }
}

void log_completion(const JobResult& result, const char* algo) {
    printf("[master] job %-3d DONE  (worker %-6d, %d ms)\n",
           result.job_id, result.worker_pid, result.duration_ms);
    if (csv_file) {
        fprintf(csv_file, "%lld,done,%d,%d,%s,%d\n",
                now_ms(), result.job_id, result.worker_pid, algo, result.duration_ms);
        fflush(csv_file);
    }
}
