#include "iirlib.h"
#include "connection.h"

// sample program as server

int main(void)
{
	Connection *to_client = new Connection (CONNECTION_SOCKET | CONNECTION_SERVER | CONNECTION_C);
	to_client->SetName ("test server");
	if (to_client->Open ("localhost", NULL, 9000)==CONNECTION_FAIL)
		{
			fprintf (stderr, "connect socket error\n");
			abort();
		}
	while (1)
		{
			double	data;
			
			to_client->Receive (CONNECTION_DOUBLE, (gpointer)&data);
			fprintf (stderr, "received data = %g\n", data);
		}
	return 1;
}
