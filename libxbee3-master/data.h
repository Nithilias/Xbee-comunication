typedef struct Packet Packet;


static const char DATA_GPS = 1;
static const char DATA_ROUTING = 2;
static const char DATA_PLAN = 3;

// Structure of the Data
struct __attribute__((__packed__)) GPS_pkt {
    	float latitude;
    	float longitude;
	float altitude;
	int dataRate;
};

struct __attribute__((__packed__)) Routing_pkt {
	int route;
};

struct __attribute__((__packed__)) Plan_pkt {
	int plan;
};

struct __attribute__((__packed__)) Header {
    	char type;
};


void print_GPS(GPS_pkt &gps)
{
  printf("lat: %f lon: %f alt: %f rate: %d\n",
	 gps.latitude, gps.longitude, gps.altitude, gps.dataRate);
};

void print_Routing(Routing_pkt &routing)
{
  printf("route: %d\n",
	 routing.route);
};

void print_Plan(Plan_pkt &plan)
{
  printf("plan: %d\n",
	 plan.plan);
};
