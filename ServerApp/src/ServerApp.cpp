
#include "server/server.h"
#include "single/Authenticator.h"

using namespace std;

int main() {
	////////////////////
	ServerTCP* server = new ServerTCP();
	int result;
	/* open socket. */
	try
	{
		result = server->open();
	}
	catch(ServerException e)
	{
		e.what();
	}
	std::cout << "Server socket open result: " << result << std::endl;
	/* listen. */
	try
	{
 		server->loop();
	}
	catch (ServerException e)
	{
		e.what();
	}
	/* close socket. */
	try
	{
		result = server->shutdown();
	}
	catch (ServerException e)
	{
		e.what();
	}
	std::cout << "Server socket close result: " << result << std::endl;

	/* free memory. */
	delete server;
	Authenticator::deleteInstance();
	return 0;
}

/**
 * de folosit thread poolul din c++.
 * studiu piata analiza comparativ cu features
 * tipuri specificare tehnologii alese side ce
 * aritectura aleasa cu desen
 * analiza use case cu explicatii
 * arhitectura pe module.
 * arhitectur de deployment
 * detaliere clase, diagrame, explivcatii cfct mai importante, cele dezv de mine
 * la sfarsit diagrama cu toate clasele a3 ca anexa.
 * testele: incarcare mem si procesor,cleitn server.
 * concluzii.
 *
 * instalez wps, de verificat facilitati creare bibliografii ca office.
 * de descarcat directorul de fonturi, update cache fonts din windows.
 */
