/*****************************************************************************/
/*                                                                           */
/*                  --- CAEN SpA - Computing Division ---                    */
/*                                                                           */
/*   CAEN HV Wrapper Library Installation and Use Instructions               */
/*		                                                             */
/*   April  2015		                                             */
/*                                                                           */
/*****************************************************************************/
  
 This archive contains the last release of the CAEN HV Wrapper Library and the
 corresponding Release Notes.

 The complete documentation can be found in the CAEN HV Wrapper Library Manual
 available once installed the Library or on CAEN's Web Site 
 at www.caen.it.


 Content of the archive
 ----------------------

 CAENUVWrapperReadme.txt        :  This file
 CAENHVWrapperReleaseNotes.txt  :  Release Notes of the last software release
 Makefile       	        :  makefile for library installation and demo 
                                   program
 
 Lib/
  libcaenhvwrapper.so.x.xx    	:  executable of the library (dynamic)
  
 Doc/
  CAENHVWrapper.pdf 	        :  user's manual of the library
 
 include/
  CAENHVWrapper.h               :  include file for use of the library
 
 HVWrapperDemo/                 :  directory with sources/executable of the demo 
                                 program 
 


 System Requirements
 -------------------
 
 - Network Interface Card + TCP/IP protocol (to control SY 1527/ SY 2527 / SY 4527 / SY 5527)
 - USB port (to control V65xx Boards, via V1718 VME Bridge)
 - Optical Link (to control V65xx Boards, via V2718 - VME-PCI Optical Link Bridge)
 - SY 1527/ SY 2527 firmware version 1.10.0 or later (recommended 1.14.03)
 - Linux gcc 2.9 or greater with gnu C/C++ compiler


 Installation notes
 ------------------

 1. It's necessary to login as root
 
 2. execute: make install
 
 The installation copies and installs the library in /usr/lib, compiles the demo 
 program and installs it in the work directory.
 To use the demo program, change to the demo program directory and launch the
 application typing ./HVWrappdemo.
   

 Note:
 -----
 Control of CAEN Power Supplies via USB/OpycalLink link requires the correct
 installation of the USB/A2818 device driver and CAENComm library. 
