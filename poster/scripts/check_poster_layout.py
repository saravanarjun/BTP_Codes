#!/usr/bin/env python3
import fitz

doc = fitz.open('poster/poster.pdf')
page = doc[0]
print(f"Page dimensions: {page.rect.width} x {page.rect.height} pt ({page.rect.width/72:.2f} x {page.rect.height/72:.2f} in)")

text = page.get_text()
checks = [
    'Dynamic Graph Algorithms on Proper Interval Graphs',
    'K. Saravanarjun',
    'Dr. Manas Jyothi Kashyop',
    'Indian Institute of Technology Bhubaneswar',
    '1. Preliminaries & Research Motivation',
    'Interval Graphs',
    'Proper Interval Graphs',
    '2. Core Structural Findings',
    'Lemma 3.1',
    'Lemma 3.2',
    'Lemma 3.3',
    'Lemma 3.4',
    'Lemma 3.5',
    'Lemma 3.6',
    'Theorem 3.7',
    'Lemma 3.8',
    'Lemma 3.10',
    '3. Maximal Clique Characterization & Static Algorithm',
    'Lemma 3.15',
    'Parent Bijection',
    '4. Dynamic Maximal Clique Maintenance',
    'Sibling Adoption',
    'At most 3 edits',
    '5. Maximum Independent Set & Dominating Set',
    'Lemma 3.11',
    'Theorem 3.12',
    'Theorem 3.17',
    'Algorithm 1: MIS Dynamic Programming',
    '6. Experimental Evaluation & Baseline Comparisons',
    '7. Amortized Break-Even Analysis',
    '8. Key Contributions & Future Directions',
    'K* = 4'
]

norm_text = ' '.join(text.split())
all_passed = True
for c in checks:
    matched = False
    if c == 'K* = 4':
        matched = any(k in text for k in ['K* = 4', 'K * = 4', 'K∗= 4'])
    else:
        matched = c in text or c in norm_text
    status = 'PASS' if matched else 'FAIL'
    if not matched:
        all_passed = False
    print(f"[{status}] {c}")

if all_passed:
    print("\n>>> ALL 31 CONTENT AND STRUCTURAL AUDIT CHECKS PASSED SUCCESSFULLY! <<<")
else:
    print("\n>>> SOME CHECKS FAILED! <<<")
