#include "iirlib.h"
#include "connection.h"

// sample program as client

int main(void)
{
	Connection *to_server = new Connection (CONNECTION_SOCKET | CONNECTION_CLIENT | CONNECTION_C);
	to_server->SetName ("test client");
	if (to_server->Open ("localhost", NULL, 9000)==CONNECTION_FAIL)
		{
			fprintf (stderr, "connect socket error\n");
			abort();
		}
	while (1)
		{
			double data;
			int    ret;
			data = rand();

			ret = to_server->Send (CONNECTION_DOUBLE, (gpointer)&data);
			if (ret==CONNECTION_FAIL)
				{
					fprintf (stderr, "socket error\n");
					abort();
				}
			fprintf (stderr, "sending data (%g)\n", data);
		}
	return 1;
}
