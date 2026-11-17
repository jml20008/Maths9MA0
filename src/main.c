#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/keycodes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_data.h"
#include "engine.h"
#define RGB(r,g,b) ((((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3))
#define NAVY RGB(15,36,64)
#define BLUE RGB(28,92,181)
#define BG RGB(240,246,252)
#define INK RGB(24,42,64)
#define MUTED RGB(100,122,148)
#define GREEN RGB(0,132,94)
#define RED RGB(192,52,61)
#define W 384
#define MAX_LINE 43
enum { MENU, QUESTIONS, READ, WIZARD, RESULT };
static int screen=MENU,t=0,c=0,view=0,top=0,scroll=0;
static int tool=0,step=0,total=0,factors=1,degree=3,result_mode=0;
static char input[32],error[100];
static int fresh=1;
static double vals[16];
static Rational ratvals[16];
static BinFactor bf[4];
static BinResult br;
static SolveOutput result;
static const Card *card(void){return &revision_topics[t].cards[c];}
static void header(const char *title,const char *sub){
    dclear(BG);drect(0,0,383,35,NAVY);
    dtext(9,4,C_WHITE,title);dtext(9,20,RGB(166,199,235),sub);
}
static void foot(const char *a,const char *b,const char *cc,const char *d,const char *e,const char *f){
    const char *v[]={a,b,cc,d,e,f};
    drect(0,197,383,215,NAVY);
    for(int i=0;i<6;i++){
        if(v[i])dtext(5+64*i,200,C_WHITE,v[i]);
        if(i)dline(i*64,197,i*64,215,MUTED);
    }
}
static void row(int y,const char *s,int active){
    drect(5,y,378,y+18,active?BLUE:C_WHITE);
    dtext(11,y+3,active?C_WHITE:INK,s);
}
#define MAX_WRAP 160
static int wrap(const char *text,char lines[MAX_WRAP][MAX_LINE+1]){
    int n=0;
    while(*text && n<MAX_WRAP){
        int len=0,last=-1;while(text[len] && text[len]!='|' && len<MAX_LINE){if(text[len]==' ')last=len;len++;}
        if(len==MAX_LINE && text[len] && text[len]!='|' && last>0)len=last;
        memcpy(lines[n],text,len);lines[n][len]=0;n++;
        text+=len;while(*text==' '||*text=='|')text++;
    }return n;
}
static void textpage(const char *s,int y){
    static char lines[MAX_WRAP][MAX_LINE+1];int n=wrap(s,lines),visible=(190-y)/18+1;
    if(scroll>=n)scroll=n>0?n-1:0;
    if(scroll>n-visible)scroll=n>visible?n-visible:0;
    for(int i=0;i<visible && i+scroll<n;i++)dtext(10,y+18*i,INK,lines[i+scroll]);
}
static void list(void){
    if(screen==MENU){
        header("PURE MATHS V3","Year 1 + Year 2 up to trig");
        if(t<top)top=t;
        if(t>=top+8)top=t-7;
        for(int i=top;i<REVISION_TOPIC_COUNT && i<top+8;i++){char b[80];snprintf(b,sizeof(b),"%02d  %s",i+1,revision_topics[i].name);row(38+(i-top)*19,b,i==t);}
    }else{
        header(revision_topics[t].name,"Choose the question you are stuck on");
        if(c<top)top=c;
        if(c>=top+8)top=c-7;
        for(int i=top;i<revision_topics[t].count&&i<top+8;i++)row(38+(i-top)*19,revision_topics[t].cards[i].title,i==c);
    }
    foot("Open",screen==MENU?"Formula":0,screen==MENU?"Tools":card()->tool?"Solve":0,0,0,"Back");
}
static void axes(int cx,int cy){dline(20,cy,354,cy,MUTED);dline(cx,40,cx,188,MUTED);dtext(357,cy-10,INK,"x");dtext(cx+4,39,INK,"y");}
static void diagram(int kind){
    if(kind==DIAG_CAST){
        axes(192,113);dtext(215,55,BLUE,"ALL +");dtext(85,55,BLUE,"SIN +");dtext(85,145,BLUE,"TAN +");dtext(215,145,BLUE,"COS +");
        dtext(198,100,INK,"0");dtext(197,41,INK,"90");dtext(20,100,INK,"180");dtext(197,173,INK,"270");
    }else if(kind==DIAG_MOD){axes(100,126);dline(35,41,220,126,BLUE);dline(220,126,350,66,BLUE);dline(220,126,90,186,MUTED);dtext(225,135,INK,"2");dtext(230,45,BLUE,"y=|x-2|");dtext(17,175,MUTED,"original below axis");}
    else if(kind==DIAG_VECTOR){axes(45,180);dline(70,155,285,63,BLUE);dline(285,63,270,64,BLUE);dline(285,63,276,76,BLUE);dtext(63,139,INK,"A");dtext(290,48,INK,"B");dtext(130,88,BLUE,"AB = b-a");}
    else if(kind==DIAG_INTERSECT){
        axes(150,155);
        dline(25,160,310,57,GREEN);
        int px=0,py=0;
        for(int x=22;x<=350;x++){
            double z=(x-150)/55.0;
            int y=155-(int)(z*z*20);
            if(x>22&&y>=40&&py>=40)dline(px,py,x,y,BLUE);
            px=x;py=y;
        }
        drect(93,133,97,137,RED);drect(258,73,262,77,RED);
        dtext(45,112,RED,"(-1,1)");dtext(265,79,RED,"(2,4)");
        dtext(262,45,GREEN,"y=x+2");dtext(20,45,BLUE,"y=x^2");
    }else {
        axes(170,139);
        int px=0,py=0;
        for(int x=22;x<=350;x++){
            double z=(x-170)/70.0,f=0;
            if(kind==DIAG_AREA)f=2*z-z*z;
            else if(kind==DIAG_TURN)f=.5*z*z-.6;
            else if(kind==DIAG_RANGE)f=.5*z*z-.6;
            else f=.6*z*z;
            int y=139-(int)(f*42);
            if(y>=40&&y<=188&&x>22&&py>=40&&py<=188)dline(px,py,x,y,BLUE);
            if(kind==DIAG_AREA&&z>=0&&z<=2&&y<=139)dline(x,y,x,139,RGB(156,202,239));
            px=x;py=y;
        }
        if(kind==DIAG_AREA)dtext(12,45,INK,"Area = integral (signed)");
        if(kind==DIAG_RANGE||kind==DIAG_TURN){dline(142,164,198,164,GREEN);dtext(202,168,INK,"minimum");}
    }
}
static void readcard(void){
    header(card()->title,view==0?"METHOD / FORMULAS":view==1?"WORKED EXAMPLE":"DIAGRAM");
    if(view==2)diagram(card()->diagram);else textpage(view?card()->example:card()->method,43);
    foot("Method","Example",card()->tool?"Tool":0,card()->diagram?"Graph":0,0,"Back");
}
static int digit(int k){
    switch(k){case KEY_0:return 0;case KEY_1:return 1;case KEY_2:return 2;case KEY_3:return 3;case KEY_4:return 4;case KEY_5:return 5;case KEY_6:return 6;case KEY_7:return 7;case KEY_8:return 8;case KEY_9:return 9;}return -1;
}
typedef struct {int id,count;const char *name,*form;const char *field[12];double defaults[12];} ExtraSpec;
static const ExtraSpec extras[]={
{TOOL_R_INTERVAL,5,"HARMONIC MAX / MIN","y=a*sin x+b*cos x+c",{"Sin coefficient a","Cos coefficient b","Constant c","Lower angle (degrees)","Upper angle (degrees)"},{8,6,2,0,90}},
{TOOL_R_DIRECT,5,"ALREADY IN R FORM","y=R*sin(x+alpha)+c",{"Amplitude R","Alpha (degrees)","Constant c","Lower angle (degrees)","Upper angle (degrees)"},{10,36.87,2,0,360}},
{TOOL_AP_KNOWN,3,"ARITHMETIC TERM / SUM","u_n=a+(n-1)d",{"First term a","Common difference d","Number of terms n"},{4,3,13}},
{TOOL_AP_N,3,"ARITHMETIC FIND n","u_n=a+(n-1)d",{"First term a","Common difference d","Target term u_n"},{4,3,40}},
{TOOL_AP_SUM_N,3,"ARITHMETIC SUM FIND n","S_n=n/2[2a+(n-1)d]",{"First term a","Common difference d","Target sum S_n"},{4,3,286}},
{TOOL_GP_TWO,5,"GEOMETRIC FIND a, r","u_n=a*r^(n-1)",{"First known term position","First known term value","Second known term position","Second known term value","Requested n for term / sum"},{2,6,4,24,5}},
{TOOL_GP_N,3,"GEOMETRIC FIND n","Positive common ratio only",{"First term a","Common ratio r","Target term u_n"},{12,.5,1.5}},
{TOOL_GP_SUM_N,3,"GEOMETRIC SUM FIND n","Positive common ratio only",{"First term a","Common ratio r","Target sum S_n"},{12,.5,22.5}},
{TOOL_EXP_TIME,4,"EXPONENTIAL FIND t","y=A*b^t+c",{"Multiplier A","Base b (e.g. 1.08)","Constant / asymptote c","Target y"},{100,1.2,0,250}},
{TOOL_EXP_VALUE,4,"EXPONENTIAL EVALUATE","y=A*b^t+c",{"Multiplier A","Base b (e.g. 1.08)","Constant / asymptote c","Time t"},{500,1.08,0,3}},
{TOOL_EXP_FIT,5,"EXPONENTIAL FIT MODEL","y=A*b^t+c",{"First time t1","First value y1","Second time t2","Second value y2","Known offset c (0 if none)"},{1,12,3,48,0}},
{TOOL_EXP_QUAD,4,"EXPONENTIAL QUADRATIC","a*(base^x)^2+b*base^x+c=0",{"Quadratic coefficient a","Linear coefficient b","Constant c","Exponential base"},{1,-5,6,2}},
{TOOL_EXP_K_TIME,4,"CONTINUOUS EXP FIND t","y=A*e^(k*t)+c",{"Multiplier A","Growth constant k","Constant / asymptote c","Target y"},{100,.2,0,250}},
{TOOL_PF2,6,"TWO DISTINCT FACTORS","(mx+n)/[(px+q)(rx+s)]",{"Numerator x coeff m","Numerator constant n","Factor 1 x coeff p","Factor 1 constant q","Factor 2 x coeff r","Factor 2 constant s"},{2,3,1,-1,1,2}},
{TOOL_PF3,9,"THREE DISTINCT FACTORS","(ax^2+bx+c)/(L*M*N)",{"Numerator x^2 coefficient","Numerator x coefficient","Numerator constant","Factor L x coeff","Factor L constant","Factor M x coeff","Factor M constant","Factor N x coeff","Factor N constant"},{6,12,-6,1,-1,1,2,1,3}},
{TOOL_PFR,7,"REPEATED LINEAR FACTOR","(ax^2+bx+c)/(L^2*M)",{"Numerator x^2 coefficient","Numerator x coefficient","Numerator constant","Repeated L x coeff","Repeated L constant","Other M x coeff","Other M constant"},{1,7,1,1,-1,1,2}},
{TOOL_DIV,9,"POLYNOMIAL DIVISION","Enter degrees then coefficients",{"Dividend degree (0-5)","Divisor degree (1-3)"},{3,1,1,-2,-1,2,1,-1}}
};
static const ExtraSpec *spec(void){for(size_t i=0;i<sizeof(extras)/sizeof(extras[0]);i++)if(extras[i].id==tool)return &extras[i];return NULL;}
static int exacttool(void){return tool==TOOL_PF2||tool==TOOL_PF3||tool==TOOL_PFR||tool==TOOL_DIV;}
static const char *prompt(void){
    static const char *ap[]={"First known term position i","Value of term u_i","Second term position j","Value of term u_j","Find term / sum at n"};
    static const char *rf[]={"Coefficient of sin x (a)","Coefficient of cos x (b)","Constant added (c)"};
    static const char *gp[]={"First term a","Common ratio r","Number of terms n"};
    static const char *bin[]={"Constant a","Coefficient b","Exponent n (can be fraction)","Power q of x inside bracket"};
    if(tool==TOOL_AP)return ap[step];
    if(tool==TOOL_R)return rf[step];
    if(tool==TOOL_GP)return gp[step];
    const ExtraSpec *ex=spec();
    if(ex){
        if(tool==TOOL_DIV&&step>=2){static char label[64];int n=(int)vals[0],m=(int)vals[1];if(step<=2+n)snprintf(label,sizeof(label),"Dividend coefficient x^%d",n-(step-2));else snprintf(label,sizeof(label),"Divisor coefficient x^%d",m-(step-3-n));return label;}
        return ex->field[step];
    }
    if(step==0)return "Number of brackets (1-4)";
    if(step==1)return "Highest power of x to calculate";
    return bin[(step-2)%4];
}
static Rational fraction(void){
    char *end;long long n=strtoll(input,&end,10),d=1;
    if(end==input)return rat(0,0);
    if(*end=='/'){char *e;d=strtoll(end+1,&e,10);if(e==end+1||*e)return rat(0,0);}
    else if(*end)return rat(0,0);
    if(n>1000000||n< -1000000||d>1000000||d< -1000000)return rat(0,0);
    return rat(n,d);
}
static void loadinput(void){
    error[0]=0;fresh=1;
    if(tool!=TOOL_BIN){if(exacttool()&&ratvals[step].den>1)snprintf(input,sizeof(input),"%lld/%lld",ratvals[step].num,ratvals[step].den);else snprintf(input,sizeof(input),"%.9g",vals[step]);return;}
    if(step<2){snprintf(input,sizeof(input),"%d",step?degree:factors);return;}
    BinFactor *b=&bf[(step-2)/4];int k=(step-2)%4;
    if(k==3){snprintf(input,sizeof(input),"%d",b->xpower);return;}
    Rational r=k==0?b->a:k==1?b->b:b->exponent;
    if(r.den==1)snprintf(input,sizeof(input),"%lld",r.num);
    else snprintf(input,sizeof(input),"%lld/%lld",r.num,r.den);
}
static void starttool(void){
    tool=card()->tool;step=0;scroll=0;screen=WIZARD;total=tool==TOOL_AP?5:3;
    if(tool==TOOL_AP){double a[]={4,11,10,29,10};memcpy(vals,a,sizeof(a));}
    else if(tool==TOOL_R){vals[0]=5;vals[1]=12;vals[2]=0;}
    else if(tool==TOOL_GP){vals[0]=12;vals[1]=.5;vals[2]=4;}
    else if(tool==TOOL_BIN){factors=1;degree=3;total=6;for(int i=0;i<4;i++)bf[i]=(BinFactor){rat(1,1),rat(1,1),rat(2,1),1};bf[0].b=rat(2,1);bf[0].exponent=rat(-1,2);}
    else {const ExtraSpec *ex=spec();if(!ex){screen=READ;return;}total=ex->count;memset(vals,0,sizeof(vals));for(int i=0;i<16;i++)ratvals[i]=rat(0,1);for(int i=0;i<ex->count;i++){vals[i]=ex->defaults[i];if(exacttool())ratvals[i]=rat((long long)vals[i],1);}if(tool==TOOL_DIV)total=4+(int)vals[0]+(int)vals[1];}
    loadinput();
}
static void calculate(void){
    out_clear(&result);scroll=0;result_mode=0;screen=RESULT;
    if(tool==TOOL_BIN){binomial_expand(bf,factors,degree,&br);return;}
    if(tool==TOOL_AP){
        double i=vals[0],j=vals[2],n=vals[4];
        if(i<1||j<1||n<1||i==j||floor(i)!=i||floor(j)!=j||floor(n)!=n){out_add(&result,"Positions must be distinct");out_add(&result,"positive integers; n positive.");return;}
        double d=(vals[3]-vals[1])/(j-i),a=vals[1]-(i-1)*d,l=a+(n-1)*d;
        out_add(&result,"d = (u_j-u_i)/(j-i) = %.9g",d);
        out_add(&result,"a = u_i-(i-1)d = %.9g",a);
        out_add(&result,"u_n = %.9g+(n-1)*%.9g",a,d);
        out_add(&result,"u_%.0f = %.9g",n,l);out_add(&result,"S_%.0f = %.9g",n,n*(a+l)/2);
    }else if(tool==TOOL_R){
        double a=vals[0],b=vals[1],offset=vals[2],r=hypot(a,b),alpha=atan2(b,a)*180/3.141592653589793;
        if(r==0){out_add(&result,"Constant: %.9g",offset);return;}
        double max=fmod(90-alpha+720,360),min=fmod(270-alpha+720,360);
        out_add(&result,"R=sqrt(a^2+b^2)=%.9g",r);
        out_add(&result,"Alpha=%.7g degrees",alpha);
        out_add(&result,"y=%.7g sin(x%+.7g)%+.7g",r,alpha,offset);
        out_add(&result,"Max=%.9g at x=%.7g deg",offset+r,max);
        out_add(&result,"Min=%.9g at x=%.7g deg",offset-r,min);
        out_add(&result,"Repeat angles every 360 degrees.");
        out_add(&result,"Restricted interval? Check endpoints.");
    }else if(tool==TOOL_GP){double v[]={vals[0],vals[1],vals[2],2};solve_problem(SOL_APGP,v,&result);}
    else revision_solve(tool,vals,ratvals,&result);
}
static int accept(void){
    if(tool==TOOL_BIN){
        Rational r=fraction();
        if(!rational_ok(r)){strcpy(error,"Use integer or fraction (F2 = /).");return 0;}
        if(step<2||(step>=2&&(step-2)%4==3)){
            int max=step==0?4:step==1?20:8, min=step==1?0:1;
            if(r.den!=1||r.num<min||r.num>max){strcpy(error,"Enter a whole number in range.");return 0;}
            if(step==0){factors=(int)r.num;total=2+4*factors;}
            else if(step==1)degree=(int)r.num;
            else bf[(step-2)/4].xpower=(int)r.num;
        }else {BinFactor *b=&bf[(step-2)/4];switch((step-2)%4){case 0:b->a=r;break;case 1:b->b=r;break;case 2:b->exponent=r;break;}}
    }else if(exacttool()){
        Rational r=fraction();
        if(!rational_ok(r)){strcpy(error,"Enter an integer or fraction (F2 /).");return 0;}
        if(tool==TOOL_DIV&&step<2){
            if(r.den!=1||r.num<(step?1:0)||r.num>(step?3:5)){strcpy(error,"Degree must be in stated range.");return 0;}
            if(vals[step]!=(double)r.num){for(int i=2;i<16;i++){vals[i]=0;ratvals[i]=rat(0,1);}}
        }
        ratvals[step]=r;vals[step]=(double)r.num/r.den;
        if(tool==TOOL_DIV&&step<2)total=4+(int)vals[0]+(int)vals[1];
    }else{
        char *end;double x=strtod(input,&end);
        if(end==input){strcpy(error,"Enter a valid number.");return 0;}
        if(*end=='/'){char *e;double d=strtod(end+1,&e);if(e==end+1||*e||d==0){strcpy(error,"Invalid fraction.");return 0;}x/=d;end=e;}
        if(end==input||*end||!isfinite(x)||fabs(x)>1e12){strcpy(error,"Enter a valid number.");return 0;}
        vals[step]=x;
    }
    if(step+1>=total)calculate();else{step++;loadinput();}return 1;
}
static void wizard(void){
    char sub[64];snprintf(sub,sizeof(sub),"Step %d of %d",step+1,total);
    const ExtraSpec *ex=spec();
    header(ex?ex->name:tool==TOOL_BIN?"BINOMIAL TOOL":tool==TOOL_R?"HARMONIC FORM":tool==TOOL_AP?"ARITHMETIC TOOL":"GEOMETRIC TOOL",sub);
    if(ex)dtext(10,47,BLUE,ex->form);
    if(tool==TOOL_BIN&&step>=2){char b[80];snprintf(b,sizeof(b),"Bracket %d: (a+b*x^q)^n",(step-2)/4+1);dtext(10,47,BLUE,b);}
    dtext(10,70,INK,prompt());
    drect(10,93,373,126,C_WHITE);dtext(17,103,BLUE,input);
    dtext(10,143,MUTED,"Type value; EXE confirms / next");
    if(error[0])dtext(10,165,RED,error);
    else dtext(10,165,MUTED,"F2 fraction /, (-) sign, DEL erase");
    foot("OK","/","Prev",0,0,"Cancel");
}
static void series_text(char *buf,size_t cap){
    size_t used=0;int terms=0;
    buf[0]=0;
    for(int k=0;k<=degree;k++){
        Coeff cc=br.coeff[k];
        if(cc.is_exact?cc.exact.num==0:cc.value==0)continue;
        char q[65],term[85];binomial_coeff_string(cc,q,sizeof(q));
        int negative=cc.is_exact?cc.exact.num<0:cc.value<0;
        const char *number=q;
        if(*number=='~')number++;
        if(*number=='-')number++;
        char xp[14]="";if(k==1)strcpy(xp,"x");else if(k>1)snprintf(xp,sizeof(xp),"x^%d",k);
        const char *sign=terms?(negative?" - ":" + "):(negative?"-":"");
        const char *coefficient=(k>0&&cc.is_exact&&(!strcmp(number,"1")))?"":number;
        snprintf(term,sizeof(term),"%s%s%s%s%s",sign,cc.is_exact?"":"~",coefficient,*coefficient&&k?"*":"",xp);
        size_t n=strlen(term);if(used+n+1>=cap)break;
        memcpy(buf+used,term,n+1);used+=n;terms++;
    }
    if(!terms)snprintf(buf,cap,"0");
}
static void showresult(void){
    char h[80];
    if(tool==TOOL_BIN)snprintf(h,sizeof(h),"Terms through x^%d | UP/DOWN scroll",degree);
    else snprintf(h,sizeof(h),"UP/DOWN scroll; EXIT returns to method");
    header("RESULT",h);
    if(tool==TOOL_BIN){
        if(!br.valid){textpage(br.error,43);}
        else{
            char b[96];
            if(isfinite(br.convergence))snprintf(b,sizeof(b),"Valid |x| < %.8g",br.convergence);else snprintf(b,sizeof(b),"No finite convergence restriction");dtext(10,43,GREEN,b);
            if(result_mode==0){char series[1800];series_text(series,sizeof(series));textpage(series,65);}
            else for(int y=0;y<7&&y+scroll<=degree;y++){
                int k=y+scroll;char q[65];binomial_coeff_string(br.coeff[k],q,sizeof(q));
                snprintf(b,sizeof(b),"x^%d : %s",k,q);dtext(10,64+18*y,INK,b);
            }
        }
    }else{
        static char joined[SOLVE_LINES*(SOLVE_LINE_LEN+1)];
        size_t u=0;
        for(int i=0;i<result.count;i++){size_t n=strlen(result.line[i]);if(u+n+2>=sizeof(joined))break;memcpy(joined+u,result.line[i],n);u+=n;joined[u++]='|';}joined[u]=0;
        textpage(joined,43);
    }
    foot("Edit",tool==TOOL_BIN?(result_mode?"Series":"Coeffs"):0,0,0,tool==TOOL_BIN?0:"End","Back");
}
int main(void){
    while(1){
        if(screen==MENU||screen==QUESTIONS)list();
        else if(screen==READ)readcard();
        else if(screen==WIZARD)wizard();
        else showresult();
        dupdate();int k=getkey().key;
        int back=k==KEY_EXIT||k==KEY_F6;
        if(screen==MENU||screen==QUESTIONS){
            int *p=screen==MENU?&t:&c,n=screen==MENU?REVISION_TOPIC_COUNT:revision_topics[t].count;
            if(screen==MENU&&(k==KEY_F2||k==KEY_F3)){t=k==KEY_F2?14:15;c=0;top=0;screen=QUESTIONS;continue;}
            if(k==KEY_UP&&*p>0)(*p)--;
            if(k==KEY_DOWN&&*p<n-1)(*p)++;
            if(screen==QUESTIONS&&k==KEY_F3&&card()->tool){view=0;starttool();}
            if(k==KEY_EXE||k==KEY_F1){if(screen==MENU){c=0;top=0;screen=QUESTIONS;}else{view=0;scroll=0;screen=READ;}}
            if(back){if(screen==MENU)return 0;screen=MENU;top=t>7?t-7:0;}
        }else if(screen==READ){
            if(back){screen=QUESTIONS;top=c>7?c-7:0;}
            else if(k==KEY_F1){view=0;scroll=0;}
            else if(k==KEY_F2){view=1;scroll=0;}
            else if(k==KEY_EXE||k==KEY_RIGHT){view=view==0?1:0;scroll=0;}
            else if(k==KEY_LEFT){view=0;scroll=0;}
            else if(k==KEY_F3&&card()->tool)starttool();
            else if(k==KEY_F4&&card()->diagram){view=2;scroll=0;}
            else if(k==KEY_UP&&scroll>0)scroll--;
            else if(k==KEY_DOWN&&scroll<MAX_WRAP-1)scroll++;
        }else if(screen==WIZARD){
            if(back){screen=READ;scroll=0;}
            else if(k==KEY_F3&&step>0){step--;loadinput();}
            else if(k==KEY_EXE||k==KEY_F1)accept();
            else{
                int d=digit(k);size_t n=strlen(input);error[0]=0;
                if(d>=0){if(fresh){input[0]=0;n=0;fresh=0;}if(n<23){input[n]=(char)('0'+d);input[n+1]=0;}}
                else if(k==KEY_F2||k==KEY_DOT){char ch=k==KEY_F2?'/':'.';if(n<23&&!strchr(input,ch)){input[n]=ch;input[n+1]=0;fresh=0;}}
                else if(k==KEY_NEG||k==KEY_SUB){if(fresh){strcpy(input,"-");fresh=0;}else if(input[0]=='-')memmove(input,input+1,n);else if(n<23){memmove(input+1,input,n+1);input[0]='-';}}
                else if(k==KEY_DEL){if(n)input[n-1]=0;fresh=0;}
                else if(k==KEY_ACON){input[0]=0;fresh=0;}
            }
        }else{
            if(back){screen=READ;scroll=0;}
            else if(k==KEY_F1){screen=WIZARD;step=0;loadinput();}
            else if(k==KEY_F2&&tool==TOOL_BIN){result_mode=!result_mode;scroll=0;}
            else if(k==KEY_UP&&scroll>0)scroll--;
            else if(k==KEY_F5&&tool!=TOOL_BIN)scroll=MAX_WRAP-1;
            else if(k==KEY_DOWN&&scroll<(tool==TOOL_BIN&&result_mode?degree:MAX_WRAP-1))scroll++;
        }
    }
}
