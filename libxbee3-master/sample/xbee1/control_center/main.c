/*
	libxbee - a C/C++ library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with libxbee. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <xbee.h>
#include <data.h>	// include the structure for the data packets
#include <iostream>
#include <GetPot>
using namespace std;

// Function to print Help if need be
void print_help(const string Application)
{
    cout << endl;
    cout << "Example to use follow()-functions:" << endl << endl;
    cout << "USAGE:" << endl;
    cout << "--help, -h" << endl;
    cout << "       get some help about this program." << endl;
    cout << "--alpha number" << endl;
    cout << "       specify the value of alpha given as number." << endl;
    cout << "--beta number" << endl;
    cout << "       specify the value of beta given as number." << endl;
    cout << "--user string number" << endl;
    cout << "       specify user name as string and id as number" << endl;
    cout << "       (default = 'You' and '0x42')." << endl;
    cout << "       multiple users possible. " << endl;
    cout << "--size, --sz, -s number1 number2" << endl;
    cout << "       specify x and y sizes" << endl;
    cout << endl;
    exit(0);
}

// Function that allows to receive the data and print it.
void myCB(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {	
	if ((*pkt)->address.addr16_enabled) {
    		printf("src addr 16-bit (0x%02X%02X)\n", 
	   	(*pkt)->address.addr16[0], (*pkt)->address.addr16[1]);
  	}
  	fflush(stdout);
  	if ((*pkt)->dataLen > sizeof(Header)) {
		Header header; 
		memcpy(&header, (*pkt)->data, sizeof(Header));
		printf("Header: %d ", (int)header.type);	
  		if (header.type == DATA_GPS )
	    	{
	      		if((*pkt)->dataLen == sizeof(Header)+sizeof(GPS_pkt))
			{
		  		GPS_pkt packet;
		  		memcpy(&packet, (*pkt)->data + sizeof(Header), sizeof(GPS_pkt));
		  		printf("rx:  ");
		  		print_GPS(packet);
			}
	    	}
	}
}

// Function to initialize GPS packets
void initializeGPS(GPS_pkt *data){	
	data->latitude = 1.0;
	data->longitude = 2.0;
	data->altitude = 3.0;
	data->dataRate = 10; 
}

// Function to initialize Routing packets
void initializeROUTING(Routing_pkt *data){	
	data->route = 12345;
}

// Function to initialize Plan packets
void initializePLAN(Plan_pkt *data){	
	data->plan = 9876543; 
}

/////////////// Beginning of Main program //////////////////
int main(int argc, char * argv[]) {
	
	// Simple Command line parser
    	GetPot   cl(argc, argv);
    	if(cl.search(2, "--help", "-h") ) print_help(cl[0]);
    	cl.init_multiple_occurrence();
    	const string  xbeeDev  = cl.follow("/dev/ttyUSB0", "--dev");      
   	const int     baudrate    = cl.follow(57600, "--baud"); 
    	cl.enable_loop();
  
	// Initialize structures
	void *d;
	struct xbee *xbee;
	struct xbee_con *con;
	struct xbee_conAddress address;
	xbee_err ret;
	struct xbee_conSettings settings;
	unsigned char buf[128];
	size_t buflen; 

	// Initialize data structure	
	Header hdr;	
	GPS_pkt gps; 	
	initializeGPS(&gps);

	Routing_pkt route;
	initializeROUTING(&route);

	Plan_pkt plan;
	initializePLAN(&plan);

	// Set up Communication
	if ((ret = xbee_setup(&xbee, "xbee1", xbeeDev.c_str(), baudrate)) != XBEE_ENONE) {
		printf("ret: %d (%s)\n", ret, xbee_errorToStr(ret));
		return ret;
	}

	// Set destination address: 0xFFFF for broadcast
	memset(&address, 0, sizeof(address));
	address.addr16_enabled = 1;
	address.addr16[0] = 0xFF;
	address.addr16[1] = 0xFF;

	if ((ret = xbee_conNew(xbee, &con, "16-bit Data", &address)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conNew() returned: %d (%s)", ret, xbee_errorToStr(ret));
		return ret;
	}

	if ((ret = xbee_conDataSet(con, xbee, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conDataSet() returned: %d", ret);
		return ret;
	}

	// Setting Catch-All function
	xbee_conSettings(con, NULL, &settings);
	settings.catchAll = 1;
	xbee_conSettings(con, &settings, NULL);
	if ((ret = xbee_conCallbackSet(con, myCB, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conCallbackSet() returned: %d", ret);
		return ret;
	}
	int cnt=0;

	// Broadcast "Data"
	for (;;) {
		if(cnt%2)
		{
			hdr.type = DATA_GPS;
			memcpy(buf, &hdr, sizeof(Header));
			memcpy(buf+sizeof(Header), &gps, sizeof(GPS_pkt));
			buflen = sizeof(Header) + sizeof(GPS_pkt);
		}
		else if (cnt%3)
		{
			hdr.type = DATA_ROUTING;
			memcpy(buf, &hdr, sizeof(Header));
			memcpy(buf+sizeof(Header), &route, sizeof(Routing_pkt));
			buflen = sizeof(Header) + sizeof(Routing_pkt);
		}
		else{
			hdr.type = DATA_PLAN;
			memcpy(buf, &hdr, sizeof(Header));
			memcpy(buf+sizeof(Header), &plan, sizeof(Plan_pkt));
			buflen = sizeof(Header) + sizeof(Plan_pkt);

		}
		cnt++;
		
		printf("SENDING %lu bytes\n", buflen);
		if ((ret = xbee_connTx(con, NULL, buf, buflen )) != XBEE_ENONE) {
			xbee_log(xbee, -1, "xbee_conTx() returned: %d", ret);
			usleep(2000000);
			continue;
		}

		usleep(1000000);
	}

	// End connection
	if ((ret = xbee_conEnd(con)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conEnd() returned: %d", ret);
		return ret;
	}

	// Shutdown Xbee
	xbee_shutdown(xbee);

	return 0;
}
