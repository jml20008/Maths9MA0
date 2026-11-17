# Maths9MA0 V3 — Pure Maths revision and guided solver

Native Casio fx-CG50 add-in source. Covers your 14 Pure topics: Year 1 and Year 2 through trigonometry, with **Year 1 calculus only**.

## Fast navigation

- Main screen: **F2 = missing-formula booklet**, **F3 = all calculators**.
- Select a topic and question using arrows and EXE.
- F1 = method, F2 = worked example, F3 = solve (when available), F4 = diagram.
- In a question list, F3 starts the selected calculator directly.
- In calculators, type a value and press EXE to continue. Defaults are examples. The first digit replaces the default. F2 types a fraction slash; (-) for negatives. F3 goes to previous input.
- Results: UP/DOWN scroll, F5 jumps to the end, F1 edits inputs, EXIT returns to method. Binomial results use F2 for coefficients.

## 21 selectable calculator modes

| Topic | Modes |
| --- | --- |
| Harmonic form | Convert a*sin(x)+b*cos(x)+c; extrema on a closed interval; given R*sin(x+alpha)+c |
| Arithmetic | Two terms -> a,d; term/sum; find n from term; find n from sum |
| Geometric | Term, finite and infinite sums; two terms -> a,r; find n from term; find n from sum |
| Exponentials | Find time for A*b^t+c and A*e^(kt)+c; fit from two points and known offset; evaluate; quadratic in b^x |
| Partial fractions | Two or three distinct linear factors; repeated L^2*M |
| Polynomial division | Dividend degree 0–5, divisor degree 1–3 |
| Binomial | Up to four brackets, powers through x^20, negative and fractional exponents |

Harmonic solvers use degrees. For unrestricted extrema use a full cycle e.g. 0–360. Restricted extrema test endpoints and critical peaks/troughs. For cosine form use equivalent sine phase or original coefficients. Example 8sin(x)+6cos(x)+2 on [0,90]: select harmonic max/min interval, enter 8,6,2,0,90. Max=12 at 53.130102 degrees, min=8 at 0.

### Exact algebra

Enter numerator coefficients and each linear factor's coefficients. E.g. (2x+3)/[(x-1)(x+2)] uses 2,3,1,-1,1,2. Results show coefficient equations plus A=5/3, B=1/3. Choose repeated mode for a squared factor; denominator must be factored. Improper fractions require polynomial division first. No irreducible quadratic denominator option.

For division, input degrees then descending coefficients (0 for missing powers). Integers or fractions accepted. Exact quotient and remainder; overflow gives an error. F5 jumps to the final result.

### Binomial

Enter bracket count (1–4), maximum x power (0–20), then a,b,n,q for each factor (a+b*x^q)^n. q=1 for ordinary x. F2 inputs fraction slash. Result includes expansion, common convergence radius, coefficient list. Example (1+2x)^(-1/2) through x^3: enter 1,3,1,2,-1/2,1. Fractional powers require positive a. Tilde marks approximate coefficients.

### Formula booklet and proofs

F2 contains formulas not printed directly in the Pearson A-level Pure booklet, plus a separate reminder for formulas already supplied. Proofs has eight worked examples (two per method: deduction, contradiction, exhaustion, counterexample) based on genuine AS and A-level past papers. See PROOF_SOURCES.md.

## Update existing GitHub repository

1. Extract the ZIP; open existing Maths9MA0 repo.
2. Add file > Upload files, drag the extracted **src** and **tests** folders into repo root and commit. No workflow changes if using the supplied workflow.
3. In Actions, run the build workflow again.
4. Download the Maths9MA0-CG50 artifact from the successful run. Extract MATHS9MA0.g3a, back up the old file and transfer the new one via CG50 USB storage.

New repo: upload all contents, including hidden .github/workflows/build-g3a.yml. If hidden folder cannot be uploaded, create that exact path and paste supplied YAML.

## Verification and limitations

Portable engine: 77 automated checks passed. Mock CG50 navigation: 16 menus, 107 cards, direct shortcuts and core solver paths passed. Native cross-compilation and physical calculator testing remain unverified here: a successful GitHub build is needed for the .g3a.

This is an offline structured revision tool, not unrestricted computer algebra or ChatGPT. GP inverse-index modes require positive ratios; two-known-term mode handles negative ratios where possible.

**Revision only.** Stored notes and symbolic tools must not be accessible in official examinations. The fx-991EX cannot install this add-in; target is the fx-CG50.
