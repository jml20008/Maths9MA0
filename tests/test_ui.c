#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/keycodes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/app_data.h"
static const int sequence[]={
 KEY_EXE,KEY_EXE,KEY_F2,KEY_F3,
 KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXE,
 KEY_EXIT,KEY_EXIT,KEY_EXIT,
 KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,
 KEY_EXE,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_EXE,KEY_F3,
 KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXIT,KEY_EXIT,KEY_EXIT,
 KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_EXE,
 KEY_EXE,KEY_F3,KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXIT,KEY_EXIT,
 KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_EXE,KEY_F3,
 KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXE,KEY_F2,
 KEY_EXIT,KEY_EXIT,KEY_EXIT,
 KEY_F2,KEY_EXE,KEY_EXIT,KEY_EXIT,
 KEY_F3,KEY_DOWN,KEY_F3,
 KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXE,KEY_EXE,KEY_F5,
 KEY_EXIT,KEY_EXIT,KEY_EXIT,KEY_EXIT
};
static size_t idx=0;
static int ap=0,rf=0,gp=0,bin=0,example=0,formula=0,extrema=0;
void dclear(int c){(void)c;}
void drect(int a,int b,int c,int d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;}
void dline(int a,int b,int c,int d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;}
void dtext(int x,int y,int c,const char*s){(void)x;(void)y;(void)c;
 if(strstr(s,"d = (u_j-u_i)/(j-i) = 3"))ap=1;
 if(strstr(s,"R=sqrt(a^2+b^2)=13"))rf=1;
 if(strstr(s,"S_inf = 24"))gp=1;
 if(strstr(s,"x^3 : -5/2"))bin=1;
 if(strstr(s,"WORKED EXAMPLE"))example=1;
 if(strstr(s,"sin(2x)=2sin x cos x"))formula=1;
 if(strstr(s,"MAX = 12 at x=53.130102"))extrema=1;
}
void dupdate(void){}
key_event_t getkey(void){if(idx>=sizeof(sequence)/sizeof(sequence[0])){fprintf(stderr,"Navigation did not exit\n");exit(1);}return (key_event_t){sequence[idx++]};}
int app_main(void);
int main(void){
 int cards=0;
 for(int i=0;i<REVISION_TOPIC_COUNT;i++)for(int j=0;j<revision_topics[i].count;j++){const Card *c=&revision_topics[i].cards[j];if(!*c->method||!*c->example)return 1;cards++;}
 int ret=app_main();
 if(ret||!ap||!rf||!gp||!bin||!example||!formula||!extrema){fprintf(stderr,"FAIL UI ap=%d rf=%d gp=%d bin=%d example=%d formula=%d extrema=%d\n",ap,rf,gp,bin,example,formula,extrema);return 1;}
 printf("PASS %d menus, %d method/example cards, formula/tool shortcuts and scripted navigation\n",REVISION_TOPIC_COUNT,cards);return 0;
}
