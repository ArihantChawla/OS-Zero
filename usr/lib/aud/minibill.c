/* Thanks for this code to Leonardo 'miniBill' Taglialegne :) */

#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

//#define M_PI 3.141

double note(int q, double st){
  double semi = pow(2,1.0/12.0);
  
  return 128*(1+sin((1<<(q/12))*pow(semi,q%12)*st));

#if 0  
  int coe;
  switch(q%7){
    case 0:
      coe = 0;
      break;
    case 1:
      coe = 2;
      break;
    case 2:
      coe = 4;
      break;
    case 3:
      coe = 5;
      break;
    case 4:
      coe = 7;
      break;
    case 5:
      coe = 9;
      break;
    case 6:
      coe = 11;
      break;
  }
  
  return 128*(1+sin((1<<(q/7))*pow(semi,coe)*st));
#endif
}

int gcd(int c, int d)
{
    int u, v, s;
 
    if (c == 0 || d == 0) { /* avoid infinite loops */
       return c | d; 
    }
 
    u = __builtin_ctz(c);
    v = __builtin_ctz(d);
 
    s = (u < v) ? u : v;
 
    u = c >> u;
    v = d >> v;
 
    while (u != v) {
        if (u > v) {
            u -= v;
            u >>= __builtin_ctz(u);
        } else {
            v -= u;
            v >>= __builtin_ctz(v);
        }
    }
 
    return u << s;
}

int arr(int q){
//  return gcd(q,60);
  return gcd(q,35);
}

double f(double t){
  double semi = pow(2,1.0/12.0);
  double tone = pow(2,1.0/6.0);
  double third = pow(2,1.0/3.0);
  double fifth = pow(2,7.0/12.0);
  double st = t * M_PI / 32.0;
  
  //return (255.0 / 2.0) * (1.0 + sin(t / 4.0));
  
  int q = t / 1000;
  return note(arr(q) % (12*4), st);
    
  /*int n = t / 200;
  while(n < 0)   n += 200;
  while(n > 200) n -= 200;
  return 255/6.0*(3+sin(st)+((n>100)?sin(third*st):sin(tone*semi*st))+sin(fifth*st));*/
  
  /*int n = t / 100;
  while(n < 0)   n += 100;
  while(n > 100) n -= 100;
  return (255/2)*(1+(n<25)*sin(t/2.0)+(n>=25&&n<50)*sin(fifth*t/2.0)+(n>=50&&n<75)*sin(third*t/2.0)+(n>=75)*sin(fifth*t/2.0));*/
}

double mint = 0;
double maxt; // = COLS

double minw = 0-256*3;
double maxw = 256+256*3;

void left(){
  double delta = maxt - mint;
  mint -= delta / 8.0;
  maxt -= delta / 8.0;
}

void right(){
  double delta = maxt - mint;
  mint += delta / 8.0;
  maxt += delta / 8.0;
}

void down(){
  double delta = maxw - minw;
  minw += delta / 8.0;
  maxw += delta / 8.0;
}

void up(){
  double delta = maxw - minw;
  minw -= delta / 8.0;
  maxw -= delta / 8.0;
}

void zoom(){
  double tdelta = maxt - mint;
  mint += tdelta / 8.0;
  maxt -= tdelta / 8.0;
  double wdelta = maxw - minw;
  minw += wdelta / 8.0;
  maxw -= wdelta / 8.0;
}

void dezoom(){
  double tdelta = maxt - mint;
  mint -= tdelta / 8.0;
  maxt += tdelta / 8.0;
  double wdelta = maxw - minw;
  minw -= wdelta / 8.0;
  maxw += wdelta / 8.0;  
}

void output(char fill){
   double tdelta = maxt - mint;
   double wdelta = maxw - minw;
   int i;
   for(i = 0; i < COLS; i++){
     double currt = mint + (tdelta * i) / COLS;
     attron(COLOR_PAIR(2));
     int outf = (0 - minw) * (LINES - 2) / wdelta + 2;
     if(outf > 1 && outf < LINES)
       mvaddch(outf, i, fill==' '?' ':'=');
     outf = (255 - minw) * (LINES - 2) / wdelta + 2;
     if(outf > 1 && outf < LINES)
       mvaddch(outf, i, fill==' '?' ':'=');

     double currf = f(currt);
     outf = (currf - minw) * (LINES - 2) / wdelta + 2;
     attron(COLOR_PAIR(1));
     if(outf > 1 && outf < LINES)
       mvaddch(outf, i, fill);
     
     attron(COLOR_PAIR(2));
     double currz = (0 - mint) * COLS / (maxt - mint);
     if(currz >= 0 && currz < COLS){
       outf = (0 - minw) * (LINES - 2) / wdelta + 2;
       mvaddch(outf, currz, fill==' '?' ':'|');
       outf = (255 - minw) * (LINES - 2) / wdelta + 2;
       mvaddch(outf, currz, fill==' '?' ':'|');
     }
   }
}

void clean(){
  output(' ');
}

void rebake(){
  output('*');
}

char playing = 0;

FILE *popen(const char *command, const char *type); 

void * play(void * foo){
  move(LINES-1,0);
  FILE * pipe = popen("aplay -r 48000 2>&1 > /dev/null","w");
  int t;
  for(t=0;;t++)
    fputc(f(t/8.0), pipe);
}

int main(int argc, char ** argv){
  pthread_t play_thread;
  
  initscr();
  erase();
  keypad(stdscr, TRUE);
  noecho();
  maxt = COLS;
  printw("q to exit, wasd/arrows/hjkl to move, +-/er/ui to zoom and dezoom. space to start/stop play. '=' are 0 and 255. '|' is 0");
  refresh();
  
  if(has_colors() == FALSE)
    mvprintw(1, 0, "You can has no colorz :(");
  else{
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
  }
  
  while(true){
    rebake();
    int ch = getch();
    clean();
    switch(ch){
      case KEY_LEFT:
      case 'h':
      case 'a':
	left();
	break;
      case KEY_RIGHT:
      case 'l':
      case 'd':
	right();
	break;
      case KEY_DOWN:
      case 'j':
      case 's':
	down();
	break;
      case KEY_UP:
      case 'k':
      case 'w':
	up();
	break;
      case '+':
      case 'e':
      case 'u':
	zoom();
	break;
      case '-':
      case 'r':
      case 'i':
	dezoom();
	break;
      case ' ':
	playing ^= 1;
	if(playing)
	    pthread_create(&play_thread, NULL, play, 0);
	else
	    pthread_cancel(play_thread);
	break;
    }
    if(ch == 'q')
      break;
    refresh();
  }
  
  endwin();
  
  return 0;
}
