#include "graphite.h"

// Graphite Client
WiFiClient graphiteClient;

/**
 * @brief Submit a metric to graphite
 *
 * @param unix_timestamp can be -1 to use current time of graphite host
 * @param metrics
 * @param value
 */
void submitToGraphite(String metrics, float value, time_t unix_timestamp)
{
	String payload = metrics + " " + value + " " + unix_timestamp + "\n";

	if (!graphiteClient.connect(GRAPHITE_HOST, GRAPHITE_PORT))
	{
		Serial.print("Sending data to graphite failed!");
		return;
	}

	graphiteClient.print(payload);
	graphiteClient.stop();
}
