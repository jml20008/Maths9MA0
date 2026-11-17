#include "../src/engine.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
static int checks=0;
static void require(int ok,const char *why){checks++;if(!ok){fprintf(stderr,"FAIL: %s\n",why);exit(1);}}
static void eq(double a,double b,const char *why){require(fabs(a-b)<1e-8,why);}
static BinFactor F(int an,int ad,int bn,int bd,int pn,int pd,int xq){return (BinFactor){ {an,ad},{bn,bd},{pn,pd},xq};}
static int has(SolveOutput *o,const char *s){for(int i=0;i<o->count;i++)if(strstr(o->line[i],s))return 1;return 0;}
static void exact(SolveOutput *o,int tool,const double *p,int n){Rational q[16];for(int i=0;i<n;i++)q[i]=rat((long long)p[i],1);require(revision_solve(tool,p,q,o),"exact solver success");}
int main(void){
 BinResult r;BinFactor f[4];
 f[0]=F(1,1,2,1,3,1,1);
 require(binomial_expand(f,1,3,&r),"simple binomial expansion");
 eq(r.coeff[0].value,1,"integer constant");eq(r.coeff[1].value,6,"integer x");eq(r.coeff[2].value,12,"integer x2");eq(r.coeff[3].value,8,"integer x3");
 require(r.coeff[3].is_exact,"simple coefficients exact");
 f[0]=F(1,1,2,1,-1,2,1);
 require(binomial_expand(f,1,5,&r),"generalised fractional");
 eq(r.coeff[1].value,-1,"fractional x");eq(r.coeff[2].value,1.5,"fractional x2");eq(r.coeff[3].value,-2.5,"fractional x3");
 require(r.coeff[3].is_exact,"generalised coefficients exact");eq(r.convergence,0.5,"generalised radius");
 f[1]=F(1,1,-1,1,2,1,1);
 require(binomial_expand(f,2,3,&r),"product of brackets");
 eq(r.coeff[1].value,-3,"product x");eq(r.coeff[2].value,4.5,"product x2");eq(r.coeff[3].value,-6.5,"product x3");
 f[0]=F(4,1,-1,1,1,2,1);
 require(binomial_expand(f,1,3,&r),"root nonunit constant");
 eq(r.coeff[0].value,2,"root constant");eq(r.coeff[1].value,-0.25,"root x");eq(r.coeff[2].value,-1.0/64,"root x2");
 require(r.coeff[2].is_exact,"perfect-root rational coefficients exact");eq(r.convergence,4,"root radius");
 f[0]=F(1,1,2,1,-1,2,2);
 require(binomial_expand(f,1,4,&r),"power x2");
 eq(r.coeff[1].value,0,"x1 absent");eq(r.coeff[2].value,-1,"x2 term");eq(r.coeff[4].value,1.5,"x4 term");
 f[0]=F(1,1,1,1,4,1,1);f[1]=F(1,1,-2,1,3,1,1);
 require(binomial_expand(f,2,3,&r),"integer binomial product");
 eq(r.coeff[3].value,8,"year 2 coeff x3");
 f[0]=F(0,1,1,1,3,1,1);
 require(binomial_expand(f,1,3,&r),"zero base finite polynomial");
 eq(r.coeff[3].value,1,"x cubed");eq(r.coeff[0].value,0,"zero constant");
 f[0]=F(0,1,1,1,-1,1,1);
 require(!binomial_expand(f,1,3,&r),"zero base negative exponent invalid");
 SolveOutput o;
 double test1[]={20,.4,12,.05,2};
 require(solve_problem(SOL_BINOMIAL_TEST,test1,&o),"binomial hypothesis");
 require(strstr(o.line[o.count-2],"Do not reject")!=NULL,"binomial decision");
 double test2[]={0,2,5};require(solve_problem(SOL_SUVAT,test2,&o),"suvat");
 require(strstr(o.line[2],"25")!=NULL,"suvat displacement");
 double test3[]={5,30,.2,9.8};require(solve_problem(SOL_INCLINE,test3,&o),"incline");
 double test4[]={3,4};require(solve_problem(SOL_RFORM,test4,&o),"r-form");
 double ri[]={8,6,2,0,30};require(revision_solve(TOOL_R_INTERVAL,ri,NULL,&o),"restricted harmonic");
 require(has(&o,"MAX = 11.196")&&has(&o,"MIN = 8 at x=0")&&has(&o,"No full max"),"endpoints determine extrema");
 double r2[]={8,6,2,0,360};require(revision_solve(TOOL_R_INTERVAL,r2,NULL,&o),"full harmonic");
 require(has(&o,"MAX = 12")&&has(&o,"MIN = -8")&&has(&o,"53.130102"),"peaks and troughs");
 double rd[]={-10,36.87,2,0,360};require(revision_solve(TOOL_R_DIRECT,rd,NULL,&o),"negative amplitude");
 require(has(&o,"MIN = -8")&&has(&o,"MAX = 12"),"negative amplitude extrema");
 double ap[]={4,3,13};require(revision_solve(TOOL_AP_KNOWN,ap,NULL,&o),"AP term and sum");require(has(&o,"u_n=40")&&has(&o,"S_n=286"),"AP correct");
 double apn[]={4,3,40};require(revision_solve(TOOL_AP_N,apn,NULL,&o),"AP from term");require(has(&o,"n=13"),"AP from term n");
 double aps[]={4,3,286};require(revision_solve(TOOL_AP_SUM_N,aps,NULL,&o),"AP from sum");require(has(&o,"n=13 valid")&&has(&o,"reject"),"AP quadratic roots checked");
 double gp[]={2,6,4,24,5};require(revision_solve(TOOL_GP_TWO,gp,NULL,&o),"GP from two terms");require(has(&o,"r=2")&&has(&o,"r=-2")&&has(&o,"u_n=48"),"both ratio signs");
 double gpn[]={3,2,48};require(revision_solve(TOOL_GP_N,gpn,NULL,&o),"GP n");require(has(&o,"n=5"),"GP from term");
 double gps[]={3,2,93};require(revision_solve(TOOL_GP_SUM_N,gps,NULL,&o),"GP sum n");require(has(&o,"n=5"),"GP from sum");
 double et[]={100,1.2,0,250};require(revision_solve(TOOL_EXP_TIME,et,NULL,&o),"exp time");require(has(&o,"t=5.025"),"exp time answer");
 double ev[]={500,1.08,0,3};require(revision_solve(TOOL_EXP_VALUE,ev,NULL,&o),"exp evaluate");require(has(&o,"y=629.856"),"exp evaluate answer");
 double ef[]={1,12,3,48,0};require(revision_solve(TOOL_EXP_FIT,ef,NULL,&o),"exp fit");require(has(&o,"y=6*(2)^t"),"exp model constants");
 double ek[]={100,.2,0,250};require(revision_solve(TOOL_EXP_K_TIME,ek,NULL,&o),"continuous exp");require(has(&o,"t=4.58145"),"continuous exp time");
 double eqd[]={1,-5,6,2};require(revision_solve(TOOL_EXP_QUAD,eqd,NULL,&o),"exp quadratic");require(has(&o,"x=1.5849625")&&has(&o,"x=1"),"exp quadratic roots");
 double pf2[]={2,3,1,-1,1,2};exact(&o,TOOL_PF2,pf2,6);require(has(&o,"A = 5/3")&&has(&o,"B = 1/3"),"two factor exact");
 double pf3[]={6,12,-6,1,-1,1,2,1,3};exact(&o,TOOL_PF3,pf3,9);require(has(&o,"A = 1")&&has(&o,"B = 2")&&has(&o,"C = 3"),"three factor exact");
 double pfr[]={1,7,1,1,-1,1,2};exact(&o,TOOL_PFR,pfr,7);require(has(&o,"A = 2")&&has(&o,"B = 3")&&has(&o,"C = -1"),"repeated exact");
 double div[]={3,1,1,-2,-1,2,1,-1};exact(&o,TOOL_DIV,div,8);require(has(&o,"QUOTIENT: x^2 - x - 2")&&has(&o,"REMAINDER: 0"),"polynomial division");
 double div2[]={2,1,1,0,1,2,1};exact(&o,TOOL_DIV,div2,7);require(has(&o,"QUOTIENT: 1/2*x - 1/4")&&has(&o,"REMAINDER: 5/4"),"non-monic division");
 Rational bad[]={ {1,1},{1,1},{1,1},{1,1},{2,1},{2,1} }; require(!revision_solve(TOOL_PF2,pf2,bad,&o),"coincident linear factors rejected");
 printf("PASS %d engine checks\n",checks);return 0;
}
