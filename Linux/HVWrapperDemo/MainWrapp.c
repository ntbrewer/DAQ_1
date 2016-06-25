/*****************************************************************************/
/*                                                                           */
/*        --- CAEN Engineering Srl - Computing Systems Division ---          */
/*                                                                           */
/*   MAINWRAPP.C                                                             */
/*                                                                           */
/*   June      2000:  Rel. 1.0                                               */
/*   Frebruary 2001:  Rel. 1.1                                               */
/*   September 2002:  Rel. 2.6                                               */
/*   November  2002:  Rel. 2.7                                               */
/*																	         */
/*****************************************************************************/
#include <signal.h>
#ifdef UNIX
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "MainWrapp.h"
#include "console.h"

#define MAX_CMD_LEN        (80)

typedef void (*P_FUN)(void);

typedef struct cmds
			{
				char  cmdName[MAX_CMD_LEN]; /* nome del comando */
				P_FUN pFun;
			} CMD;

static CMD function[] = {
		{ "LIBRARYRELEASE", HVLibSwRel },
        { "LOGIN", HVSystemLogin },
        { "LOGOUT", HVSystemLogout },
        { "GETCHNAME", HVGetChName },
        { "SETCHNAME", HVSetChName }, 
		{ "GETCHPARAMPROP", HVGetChParamProp },
        { "GETCHPARAM", HVGetChParam },
        { "SETCHPARAM", HVSetChParam },
       /* { "TSTBDPRES", HVTstBdPres },*/
		{ "GETBDPARAMPROP", HVGetBdParamProp },
		{ "GETBDPARAM", HVGetBdParam },
		{ "SETBDPARAM", HVSetBdParam },					// Rel. 2.7
     /*   { "GETGRPCOMP", HVGetGrpComp },
        { "ADDCHTOGRP", HVAddChToGrp },
        { "REMCHFROMGRP", HVRemChFromGrp },*/
        { "GETCRATEMAP", HVGetCrateMap },
	/*	{ "GETSYSCOMP", HVGetSysComp },*/
		{ "GETEXECLIST", HVGetExecList }, 
		{ "GETSYSPROP", HVGetSysProp },
		{ "SETSYSPROP", HVSetSysProp },
		{ "EXECOMMAND", HVExecComm },
        { "NOCOMMAND", HVnoFunction }
                 };

static char  alpha[] =
		{
			"abcdefghijklmnopqrstuv"
		};

HV System[MAX_HVPS];
int loop;

/*****************************************************************************/
/*                                                                           */
/*  COMMANDLIST                                                              */
/*                                                                           */
/*  Aut: A. Morachioli                                                       */
/*                                                                           */
/*****************************************************************************/
static void commandList(void)
{
	unsigned short  nOfSys = 0, nOfCmd = 0, pageSys = 0, 
					i, j, page = 0, row, column;
	int				cmd;

	while( strcmp(function[nOfCmd].cmdName, "NOCOMMAND") )
		nOfCmd++;

	while( 1 )
	{
		clrscr();
		con_puts("       --- Demonstration of use of CAEN HV Wrapper Library --- ");
		gotoxy(1, 3);
		for(i=page*20;(i<(page*20+20))&&(strcmp(function[i].cmdName,"NOCOMMAND"));i++)
		{
			row = 3 + (i - page * 20)%10;
			column = ((i - page * 20) > 9 ? 30 : 1);
			gotoxy(column, row);
			con_printf("[%c] %s", alpha[i], function[i].cmdName);
		}

		for( j = pageSys*10; (j<pageSys*10+10)&&(System[j].ID!=-1); j++ )
		{
			row = 3+(j-pageSys*10)%10;
			gotoxy(60,row);
			con_printf("System[%d]: %d", j, System[j].Handle);
		}

		gotoxy(1, 14);
		con_printf("[r] Loop = %s",loop ? "Yes" : "No");

		gotoxy(1, 15);
		con_printf("[x] Exit \n\n");

		switch(cmd = tolower(con_getch()))
		{
			// Handle future next page command
			//	case "next page"
			//		if( nOfCmd > page*20 + 20 )
			//			page++;
			//		else
			//			page = 0;
			//       break;

			// Handle future next system command
			//  case "next system"
			//		while( System[nOfSys].ID != -1 )
            //			nOfSys++;
			//		if( nOfSys > pageSys*10+10 )
            //			pageSys++;
			//		else
            //			pageSys = 0;
			//        break;

	   case 'r':
	     loop = (loop ? 0 : 1);
	    break;

		case 'x':
			quitProgram();
			break;

	    default:
			if( cmd >= 'a' && cmd < 'a' + nOfCmd )
				(*function[cmd-'a'].pFun)();      
	        break;
      }
  }
}

/*****************************************************************************/
/*                                                                           */
/*  MAIN                                                                     */
/*                                                                           */
/*  Aut: A. Morachioli                                                       */
/*                                                                           */
/*****************************************************************************/
int main(void)
{
	int   ret;
	char  esc = 0;

    loop = 0;

	for( ret = 0; ret < MAX_HVPS ; ret++ )
		System[ret].ID = -1;

	con_init();
	commandList();
	con_end();

	return 0;
}
