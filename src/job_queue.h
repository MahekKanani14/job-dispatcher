#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <vector>
#include "job.h"

// Generates `count` jobs with duration_ms randomized in [min_ms, max_ms].
// Job ids start at 1 and increment.
std::vector<Job> generate_jobs(int count, int min_ms, int max_ms);

#endif // JOB_QUEUE_H
