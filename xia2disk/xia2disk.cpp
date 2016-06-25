// Rev. D crate 
// Feb 2015

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include <cstring>

#include <errno.h>
#include <getopt.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>

// Needed for LIST_MODE_RUNx defs. Incorporate into PixieInterface
#include "pixie16app_defs.h"
#include "../include/StatsHandler.h"         // was .hpp   ???
//#include "Buffer_Structure.h"

// Interface for the PIXIE-16
#include "Display.h"
#include "PixieInterface.h"
#include "Utility.h"

// mapped file user data definitions
#include <fcntl.h>
#include <sys/mman.h>
//#include "../include/xia2disk.h"
#include "../include/usrdata.h"  // user data 
#include "../include/xia2disk.h"  // daq command data and ldf file name
#include "../include/online.h"  // on-line data 

// only one kind of word we're interested in
typedef PixieInterface::word_t word_t;
const size_t maxEventSize = (0x3FFE0000 >> 17);
typedef word_t eventdata_t[maxEventSize];

using namespace std;
using namespace Display;
static volatile bool isInterrupted = false;

// function prototypes
static void interrupt(int sig);
bool SynchMods(PixieInterface &pif);
int SendData(word_t *data, size_t nWords);

// stuff for mapped LDF output
int mmapListData();                                  // sets up the mapped memory

#define LISTDATA_INTS (268435456)                             // expect unsigned int to be 32-bit = to word_t = uint32_t from PixieInterface.h
#define LISTDATA_SIZE (LISTDATA_INTS * sizeof(unsigned int))  // 1 GB of 4 byte ints; this is 262144 pages of 1024 * 4 bytes
unsigned int *xxxptr;            //int xxx[268435456];        // mapped array of ints (32 bit = 4 bytes)...268,435,456
int openListFile();                                  // opens list mode file and sets up the mapped memory
void closeListFile(int mapList);                     // unmaps and closes list mode file
int mapList =0;              // integer file descriptor
//why commented out??
#define ONLINEDATA_FILE "../data/online.bin"
#define ONLINEDATA_INTS (EXTERNAL_FIFO_LENGTH + 1032)           // EXTERNAL_FIFO_LENGTH = 131072 from pixie16app_defs.h + 1024 from users data + 8 for header
#define ONLINEDATA_SIZE (ONLINEDATA_INTS * sizeof(unsigned int))  // 1 GB of 4 byte ints; this is 262144 pages of 1024 * 4 bytes
unsigned int *onlptr;            //int xxx[268435456];        // mapped array of ints (32 bit = 4 bytes)...268,435,456
int openOnlineFile();                                  // opens list mode file and sets up the mapped memory
void closeOnlineFile(int mapOnline);                    // unmaps and closes list mode file
int mapOnline =0;              // integer file descriptor

int makeOnLine(word_t *data, size_t nWords);
void SendListData(word_t *data, size_t nWords);      // records list mode data to a memory mapped file
void SendStatData(word_t *data, size_t nWords);      // records statistical data to the listmode memory mapped file
int onum=0;                // number appended to file for auto close and open at 1 GB
long time_x=0;
off_t pos=0;
unsigned int onlSpillNum=0;
unsigned int spillNum=0;
unsigned int lastSpill=0;                 // record last spill position to get off nn=0 on first/second write to file
int xsize=0;

//#include "../include/xia2disk.h"
//#include "../include/usrdata.h"  // user data 
//#include "../include/daqcom.h"   // daq command data and ldf file name

int openUserData();
void  closeUserData(int mapUser);
int mapUser=0;                // integer file descriptor
//int openDaqCom();
//void  closeDaqCom(int mapCom);
//int mapCom=0;                 // integer file descriptor

//const int sizeData = 1024;    // user data parameters 
//int usrData[1024];            // user data buffer to contain data for thermometers, LN, HV, etc.
//int *usrptr;                  // mapped array pointing to usrData


