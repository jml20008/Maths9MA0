#ifndef MATHS9MA0_ENGINE_H
#define MATHS9MA0_ENGINE_H

/* Portable computation engine. Does not require Casio headers. */
#define BIN_MAX_DEG 20
#define BIN_MAX_FACTORS 4
#define SOLVE_LINES 48
#define SOLVE_LINE_LEN 220

typedef struct { long long num, den; } Rational;
typedef struct { Rational a, b, exponent; int xpower; } BinFactor;
typedef struct { double value; Rational exact; int is_exact; } Coeff;
typedef struct {
    Coeff coeff[BIN_MAX_DEG+1];
    int degree;             /* requested max power */
    int finite;             /* 1 if all factors form a finite polynomial */
    int valid;              /* success flag */
    double convergence;    /* |x| < this, INFINITY if no restriction */
    char error[96];
} BinResult;

typedef struct { int count; char line[SOLVE_LINES][SOLVE_LINE_LEN]; } SolveOutput;

Rational rat(long long numerator, long long denominator);
int rational_ok(Rational r);
int binomial_expand(const BinFactor *factors, int factor_count,
                    int degree, BinResult *out);
void binomial_coeff_string(Coeff c, char *out, int out_cap);
void out_clear(SolveOutput *o);
void out_add(SolveOutput *o, const char *fmt, ...);

/* Every solver accepts double values; see solver_specs in app UI. */
int solve_problem(int solver_id, const double *v, SolveOutput *out);

enum {TOOL_NONE, TOOL_AP, TOOL_R, TOOL_BIN, TOOL_GP,
      TOOL_R_INTERVAL, TOOL_R_DIRECT, TOOL_AP_KNOWN, TOOL_AP_N,
      TOOL_GP_TWO, TOOL_GP_N, TOOL_EXP_TIME, TOOL_EXP_FIT, TOOL_EXP_VALUE,
      TOOL_EXP_QUAD, TOOL_PF2, TOOL_PF3, TOOL_PFR, TOOL_DIV, TOOL_EXP_K_TIME,
      TOOL_AP_SUM_N, TOOL_GP_SUM_N};
/* Numeric inputs v; exact fraction inputs q for algebra tools. */
int revision_solve(int tool,const double *v,const Rational *q,SolveOutput *out);

/* IDs shared with GUI; 17 focussed solvers (no fx-991EX duplicates). */
enum {
    SOL_BINOMIAL=0, SOL_RFORM, SOL_APGP, SOL_PARTIAL_FRACTIONS,
    SOL_POLY_DERIV, SOL_POLY_INTEGRATE, SOL_NEWTON,
    SOL_STRATIFIED, SOL_GROUPED, SOL_CONDITIONAL,
    SOL_BINOMIAL_TEST, SOL_NORMAL_TEST, SOL_SUVAT,
    SOL_PROJECTILE, SOL_INCLINE, SOL_PULLEY, SOL_MOMENT,
    SOLVER_COUNT
};
#endif
