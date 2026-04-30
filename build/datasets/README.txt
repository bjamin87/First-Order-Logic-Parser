TPTP-style parser-compatible benchmark set
These are synthetic first-order/propositional formulas inspired by common ATP benchmark structures.
They are NOT official TPTP problems; they are designed to fit Ben's current parser grammar.

Files:
- tptp_style_medium_100.txt: controlled FOL/propositional formulas
- tptp_style_hard_100.txt: deeper quantifier/implication formulas
- tptp_style_very_hard_100.txt: heavier branching + quantifier formulas
- tptp_style_combined_300.txt: all formulas combined

Formula format: one parser-compatible formula per line.
Design goal: stress sequent-calculus rule ordering without SATLIB-style CNF explosion.
