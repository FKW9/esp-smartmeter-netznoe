#include "graphite.h"

#ifdef USE_GRAPHITE

// Graphite Client
WiFiClient graphiteClient;

/**
 * @brief Submit a metric to graphite
 *
 * @param unix_timestamp can be -1 to use current time of graphite host
 * @param metrics
 * @param value
 */
void submitToGraphite(time_t unix_timestamp, String metrics, float value)
{
	if (!graphiteClient.connect(GRAPHITE_HOST, GRAPHITE_PORT))
	{
		Serial.print("Sending data to graphite failed!");
		delay(200);
		return;
	}

	String payload = metrics + " " + value + " " + unix_timestamp + "\n";

	graphiteClient.print(payload);
	graphiteClient.stop();
}

#endif