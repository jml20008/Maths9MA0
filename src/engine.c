#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>

#define EPS 1.0e-10
#define PI 3.14159265358979323846

static unsigned long long uabsll(long long n) { return n<0 ? (unsigned long long)(-(n+1))+1ULL : (unsigned long long)n; }
static unsigned long long ugcd(unsigned long long a, unsigned long long b) {
    while(b) { unsigned long long t=a%b; a=b; b=t; } return a?a:1ULL;
}
Rational rat(long long n, long long d) {
    Rational r = {n,d};
    if(d==0) return r;
    if(d<0) { if(d==LLONG_MIN || n==LLONG_MIN) return (Rational){0,0}; n=-n; d=-d; }
    unsigned long long g=ugcd(uabsll(n),(unsigned long long)d);
    return (Rational){n/(long long)g,d/(long long)g};
}
int rational_ok(Rational r){return r.den>0;}
static double asdouble(Rational r){return r.den ? (double)r.num/(double)r.den : NAN;}
/* All exact multiplication is overflow-checked. Fallback uses double. */
static int safe_mult(long long a,long long b,long long *out){
    if(a==0 || b==0){*out=0;return 1;}
    unsigned long long ua=uabsll(a),ub=uabsll(b);
    unsigned long long lim=((a<0) != (b<0)) ? (1ULL<<63) : LLONG_MAX;
    if(ua>lim/ub)return 0;
    if(ua*ub==(1ULL<<63)) { *out=LLONG_MIN;return 1; }
    *out=a*b;return 1;
}
static int qmul(Rational a,Rational b,Rational *out){
    if(!rational_ok(a)||!rational_ok(b))return 0;
    unsigned long long g1=ugcd(uabsll(a.num),(unsigned long long)b.den);
    unsigned long long g2=ugcd(uabsll(b.num),(unsigned long long)a.den);
    long long n,d;
    if(!safe_mult(a.num/(long long)g1,b.num/(long long)g2,&n))return 0;
    if(!safe_mult(a.den/(long long)g2,b.den/(long long)g1,&d))return 0;
    *out=rat(n,d);return rational_ok(*out);
}
static int qadd(Rational a,Rational b,Rational *out){
    if(!rational_ok(a)||!rational_ok(b))return 0;
    unsigned long long g=ugcd((unsigned long long)a.den,(unsigned long long)b.den);
    long long an,bn,d;
    if(!safe_mult(a.num,b.den/(long long)g,&an))return 0;
    if(!safe_mult(b.num,a.den/(long long)g,&bn))return 0;
    if((bn>0 && an>LLONG_MAX-bn)||(bn<0 && an<LLONG_MIN-bn))return 0;
    if(!safe_mult(a.den/(long long)g,b.den,&d))return 0;
    *out=rat(an+bn,d);return rational_ok(*out);
}
static int qdiv(Rational a,Rational b,Rational *out){
    if(b.num==0)return 0;
    Rational rec=rat(b.den,b.num);
    return qmul(a,rec,out);
}
static int qpow(Rational a,int p,Rational *out){
    Rational result=rat(1,1),base=a;
    if(p<0){if(!a.num)return 0;base=rat(a.den,a.num);p=-p;}
    while(p){if(p&1){if(!qmul(result,base,&result))return 0;}
        p>>=1;if(p && !qmul(base,base,&base))return 0;}
    *out=result;return 1;
}
static int root_exact(long long val,int q,long long *root){
    if(q<1||q>24 || (val<0 && q%2==0))return 0;
    double x=pow((double)uabsll(val),1.0/q);
    if(!isfinite(x)||x>1000000000.0)return 0;
    long long k=(long long)llround(x);
    for(long long z=(k>1?k-1:k);z<=k+1;z++){
        long long product=1;int ok=1;
        for(int j=0;j<q;j++)if(!safe_mult(product,z,&product)){ok=0;break;}
        if(ok && (unsigned long long)product==uabsll(val)){
            *root=(val<0?-z:z);return 1;
        }
    }
    return 0;
}
static int qpow_rational(Rational a,Rational exponent,Rational *out){
    if(exponent.den==1){if(uabsll(exponent.num)>30)return 0;return qpow(a,(int)exponent.num,out);}
    if(a.num<0 || exponent.den>24)return 0;
    long long rn,rd;
    if(!root_exact(a.num,(int)exponent.den,&rn) ||
       !root_exact(a.den,(int)exponent.den,&rd)) return 0;
    if(uabsll(exponent.num)>30)return 0;
    return qpow(rat(rn,rd),(int)exponent.num,out);
}
static Coeff make_coeff(double val,Rational exact,int is_exact){
    Coeff c={val,exact,is_exact};return c;
}
static Coeff cmul(Coeff a,Coeff b){
    Rational q;int ok=a.is_exact&&b.is_exact&&qmul(a.exact,b.exact,&q);
    return make_coeff(a.value*b.value,ok?q:rat(0,1),ok);
}
static Coeff cadd(Coeff a,Coeff b){
    Rational q;int ok=a.is_exact&&b.is_exact&&qadd(a.exact,b.exact,&q);
    return make_coeff(a.value+b.value,ok?q:rat(0,1),ok);
}
static int factor_terms(BinFactor f,int deg,Coeff *poly,double *radius,int *finite,char *error){
    if(!rational_ok(f.a)||!rational_ok(f.b)||!rational_ok(f.exponent)||f.xpower<1||f.xpower>8){
        strcpy(error,"Invalid fraction or x power (1..8)");return 0;
    }
    double a=asdouble(f.a),b=asdouble(f.b),n=asdouble(f.exponent);
    if(f.exponent.den>24 || fabs(n)>40 || fabs(a)>1e6 || fabs(b)>1e6){
        strcpy(error,"Input outside supported range");return 0;
    }
    *finite=(f.exponent.den==1 && f.exponent.num>=0);
    if(!*finite && a==0){strcpy(error,"Series needs nonzero constant a");return 0;}
    if(f.exponent.den!=1 && a<0){strcpy(error,"Fractional exponent requires a>0");return 0;}
    *radius=INFINITY;
    if(!*finite && b!=0)*radius=pow(fabs(a/b),1.0/f.xpower);
    Coeff zero=make_coeff(0,rat(0,1),1);
    for(int i=0;i<=deg;i++)poly[i]=zero;
    if(*finite){
        int p=(int)f.exponent.num;
        double c=1.0;
        for(int k=0;k<=p && k*f.xpower<=deg;k++){
            if(k)c=c*(p-k+1)/k;
            double v=c*pow(a,p-k)*pow(b,k);
            Rational qa,qb,qc,part,cc=rat(1,1);
            int exact=qpow(f.a,p-k,&qa)&&qpow(f.b,k,&qb)&&qmul(qa,qb,&part);
            if(exact){ /* combinatorial coefficient, checked in rational arithmetic */
                for(int j=1;j<=k;j++){
                    Rational mul=rat(p-j+1,j);
                    if(!qmul(cc,mul,&cc)){exact=0;break;}
                }
                if(exact)exact=qmul(cc,part,&qc);
            }
            poly[k*f.xpower]=make_coeff(v,exact?qc:rat(0,1),exact);
        }
    } else {
        Rational exact_pref=rat(0,1);
        int pref_exact=qpow_rational(f.a,f.exponent,&exact_pref);
        Coeff prev=make_coeff(pow(a,n),exact_pref,pref_exact);
        if(!isfinite(prev.value)){strcpy(error,"Power overflow / invalid real value");return 0;}
        Rational ratio;int ratio_exact=qdiv(f.b,f.a,&ratio);
        for(int k=0;k*f.xpower<=deg;k++){
            if(k){
                Rational nk=rat(f.exponent.num-(long long)(k-1)*f.exponent.den,
                                f.exponent.den*(long long)k);
                Rational mul;
                int exact=prev.is_exact&&ratio_exact&&qmul(nk,ratio,&mul);
                Rational result;
                if(exact)exact=qmul(prev.exact,mul,&result);
                prev=make_coeff(prev.value*((n-k+1)/k)*(b/a),
                    exact?result:rat(0,1),exact);
            }
            poly[k*f.xpower]=prev;
        }
    }
    return 1;
}
int binomial_expand(const BinFactor *factors,int count,int degree,BinResult *out){
    memset(out,0,sizeof(*out));
    if(!factors || count<1||count>BIN_MAX_FACTORS||degree<0||degree>BIN_MAX_DEG){
        strcpy(out->error,"Use 1..4 factors; degree 0..20");return 0;
    }
    out->degree=degree;out->finite=1;out->convergence=INFINITY;
    Coeff accum[BIN_MAX_DEG+1], tmp[BIN_MAX_DEG+1],fpoly[BIN_MAX_DEG+1];
    Coeff zero=make_coeff(0,rat(0,1),1);
    for(int i=0;i<=degree;i++)accum[i]=zero;
    accum[0]=make_coeff(1,rat(1,1),1);
    for(int f=0;f<count;f++){
        double radius;int finite;
        if(!factor_terms(factors[f],degree,fpoly,&radius,&finite,out->error))return 0;
        if(!finite)out->finite=0;
        if(radius<out->convergence)out->convergence=radius;
        for(int i=0;i<=degree;i++){
            tmp[i]=zero;
            for(int j=0;j<=i;j++)tmp[i]=cadd(tmp[i],cmul(accum[j],fpoly[i-j]));
            if(!isfinite(tmp[i].value)){
                strcpy(out->error,"Numerical overflow - lower degree");return 0;
            }
        }
        for(int i=0;i<=degree;i++)accum[i]=tmp[i];
    }
    for(int i=0;i<=degree;i++)out->coeff[i]=accum[i];
    out->valid=1;return 1;
}
void binomial_coeff_string(Coeff c,char *buf,int cap){
    if(!isfinite(c.value)){snprintf(buf,cap,"undefined");return;}
    if(c.is_exact && c.exact.den>0){
        if(c.exact.den==1)snprintf(buf,cap,"%lld",c.exact.num);
        else snprintf(buf,cap,"%lld/%lld",c.exact.num,c.exact.den);
    } else snprintf(buf,cap,"~%.9g",fabs(c.value)<1e-300?0.0:c.value);
}
void out_clear(SolveOutput *o){memset(o,0,sizeof(*o));}
void out_add(SolveOutput *o,const char *fmt,...){
    if(o->count>=SOLVE_LINES)return;
    va_list ap;va_start(ap,fmt);
    vsnprintf(o->line[o->count],SOLVE_LINE_LEN,fmt,ap);
    va_end(ap);o->count++;
}
static int err(SolveOutput *o,const char *s){out_clear(o);out_add(o,"%s",s);return 0;}
static int whole(double v){return isfinite(v)&&v>=1&&v<=100000&&floor(v)==v;}
static int qsub(Rational a,Rational b,Rational *r){
    if(b.num==LLONG_MIN)return 0;
    b.num=-b.num;return qadd(a,b,r);
}
static void qstr(Rational a,char *s,size_t n){
    if(a.den==1)snprintf(s,n,"%lld",a.num);
    else snprintf(s,n,"%lld/%lld",a.num,a.den);
}
static void polystr(const Rational *p,int n,char *s,size_t cap){
    size_t u=0;int seen=0;s[0]=0;
    for(int i=n;i>=0;i--){
        if(!p[i].num)continue;
        char a[52],term[80],powx[16]="";
        qstr(p[i],a,sizeof(a));
        const char *mag=a[0]=='-'?a+1:a;
        if(i==1)strcpy(powx,"x");else if(i>1)snprintf(powx,sizeof(powx),"x^%d",i);
        const char *cn=i&&strcmp(mag,"1")==0?"":mag;
        snprintf(term,sizeof(term),"%s%s%s%s",seen?(p[i].num<0?" - ":" + "):(p[i].num<0?"-":""),cn,i&&*cn?"*":"",powx);
        size_t k=strlen(term);if(u+k>=cap)break;
        memcpy(s+u,term,k+1);u+=k;seen=1;
    }
    if(!seen)snprintf(s,cap,"0");
}
static int polyprod(Rational a[2],Rational b[2],Rational dst[3]){
    for(int i=0;i<3;i++)dst[i]=rat(0,1);
    for(int i=0;i<2;i++)for(int j=0;j<2;j++){
        Rational tmp;
        if(!qmul(a[i],b[j],&tmp)||!qadd(dst[i+j],tmp,&dst[i+j]))return 0;
    }return 1;
}
/* Proper rational functions with 2 or 3 linear factors. All algebra exact. */
static int partial(int tool,const Rational *v,SolveOutput *o){
    int n=tool==TOOL_PF2?2:3,off=n;
    Rational basis[3][3],m[3][4],f[3][2];
    for(int i=0;i<3;i++)for(int j=0;j<3;j++)basis[i][j]=rat(0,1);
    int nf=tool==TOOL_PF3?3:2;
    for(int i=0;i<off+2*nf;i++)if(!rational_ok(v[i]))return err(o,"Invalid fraction input.");
    for(int i=0;i<nf;i++){
        f[i][1]=v[off+2*i];f[i][0]=v[off+2*i+1];
        if(!f[i][1].num)return err(o,"Each linear factor needs nonzero x coefficient.");
    }
    if(n==2){for(int j=0;j<2;j++){basis[0][j]=f[1][j];basis[1][j]=f[0][j];}}
    else if(tool==TOOL_PF3){
        for(int i=0;i<3;i++)if(!polyprod(f[(i+1)%3],f[(i+2)%3],basis[i]))return err(o,"Exact coefficient overflow.");
    }else{
        if(!polyprod(f[0],f[1],basis[0])||!polyprod(f[0],f[0],basis[2]))return err(o,"Exact coefficient overflow.");
        for(int j=0;j<2;j++)basis[1][j]=f[1][j];
    }
    out_add(o,"Multiply by the denominator.");
    char p[3][120],num[120];
    Rational numer[3];for(int j=0;j<n;j++)numer[j]=v[n-1-j];
    polystr(numer,n-1,num,sizeof(num));out_add(o,"Numerator = %s",num);
    for(int i=0;i<n;i++){polystr(basis[i],n-1,p[i],sizeof(p[i]));out_add(o,"%c times (%s)",'A'+i,p[i]);}
    out_add(o,"Compare coefficients of each power:");
    for(int j=n-1;j>=0;j--){
        char a[52],b[52],c[52],d[52];
        qstr(basis[0][j],a,sizeof(a));qstr(basis[1][j],b,sizeof(b));
        qstr(numer[j],d,sizeof(d));
        if(n==2)out_add(o,"x^%d: (%s)A+(%s)B=%s",j,a,b,d);
        else{qstr(basis[2][j],c,sizeof(c));out_add(o,"x^%d: (%s)A+(%s)B+(%s)C=%s",j,a,b,c,d);}
    }
    for(int i=0;i<n;i++){for(int j=0;j<n;j++)m[i][j]=basis[j][n-1-i];m[i][n]=numer[n-1-i];}
    for(int i=0;i<n;i++){
        int k=i;while(k<n&&!m[k][i].num)k++;
        if(k==n)return err(o,"Factors not distinct. Choose repeated-factor tool if needed.");
        if(k!=i)for(int j=i;j<=n;j++){Rational tmp=m[i][j];m[i][j]=m[k][j];m[k][j]=tmp;}
        Rational pivot=m[i][i];
        for(int j=i;j<=n;j++)if(!qdiv(m[i][j],pivot,&m[i][j]))return err(o,"Exact coefficient overflow.");
        for(int k2=0;k2<n;k2++)if(k2!=i){
            Rational factor=m[k2][i];
            for(int j=i;j<=n;j++){Rational tmp;if(!qmul(factor,m[i][j],&tmp)||!qsub(m[k2][j],tmp,&m[k2][j]))return err(o,"Exact coefficient overflow.");}
        }
    }
    out_add(o,"Solving simultaneous equations:");
    for(int i=0;i<n;i++){char a[52];qstr(m[i][n],a,sizeof(a));out_add(o,"%c = %s",'A'+i,a);}
    out_add(o,"Final decomposition:");
    for(int i=0;i<n;i++){char a[52],b[80];qstr(m[i][n],a,sizeof(a));
        int k=tool==TOOL_PFR?(i==2?1:0):i;
        polystr(f[k],1,b,sizeof(b));out_add(o,"%s(%s)/(%s)%s",i?"+ ":"",a,b,tool==TOOL_PFR&&i==1?"^2":"");}
    return 1;
}
static int division(const double *v,const Rational *q,SolveOutput *o){
    if(v[0]<0||v[0]>5||v[1]<1||v[1]>3)return err(o,"Degrees: dividend 0-5, divisor 1-3.");
    int n=(int)v[0],m=(int)v[1];
    if(n<0||n>5||m<1||m>3||v[0]!=n||v[1]!=m)return err(o,"Degrees: dividend 0-5, divisor 1-3.");
    Rational p[6],d[4],quo[6],remainder[6];
    for(int i=0;i<6;i++)p[i]=quo[i]=remainder[i]=rat(0,1);
    for(int i=0;i<=n;i++){p[n-i]=q[2+i];if(!rational_ok(p[n-i]))return err(o,"Invalid coefficient.");}
    for(int j=0;j<=m;j++){d[m-j]=q[3+n+j];if(!rational_ok(d[m-j]))return err(o,"Invalid coefficient.");}
    if(!p[n].num&&n>0)return err(o,"Dividend leading coefficient cannot be zero.");
    if(!d[m].num)return err(o,"Divisor leading coefficient cannot be zero.");
    memcpy(remainder,p,sizeof(p));
    for(int k=n-m;k>=0;k--){
        if(!qdiv(remainder[m+k],d[m],&quo[k]))return err(o,"Exact arithmetic overflow.");
        char a[52],b[52],c[52],s[200];qstr(remainder[m+k],a,sizeof(a));qstr(d[m],b,sizeof(b));qstr(quo[k],c,sizeof(c));
        out_add(o,"Step %d: (%s)/(%s) = %s",n-m-k+1,a,b,c);
        out_add(o,"Quotient term: (%s)x^%d",c,k);
        for(int j=0;j<=m;j++){Rational tmp;if(!qmul(quo[k],d[j],&tmp)||!qsub(remainder[j+k],tmp,&remainder[j+k]))return err(o,"Exact arithmetic overflow.");}
        polystr(remainder,n,s,sizeof(s));out_add(o,"Subtract product: %s",s);
    }
    char qs[200],rs[200];polystr(quo,n>=m?n-m:0,qs,sizeof(qs));polystr(remainder,n,rs,sizeof(rs));
    out_add(o,"QUOTIENT: %s",qs);out_add(o,"REMAINDER: %s",rs);
    out_add(o,"P(x)=D(x)*Q(x)+R(x).");return 1;
}
int revision_solve(int tool,const double *v,const Rational *q,SolveOutput *o){
    out_clear(o);
    if(tool==TOOL_PF2||tool==TOOL_PF3||tool==TOOL_PFR)return partial(tool,q,o);
    if(tool==TOOL_DIV)return division(v,q,o);
    if(tool==TOOL_R_INTERVAL||tool==TOOL_R_DIRECT){
        double R,alpha,c=v[2],lo=v[3],hi=v[4],rad=PI/180;
        if(tool==TOOL_R_DIRECT){R=v[0];alpha=v[1];if(R<0){R=-R;alpha+=180;out_add(o,"Negative R: reverse sign, add 180 deg.");}}
        else{R=hypot(v[0],v[1]);alpha=atan2(v[1],v[0])/rad;out_add(o,"R=sqrt(a^2+b^2)=%.9g",R);}
        if(hi<lo||fabs(lo)>1e7||fabs(hi)>1e7||fabs(alpha)>1e7)return err(o,"Invalid or excessively large interval.");
        if(R==0){out_add(o,"Constant: %.9g",c);out_add(o,"Max and min at every angle.");return 1;}
        out_add(o,"Alpha=%.9g degrees",alpha);
        out_add(o,"y=%.7g sin(x%+.7g)%+.7g",R,alpha,c);
        double fl=c+R*sin((lo+alpha)*rad),fh=c+R*sin((hi+alpha)*rad);
        out_add(o,"At x=%.8g, y=%.9g",lo,fl);
        out_add(o,"At x=%.8g, y=%.9g",hi,fh);
        double maxx=90-alpha+360*ceil((lo-90+alpha)/360-1e-12);
        double minx=270-alpha+360*ceil((lo-270+alpha)/360-1e-12);
        double mx=fl>=fh?fl:fh,mn=fl<=fh?fl:fh;
        double xm=fl>=fh?lo:hi,xn=fl<=fh?lo:hi;
        int imax=maxx<=hi+1e-9,imin=minx<=hi+1e-9;
        if(imax){mx=c+R;xm=maxx;out_add(o,"Max peak x=%.9g is in interval.",xm);}
        else out_add(o,"No full max peak in interval.");
        if(imin){mn=c-R;xn=minx;out_add(o,"Min trough x=%.9g is in interval.",xn);}
        else out_add(o,"No full min trough in interval.");
        out_add(o,"MAX = %.9g at x=%.9g deg",mx,xm);
        out_add(o,"MIN = %.9g at x=%.9g deg",mn,xn);
        if(imax||imin)out_add(o,"Peak/trough repeats every 360 deg within interval.");
        return 1;
    }
    if(tool==TOOL_AP_KNOWN){
        double a=v[0],d=v[1],n=v[2];if(!whole(n))return err(o,"n must be a positive integer.");
        double l=a+(n-1)*d;out_add(o,"u_n=a+(n-1)d");out_add(o,"=%.8g+(%.0f-1)*%.8g",a,n,d);out_add(o,"u_n=%.9g",l);out_add(o,"S_n=n(a+u_n)/2");out_add(o,"=%.0f*(%.8g+%.8g)/2",n,a,l);out_add(o,"S_n=%.9g",n*(a+l)/2);return 1;
    }
    if(tool==TOOL_AP_N){
        double a=v[0],d=v[1],u=v[2];if(d==0){out_add(o,u==a?"Every positive n works.":"No term matches.");return 1;}
        double n=1+(u-a)/d;
        out_add(o,"u_n=a+(n-1)d");out_add(o,"n=1+(u_n-a)/d");out_add(o,"n=1+(%.8g-%.8g)/%.8g",u,a,d);
        out_add(o,"n=%.9g",n);if(n<1||fabs(n-round(n))>1e-8)out_add(o,"Not a positive integer: no such term.");else out_add(o,"S_n=%.9g",round(n)*(a+u)/2);return 1;
    }
    if(tool==TOOL_AP_SUM_N){
        double a=v[0],d=v[1],S=v[2],b=2*a-d;
        out_add(o,"S_n=n/2[2a+(n-1)d]");
        out_add(o,"d*n^2+(2a-d)n-2S=0");
        out_add(o,"%.8g*n^2%+.8g*n%+.8g=0",d,b,-2*S);
        if(d==0){if(a==0){out_add(o,S==0?"All positive n work.":"No solution.");return 1;}double n=S/a;out_add(o,"n=S/a=%.9g",n);if(!whole(n))out_add(o,"Not a positive integer.");return 1;}
        double D=b*b+8*d*S;if(D<0){out_add(o,"No real solutions.");return 1;}double n1=(-b+sqrt(D))/(2*d),n2=(-b-sqrt(D))/(2*d);
        out_add(o,"Discriminant=%.9g",D);
        out_add(o,"n=%.9g %s",n1,(n1>=1&&fabs(n1-round(n1))<1e-7)?"valid":"reject (not positive integer)");
        if(D>0)out_add(o,"n=%.9g %s",n2,(n2>=1&&fabs(n2-round(n2))<1e-7)?"valid":"reject (not positive integer)");
        return 1;
    }
    if(tool==TOOL_GP_TWO){
        double i=v[0],u=v[1],j=v[2],w=v[3],n=v[4];
        if(!whole(i)||!whole(j)||!whole(n)||i==j||u==0||w==0)return err(o,"Use distinct positive positions and nonzero terms.");
        if(i>j){double tmp=i;i=j;j=tmp;tmp=u;u=w;w=tmp;}
        int k=(int)(j-i);double ratio=w/u,r=pow(fabs(ratio),1.0/k);
        if(!isfinite(r)||r==0)return err(o,"Ratio outside supported numeric range.");
        out_add(o,"Divide u_j by u_i: r^(j-i)=u_j/u_i");out_add(o,"r^%d=%.9g",k,ratio);
        if(ratio<0&&k%2==0)return err(o,"No real ratio: even power is negative.");
        if(ratio<0)r=-r;
        int count=ratio>0&&k%2==0?2:1;
        for(int s=0;s<count;s++){
            double rr=s?-r:r,a=u/pow(rr,i-1),un=a*pow(rr,n-1),sum=fabs(rr-1)<EPS?n*a:a*(1-pow(rr,n))/(1-rr);
            if(!isfinite(a)||!isfinite(un)||!isfinite(sum))return err(o,"Numerical overflow; reduce term positions.");
            out_add(o,"r=%.9g",rr);out_add(o,"a=u_i/r^(i-1)=%.9g",a);out_add(o,"u_n=%.9g; S_n=%.9g",un,sum);if(fabs(rr)<1)out_add(o,"S_inf=%.9g",a/(1-rr));
        }return 1;
    }
    if(tool==TOOL_GP_N){
        double a=v[0],r=v[1],u=v[2];if(a==0||r<=0||u/a<=0||r==1)return err(o,"Use a!=0, r>0 (not 1), u/a>0.");
        double n=1+log(u/a)/log(r);out_add(o,"u_n=a*r^(n-1)");out_add(o,"r^(n-1)=%.9g",u/a);out_add(o,"n=1+ln(u_n/a)/ln(r)");out_add(o,"n=%.9g",n);if(n<1||fabs(n-round(n))>1e-8)out_add(o,"No exact positive integer term.");return 1;
    }
    if(tool==TOOL_GP_SUM_N){
        double a=v[0],r=v[1],S=v[2];if(a==0||r<=0)return err(o,"Use nonzero a and positive ratio r.");
        if(r==1){double n=S/a;out_add(o,"S_n=na; n=S/a=%.9g",n);if(n<1||fabs(n-round(n))>1e-7)out_add(o,"No positive integer n.");return 1;}
        double z=1-S*(1-r)/a;if(z<=0)return err(o,"No finite real n: r^n must be positive.");
        double n=log(z)/log(r);out_add(o,"S_n=a(1-r^n)/(1-r)");out_add(o,"r^n=1-S_n(1-r)/a");out_add(o,"r^n=%.9g",z);out_add(o,"n=ln(%.9g)/ln(%.9g)",z,r);out_add(o,"n=%.9g",n);if(n<1||fabs(n-round(n))>1e-7)out_add(o,"No exact positive integer n.");return 1;
    }
    if(tool==TOOL_EXP_K_TIME){
        double A=v[0],k=v[1],c=v[2],y=v[3];if(A==0||k==0)return err(o,"Require nonzero A and k.");
        double r=(y-c)/A;if(r<=0)return err(o,"No real t: (target-c)/A must be positive.");
        out_add(o,"y=A*e^(k*t)+c");out_add(o,"Subtract c: y-c=%.9g",y-c);out_add(o,"Divide by A: e^(kt)=%.9g",r);out_add(o,"Take ln: kt=ln(%.9g)",r);out_add(o,"kt=%.9g",log(r));out_add(o,"t=%.9g/%.9g",log(r),k);out_add(o,"t=%.9g",log(r)/k);return 1;
    }
    if(tool==TOOL_EXP_TIME||tool==TOOL_EXP_VALUE){
        double A=v[0],b=v[1],c=v[2],x=v[3];
        if(A==0||b<=0)return err(o,"Requires A!=0 and base b>0.");
        out_add(o,"Model y=A*b^t+c");
        out_add(o,"y=%.8g*(%.8g)^t%+.8g",A,b,c);
        if(tool==TOOL_EXP_VALUE){
            double p=pow(b,x),y=A*p+c;if(!isfinite(y))return err(o,"Numerical overflow.");
            out_add(o,"Substitute t=%.8g",x);out_add(o,"b^t=%.9g",p);out_add(o,"A*b^t=%.9g",A*p);out_add(o,"Add c: y=%.9g",y);return 1;
        }
        double z=(x-c)/A;if(b==1){out_add(o,x==A+c?"Every t works.":"Constant model; no solution.");return 1;}
        if(z<=0)return err(o,"No real solution: (target-c)/A must be positive.");
        double t=log(z)/log(b);out_add(o,"Subtract c: y-c=%.9g",x-c);out_add(o,"Divide by A: b^t=%.9g",z);out_add(o,"Take ln: t*ln(b)=ln(%.9g)",z);out_add(o,"ln(ratio)=%.9g",log(z));out_add(o,"ln(b)=%.9g",log(b));out_add(o,"Divide: t=%.9g",t);return 1;
    }
    if(tool==TOOL_EXP_FIT){
        double t1=v[0],y1=v[1],t2=v[2],y2=v[3],c=v[4];
        double u=y1-c,w=y2-c;
        if(t1==t2||u==0||w/u<=0)return err(o,"Distinct times; adjusted values same nonzero sign.");
        double k=log(w/u)/(t2-t1),b=exp(k),A=u*exp(-k*t1);
        if(!isfinite(b)||!isfinite(A)||b<=0||A==0)return err(o,"Numerical range exceeded.");
        out_add(o,"Subtract c from both values.");out_add(o,"A*b^t1=%.9g; A*b^t2=%.9g",u,w);
        out_add(o,"Divide: b^(t2-t1)=%.9g",w/u);out_add(o,"ln(b)=ln(ratio)/(t2-t1)=%.9g",k);
        out_add(o,"b=e^(%.8g)=%.9g",k,b);out_add(o,"A=(y1-c)/b^t1=%.9g",A);
        out_add(o,"y=%.8g*(%.8g)^t%+.8g",A,b,c);out_add(o,"Equivalent y=%.8g*e^(%.8g*t)%+.8g",A,k,c);return 1;
    }
    if(tool==TOOL_EXP_QUAD){
        double a=v[0],b=v[1],c=v[2],base=v[3];
        if(a==0||base<=0||base==1)return err(o,"Need nonzero a, positive base not 1.");
        double disc=b*b-4*a*c;
        out_add(o,"Set u=base^x; u>0.");
        out_add(o,"Solve %.8g*u^2%+.8g*u%+.8g=0",a,b,c);
        out_add(o,"Discriminant=%.9g",disc);
        if(disc<0){out_add(o,"No real roots.");return 1;}
        double r=sqrt(disc),roots[2]={(-b+r)/(2*a),(-b-r)/(2*a)};
        for(int i=0;i<(disc==0?1:2);i++){double u=roots[i];out_add(o,"u=%.9g",u);if(u<=0)out_add(o,"Reject: base^x is always positive.");else{out_add(o,"x=ln(%.8g)/ln(%.8g)",u,base);out_add(o,"x=%.9g",log(u)/log(base));}}
        return 1;
    }
    return err(o,"Unknown tool.");
}
static double choose(int n,int k){
    if(k<0||k>n)return 0;
    if(k>n-k)k=n-k;
    double v=1;
    for(int i=1;i<=k;i++)v*=((double)n-i+1)/i;
    return v;
}
static double binom_upper(int n,double p,int k){
    if(k<=0)return 1;
    if(k>n)return 0;
    double sum=0;
    for(int j=k;j<=n;j++)sum+=choose(n,j)*pow(p,j)*pow(1-p,n-j);
    return fmin(1,fmax(0,sum));
}
static double binom_lower(int n,double p,int k){
    if(k<0)return 0;
    if(k>=n)return 1;
    double sum=0;
    for(int j=0;j<=k;j++)sum+=choose(n,j)*pow(p,j)*pow(1-p,n-j);
    return fmin(1,fmax(0,sum));
}
static int between(double x,double lo,double hi){return isfinite(x)&&x>=lo&&x<=hi;}
static double normal_upper(double z){return 0.5*erfc(z/sqrt(2.0));}
/* v sizes and meaning are documented in src/app_data.c. */
int solve_problem(int id,const double *v,SolveOutput *o){
    out_clear(o);
    switch(id){
    case SOL_RFORM: {double a=v[0],b=v[1],r=hypot(a,b);if(r<EPS)break;
        double alpha=atan2(b,a)*180/PI;
        out_add(o,"a*sin(x)+b*cos(x) = R*sin(x+A)");
        out_add(o,"R = sqrt(a^2+b^2) = %.9g",r);
        out_add(o,"A = atan2(b,a) = %.7g deg",alpha);
        out_add(o,"Quadrant included using atan2.");return 1;}
    case SOL_APGP:{double a=v[0],step=v[1],n=v[2],kind=v[3];
        if(!between(n,1,100000)||floor(n)!=n||!between(kind,1,2))break;
        if(kind==1){out_add(o,"AP: u_n = a+(n-1)d");out_add(o,"u_n = %.10g",a+(n-1)*step);
            out_add(o,"S_n = n/2(2a+(n-1)d)");out_add(o,"S_n = %.10g",n*(2*a+(n-1)*step)/2);}
        else{out_add(o,"GP: u_n = a*r^(n-1)");out_add(o,"u_n = %.10g",a*pow(step,n-1));
            if(fabs(step-1)<EPS)out_add(o,"S_n = %.10g",n*a);
            else out_add(o,"S_n = a(1-r^n)/(1-r) = %.10g",a*(1-pow(step,n))/(1-step));
            if(fabs(step)<1)out_add(o,"S_inf = %.10g",a/(1-step));}
        return 1;}
    case SOL_PARTIAL_FRACTIONS:{/* (Ax+B)/((x-p)(x-q)) */
        double a=v[0],b=v[1],p=v[2],q=v[3];if(fabs(p-q)<EPS)break;
        double c=(a*p+b)/(p-q),d=(a*q+b)/(q-p);
        out_add(o,"(Ax+B)/[(x-p)(x-q)]");
        out_add(o,"= C/(x-p) + D/(x-q)");
        out_add(o,"C = (Ap+B)/(p-q) = %.9g",c);
        out_add(o,"D = (Aq+B)/(q-p) = %.9g",d);return 1;}
    case SOL_POLY_DERIV:{/* coefficients a0..a4, x */
        double x=v[5],y=0,dy=0,d2=0;
        for(int j=4;j>=0;j--)y=y*x+v[j];
        for(int j=4;j>=1;j--)dy=dy*x+j*v[j];
        for(int j=4;j>=2;j--)d2=d2*x+j*(j-1)*v[j];
        out_add(o,"f(x)=a0+a1*x+...+a4*x^4");
        out_add(o,"f(%.6g) = %.10g",x,y);
        out_add(o,"f'(%.6g) = %.10g",x,dy);
        out_add(o,"f''(%.6g) = %.10g",x,d2);
        out_add(o,"Tangent: y - %.6g",y);
        out_add(o," = %.6g (x - %.6g)",dy,x);
        if(fabs(dy)>EPS)out_add(o,"Normal gradient = %.9g",-1/dy);
        else out_add(o,"Normal is x = %.6g",x);
        return 1;}
    case SOL_POLY_INTEGRATE:{double lo=v[5],hi=v[6],result=0;
        for(int k=0;k<=4;k++)result+=v[k]*(pow(hi,k+1)-pow(lo,k+1))/(k+1);
        out_add(o,"Integral of polynomial sum a_k*x^k");
        out_add(o,"Anti-derivative: sum a_k*x^(k+1)/(k+1)");
        out_add(o,"F(upper)-F(lower) = %.10g",result);
        out_add(o,"Signed integral (not always area).");return isfinite(result);}
    case SOL_NEWTON:{double x=v[5];if(!isfinite(x))break;
        out_add(o,"Newton: next = x - f(x)/f'(x)");
        for(int it=0;it<6;it++){
            double f=v[4],fp=4*v[4];
            for(int j=3;j>=0;j--)f=f*x+v[j];
            for(int j=3;j>=1;j--)fp=fp*x+j*v[j];
            if(fabs(fp)<EPS){out_add(o,"Stopped: derivative near zero");return 1;}
            double nx=x-f/fp;
            if(!isfinite(nx)||fabs(nx)>1e12){out_add(o,"Stopped: diverging");return 1;}
            out_add(o,"Step %d: x = %.10g",it+1,nx);
            if(fabs(nx-x)<1e-11)break;
            x=nx;
        }
        out_add(o,"Iteration does not prove root exists.");return 1;}
    case SOL_STRATIFIED:{double Nh=v[0],N=v[1],n=v[2];if(N<=0||Nh<0||Nh>N||n<0)break;
        out_add(o,"Stratum / total * sample size");
        out_add(o,"n_h = %.0f / %.0f * %.0f",Nh,N,n);
        out_add(o,"Exact allocation = %.10g",Nh*n/N);
        out_add(o,"Round so ALL strata sum to target.");return 1;}
    case SOL_GROUPED:{double L=v[0],F=v[1],f=v[2],w=v[3],k=v[4];if(f<=0||w<=0||k<F||k>F+f)break;
        out_add(o,"Grouped-data interpolation:");
        out_add(o,"L+(k-F)*w/f");
        out_add(o,"= %.10g",L+(k-F)*w/f);
        out_add(o,"Check quartile position convention.");return 1;}
    case SOL_CONDITIONAL:{double pa=v[0],pb=v[1],pi=v[2];
        if(!between(pa,0,1)||!between(pb,0,1)||!between(pi,0,fmin(pa,pb))||pb==0||pi<pa+pb-1-EPS)break;
        out_add(o,"P(A|B) = P(A and B)/P(B)");
        out_add(o,"P(A|B) = %.10g",pi/pb);
        out_add(o,"P(A or B) = %.10g",pa+pb-pi);
        out_add(o,"Independent? %s",fabs(pi-pa*pb)<1e-8?"yes":"no");return 1;}
    case SOL_BINOMIAL_TEST:{int n=(int)v[0],k=(int)v[2],tail=(int)v[4];double p=v[1],alpha=v[3];
        if(n<1||n>150||v[0]!=n||k<0||k>n||v[2]!=k||!between(p,0,1)||!between(alpha,0,1)||alpha<=0||tail<1||tail>3)break;
        double pv=tail==1?binom_lower(n,p,k):tail==2?binom_upper(n,p,k):fmin(1,2*fmin(binom_lower(n,p,k),binom_upper(n,p,k)));
        out_add(o,"H0: p=%.6g; H1: %s",p,tail==1?"p<p0":tail==2?"p>p0":"p!=p0");
        out_add(o,"X~B(%d, %.6g), observed k=%d",n,p,k);
        out_add(o,"p-value = %.10g",pv);
        if(tail==3)out_add(o,"Two-tail uses doubled smaller tail.");
        out_add(o,"Alpha = %.6g",alpha);
        out_add(o,pv<=alpha?"Reject H0 at this level.":"Do not reject H0.");
        out_add(o,pv<=alpha?"Evidence for H1.":"Not enough evidence for H1.");
        return 1;}
    case SOL_NORMAL_TEST:{double mu0=v[0],sigma=v[1],n=v[2],obs=v[3],alpha=v[4],tail=v[5];
        if(sigma<=0||n<1||floor(n)!=n||alpha<=0||alpha>=1||tail<1||tail>3||floor(tail)!=tail)break;
        double z=(obs-mu0)/(sigma/sqrt(n));double left=normal_upper(-z),right=normal_upper(z);
        double pv=tail==1?left:tail==2?right:2*fmin(left,right);
        out_add(o,"Z=(mean-mu0)/(sigma/sqrt(n))");
        out_add(o,"Z = %.9g",z);
        out_add(o,"p-value = %.9g",pv);
        out_add(o,pv<=alpha?"Reject H0.":"Do not reject H0.");
        out_add(o,"Assumes known population sigma.");return 1;}
    case SOL_SUVAT:{/* u,a,t (constant acceleration) */
        double u=v[0],a=v[1],t=v[2];if(t<0)break;
        out_add(o,"Constant acceleration model");
        out_add(o,"v=u+at = %.9g",u+a*t);
        out_add(o,"s=ut+0.5at^2 = %.9g",u*t+0.5*a*t*t);
        return 1;}
    case SOL_PROJECTILE:{double u=v[0],deg=v[1],h=v[2],g=v[3];
        if(u<0||g<=0||h<0)break;
        double rad=deg*PI/180,vx=u*cos(rad),vy=u*sin(rad);
        double disc=vy*vy+2*g*h;
        double t=(vy+sqrt(disc))/g;
        if(t<0)break;
        out_add(o,"Horizontal: vx=%.9g",vx);
        out_add(o,"Vertical: vy=%.9g",vy);
        out_add(o,"y=h+vy*t - gt^2/2");
        out_add(o,"Time to ground = %.9g s",t);
        out_add(o,"Horizontal range = %.9g m",vx*t);
        out_add(o,"Max height = %.9g m",h+(vy>0?vy*vy/(2*g):0));return 1;}
    case SOL_INCLINE:{double m=v[0],theta=v[1]*PI/180,mu=v[2],g=v[3];
        if(m<=0||mu<0||g<=0||theta<0||theta>=PI/2)break;
        double R=m*g*cos(theta),driving=m*g*sin(theta),fr=mu*R;
        out_add(o,"Assume sliding DOWN the plane:");
        out_add(o,"R=mg cos(theta)=%.9g N",R);
        out_add(o,"F=mu*R=%.9g N",fr);
        out_add(o,"ma=mg sin(theta)-F");
        out_add(o,"a=%.9g m/s^2",(driving-fr)/m);
        out_add(o,"If negative: object decelerates if moving down.");return 1;}
    case SOL_PULLEY:{double ma=v[0],mb=v[1],g=v[2];if(ma<=0||mb<=0||g<=0)break;
        double a=(mb-ma)*g/(ma+mb),t=ma*(g+a);
        out_add(o,"Ideal pulley; A up, B down.");
        out_add(o,"(mB-mA)g=(mA+mB)a");
        out_add(o,"a=%.9g m/s^2",a);
        out_add(o,"T=mA(g+a)=%.9g N",t);
        out_add(o,"Negative a means opposite movement.");return 1;}
    case SOL_MOMENT:{double f=v[0],d=v[1],lever=v[2];
        if(d<0||lever<=0)break;
        out_add(o,"Moment = force * perp distance");
        out_add(o,"Moment = %.9g N m",f*d);
        out_add(o,"Equilibrating perpendicular force");
        out_add(o,"F2*%.6g = %.9g",lever,f*d);
        out_add(o,"F2 = %.9g N",f*d/lever);return 1;}
    default:break;
    }
    out_add(o,"Invalid inputs or unsupported domain.");
    out_add(o,"Check values and stated assumptions.");return 0;
}