int main(int argc, char **argv)
{
  // values associated with the minimum timing between pixie calls (in us)
  const unsigned int endRunPause = 100;
  const unsigned int pollPause   = 1;
  const unsigned int readPause   = 10;
  const unsigned int waitPause   = 10;
  const unsigned int pollTries   = 100;
  const unsigned int waitTries   = 100;

  const int listMode = LIST_MODE_RUN0; // full header w/ traces
  time_t pollClock;                    // not sure this is needed with time inclued in new format


/************************ OPEN MAPPED FILES *****************************************************/
  mapCom  = openDaqCom();
  mapUser = openUserData();
  mapOnline = openOnlineFile();
  mapList = openListFile();

/*****************************************************************************/


/* 
   set-up parameters needed at start of run  - should also be adjustable by daq.c
*/
  daqptr->fastBoot = false;
  daqptr->zeroClocks = false;
  daqptr->quiet = true;
  daqptr->showModuleRates = false;
  daqptr->histoYES = false;             // set histograms off
  daqptr->statsYES = false;             // set statistics off
  daqptr->histoInterval = 10;           // -1 off,  in seconds
  daqptr->statsInterval =  1;           // -1 off, in seconds
  daqptr->threshPercent = 50;           // read the FIFO when it is this full
  daqptr->daq =0;                       // set daq off
  daqptr->com0 =0;                      // set com0=0
  daqptr->response = false;             // nothing to resond to
  daqptr->file = false;                 // set write-to-file off
  daqptr->usr2daq = false;              // set record user data off
  strcpy(daqptr->daqfile,"runTest");    // load dummy file


 BOOT: ;                                // goto label when booting

  //  const word_t threshWords = EXTERNAL_FIFO_LENGTH * threshPercent / 100;
  word_t threshWords = EXTERNAL_FIFO_LENGTH * daqptr->threshPercent / 100;

  // initialize the pixie interface and boot
  PixieInterface pif("pixie.cfg");
  pif.GetSlots();
  pif.Init();
  if (daqptr->fastBoot) {
    pif.Boot(PixieInterface::DownloadParameters |
	     PixieInterface::SetDAC |
	     PixieInterface::ProgramFPGA);
    daqptr->fastBoot = false;
  }
  else
    pif.Boot(PixieInterface::BootAll);
  daqptr->response = true;
 
  //  signal(SIGINT, interrupt);        // do we need this now or not?

  // check the scheduler
  LeaderPrint("Checking scheduler");
  int startScheduler = sched_getscheduler(0);
  if (startScheduler == SCHED_BATCH)
      cout << InfoStr("BATCH") << endl;
  else if (startScheduler == SCHED_OTHER)
      cout << InfoStr("STANDARD") << endl;
  else
      cout << WarningStr("UNEXPECTED") << endl;

  if (!SynchMods(pif))
    return EXIT_FAILURE;

  cout << "modules should be synched" << endl;

  // allocate memory buffers for FIFO
  size_t nCards = pif.GetNumberCards();
  
  // two extra words to store size of data block and module number
  cout << "Allocating memory to store FIFO data ("
       << sizeof(word_t) * (EXTERNAL_FIFO_LENGTH + 2) * nCards / 1024 
       << " KiB)" << endl;
  word_t *fifoData = new word_t[(EXTERNAL_FIFO_LENGTH + 2) * nCards];

  // allocate data for partial events
  cout << "Allocating memory for partial events ("
       << sizeof(eventdata_t) * nCards / 1024
       << " KiB)" << endl;
  eventdata_t partialEventData[nCards];
  vector<word_t> partialEventWords(nCards);
  vector<word_t> waitWords(nCards);
  StatsHandler statsHandler(nCards);
/*
  UDP_Packet command;                    // no longer needed since since we won't send across networks
  command.DataSize = 100;                // but keep around in case we want to implement later
*/ 
  bool isStopped = true;
  bool justEnded = false;
  // Enter data acquisition loop
  cout.setf(ios_base::showbase);

  size_t dataWords = 0;

  //  int commandCount = 0;
  double startTime;
  double spillTime, lastSpillTime, durSpill;
  double parseTime, waitTime, readTime, sendTime, pollTime;
  double lastStatsTime, statsTime = 0;
  double lastHistoTime, histoTime = 0;

  cout << "histogram setup started" << endl;
/////////////////////////////////////////////////////////// histogram stuff below
  typedef pair<unsigned int, unsigned int> chanid_t;
  map<chanid_t, PixieInterface::Histogram> histoMap;

  if (daqptr->histoInterval != -1.) {
    cout << "Allocating memory to store HISTOGRAM data ("
	 << sizeof(PixieInterface::Histogram) * nCards * pif.GetNumberChannels() / 1024
	 << " KiB)" << endl;
      
    for (unsigned int mod=0; mod < nCards; mod++) {
      for (unsigned int ch=0; ch < pif.GetNumberChannels(); ch++) {
	chanid_t id(mod, ch);
	
	histoMap[id] = PixieInterface::Histogram();
      }
    }
  }
    
  PixieInterface::Histogram deltaHisto;
/////////////////////////////////////////////////////////// histogram stuff above
  cout << "histogram setup ended" << endl;

  bool runDone[nCards];
  bool isExiting = false;

  int waitCounter = 0, nonWaitCounter = 0;
  int partialBufferCounter = 0;

  // enter the data acquisition loop
  //  socket_poll(0); // clear the file descriptor set we poll            //suspect this can go away 

  while (daqptr->daq != 3) {                                      //~!@#$ could make my own bool here from daqcom
    // see if we have any commands from pacman
    //    cout <<" in the loop" << endl;
    //    sleep(1);

    time_t curTime;
    time(&curTime);

    // only update the display if the time has changed
    if ( isStopped && daqptr->daqRead != curTime) {          //~!@#$ isStopped -> own bool
      string timeString(ctime(&curTime));
      // strip the trailing newline
      timeString.erase(timeString.length()-1, 1);

      LeaderPrint("Waiting for START command");
      cout << InfoStr(timeString.c_str()) << '\r' << flush;
    }
    daqptr->daqRead = curTime;

    //    printf("time = %li\n",daqptr->daqRead);

    if (isInterrupted) {                                     //~!@#$ own interrupt signal
      if (isStopped) {
	cout << endl << CriticalStr("INTERRUPTED") << endl;
	isExiting = true;	
      } else {
	cout << CriticalStr("INTERRUPTED: Ending run") << endl;
	pif.EndRun();
	justEnded = true;
	isStopped = true;
	usleep(endRunPause);
      }
    }
/*
    if (socket_poll(1)) {                                    //~!@#$ command options here, own bool or command
      spkt_recv(&command);
	
      cout << endl << "receive command[" << commandCount++ 
	   << "] = " << hex << (int)command.Data[0] << dec << endl;
*/
    if (!daqptr->response) {       // got a command request   
    switch (daqptr->daq) {                                  // daqcom->com0 = 
      //    switch (daqptr->daq) {                                  // daqcom->com0 = 
    case 0:                        // received a stop command
      pif.EndRun();
      justEnded = true;            // to take one more run through to read remainder of FIFO
      isStopped = true;
      usleep(endRunPause);
      daqptr->response = true;     // send response that we've stopped
      switch (daqptr->com0){
      case 0:                      // stopped with nothing to do
	break;
      case 1:                      // stopped to change the threshWords - threshPercent values
	threshWords = EXTERNAL_FIFO_LENGTH * daqptr->threshPercent / 100;    // recalculate threshwords in case it changed
	daqptr->response = true;   // send response that changes were checked
	daqptr->com0 = 0;          // set com0 back to 0
	break;
      case 2:                      // stop to change the name of the output file
	closeListFile(mapList);
	onum=0;                    // set file counter to 0 for new files
	mapList = openListFile();
	spillNum=0;
	daqptr->com0 = 0;
	daqptr->file = 0;
	daqptr->response = true;   // send response that we've stopped
	break;
      case 3:                      // stopped to boot pixie...either fastboot or whole boot
	goto BOOT;
      default:
	break;  
      }
      break;
      ///////////////////////////////////////////// end com0 switch
    case 1:                        // received a run command
      if (isStopped) {
	// reset variables
	for (size_t mod=0; mod < nCards; mod++) {
	  runDone[mod] = false;
	}
	if (daqptr->zeroClocks) {                                     // zeroClock option
	  SynchMods(pif);
	  daqptr->zeroClocks = false;
	}
	startTime = usGetTime(0);
	lastHistoTime = lastStatsTime = lastSpillTime = usGetTime(startTime);
	if ( pif.StartListModeRun(listMode, NEW_RUN) ) {
	  isStopped = false;
	  waitCounter = 0;
	  nonWaitCounter = 0;
	}
      }
      daqptr->response = true;   // send response that we've stopped
      break;
    case 2:              // received an exit command
      daqptr->daq = 3;   // set the ending for the while statement but we'll go through one last time
      isExiting = true;  // set isExiting true
      printf("Ending DAQ - daqptr->daq = %i\n",daqptr->daq);   // roof the program got the command to end
      daqptr->response = true;   // send response that we've stopped
      break;
    default:
      break;
    }
    }
    ///////////////////////// end daq switch
/*
	// artificial, but i work with what pacman gives me
	command.Data[0] = ACQ_OK;
	if (isStopped)
	  isExiting = true;
	spkt_send(&command);
      case STOP_ACQ:
	if (isStopped) {
	  command.Data[0] = ACQ_STP_HALT;
	} else {
	  command.Data[0] = ACQ_OK;
	  pif.EndRun();
	  justEnded = true; // to take one more run through to read remainder of FIFO
	  isStopped = true;
	  usleep(endRunPause);
	}
	spkt_send(&command);
	break;
      case STATUS_ACQ:
	command.Data[0] = ACQ_OK; 
	command.Data[1] = isStopped ? ACQ_STOP : ACQ_RUN;
	spkt_send(&command);
	break;
      case START_ACQ:
	if (isStopped) {
	  command.Data[0] = ACQ_OK;
	  // reset variables
	  for (size_t mod=0; mod < nCards; mod++) {
	    runDone[mod] = false;
	  }
	  if (zeroClocks)                                                       //zeroclocks
	    SynchMods(pif);
	  startTime = usGetTime(0);
	  lastHistoTime = lastStatsTime = lastSpillTime = usGetTime(startTime);
	  if ( pif.StartListModeRun(listMode, NEW_RUN) ) {
	    isStopped = false;
	    waitCounter = 0;
	    nonWaitCounter = 0;
	  }
	} else {
	  command.Data[0] = ACQ_STR_RUN;
	}
	spkt_send(&command);
	break;
      case ZERO_CLK:
	command.Data[0] = ACQ_OK;
	spkt_send(&command);
	spkt_close();
	
	delete[] fifoData;
	return EXIT_SUCCESS;
      default:
	// unrecognized command
	break;
      } // end switch
    } // if socket polled
    //                                     ~!@#$ end of commands from pacman I think 
    // wait for more commands
*/
  if (isStopped && !justEnded)           // does this do anything?
      continue;


    vector<word_t> nWords(nCards);
    vector<word_t>::iterator maxWords;

    parseTime = waitTime = readTime = 0.;

    // check if it's time to dump statistics
    if (daqptr->statsYES) {    /////////////////////////////////////////////////////////// statistics stuff below 
      if ( daqptr->statsInterval != -1 && 
	   usGetTime(startTime) > lastStatsTime + daqptr->statsInterval * 1e6 ) {
	usGetDTime(); // start timer
	for (size_t mod = 0; mod < nCards; mod++) {
	  pif.GetStatistics(mod);
	  PixieInterface::stats_t &stats = pif.GetStatisticsData();
	  fifoData[dataWords++] = PixieInterface::STAT_SIZE + 3;
	  fifoData[dataWords++] = mod;
	  fifoData[dataWords++] = 
	    ( (PixieInterface::STAT_SIZE + 1) << 17 ) & // event size
	    ( 1 << 12 ) & // header length
	    ( pif.GetSlotNumber(mod) << 4); // slot number
	  memcpy(&fifoData[dataWords], &stats, sizeof(stats));
	  dataWords += PixieInterface::STAT_SIZE;

	  if (daqptr->quiet)
	    cout << endl;
	  
	  cout << "STATS " << mod << " : ICR ";
	  for (size_t ch = 0; ch < pif.GetNumberChannels(); ch++) {
	    cout.precision(2);
	    cout << " " << pif.GetInputCountRate(mod, ch);
	  }
	  cout << endl << flush;
	  
	}
	//      SendData(fifoData, dataWords);
	if (daqptr->file) SendStatData(fifoData, dataWords);                  // stat data only send if file is open
	dataWords = 0;
	statsTime += usGetDTime();
	lastStatsTime = usGetTime(startTime);
      }
    }  /////////////////////////////////////////////////////////// statistics stuff above

    // check whether it is time to dump histograms
    if (daqptr->histoYES){     /////////////////////////////////////////////////////////// histogram stuff below
      if ( daqptr->histoInterval != -1 &&
	   usGetTime(startTime) > lastHistoTime + daqptr->histoInterval * 1e6 ) {
	usGetDTime(); // start timer
	ofstream out("histo.dat", ios::trunc);
	ofstream deltaout("deltahisto.dat", ios::trunc);
	
	for (size_t mod=0; mod < nCards; mod++) {
	  for (size_t ch=0; ch < pif.GetNumberChannels(); ch++) {
	    chanid_t id(mod, ch);
	    PixieInterface::Histogram &histo = histoMap[id];
	    
	    // copy the old histogram data to the delta histogram temporarily
	    deltaHisto = histo;
	    // performance improvement possible using Pixie16EMbufferIO directly to fetch all channels
	    histo.Read(pif, mod, ch);
	    histo.Write(out);
	    // calculate the change using the temporarily stored previous histogram
	    deltaHisto = histo - deltaHisto;
	    deltaHisto.Write(deltaout);
	  }
	}
	out.close();
	deltaout.close();
	
	histoTime += usGetDTime();
	lastHistoTime = usGetTime(startTime);
      }
      
    }  /////////////////////////////////////////////////////////// histogram stuff above

    // check whether we have any data    /////////////////////////////////////////////////////////// start poll pixie modules
    usGetDTime(); // start timer
    for (unsigned int timeout = 0; timeout < (justEnded ? 1 : pollTries);
	 timeout++) {
      // see if the modules have any data for us

      for (size_t mod = 0; mod < nCards; mod++) {
	if (!runDone[mod]) {
	  nWords[mod] = pif.CheckFIFOWords(mod);
	} else {
	  nWords[mod] = 0;
	}
      }
      maxWords = max_element(nWords.begin(), nWords.end());
      if (*maxWords > threshWords)
	break;
      usleep(pollPause);
    }
    time(&pollClock);
    pollTime = usGetDTime();
                      /////////////////////////////////////////////////////////// end poll pixie modules
    int maxmod = maxWords - nWords.begin();
    bool readData = ( *maxWords > threshWords || justEnded );
                   
    if (readData) {     /////////////////////////////////////////////////////////// read pixie modules
      // if not timed out, we have data to read	
      // read the data, starting with the module with the most words      
      int mod = maxmod;      
      mod = maxmod = 0; //! tmp, read out in a fixed order

      usleep(readPause);
      do {
	bool fullFIFO = (nWords[mod] == EXTERNAL_FIFO_LENGTH);

	if (nWords[mod] > 0) {
	  usGetDTime(); // start read timer
	  
	  word_t &bufferLength = fifoData[dataWords];

	  // fifoData[dataWords++] = nWords[mod] + 2
	  fifoData[dataWords++] = nWords[mod] + partialEventWords[mod] + 2;
	  fifoData[dataWords++] = mod;
	  word_t beginData = dataWords;

	  //? only add to fifo stream if we have enough words to complete event?
	  if (partialEventWords[mod] > 0) {
	      memcpy(&fifoData[dataWords], partialEventData[mod],
		     sizeof(word_t) * partialEventWords[mod]);
	      dataWords += partialEventWords[mod];
	      partialEventWords[mod] = 0;
	  }

	  if (!pif.ReadFIFOWords(&fifoData[dataWords], nWords[mod], mod)) {
	    cout << "Error reading FIFO, bailing out!" << endl;

	    //	    spkt_close();
	    delete[] fifoData;

	    return EXIT_FAILURE;
	    // something is wrong
	  } else {
	      word_t parseWords = beginData;
	      word_t eventSize;

	      waitWords[mod] = 0; // no longer waiting (hopefully)

	    readTime += usGetDTime(); // and starts parse timer
	    // unfortuantely, we have to parse the data to make sure 
	    //   we grabbed complete events
	    do {
	      word_t slotRead = ((fifoData[parseWords] & 0xF0) >> 4);
	      word_t chanRead = (fifoData[parseWords] & 0xF);
	      word_t slotExpected = pif.GetSlotNumber(mod);

	      eventSize = ((fifoData[parseWords] & 0x1FFE0000) >> 17);
	      statsHandler.AddEvent(mod, chanRead, sizeof(word_t) * eventSize);

	      if (eventSize == 0 || slotRead != slotExpected ) {
		if ( slotRead != slotExpected )
		  cout << "Slot read (" << slotRead << ") not the same as"
		       << " module expected (" << slotExpected << ")" << endl;
		if (eventSize == 0)
		  cout << "ZERO EVENT SIZE" << endl;
		cout << "First header words: " << hex << fifoData[parseWords] 
		     << " " << fifoData[parseWords + 1] 
		     << " " << fifoData[parseWords + 2]
		     << " at position " << dec << parseWords 
		     << "\n  parse started at position " << beginData
		     << " reading " << nWords[mod] << " words." << endl;
		//! how to proceed from here
		return EXIT_FAILURE;
	      }
	      parseWords += eventSize;	      
	    } while ( parseWords < dataWords + nWords[mod]);	   
	    parseTime += usGetDTime();

	    if (parseWords > dataWords + nWords[mod]) {
	      waitCounter++;
	      // if we have ended the run, we should have all the words
	      if (justEnded) {
		cout << ErrorStr("Words missing at end of run.") << endl;
		return EXIT_FAILURE; // for now
	      } else {
		// we have a deficit of words, now we must wait for the remainder
		if ( fullFIFO ) {
		  // the FIFO was full so the rest of the partial event is likely lost
		  parseWords -= eventSize;
		  // update the buffer length
		  nWords[mod]  = parseWords;
		  bufferLength = nWords[mod] + 2;
		} else {
		  waitWords[mod] = parseWords - (dataWords + nWords[mod]);
		  unsigned int timeout = 0;
		
		  usGetDTime(); // start wait timer
		  
		  if (!daqptr->quiet) 
		    cout << "Waiting for " << waitWords[mod] << " words in module " << mod << flush;
		  
		  usleep(waitPause);
		  word_t testWords = 0;

		  while (timeout++ < waitTries) {
		      testWords = pif.CheckFIFOWords(mod);
		      if ( testWords >= max(waitWords[mod], 9U) )  // zero event length fix from VANDLE (9 as unsigned int)
			  break;
		      usleep(pollPause);
		  } 
		  waitTime += usGetDTime();
		  
		  if (timeout >= waitTries) {
		    if (!daqptr->quiet)
		      cout << " --- TIMED OUT," << endl
			   << InfoStr("    moving partial event to next buffer") << endl;

		    partialBufferCounter++;
		    partialEventWords[mod] = eventSize - waitWords[mod];
		    memcpy(partialEventData[mod], 
			   &fifoData[dataWords + nWords[mod] - partialEventWords[mod]],
			   sizeof(word_t) * partialEventWords[mod]);
		    nWords[mod] = parseWords - beginData - eventSize;

		    // update the size of the buffer;
		    bufferLength = nWords[mod] + 2;
		  } else {
		    if (!daqptr->quiet)
		      cout << endl;
		    usleep(readPause);
		    int testWords = pif.CheckFIFOWords(mod);
		    if ( !pif.ReadFIFOWords(&fifoData[dataWords + nWords[mod]],
					    waitWords[mod], mod) ) {
		      cout << "Error reading FIFO, bailing out! =>testWords = " << testWords << endl;
		      //		      spkt_close();

		      delete[] fifoData;

		      return EXIT_FAILURE;
		      // something is wrong 
		    } else {
		      nWords[mod] += waitWords[mod];
		      // no longer waiting for words
		      waitWords[mod] = 0;
		      // and update the length of the buffer
		      bufferLength = nWords[mod] + 2;
		    } // check success of read
		  } // if we DID NOT time out waiting for words
		} // if we DID NOT have a full FIFO
	      } // if we ARE NOT on the final read at the end of a run
	    } // if there are words remaining  
	    else {
	      nonWaitCounter++;
	    }
	  } // check success of read
	} else { // if we had any words
	  // write an empty buffer if there is no data
	  fifoData[dataWords++] = 4;      //was 2
	  fifoData[dataWords++] = mod;	    
	  fifoData[dataWords++] = 0;     //added to have a 128-bit record...may have to change this back
	  fifoData[dataWords++] = 0;     // may ned to delete these 2 lines and put 2 back
	}
	if (nWords[mod] > EXTERNAL_FIFO_LENGTH * 9 / 10) {
	  cout << "Abnormally full FIFO with " << nWords[mod] 
	       << " words in module " << mod << endl;

	  if (fullFIFO) {
	      pif.EndRun();
	      justEnded = true;
	      isStopped = true;
	  }
	}
	if (!daqptr->quiet) {
	  if (fullFIFO)
	    cout << "Read " << WarningStr("full FIFO") << " in";
	  else 
	    cout << "Read " << nWords[mod] << " words from";
	  cout << " module " << mod 
	       << " to buffer position " << dataWords;
	  if (partialEventWords[mod] > 0) 
	      cout << ", " << partialEventWords[mod] << " words reserved.";
	  cout << endl;
	}
	dataWords += nWords[mod];
	
	// read the remainder of the modules in a modulo ring
	mod = (mod + 1) % nCards;
      } while ( mod != maxmod );            // end of do statement
    } // if we have data to read             /////////////////////////////////////////////////////////// end reading pixie modules
    
    // update whether the run has ended with the data read out
    for (size_t mod = 0; mod < nCards; mod++) {
      if (!justEnded && !pif.CheckRunStatus(mod)) {
	runDone[mod] = true;
	cout << "Run ended in module " << mod << endl;
      }
      if (justEnded && pif.CheckRunStatus(mod)) {
	cout << "Run not properly finished in module " << mod << endl;
      }
    }

    // if we don't have enough words, poll socket and modules once more
    if (!readData)
	continue;

    spillTime = usGetTime(startTime);
    durSpill = spillTime - lastSpillTime;
    lastSpillTime = spillTime;

    usGetDTime(); // start send timer

    /*************************************************************************************************/
    //MEMORY MAP LIST DATA 
   //    int nBufs = SendData(fifoData, dataWords);
    int nBufs = 1;
    if (daqptr->daq == 1 && daqptr->file) SendListData(fifoData, dataWords);    // MEMORY MAP LIST DATA

    sendTime = usGetDTime();

    statsHandler.AddTime(durSpill * 1e-6);
    /*    if (daqptr->daq == 1){
      if (!daqptr->quiet) {
	cout << nBufs << " BUFFERS with " << dataWords << " WORDS, " << endl;
	cout.setf(ios::scientific, ios::floatfield);
	cout.precision(1);
	cout << "    SPILL " << durSpill << " us "
	     << " POLL  " << pollTime << " us "
	     << " PARSE " << parseTime << " us" << endl
	     << "    WAIT  " << waitTime << " us "
	     << " READ  " << readTime << " us "
	     << " SEND  " << sendTime << " us" << endl;	 
	// add some blank spaces so STATS or HISTO line up
	cout << "   ";
	if (daqptr->statsInterval != -1) {
	  cout << " STATS " << statsTime << " us ";
	}
	if (daqptr->histoInterval != -1) {
	  cout << " HISTO " << histoTime << " us ";
	}
	if (daqptr->statsInterval != -1 || daqptr->histoInterval != -1) {
	  cout << endl;
	}

	cout << endl;
      } else {
	cout.setf(ios::scientific, ios::floatfield);
	cout.precision(1);

	if (!daqptr->showModuleRates) {
	  cout << nBufs << " bufs : " 
	       << "SEND " << sendTime << " / SPILL " << durSpill << "     \r";
	} else {      
	  for (size_t i=0; i < nCards; i++) {
	    cout << "M" << i << ", "
		 << statsHandler.GetEventRate(i) / 1000. << " kHz";
	    cout << " (" << statsHandler.GetDataRate(i) / 1000000. << " MB/s)";
	  }  
	  cout << "    \r";
	}
      }
    }*/
    // reset the number of words of fifo data
    dataWords = 0;
    histoTime = statsTime = 0;
    justEnded = false;
  }

  //  spkt_close();
  // deallocate memory
  delete[] fifoData;

  if (waitCounter + nonWaitCounter != 0) {
    cout << "Waiting for " << waitCounter * 100 / (waitCounter + nonWaitCounter)
	 << "% of the spills." << endl;
    cout << "  " << partialBufferCounter << " partial buffers" << endl;
  }

  daqptr->response = true;   // send response that we are ending

/**********************  CLOSE MAPPED FILES  *******************************************************/
  closeListFile(mapList);
  closeOnlineFile(mapOnline);
  closeUserData(mapUser);
  closeDaqCom(mapCom);

  return EXIT_SUCCESS;
}

