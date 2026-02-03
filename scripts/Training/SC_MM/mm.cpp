#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "Basic.h"
#include "Server.h"
#include "Client.h"

using namespace std;

void usage();
void runServer( int port, int numClient );
void runClient( string sIP, string port, int numThread );

int main( int argc, char* argv[] )
{
	if( argc==4 && strcmp(argv[1],"server")==0 ) {
		// mm server port numClinet
		Server server(atoi(argv[2]),atoi(argv[3]));
		server.run();
	} else if( argc==5 && strcmp(argv[1],"client")==0 ) {
		// mm client ip port numThread
		Client client(argv[2],argv[3],atoi(argv[4]));
		client.run();
	} else {
		usage();
	}
	
	return 0;
}

void usage()
{
	cerr << errorColor("Choose from following two mode:") << endl;
	cerr << errorColor("\t./mm server port numClinet") << endl;
	cerr << errorColor("\t./mm client ip port numThread") << endl;
	exit(0);
}