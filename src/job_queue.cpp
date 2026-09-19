#include "job_queue.h"
#include <cstdlib>

std::vector<Job> generate_jobs(int count, int min_ms, int max_ms) {
    std::vector<Job> jobs;
    jobs.reserve(count);
    for (int i = 1; i <= count; ++i) {
        int span = (max_ms - min_ms) + 1;
        int duration = min_ms + (std::rand() % span);
        jobs.push_back(Job{i, duration});
    }
    return jobs;
}
