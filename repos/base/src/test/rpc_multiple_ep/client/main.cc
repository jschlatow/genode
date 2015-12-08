/*
 * \brief  Test client for a server with multiple entrypoints
 * \author Johannes Schlatow
 * \date   2014-07-03
 */

/* Genode includes */
#include <base/env.h>
#include <log_session/connection.h>

using namespace Genode;


int main()
{
	Log_connection log;

	log.write("First message\n");

	/* should never be reached */
	log.write("Second message\n");

	return 0;
}
