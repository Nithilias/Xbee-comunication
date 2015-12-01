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
#include <iostream>
#include <GetPot>
using namespace std;

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

void myCB(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	if ((*pkt)->dataLen > 0) {
		printf("rx: [%s]\n", (*pkt)->data);
	}
}

int main(int argc, char * argv[]) {
	
    	GetPot   cl(argc, argv);
    	if(cl.search(2, "--help", "-h") ) print_help(cl[0]);

    	// read arguments one by one on the command line
    	//  (lazy command line parsing)
    	//const double Alpha = cl.follow(0.,    "--alpha");   // [rad]
    	//const int    Beta  = cl.follow(256,   "--beta"); // [1/s]
    	cl.init_multiple_occurrence();
    	const string  xbeeDev  = cl.follow("/dev/ttyUSB0", "--dev");      
   	const int     baudrate    = cl.follow(57600, "--baud"); 
   	//const string  User2 = cl.follow("me too", "--user"); // second user specified ?
   	//const int     No2   = cl.next(0x43); 
    	cl.enable_loop();
    	//const double  XSz   = cl.follow(0., 3, "--size", "--sz", "-s"); // [cm]
    	//const double  YSz   = cl.next(0.);     

	void *d;
	struct xbee *xbee;
	struct xbee_con *con;
	struct xbee_conAddress address;
	xbee_err ret;

	if ((ret = xbee_setup(&xbee, "xbee1", xbeeDev.c_str(), baudrate)) != XBEE_ENONE) {
		printf("ret: %d (%s)\n", ret, xbee_errorToStr(ret));
		return ret;
	}

	memset(&address, 0, sizeof(address));
	address.addr64_enabled = 1;
	address.addr64[0] = 0x00;
	address.addr64[1] = 0xFF;

	if ((ret = xbee_conNew(xbee, &con, "16-bit Data", &address)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conNew() returned: %d (%s)", ret, xbee_errorToStr(ret));
		return ret;
	}

	if ((ret = xbee_conDataSet(con, xbee, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conDataSet() returned: %d", ret);
		return ret;
	}

	if ((ret = xbee_conCallbackSet(con, myCB, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conCallbackSet() returned: %d", ret);
		return ret;
	}

	for (;;) {
		if ((ret = xbee_conTx(con, NULL, "Hello?")) != XBEE_ENONE) {
			xbee_log(xbee, -1, "xbee_conTx() returned: %d", ret);
			usleep(2000000);
			continue;
		}

		usleep(1000000);
	}

	if ((ret = xbee_conEnd(con)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conEnd() returned: %d", ret);
		return ret;
	}

	xbee_shutdown(xbee);

	return 0;
}
