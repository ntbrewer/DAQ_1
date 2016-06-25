struct chan_struc {
  int onoff;            /* mpod channel power on or off   */
  int Itrip;            /* mpod channel max current trip enable   */
  float Vset;           /* mpod channel set voltage       */
  float Vmeas;          /* mpod channel measured voltage  */
  float VTmeas;         /* mpod channnel measured terminal voltage */
  float Vmax;           /* mpod channnel maximum voltage */
  float Vramp;          /* mpod channnel voltage ramp*/
  float Imeas;          /* mpod channel current      */
  float Imax;           /* mpod channel max current  */
  unsigned int Sbits;   /* mpod channel status bits   */
  char name[10];        /* mpod channel mtas name     */
  char uname[10];       /* mpod channel name (crate,slot.channel) */
  char color[20];       /* status color code for gtk+ figure       */
  double xplot;         /* cairo plotting coordinates       */
  double yplot;         /* cairo plotting coordinates       */
  int clover;           /* physical ORNL clover 1-11      */    
  int ktemp;            /* temperature of clover      */    
  int klimit;           /* temperature limit of clover      */    
  char LNname[10];      /* LNfill detector name (crate,slot.channel) */
  int SDloaded;         /* Flag to indicate shutdown message is loaded into LNFILL */
} chan;

struct slot_struc {
	int number;
	struct chan_struc chan[16];
} slot ;

struct crate_struc {
	struct slot_struc slot[4];
	int name;
	int temp;
	int fanRPM;
} crate[2], input;

struct clover {
  int numb;
  int ktemp;
  int klim;
  int kovflo;
  int kfloset;
  int rtdtemp;
  int rtdlim;
  int rtdovflo;
  int rtdfloset;
  int rtddet;
  int rtdovr;
  char onoff[10]; 
}ge;


int iisave=0, jjsave=0, kksave=0;  // global x,y,z, values for checkbox to alter single channels 
char det_edit[10]="\0";             // global x,y,z, values for checkbox to alter single channels 

FILE *input_file;
char mtas_conf[100]="include/hv.conf";
char mpod_conf[100]="include/mpod.conf";
char MPOD_IP[20]="\0";




void snmp(int setget, char *cmd, char *cmdResult);
float readFloat(char *cmdResult);
int readInt(char *cmdResult);
unsigned int readBits(char *cmdResult);
float vCheck(int ii, int jj, int kk, float zz);


void mtasStrucLoad();
void mtasWriteNew();
void mtasStatus(int ii, int jj, int kk);
void hvmon(time_t curtime);
void tempShutdown(int index);

