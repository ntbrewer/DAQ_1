/***************************************************************************/
/*                                                                         */
/*        --- CAEN Engineering Srl - Computing Systems Division ---        */
/*                                                                         */
/*    CONSOLE.C                                              			         */
/*                                                                         */
/*    This file belongs to the SY527DEMO project; it contains the          */
/*    declarations of the functions implemented in CONSOLE.C and which     */
/*    permit cursor positioning, clear screen, ...                         */
/*                                                                         */
/*    Source written in Ansi C                                             */
/*                                                                         */
/*    Created: January 2000                                                */
/*                                                                         */
/***************************************************************************/
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include "console.h"

void  con_init(void)
{
initscr();
cbreak();
noecho();
nodelay(stdscr, FALSE);
curs_set(FALSE);
}

void  con_end(void)
{
endwin();
}

void  clrscr(void)
{
 clear();  
 move(0,0); 
 refresh();
}

void  highvideo(void)
{
    
}
    
void  normvideo(void)
{
    
}

int   con_getch(void)
{
int i;
 
while( ( i = getch() ) == ERR );
return i;
}

int   con_putch(int ch)
{
 int c = addch(ch);
 refresh();
 return c;    
}

int   con_kbhit(void)
{
int i, g;  
  
nodelay(stdscr, TRUE);
i = ( ( g = getch() ) == ERR ? 0 : 1 );
if( i )
    ungetch(g);
nodelay(stdscr, FALSE);

return i;    
}

int   con_printf(char *fmt, ...)
{
int     i;
va_list marker;

#if 0
char    buf[256];

va_start(marker,fmt);
i = vsprintf(buf,fmt,marker);
va_end(marker);

if( buf[i-1] == '\n' )
  {
    buf[i-1] = '\r'; 
    buf[i] = '\n'; 
    buf[i+1] = '\0'; 
    i++;
  }

printf(buf);
#else
{
va_start(marker,fmt);
i = vwprintw(stdscr,fmt,marker);
va_end(marker);
}
#endif
refresh();
return i;
}

int   con_puts(char *str)
{
return con_printf("%s\n",str);
}

int   con_scanf(char *fmt, void *app)
{
int i;

echo();
i = scanw(fmt,app);    
refresh();
noecho();
return i;
}

void  gotoxy(int x, int y)
{
move(y-1, x-1);     
refresh();
}

void  delay(int msec)
{
 usleep(msec*1000);   
}
