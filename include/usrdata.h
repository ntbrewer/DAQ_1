/*
 Include file for usrdata programs
*/
struct usrdat {
  int com0;               // command to usr2xia
  int com1;               // extra command
  bool onoff;             // usrdata on/off status
  bool kelvin;            // kelvin on/off status
  bool geLN;              // geLN on/off status
  bool caenHV;            // mtasHV on/off status
  bool mpodHV;            // mtasHV on/off status
  time_t time;            // usrData time up
  time_t kelvinTime;      // kelvin time up
  time_t geLNTime;        // geLN time up
  time_t caenHVTime;      // caenHV time up
  time_t mpodHVTime;      // mpodHV time up
  int data[1024];         // user data buffer to contain data for thermometers, LN, HV, etc.
};

struct usrdat *usrptr;              // mapped array pointing to usrData

#define USRDATAPATH "../data/usrdata.bin"             // com data for control of daq programs
#define USRDATASIZE sizeof(struct usrdat)


/*
usrData [0] = commands to usr2xia program
usrData [1] = time of last updated data     is this necessary?
usrData [2-1023] = user data
 
int openUserData();
void closeUserData(int mapUser);
int mapUser=-1;
*/
