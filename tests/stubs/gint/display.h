#ifndef MOCK_DISPLAY_H
#define MOCK_DISPLAY_H
#define C_WHITE 65535
void dclear(int);
void drect(int,int,int,int,int);
void dline(int,int,int,int,int);
void dtext(int,int,int,const char*);
void dupdate(void);
#endif
