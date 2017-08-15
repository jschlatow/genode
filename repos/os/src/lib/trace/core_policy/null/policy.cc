#include <trace/core_policy.h>

using namespace Genode;

size_t max_event_size()
{
	return 0;
}


size_t core_event(char *dst, char const *event_name)
{
	return 0;
}

size_t job_process(char *dst, unsigned int job_id)
{
	return 0;
}
