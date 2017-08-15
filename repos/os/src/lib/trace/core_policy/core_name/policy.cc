#include <util/string.h>
#include <trace/core_policy.h>

using namespace Genode;

enum { MAX_EVENT_SIZE = 64 };

size_t max_event_size()
{
	return MAX_EVENT_SIZE;
}


size_t job_process(char *dst, unsigned int *job_id)
{
	return 0;
}
