#include "app_data.h"
#define C(t,m,e,d,s) {t,m,e,d,s}
#define N(a) (int)(sizeof(a)/sizeof(a[0]))
static const Card ap[]={
C("Find a and d","u_n=a+(n-1)d|Write equations for both terms.|Subtract to eliminate a.|Divide to find d.|Substitute to find a.","u4=11, u10=29|a+3d=11; a+9d=29|6d=18, so d=3|a=11-9=2|u_n=2+3(n-1)",0,TOOL_AP),
C("Find nth term / n","u_n=a+(n-1)d|n=1+(u_n-a)/d if d!=0.|n must be a positive integer.","a=4,d=3,last term=40|40=4+(n-1)3|36=3(n-1), n=13",0,TOOL_AP_N),
C("Sum of arithmetic series","S_n=n/2[2a+(n-1)d]|OR S_n=n(a+l)/2.|Find number of terms first.|u_n is one term; S_n is a sum.","a=4,d=3,n=13|last term=40|S_13=13(4+40)/2=286",0,TOOL_AP_KNOWN),
C("Find n from arithmetic sum","Set S_n equal to target.|Expand into a quadratic in n.|Keep positive integer solutions.","a=4,d=3,S_n=286.|286=n(3n+5)/2.|3n^2+5n-572=0.|Positive integer solution n=13.",0,TOOL_AP_SUM_N)};
static const Card proof[]={
C("Deduction 1 - Jun23 P1 Q14","Expand or factorise.|Use facts true for all integers.|Conclude in the required form.|Even=2k, odd=2k+1 (k integer).","9MA0/01 June 2023 Q14.|(n+1)^3-n^3=3n^2+3n+1.|=3n(n+1)+1.|Consecutive integers: one is even.|Thus 3n(n+1) is even.|Adding 1 gives an odd integer.",0,0),
C("Deduction 2 - Nov21 AS Q10a","For odd n, write n=2k+1.|Factor; show multiple of 4.|Explicitly state k is integer.","8MA0/01 Nov 2021 Q10(a).|n^3-n=n(n-1)(n+1).|=(2k+1)(2k)(2k+2).|=4k(k+1)(2k+1).|The remaining factor is integer.|Hence divisible by 4.",0,0),
C("Contradiction 1 - Nov21 Q15ii","Assume opposite of conclusion.|Use the given premise too.|Reach contradiction, reject assumption.","9MA0/01 Nov 2021 Q15(ii).|Given m^3+5 is odd. Assume m odd.|m=2k+1.|m^3+5=8k^3+12k^2+6k+6.|=2(4k^3+6k^2+3k+3), even.|Contradicts odd premise.|Therefore m is even.",0,0),
C("Contradiction 2 - Jun19 Q10i","Assume the claimed impossibility|actually happens. Split into parity|cases. Show neither case works.","9MA0/01 June 2019 Q10(i).|Assume 4 divides n^2+2.|If n=2k: n^2+2=4k^2+2.|If n=2k+1: n^2+2=4k(k+1)+3.|Remainders 2 or 3, never 0.|Contradiction; not divisible by 4.",0,0),
C("Exhaustion 1 - Nov21 Q15i","List every permitted case.|Calculate both sides in each case.|State that all cases satisfy claim.","9MA0/01 Nov 2021 Q15(i).|Compare (n+1)^3 and 3^n, n=1..4.|n=1: 8>3.|n=2: 27>9.|n=3: 64>27.|n=4: 125>81.|All allowed cases work.",0,0),
C("Exhaustion 2 - Jun19 Q10i","Alternative solution: exhaust all|possible remainders modulo 4.|A number is divisible by 4 iff|its remainder is zero.","9MA0/01 June 2019 Q10(i).|n mod 4 is 0,1,2,3.|n^2+2 mod 4: 2,3,2,3.|None is 0.|These cover every integer.|Thus 4 cannot divide n^2+2.",0,0),
C("Counterexample 1 - Jun19 Q10ii","A universal claim needs only one|valid input where it fails.|For sometimes true, show a true|case AND a false case.","9MA0/01 June 2019 Q10(ii).|Compare abs(3x-28)>x-9.|x=9.4: 0.2>0.4 is false.|x=12: 8>3 is true.|So the claim is sometimes true.",0,0),
C("Counterexample 2 - Nov21 AS Q10b","Choose a simple value allowed by|the claim. Calculate and show it|violates the conclusion.","8MA0/01 Nov 2021 Q10(b).|Does n^3-n always divide by 4?|Take n=2: 8-2=6.|6 is not a multiple of 4.|So not true for every integer.",0,0)};
static const Card integ[]={
C("Definite integral","Find antiderivative F(x).|Substitute upper limit b.|Subtract lower value a.|Integral_a^b f(x)dx=F(b)-F(a).|No +C in definite answer.","Integrate 3x^2 from 1 to 3.|F=x^3|F(3)-F(1)=27-1=26",DIAG_AREA,0),
C("Area under / between curves","Find intersections or roots.|Split at sign changes.|Area under curve=integral of abs(y).|Between curves: top minus bottom.|Area must be non-negative.","y=2x and y=x^2.|2x=x^2 -> x=0,2.|Integrate 2x-x^2 from 0 to 2.|[x^2-x^3/3] from 0 to 2=4/3",DIAG_AREA,0),
C("Find constant of integration","Indefinite integral has +C.|Substitute known (x,y).|Solve for C; write full equation.","dy/dx=6x+2, y(1)=7.|y=3x^2+2x+C.|7=3+2+C, so C=2.|y=3x^2+2x+2.",0,0)};
static const Card howint[]={
C("Power rule","Add 1 to power, divide by new power.|Keep coefficient. Add +C.|Valid for n!=-1.|Year 1 power-rule methods only.","Integrate 6x^2+4x-3.|=6x^3/3+4x^2/2-3x+C|=2x^3+2x^2-3x+C.",0,0),
C("Fractions and roots","sqrt(x)=x^(1/2).|1/x^m=x^(-m).|Rewrite then integrate each term.|n=-1 outside this Year 1 method.","Integrate 4sqrt(x)+6/x^2.|=integral [4x^(1/2)+6x^(-2)]dx|=(8/3)x^(3/2)-6x^(-1)+C.",0,0),
C("Expand before integrating","Expand brackets or divide terms|by common monomial first.|Then integrate each term.|Never integrate product factorwise.","Integrate x(x+3).|Expand x^2+3x.|Answer x^3/3+3x^2/2+C.",0,0)};
static const Card model[]={
C("Rates of change","Derivative=instantaneous rate.|Integral of rate=total change.|Write variable meanings and units.","V(t)=t^2+3t+5 litres.|V'=2t+3 litres/min.|At t=4 rate=11 L/min.|V(4)-V(0)=28 litres.",0,0),
C("Find optimal value","Differentiate model, set f'=0.|Solve, classify using sign or f''.|Check domain and endpoints.|Give answer in context.","A=12x-x^2, 0<=x<=12.|A'=12-2x=0 -> x=6.|A''=-2, maximum.|A(6)=36; endpoints give 0.",DIAG_TURN,0),
C("Recover quantity from rate","Integrate rate function.|Use starting value to find C.|For change use definite integral.","Q'=4t+2, Q(0)=5.|Q=2t^2+2t+5.|Q(3)=29; change=24.",0,0)};
static const Card mod[]={
C("Sketch y=|f(x)|","Sketch original y=f(x).|Keep parts above x-axis.|Reflect parts below x-axis upwards.|Roots stay in place; range y>=0.","f=x-2.|x>=2: y=x-2; x<2: y=2-x.|V vertex (2,0); intercept (0,2).",DIAG_MOD,0),
C("Sketch y=f(|x|)","Keep original right half x>=0.|Reflect it in y-axis.|Discard original left half.|Symmetric about y-axis.","f=x+1.|f(abs(x))=abs(x)+1.|Vertex (0,1), range y>=1.",0,0),
C("Solve modulus equations","If abs(A)=k, k>=0: A=k or A=-k.|k<0: no solutions.|If RHS variable, require RHS>=0.|Check candidates in original.","abs(2x-3)=5.|2x-3=5 -> x=4.|2x-3=-5 -> x=-1.",0,0),
C("Modulus inequalities","For k>0, abs(A)<k means -k<A<k.|For abs(A)>k, A>k OR A<-k.|Adjust endpoints for <= or >=.","abs(2x-3)<5.|-5<2x-3<5.|-2<2x<8.|-1<x<4.",0,0)};
static const Card exp[]={
C("Growth / decay","Growth y=A(1+r)^t.|Decay y=A(1-r)^t.|Continuous y=Ae^(kt).|A initial value, k sign gives trend.|Percent to decimal.","500 grows 8% per year.|y=500(1.08)^t.|After 3 years: 629.856.",0,TOOL_EXP_VALUE),
C("Find time using logs","Divide by initial value A.|Take ln both sides.|t=ln(y/A)/ln(b) for Ab^t.|For Ae^(kt), t=ln(y/A)/k.","100(1.2)^t=250.|1.2^t=2.5.|t=ln(2.5)/ln(1.2)=5.0257.|First whole year t=6 for above target.",0,TOOL_EXP_TIME),
C("Find model constants","For y=Ab^t, divide two equations|to eliminate A.|b^(t2-t1)=y2/y1.|Find b then substitute for A.","y(1)=12, y(3)=48.|Ab=12; Ab^3=48.|b^2=4 -> b=2 (positive base).|A=6; y=6*2^t.",0,TOOL_EXP_FIT),
C("Log laws / equations","log(ab)=log a+log b.|log(a/b)=log a-log b.|log(a^k)=k log a.|Combine, solve, check arguments >0.","log_2(x)+log_2(x-2)=3.|x(x-2)=8 -> x=4,-2.|Reject -2; domain x>2.",0,0),
C("Continuous exponential time","For y=Ae^(kt)+c, subtract c.|Divide by A; take ln.|t=ln((y-c)/A)/k.","100e^(0.2t)=250.|e^(0.2t)=2.5.|0.2t=ln(2.5).|t=4.58145.",0,TOOL_EXP_K_TIME),
C("Quadratic exponential equation","Let u=b^x, hence u>0.|Solve quadratic in u.|Discard u<=0.|x=ln(u)/ln(b).","2^(2x)-5*2^x+6=0.|u^2-5u+6=0.|u=2 or 3.|x=1 or ln(3)/ln(2).",0,TOOL_EXP_QUAD)};
static const Card trig[]={
C("Exact values and CAST","Angles: 0,30,45,60,90 deg.|sin: 0,1/2,sqrt2/2,sqrt3/2,1|cos: 1,sqrt3/2,sqrt2/2,1/2,0|tan=sin/cos; tan90 undefined.|CAST: Q1 all,Q2 sin,Q3 tan,Q4 cos.","sin150=sin30=1/2.|cos120=-cos60=-1/2.|tan225=tan45=1.|Check calculator degree mode.",DIAG_CAST,0),
C("Trig equations / all solutions","Isolate sin, cos or tan.|Find reference angle with inverse.|Use CAST to choose quadrants.|sin/cos period 360; tan period 180.|Keep every root in interval.","2sin x=1, 0<=x<360.|Reference angle=30 deg.|Q1,Q2 positive.|x=30,150 degrees.",DIAG_CAST,0),
C("Trig identities","sin^2 x+cos^2 x=1.|tan x=sin x/cos x.|sec x=1/cos x; cosec x=1/sin x.|cot x=cos x/sin x.|1+tan^2 x=sec^2 x.|1+cot^2 x=cosec^2 x.","Prove (1-cos^2 x)/sin x=sin x.|=sin^2 x/sin x=sin x.|Where defined.",0,0),
C("Double angle / addition","sin(A+B)=sin A cos B+cos A sin B.|cos(A+B)=cos A cos B-sin A sin B.|For A-B reverse second-term sign.|sin 2x=2sin x cos x.|cos 2x=1-2sin^2 x=2cos^2 x-1.|tan 2x=2tan x/(1-tan^2 x).","sin 2x=sin x, 0<=x<360.|sin x(2cos x-1)=0.|sin x=0 OR cos x=1/2.|x=0,60,180,300.",0,0),
C("Trig identity proof","Start with harder side.|Convert sec/cosec/tan to sin/cos.|Combine fractions, factorise.|Apply sin^2+cos^2=1.|Do not assume identity.","sec^2 x-tan^2 x|=(1-sin^2 x)/cos^2 x|=cos^2 x/cos^2 x=1.",0,0),
C("Harmonic form R sin(x+alpha)","a sin x+b cos x=R sin(x+alpha).|R=sqrt(a^2+b^2).|R cos alpha=a; R sin alpha=b.|tan alpha=b/a; check quadrant.|R>0. Tool handles quadrant.","5sin x+12cos x.|R=sqrt(25+144)=13.|alpha=atan(12/5)=67.38 deg.|=13sin(x+67.38).",0,TOOL_R),
C("Harmonic max/min and angle","y=a sin x+b cos x+c.|Convert to c+R sin(x+alpha).|Max=c+R; min=c-R.|Max x=90-alpha+360k.|Min x=270-alpha+360k.|On closed intervals check endpoints.","y=8sin x+6cos x+2.|R=10, alpha=36.87 deg.|On 0<=x<=90, max=12 at 53.13.|Min=8 at x=0.",0,TOOL_R_INTERVAL),
C("Harmonic equations","Rewrite in R form; divide by R.|Solve angle x+alpha using CAST.|Subtract alpha.|Apply period; check interval.","10sin(x+36.87)=5.|x+36.87=30 or 150 (+360k).|x=-6.87 or 113.13 (+360k).|0<=x<360:113.13,353.13.",DIAG_CAST,0),
C("Radians, sectors, triangles","Radians=degrees*pi/180.|Arc=r*theta; area=r^2*theta/2.|Sine rule a/sin A=b/sin B.|Cos rule a^2=b^2+c^2-2bc cos A.|Triangle area=ab sin C/2.","r=6, theta=pi/3 rad.|Arc=2pi.|Sector area=6pi.|Use radians for both.",0,0),
C("Max/min given R*sin(x+alpha)","For R>=0, max=c+R, min=c-R.|If R is negative, change sign and|add 180 degrees to alpha first.|Check restricted interval endpoints.","y=10sin(x+36.87)+2.|Max=12; min=-8.|Max at 53.13+360k degrees.|Min at 233.13+360k degrees.",0,TOOL_R_DIRECT)};
static const Card curves[]={
C("Two curve intersections","Set y expressions equal.|Rearrange into equation in x.|Use fx-991EX equation solver.|Substitute each x for y.|Give coordinate pairs.","y=x^2, y=x+2.|(x-2)(x+1)=0.|x=2,-1.|Points (2,4),(-1,1).",DIAG_INTERSECT,0),
C("Line-circle intersections","Circle (x-a)^2+(y-b)^2=r^2.|Substitute line into circle.|Solve quadratic.|D=b^2-4ac: >0 two roots,|=0 tangent, <0 none.","x^2+y^2=25, y=3.|x^2+9=25 -> x=+/-4.|Points (-4,3),(4,3).",0,0),
C("Tangent to circle / line","m=(y2-y1)/(x2-x1).|Line y-y1=m(x-x1).|Radius perpendicular to tangent.|m_t=-1/m_r if defined.","Centre (0,0), point (3,4).|Radius gradient=4/3.|Tangent gradient=-3/4.|y-4=-(3/4)(x-3).",0,0)};
static const Card diff[]={
C("Differentiate powers","d(x^n)/dx=n*x^(n-1).|Multiply by old power; subtract 1.|Constants differentiate to zero.|Rewrite roots and fractions.|Year 1: expand products first.","y=3x^4-2x^2+7.|dy/dx=12x^3-4x.|y=4sqrt(x) -> dy/dx=2x^-1/2.",0,0),
C("Stationary points / classification","Differentiate; solve f'=0.|Find y by substitution.|f''>0 min; f''<0 max.|If f''=0, use sign test.|+ to - max, - to + min.","f=x^3-3x.|f'=3x^2-3 -> x=+/-1.|f''=6x.|(-1,2) max; (1,-2) min.",DIAG_TURN,0),
C("Tangent and normal","Find y0=f(x0), m=f'(x0).|Tangent y-y0=m(x-x0).|Normal gradient=-1/m.|If m=0, normal is x=x0.","y=x^2, x0=2.|y0=4, m=4.|Tangent y-4=4(x-2).|Normal y-4=-(x-2)/4.",DIAG_TURN,0),
C("Increasing / decreasing","Increasing where f'>0.|Decreasing where f'<0.|Find f' roots then sign intervals.","f=x^2-4x; f'=2x-4.|Decreasing x<2.|Increasing x>2.",DIAG_TURN,0)};
static const Card funcs[]={
C("Domain and range","Domain=allowed x inputs.|Range=possible y outputs.|Denominator !=0; sqrt input >=0;|log argument >0.|Check given restrictions.","f=1/(x-3).|Domain: real x except 3.|Range: real y except 0.",0,0),
C("Range using turning point","Complete square (x-h)^2+k.|Minimum=k; range y>=k.|k-(x-h)^2: max=k; range y<=k.|Restricted domain: check endpoints.|Include equality if reached.","f=x^2+8x+10=(x+4)^2-6.|Vertex (-4,-6).|Domain all real, range y>=-6.",DIAG_RANGE,0),
C("Composite functions","fg(x)=f(g(x)).|Do inside function first.|Substitute whole expression.|Check both domains.","f=2x+1, g=x^2.|fg=2x^2+1; gf=(2x+1)^2.|fg(3)=19.",0,0),
C("Inverse functions","Write y=f(x); swap x and y.|Rearrange for y.|Swap domain/range.|Original must be one-to-one.","f(x)=3x-4.|x=3y-4 -> y=(x+4)/3.|f^(-1)(x)=(x+4)/3.",0,0),
C("Transformations","f(x)+a: up a; f(x+a): left a.|af(x): vertical scale a.|f(ax): horizontal scale 1/a (a>0).|-f(x): x-axis reflection.|f(-x): y-axis reflection.","f=x^2.|f(x-3)+2=(x-3)^2+2.|Vertex moves to (3,2).",DIAG_RANGE,0)};
static const Card vec[]={
C("Position and magnitude","AB=b-a (from A to B).|Magnitude=sqrt(x^2+y^2).|Unit vector=v/magnitude(v).","A=(2,1),B=(5,5).|AB=(3,4), magnitude=5.|Unit vector=(3/5,4/5).",DIAG_VECTOR,0),
C("Parallel / collinear","Parallel vectors are scalar multiples.|Find AB and AC.|If AC=kAB, points collinear.","A=(1,2),B=(3,4),C=(7,8).|AB=(2,2), AC=(6,6).|AC=3AB; points collinear.",DIAG_VECTOR,0),
C("Midpoint / ratio proof","Midpoint=(a+b)/2.|If AP:PB=m:n then|p=(n*a+m*b)/(m+n).","A=(0,0),B=(6,9), AP:PB=1:2.|p=(2A+B)/3=(2,3).",DIAG_VECTOR,0)};
static const Card seq[]={
C("Geometric term and sum","u_n=a*r^(n-1).|S_n=a(1-r^n)/(1-r), r!=1.|r=1: S_n=na.|S_inf=a/(1-r), only abs(r)<1.","a=12,r=1/2.|u4=1.5; S4=22.5.|S_inf=24.",0,TOOL_GP),
C("Find a and r","Write two u_n equations.|Divide to eliminate a.|r^(j-i)=u_j/u_i.|Even powers can give +/- roots.|Substitute back and check.","u2=6,u4=24.|ar=6; ar^3=24.|r^2=4 -> r=+/-2.|r=2 -> a=3; r=-2 -> a=-3.",0,TOOL_GP_TWO),
C("Sigma / recurrence","SUM r=1..n of 1=n.|SUM r=n(n+1)/2.|SUM r^2=n(n+1)(2n+1)/6.|Split into separate sums.|Recurrence: use preceding term.","SUM(4r-3), r=1..n.|=4[n(n+1)/2]-3n|=2n^2-n; n=5 -> 45.",0,0),
C("Binomial expansion Y1","(a+bx)^n, integer n>=0.|Coeff x^k=nCk*a^(n-k)*b^k.|k=0 through n.|Tool calculates all terms.","(2+x)^3.|=8+12x+6x^2+x^3.|Coefficient x^2=6.",0,TOOL_BIN),
C("Binomial expansion Y2","Factor a: a^n(1+(b/a)x)^n.|(1+u)^n=1+nu+n(n-1)u^2/2!+...|Infinite series: abs(u)<1.|Multiply expansions, collect terms.|Intersect validity conditions.","(1+2x)^(-1/2)|=1-x+(3/2)x^2-(5/2)x^3+...|Valid abs(x)<1/2.|Times (1-x)^2 through x^3:|1-3x+(9/2)x^2-(13/2)x^3.",0,TOOL_BIN),
C("GP: find n from a term","For r>0 and r!=1, divide by a.|Take logs to solve n-1.|Keep positive integer n.","a=3,r=2,u_n=48.|2^(n-1)=16.|n=1+ln(16)/ln(2)=5.",0,TOOL_GP_N),
C("GP: find n from a sum","Use S_n=a(1-r^n)/(1-r).|Rearrange for r^n.|For positive r use logs.|Check n is a positive integer.","a=3,r=2,S_n=93.|2^n=32.|n=ln(32)/ln(2)=5.",0,TOOL_GP_SUM_N)};
static const Card alg[]={
C("Factor / remainder theorem","Remainder dividing by (x-a)=f(a).|f(a)=0 -> factor (x-a).|Test simple roots; divide polynomial.|Factor remaining quadratic.","f=x^3-2x^2-x+2.|f(1)=0; divide by (x-1).|Quotient x^2-x-2=(x-2)(x+1).|f=(x-1)(x-2)(x+1).",0,TOOL_DIV),
C("Complete the square","x^2+bx+c=(x+b/2)^2+c-b^2/4.|Leading coefficient !=1?|Factor it first.","2x^2+8x+3|=2(x^2+4x)+3|=2(x+2)^2-5.",DIAG_RANGE,0),
C("Quadratic inequalities","Find roots (fx-991EX).|Sketch parabola, check sign.|Choose intervals satisfying inequality.|Include endpoints only if allowed.","x^2-x-6<0.|(x-3)(x+2)<0.|Upward parabola below zero|between roots: -2<x<3.",0,0),
C("Partial fractions: two factors","Factor denominator.|Distinct: A/(px+q)+B/(rx+s).|Multiply by denominator.|Substitute roots or compare coeffs.|Divide first if numerator improper.","(2x+3)/[(x-1)(x+2)].|2x+3=A(x+2)+B(x-1).|x=1: A=5/3; x=-2: B=1/3.|=5/[3(x-1)]+1/[3(x+2)].",0,TOOL_PF2),
C("Indices and surds","x^a*x^b=x^(a+b); division subtract.|(x^a)^b=x^(ab).|x^-a=1/x^a; x^(1/n)=nth root.|Rationalise using conjugate.","3/(2+sqrt(3)).|Multiply by (2-sqrt(3)) top/bottom.|Denominator=4-3=1.|Answer=6-3sqrt(3).",0,0),
C("Partial fractions: 3 factors","For distinct L,M,N, use A/L+B/M+C/N.|Multiply by LMN.|Compare coefficients or substitute|a root of each factor.","(6x^2+12x-6)/[(x-1)(x+2)(x+3)].|A=1, B=2, C=3.|1/(x-1)+2/(x+2)+3/(x+3).",0,TOOL_PF3),
C("Partial fractions: repeated","For L^2*M, use A/L+B/L^2+C/M.|Multiply by L^2*M.|Compare coefficients of each power.","(x^2+7x+1)/[(x-1)^2(x+2)].|A=2, B=3, C=-1.|2/(x-1)+3/(x-1)^2-1/(x+2).",0,TOOL_PFR),
C("Polynomial long division","Divide leading terms.|Multiply divisor by that term.|Subtract from dividend.|Repeat until remainder degree|is less than divisor degree.","(x^3-2x^2-x+2)/(x-1).|First term x^2, subtract x^3-x^2.|Next term -x, then -2.|Quotient x^2-x-2; remainder 0.",0,TOOL_DIV)};
static const Card formula[]={
C("Y2 trig: double angles","sin(2x)=2sin x cos x.|cos(2x)=cos^2 x-sin^2 x.|=1-2sin^2 x=2cos^2 x-1.|tan(2x)=2tan x/(1-tan^2 x).|Compound angles ARE in booklet.","cos(2x)=0.5 -> sin^2 x=0.25.|sin x=+/-0.5; use CAST.",0,0),
C("Y2 trig: reciprocal identities","sec x=1/cos x.|cosec x=1/sin x.|cot x=1/tan x=cos x/sin x.|sec^2 x=1+tan^2 x.|cosec^2 x=1+cot^2 x.","tan x=3 -> sec^2 x=10.|sec x=+/-sqrt10, check quadrant.",0,0),
C("Y2 trig: R form / extremes","a sin x+b cos x=R sin(x+alpha).|R=sqrt(a^2+b^2).|R cos alpha=a; R sin alpha=b.|Max/min with offset c: c +/- R.|Check angles and interval endpoints.","8sin x+6cos x+2.|R=10; alpha=36.87 deg.|Global max=12, min=-8.",0,TOOL_R_INTERVAL),
C("Y1 trig: basic identities","sin^2 x+cos^2 x=1.|tan x=sin x/cos x.|sin(-x)=-sin x.|cos(-x)=cos x.|tan(-x)=-tan x.","sin x=3/5 gives cos x=+/-4/5.|Choose sign from quadrant.",DIAG_CAST,0),
C("Y1 trig: exact values","Angles: 0,30,45,60,90 degrees.|sin: 0,1/2,sqrt2/2,sqrt3/2,1.|cos: 1,sqrt3/2,sqrt2/2,1/2,0.|tan: 0,1/sqrt3,1,sqrt3,undefined.","sin 150=1/2; cos 150=-sqrt3/2.|tan 150=-1/sqrt3.",DIAG_CAST,0),
C("Trig: triangles / radians","Sine rule a/sin A=b/sin B.|Cos rule a^2=b^2+c^2-2bc cos A.|Area=ab sin C/2.|Arc=r*theta, sector=r^2*theta/2.|theta radians; pi rad=180 deg.","r=4, theta=pi/2.|Arc=2pi; sector=4pi.",0,0),
C("Sequences: nth terms","AP: u_n=a+(n-1)d.|GP: u_n=a*r^(n-1).|AP: d=u_(n+1)-u_n.|GP: r=u_(n+1)/u_n.|Sum formulas ARE in booklet.","a=4,d=3: u13=40.|a=3,r=2: u5=48.",0,0),
C("Indices / log laws","a^m*a^n=a^(m+n).|a^m/a^n=a^(m-n).|(a^m)^n=a^(mn).|log(ab)=log a+log b.|log(a/b)=log a-log b.|log(a^n)=n log a.","2^x=7 -> x=ln(7)/ln(2).|Log arguments must be positive.",0,0),
C("Exponential models","y=A*b^t+c or A*e^(kt)+c.|t=ln((y-c)/A)/ln(b).|Continuous: t=ln((y-c)/A)/k.|Growth b=1+r; decay b=1-r.","500 grows 8 percent yearly.|A=500, b=1.08.",0,TOOL_EXP_TIME),
C("Coordinate geometry","m=(y2-y1)/(x2-x1).|y-y1=m(x-x1).|Perpendicular gradients product=-1.|Circle: (x-a)^2+(y-b)^2=r^2.|Distance=sqrt(dx^2+dy^2).","Centre (2,3), radius 4.|(x-2)^2+(y-3)^2=16.",0,0),
C("Quadratics / algebra","x=(-b+/-sqrt(b^2-4ac))/(2a).|D=b^2-4ac.|D>0: two roots; D=0: repeated.|x^2+bx+c=(x+b/2)^2+c-b^2/4.|Factor theorem: f(a)=0 iff x-a factor.","x^2+8x+10=(x+4)^2-6.|Minimum -6, x=-4.",DIAG_RANGE,0),
C("Y1 differentiation","d(x^n)/dx=n*x^(n-1).|Tangent gradient=f'(x).|Normal gradient=-1/f'(x).|Stationary: f'=0.|f''>0 min, f''<0 max.","y=x^3-3x; f'=3x^2-3.|Stationary x=+/-1.",0,0),
C("Y1 integration","Integral x^n dx=x^(n+1)/(n+1)+C.|n!=-1.|Definite: F(b)-F(a).|Area between curves: upper-lower.|Split at roots if finding area.","Integral 3x^2 dx=x^3+C.|From 1 to 3: 26.",DIAG_AREA,0),
C("Vectors and functions","AB=b-a; length=sqrt(dx^2+dy^2).|Unit vector=v/magnitude(v).|Midpoint=(a+b)/2.|fg(x)=f(g(x)).|Inverse: swap x and y; solve y.","A=(2,1), B=(5,5).|AB=(3,4), length=5.",DIAG_VECTOR,0),
C("Already IN Pearson booklet","AP and GP sum formulas.|General binomial expansions.|Compound-angle trig identities.|Small-angle approximations.|Sum-to-product trig identities.|Log change of base; a^x=e^(x ln a).|Differentiation from first principles.","Derive double-angle from compound|angle by setting A=B=x.|Nth terms and reciprocal trig|identities are not directly printed.",0,0)};
static const Card quick[]={
C("Harmonic: a sin x+b cos x","Enter a and b to find R and alpha.","5sin x+12cos x: R=13.",0,TOOL_R),
C("Harmonic: max/min interval","Enter a,b,c and a closed degree interval.","8sin x+6cos x+2 on [0,90].",0,TOOL_R_INTERVAL),
C("Harmonic: given R and alpha","Enter R, angle, offset and interval.","10sin(x+36.87)+2.",0,TOOL_R_DIRECT),
C("AP: find a and d","Enter two term indices and values.","u4=11, u10=29.",0,TOOL_AP),
C("AP: term and sum","Enter a,d,n.","a=4,d=3,n=13.",0,TOOL_AP_KNOWN),
C("AP: find n from term","Enter a,d,target term.","a=4,d=3,u_n=40.",0,TOOL_AP_N),
C("AP: find n from sum","Enter a,d,target sum.","a=4,d=3,S_n=286.",0,TOOL_AP_SUM_N),
C("GP: term / sums","Enter a,r,n.","a=12,r=1/2,n=4.",0,TOOL_GP),
C("GP: find a and r","Enter two nonzero terms.","u2=6,u4=24.",0,TOOL_GP_TWO),
C("GP: find n from term","Positive ratio; enter a,r,u_n.","a=3,r=2,u_n=48.",0,TOOL_GP_N),
C("GP: find n from sum","Positive ratio; enter a,r,S_n.","a=3,r=2,S_n=93.",0,TOOL_GP_SUM_N),
C("Exp: find time (base b)","Solve A*b^t+c=target.","100*1.2^t=250.",0,TOOL_EXP_TIME),
C("Exp: find time (base e)","Solve A*e^(kt)+c=target.","100e^(0.2t)=250.",0,TOOL_EXP_K_TIME),
C("Exp: fit model from points","Fit A*b^t+c given c and two points.","y(1)=12,y(3)=48,c=0.",0,TOOL_EXP_FIT),
C("Exp: evaluate model","Evaluate A*b^t+c.","500*1.08^3.",0,TOOL_EXP_VALUE),
C("Exp: quadratic in b^x","Let u=b^x; solve quadratic.","2^(2x)-5*2^x+6=0.",0,TOOL_EXP_QUAD),
C("Partial fractions: 2 linear","Proper fraction; two distinct linear factors.","(2x+3)/[(x-1)(x+2)].",0,TOOL_PF2),
C("Partial fractions: 3 linear","Proper fraction; three distinct linear factors.","(6x^2+12x-6)/[(x-1)(x+2)(x+3)].",0,TOOL_PF3),
C("Partial fractions: repeated","Proper fraction L^2*M, distinct L,M.","(x^2+7x+1)/[(x-1)^2(x+2)].",0,TOOL_PFR),
C("Polynomial long division","Degree <=5 numerator, 1..3 divisor.","(x^3-2x^2-x+2)/(x-1).",0,TOOL_DIV),
C("Binomial expansion Y1 / Y2","Up to 4 brackets; fractional powers.|Enter coefficients and powers separately.","(1+2x)^(-1/2) times (1-x)^2.",0,TOOL_BIN)};
const Topic revision_topics[REVISION_TOPIC_COUNT]={
{"Arithmetic sequences",ap,N(ap)},{"Proofs",proof,N(proof)},
{"Integration",integ,N(integ)},{"How to integrate",howint,N(howint)},
{"Calculus modelling",model,N(model)},{"Modulus graphs",mod,N(mod)},
{"Exponential modelling",exp,N(exp)},{"Trigonometry Y1 + Y2",trig,N(trig)},
{"Curves and intersections",curves,N(curves)},{"Differentiation",diff,N(diff)},
{"Functions / domain / range",funcs,N(funcs)},{"Vectors",vec,N(vec)},
{"Sequences and series",seq,N(seq)},{"Algebraic methods",alg,N(alg)},
{"Missing formula booklet",formula,N(formula)},{"All calculators",quick,N(quick)}};
