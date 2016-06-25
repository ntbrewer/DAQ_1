/***************************************************************************/
/*                                                                         */
/*        --- CAEN Engineering Srl - Computing Systems Division ---        */
/*                                                                         */
/*    CONSOLE.H                                              			         */
/*                                                                         */
/*    Declarations of routines which permit cursor positioning, clear      */
/*    screen, and other functionalities which are not implemented in the   */
/*    standard C library                                                   */
/*                                                                         */
/*    Created: January 2000                                                */
/*                                                                         */
/***************************************************************************/


/***------------------------------------------------------------------------

  con_init
  Must be called before any other call to routines declared here.
  It initializes the console.
  Arguments:
    none
  Return value:
    none

    --------------------------------------------------------------------***/
void  con_init(void);

/***------------------------------------------------------------------------

  con_end
  Can be called at program shutdown the reset the console to the default 
  values.
  Arguments:
    none
  Return value:
    none

    --------------------------------------------------------------------***/
void  con_end(void);

/***------------------------------------------------------------------------

  clrscr
  Clears the screen.
  Arguments:
    none
  Return value:
    none

    --------------------------------------------------------------------***/
void  clrscr(void);

/***------------------------------------------------------------------------

  highvideo
  Enhances any subsequent print on the screen since a call to normvideo
  Arguments:
    none
  Return value:
    none

    --------------------------------------------------------------------***/
void  highvideo(void);

/***------------------------------------------------------------------------

  normvideo
  End of enhanced printing.                                           
  Arguments:
    none
  Return value:
    none

    --------------------------------------------------------------------***/
void  normvideo(void);

/***------------------------------------------------------------------------

  con_getch
  Reads a character from the console without waiting an enter.          
  Arguments:
    none
  Return value:
    the character read

    --------------------------------------------------------------------***/
int   con_getch(void);

/***------------------------------------------------------------------------

  con_putch
  Sends a character to the console.          
  Arguments:
    the character to print
  Return value:
    the character printed

    --------------------------------------------------------------------***/
int   con_putch(int ch);

/***------------------------------------------------------------------------

  con_puts
  Sends a string to the console.       
  Arguments:
    the string to put
  Return value:
    the number of character printed

    --------------------------------------------------------------------***/
int   con_puts(char *s);

/***------------------------------------------------------------------------

  con_kbhit
  Waits a until a key is pressed.          
  Arguments:
    none
  Return value:
    1 if a key is pressed, 0 if none

    --------------------------------------------------------------------***/
int   con_kbhit(void);

/***------------------------------------------------------------------------

  con_printf.          
  Arguments:
    like printf
  Return value:
    like printf

    --------------------------------------------------------------------***/
int   con_printf(char *buf, ...);

/***------------------------------------------------------------------------

  con_scanf
  It emulates the scanf function; it only works with a fixed number of 
  parameters.
  Arguments:
    like scanf, but only one parameter is admitted
  Return value:
    like scanf

    --------------------------------------------------------------------***/
int   con_scanf(char *fmt, void *app);

/***------------------------------------------------------------------------

  gotoxy
  Moves the cursor position at (x,y)                                  
  Arguments:
    the x and y positions of the cursor
  Return value:
    none

    --------------------------------------------------------------------***/
void  gotoxy(int x, int y);

/***------------------------------------------------------------------------

  delay
  Waits the specified time (possibly not CPU wasting).          
  Arguments:
    the time to wait in milliseconds.
  Return value:
    none

    --------------------------------------------------------------------***/
void  delay(int msec);
