/*
 Include file for usrdata programs
*/
#define COMDATAPATH "../data/xia2disk.bin"             // com data for control of daq programs
#define COMDATASIZE sizeof(struct comData)

struct comData {
  char daqfile[200];  // data acquisition file..ie ldf file
  bool file;          // commands data file (event-by-event)    // informs file that name has changed, close old, open new
  int daq;            // commands controlling daq status/conditions
  bool usr2daq;       // commands controlling user data
  int kelvin;         // commands concerning kelvin data
  int geln;           // commands concerning geln data
  int mtas_hv;        // commands concerning mtas_hv data
  
  int com0;           // command to provide instructions to xia2disk
  int com1;           // command to provide instructions to 
  int com2;           // command to provide instructions to 
  
  time_t daqRead;        // time of last read of data
  time_t kelvinRead;     // time of last read of data
  time_t gelnRead;       // time of last read of data
  time_t mtas_hvRead;    // time of last read of data

  bool fastBoot;         // options
  bool quiet;
  bool showModuleRates;
  bool zeroClocks;

  bool histoYES;        // turns on histograms
  bool statsYES;        // turns on statistcs
  int histoInterval;    // sets interval
  int statsInterval;    // sets interval
  unsigned int threshPercent;   // sets level on wen to read out FIFOdata buffer (% full)

  bool response;        // response from DAQ
};

struct comData *daqptr;

int openDaqCom();                // opens memory mapped file
void closeDaqCom(int mapCom);    // closes memory mapped file
int mapCom=-1;                   // file descriptor for mempry mapped file



