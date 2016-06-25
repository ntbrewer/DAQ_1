/*****************************************************************************/
/*                                                                           */
/*        --- CAEN Engineering Srl - Computing Systems Division ---          */
/*                                                                           */
/*   MAINWRAPP.H                                                             */
/*                                                                           */
/*   June      2000:  Rel. 1.0                                               */
/*   February  2001:  Rel. 1.1 added HVCaenetComm                            */
/*   November  2002:  Rel. 2.7 added HVSetBdParam                            */
/*   March     2013:  Rel. 3.0 added new CAENHVWrapper functions             */
/*                                                                           */
/*****************************************************************************/
#define  MAX_HVPS          (5)

/* struttura che contiene gli indici di sistema */
typedef struct sys
			{
				int Handle;
				int ID;
			} HV;

void HVnoFunction(void);
void HVSystemLogin(void);
void HVSystemLogout(void);
void HVLibSwRel(void);
void HVGetChName(void);
void HVSetChName(void);
void HVGetChParamProp(void);
void HVGetChParam(void);
void HVSetChParam(void);
void HVTstBdPres(void);
void HVGetBdParamProp(void);
void HVGetBdParam(void);
void HVSetBdParam(void);		// Rel. 2.7
void HVGetGrpComp(void);
void HVAddChToGrp(void);
void HVRemChFromGrp(void);
void HVGetCrateMap(void);
void HVGetExecList(void);
void HVGetSysProp(void);
void HVSetSysProp(void);
void HVExecComm(void);
void HVCaenetComm(void);     // Rel. 1.1
void quitProgram(void);

extern HV System[];