/**************************************************************/
static void interrupt(int sig)
{
  isInterrupted = true;
}

/**************************************************************/
bool SynchMods(PixieInterface &pif)
{
  static bool firstTime = true;
  static char synchString[] = "IN_SYNCH";
  static char waitString[] = "SYNCH_WAIT";

  bool hadError = false;
   
  LeaderPrint("Synchronizing");

  if (firstTime) {
    // only need to set this in the first module once
    if (!pif.WriteSglModPar(waitString, 1, 0))
      hadError = true;
    firstTime = false;
  }
  
  for (unsigned int mod = 0; mod < pif.GetNumberCards(); mod++)
    if (!pif.WriteSglModPar(synchString, 0, mod))
      hadError = true;

  if (!hadError)
    cout << OkayStr() << endl;
  else
    cout << ErrorStr() << endl;

  return !hadError;
}

/**************************************************************/
int openListFile() {
  char infile[200] = "\0";   // file to open
  char oname[190]="\0";      // name of file to open
  int fd=0;                // mapped file descriptor
//    int zero=0;
  off_t offset=0;
  ssize_t result=0;
    /*
     Open a file for writing.
     - Creating the file if it doesn't exist.
     - Truncating it to 0 size if it already exists. (not really needed)
     Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
  strcpy(oname,daqptr->daqfile);
  if (strlen(daqptr->daqfile) == 0) strcpy(oname,"runTest");
  printf("Setting up memory mapped file: %s\n", oname);

/*
     Function to open and map list mode output file
*/
  sprintf(infile,"%s-%i",oname,++onum);  // add an integer for automatic file closing and opening at ~1 GB
  time_x = time(NULL);
  printf("Starting at time ...%li %lx \n",time_x, time_x);

  fd = open(infile, O_RDWR | O_CREAT | O_EXCL | O_TRUNC, (mode_t)0644);
  if (fd == -1) {
    perror("Error opening file path for writing");
    exit(EXIT_FAILURE);
  }
    /*
     Stretch the file size to the size of the (mmapped) array/structure/etc.
     We choose to write to the entire file rather than seeking the end and writing 1 word
     */
  if (lseek(fd,offset,SEEK_END) < 5 ){
    printf ("going to end of file\n");
    result = lseek(fd, LISTDATA_SIZE-1, SEEK_SET);  //268435455 - effectively zeros the array according to man lseek
    if (result == -1) {
      close(fd);
      perror("Error calling lseek() to 'stretch' the file");
      exit(EXIT_FAILURE);
    }
  }
//    for (int ii=LISTDATA_INTS; ii>0; ii--){
  result = write(fd, "\0", 1);  // just write anything
  if (result != 1) {
    close(fd);
    perror("Error writing last element to be \"0\" to the file");
    exit(EXIT_FAILURE);
  }
//    }
    /* Something needs to be written at the end of the file to
     * have the file actually have the new size.
     * Just writing an empty string at the current file position will do.
     *
     * Note:
     *  - The current position in the file is at the end of the stretched
     *    file due to the call to lseek().
     *  - An empty string is actually a single '\0' character, so a zero-byte
     *    will be written at the last byte of the file.
     */
    /*
     result = write(fd, "", 1);
     if (result != 1) {
     close(fd);
     perror("Error writing last byte of the file");
     exit(EXIT_FAILURE);
     }
     */
    /*
     Now the file is ready to be mmapped.
     */
  xxxptr = (unsigned int*) mmap(0, LISTDATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (xxxptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
  xxxptr[LISTDATA_INTS-1] = 0;

    /*
     Don't forget to free the mmapped memory usually at end of main
     */
    /*
    for (int ii=0; ii<LISTDATA_INTS; ii++){
        if (ii<1000000) *(xxxptr+ii)=ii;    // equivalent statement: xxxptr[ii] = ii; // just write anything
        else *(xxxptr+ii)=1;                // equivalent statement: xxxptr[ii] = 1;   // just write anything   *(xxxptr+ii)=1;
    }
    xxxptr[LISTDATA_INTS-1] = 0;
    pos = 1000000*sizeof(int);
*/
  return (fd);
}

/**************************************************************/
void closeListFile(int mapList) {
/*
     Function to unmap and close list mode output file
*/
//  xxxptr[LISTDATA_INTS-1] = LISTDATA_INTS-1;
    printf("xsize in close file %i\n",xxxptr[LISTDATA_INTS-1]);

    int nn = xxxptr[LISTDATA_INTS-1];
    printf("Unmapping... last value will be: %i\n",nn);
    munmap(xxxptr,sizeof (*xxxptr));
    int kk = ftruncate(mapList,(nn * sizeof(int)));  // truncate to end of last data
    if (kk == -1) perror("Error truncating list file at closing");

//printf ("kk = %i\n",kk);
    close(mapList);                                  // close file
    printf("File unmapped, truncated, and closed...dt = %li\n",(time(NULL)-time_x));
    
    return;
}
/**************************************************************/
int openOnlineFile() {
  //  char infile[200] = "\0";   // file to open
  //  char oname[190]="\0";      // name of file to open
  int fd=0;                // mapped file descriptor
//    int zero=0;
  off_t offset=0;
  ssize_t result=0;
    /*
     Open a file for writing.
     - Creating the file if it doesn't exist.
     - Truncating it to 0 size if it already exists. (not really needed)
     Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
  //  strcpy(oname,"../data/online.bin");
  //  printf("Setting up memory mapped file: %s\n", oname);
/*
     Function to open and map list mode output file
*/
//  sprintf(infile,"%s-%i",oname,++onum);  // add an integer for automatic file closing and opening at ~1 GB

  fd = open(ONLINEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);  // will recreate the file
  if (fd == -1) {
    perror("Error opening file path for writing");
    exit(EXIT_FAILURE);
  }
    /*
     Stretch the file size to the size of the (mmapped) array/structure/etc.
     We choose to write to the entire file rather than seeking the end and writing 1 word
     */
  if (lseek(fd,offset,SEEK_END) < 5 ){
    printf ("going to end of file\n");
    result = lseek(fd, ONLINESIZE-1, SEEK_SET);  // - effectively zeros the array according to man lseek
    if (result == -1) {
      close(fd);
      perror("Error calling lseek() to 'stretch' the file");
      exit(EXIT_FAILURE);
    }
  }
//    for (int ii=LISTDATA_INTS; ii>0; ii--){
  result = write(fd, "\0", 1);  // just write anything
  if (result != 1) {
    close(fd);
    perror("Error writing last element to be \"0\" to the file");
    exit(EXIT_FAILURE);
  }
//    }
    /* Something needs to be written at the end of the file to
     * have the file actually have the new size.
     * Just writing an empty string at the current file position will do.
     *
     * Note:
     *  - The current position in the file is at the end of the stretched
     *    file due to the call to lseek().
     *  - An empty string is actually a single '\0' character, so a zero-byte
     *    will be written at the last byte of the file.
     */
    /*
     result = write(fd, "", 1);
     if (result != 1) {
     close(fd);
     perror("Error writing last byte of the file");
     exit(EXIT_FAILURE);
     }
     */
    /*
     Now the file is ready to be mmapped.  (unsigned int*) 
     */
  onlptr = (struct comDataOnLine*) mmap(0, ONLINEDATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (onlptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
  onlptr[ONLINEDATA_INTS-1] = 0;

    /*
     Don't forget to free the mmapped memory usually at end of main
     */
    /*
    for (int ii=0; ii<LISTDATA_INTS; ii++){
        if (ii<1000000) *(xxxptr+ii)=ii;    // equivalent statement: xxxptr[ii] = ii; // just write anything
        else *(xxxptr+ii)=1;                // equivalent statement: xxxptr[ii] = 1;   // just write anything   *(xxxptr+ii)=1;
    }
    xxxptr[LISTDATA_INTS-1] = 0;
    pos = 1000000*sizeof(int);
*/
  return (fd);
}

/**************************************************************/
void closeOnlineFile(int mapList) {
/*
     Function to unmap and close list mode output file
*/
    printf("Unmapping online file ... \n");
    munmap(onlptr,sizeof (*onlptr));
    close(mapOnline);                                  // close file
    printf("On-line file unmapped and closed...dt = %li\n",(time(NULL)-time_x));
    
    return;
}

/**************************************************************/
int makeOnLine(word_t *data, size_t nWords) {
  word_t onlSpillNum = 0;
  word_t maxwordsize=0xFFFFFFFF;
  int nn = 0;
  int nn1 = 8 + (int) nWords;          // minimum number of ints (8 for header + pixie data)
  int nextSpill = 0;                   // initiall set next spill location to 0 => will set last
  int byte_size = sizeof (word_t);     // size of 1 datum of pixie data (32-bit)
  int byte_head = byte_size * 7;       // onlptr elements 0-6
  int byte_usr = byte_size;            // length in bytes of usr data if there is none (onlptr[7] = 0)
  if (usrptr->data[0] != 0) byte_usr = byte_size * usrptr->data[0]; // length in bytes of usr data (includes onlptr[7])
  int byte_pix = byte_size * nWords;   // length in bytes of pixie data
  //  int byte_pix = byte_size;                // length in bytes of pixie data  ???why was this commented in and above out????
  int byte_onl = byte_head + byte_usr + byte_pix;   // total length of current online buffer
  
  if (daqptr->usr2daq) nn1 += usrptr->data[0] - 1;    //user data length
  nextSpill = nn + nn1;
   //  printf("spill= %i next=%i  last= %i num of bytes=%i\n", spillNum, nextSpill, lastSpill,byte_num);
  if (onlSpillNum == maxwordsize) onlSpillNum = 0;   
  onlptr[0] = 'S';
  onlptr[1] = 'P';
  onlptr[2] = 'I';
  onlptr[3] = 'L';
  onlptr[4] = onlSpillNum++;
  onlptr[5] = 0;                   // now set the spill length pointer to the start of the next spill written in the file
  onlptr[6] = (int)time(NULL);
  onlptr[7] = 0;                   // this number (user data length) gets overwritten if there is user data to be written
  printf (" daqptr-usr2daq= %i byte-user =%i byte_pix=%i \n",daqptr->usr2daq, byte_usr,byte_pix);
  if (daqptr->usr2daq) {
    memcpy(&onlptr[7],usrptr->data,byte_usr);   // load usr data, first data is number of bytes to copy
    memcpy(&onlptr[7+usrptr->data[0]],data,byte_pix);      // load spill data from pixie
    //    memcpy(&onlptr[7],usrptr->data,(4*usrptr->data[0]));   // load usr data, first data is number of bytes to copy
    //    memcpy(&onlptr[7+usrptr->data[0]],data,byte_num);      // load spill data from pixie
  }
  else {
    memcpy(&onlptr[8],data,byte_pix);                      // load spill data from pixie
  }
    // For online data I do not need to increment onlptr[5] since the online scan knows its information
    // comes only from onlptr[7] for user data length and either orlptr[8] or onlptr[7 + onlptr[7]] 
    // if 7+onlptr[7] >= 8 pixie data starts at 7+onlptr[7] else onlptr[8]. 
  return (byte_onl);    // returns the size in bytes of the entire buffer to be written to disk
}

/**************************************************************/
void SendListData(word_t *data, size_t nWords) {
//int SendData2(word_t *data, size_t nWords) {
/*
     Function to form spill buffers and write to memory mapped array.
*/
  /*    
  int nextSpill=0;
  int nn = xxxptr[LISTDATA_INTS-1];   // initially 0 then set to lastSpill
  int nn1 = 8 + (int) nWords;

  if (daqptr->usr2daq) nn1 += usrptr->data[0] - 1;    //user data length
  nextSpill = nn + nn1;
  //    printf ("%s %i   %i %i  %i \n",spil.x, spillNum,nn,nextSpill,(int) nWords);
  if (nextSpill > (LISTDATA_INTS - 4)){   // 4 to keep things to 128-bit boundary, 2 = user changed file
    closeListFile(mapList);
    mapList = openListFile();
    spillNum=0;
    nn = 0;
    nextSpill = nn + nn1;
  }
  int byte_num = (sizeof (word_t)) * nWords;
  printf("spill= %i next=%i  last= %i num of bytes=%i\n", spillNum, nextSpill, lastSpill,byte_num);
  // replace below with makeOnLine

  xxxptr[nn+0] = 'S';
  xxxptr[nn+1] = 'P';
  xxxptr[nn+2] = 'I';
  xxxptr[nn+3] = 'L';
  xxxptr[nn+4] = spillNum++;
  //  xxxptr[nn+5] = nextSpill;        // Set at end of this section to indicate new data is loaded
  xxxptr[nn+6] = (int)time(NULL);
  xxxptr[nn+7] = 0;                   // this number gets overwritten if there is user data to be written
  if (daqptr->usr2daq) {
    memcpy(&xxxptr[nn+7],usrptr->data,(4*usrptr->data[0]));   // load usr data, first data is number of bytes to copy
    memcpy(&xxxptr[nn+7+usrptr->data[0]],data,byte_num);      // load spill data from pixie
  }
  else {
    memcpy(&xxxptr[nn+8],data,byte_num);                      // load spill data from pixie
  }
    // could check that nextSpill = 8 + xxxptr[nn+9] + xxxptr[nn + 9 + xxxptr[nn+9]] + .....
    */
  // replace above with makeOnLine
  // 
  int byte_onl = makeOnLine(data, nWords);   // create online buffer and buffer to write to disk
  int byte_size = sizeof(word_t);            // size of one element in the array (in bytes)
  int nn = xxxptr[LISTDATA_INTS-1];          // initially 0 then set to lastSpill
  int nextSpill = nn + byte_onl/byte_size;   // position of the next spill in the data file
  if (nextSpill > (LISTDATA_INTS - 4)){      // 4 to keep things to 128-bit boundary, 2 = user changed file
    closeListFile(mapList);
    mapList = openListFile();
    spillNum=0;
    nn = 0;
    nextSpill = byte_onl/byte_size;
  }
  memcpy(&xxxptr[nn],onlptr,byte_onl);   // copy the online buffer to disk
  xxxptr[nn+4] = spillNum++;             // increment spillNum and next Spill elements in header
  xxxptr[nn+5] = nextSpill;              // now set the spill length pointer to the start of the next spill written in the file
                                         // this indicates the data is finished and ready on disk
  lastSpill = nn;                        // lastSpill 
  if (spillNum > 1) {
    xxxptr[LISTDATA_INTS-1] = nextSpill;   // and set last value of array for scan to use latest on-line data
  }
  else {
    xxxptr[LISTDATA_INTS-1] = 0;     // and set last value of array for scan to use to 0
    lastSpill = nextSpill;           // save the 1st next spill position to use as the last spill position after the 1st pass
  }
  onlptr[0] = 'X';    
  printf("spill= %i next=%i  last= %i\n", spillNum, nextSpill, lastSpill);
  return;
}

/**************************************************************/
/**************************************************************/
void SendStatData(word_t *data, size_t nWords) {
//int SendData2(word_t *data, size_t nWords) {

    /*
     Function to form spill buffers and write to memory mapped array.
*/
    int nextSpill=0;
    int nn = xxxptr[LISTDATA_INTS-1];   // initially 0 then set to lastSpill

    nextSpill = nn + 8 + (int) nWords;

    if (nextSpill > (LISTDATA_INTS - 4)){   // 4 to keep things to 128-bit boundary
        closeListFile(mapList);
        mapList = openListFile();
        spillNum=0;
        nextSpill = 8 + (int) nWords;
        nn = 0;
    }
    
    xxxptr[nn+0] = 'S';
    xxxptr[nn+1] = 'T';
    xxxptr[nn+2] = 'A';
    xxxptr[nn+3] = 'T';
    xxxptr[nn+4] = spillNum;
    xxxptr[nn+5] = nextSpill;
    xxxptr[nn+6] = (int)time(NULL);
    xxxptr[nn+7] = 0;
    memcpy(&xxxptr[nn+8],data,nWords);

    // could check that nextSpill = 8 + xxxptr[nn+9] + xxxptr[nn + 9 + xxxptr[nn+9]] + .....

    xxxptr[nn+5]=nextSpill;          // now set the spill length pointer to the start of the next spill written in the file

    lastSpill = nn;
    if (spillNum != 1) {
        xxxptr[LISTDATA_INTS-1] = lastSpill;   // and set last value of array for scan to use latest on-line data
    }
    else {
        xxxptr[LISTDATA_INTS-1] = 0;     // and set last value of array for scan to use to 0
        lastSpill = nextSpill;     // save the first next spill position to use as the last spill position after the first pass
    }
    return;
}

/**************************************************************/

/***************************************************** User Data Section Below ***************************************************************************************/
int openUserData() {
    int fd=0;                // mapped file descriptor
    //    char fileData[200] = "data/usrdata.bin";
    //    int zero=0;
    //    ssize_t result=0;
    /*
     Open a file for writing.
     - Creating the file if it doesn't exist.
     - read/write/create/fail if exists/truncate to 0 size      u:rw, g:r o:r
     - Truncating it to 0 size if it already exists. (not really needed)
     - include O_RDWR | O_CREAT | O_EXCL | O_TRUNC if you don't want to overwrite existing data
     Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
    
    printf("Setting up memory mapped file: %s\n", USRDATAPATH);
    fd = open(USRDATAPATH, O_RDWR, (mode_t)0644);
    if (fd == -1) {
        perror("Error opening usrdata for writing ");
        printf (" %s \n",USRDATAPATH);
        exit(EXIT_FAILURE);
    }
    /*
     Now the file is ready to be mmapped.
     */
     //     usrptr = (int*) mmap(0, sizeData, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // might be uint32!
     usrptr = (struct usrdat*) mmap(0, USRDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // might be uint32!
    if (usrptr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    
    /*
     Don't forget to free the mmapped memory usually at end of main
     */
    return (fd);
}
/**************************************************************/
void closeUserData(int mapUser){
    
  if (mapUser != -1 ) {                                  // if file is active
    if (munmap(usrptr, sizeof (struct usrdat)) == -1) {      // unmap memory
      perror("Error un-mmapping the usrData file");
    }
    close(mapUser);                                    // close usrData file
  }
  
  return;
}

/**************************************************************/
/***************************************************** DAQ Communication Section Below ******************************************************************************/
/**************************************************************/
int openDaqCom() {
  int fd=0;     // mapped file descriptor

  /* Open a file for writing.
   *  - Creating the file if it doesn't exist.
   *  - Truncating it to 0 size if it already exists. (not really needed)
   *
   * Note: "O_WRONLY" mode is not sufficient when mmaping.
   */
  fd = open(COMDATAPATH, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening daq command file");
    exit(EXIT_FAILURE);
  }
        
  /* Now the file is ready to be mmapped.
   */
  daqptr = (struct comData*) mmap(0, COMDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (daqptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the daq command file");
    exit(EXIT_FAILURE);
  }
  /* Don't forget to free the mmapped memory ... usually at end of main
   */
   
  return (fd);
}

/**************************************************************/
void closeDaqCom(int mapCom){

  if (mapCom != -1 ) {            // if LN active
    if (munmap(daqptr, sizeof (struct comData*)) == -1) {   // unmap memory
      perror("Error un-mmapping the daq command file");
    }
    close(mapCom);                                    // close daqCom file
  }

  return;
}

/**************************************************************/

/********************************************************************************************************************************************************************************/















/*  
  
daqcom->
daqcom->
daqcom->

*/  
