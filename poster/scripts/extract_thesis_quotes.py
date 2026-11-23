import fitz
import re

pdf_path = r'C:\Users\sarav\.gemini\antigravity\brain\73e6a4d7-ad87-418e-8cd1-48d5c7a42fce\.user_uploaded\media_1793512184274_e79796d5.pdf'
doc = fitz.open(pdf_path)
print(f'Total pages in thesis: {len(doc)}')

terms = [
    'Manas', 'Lemma 3.1', 'Lemma 3.2', 'Lemma 3.3', 'Lemma 3.4', 'Lemma 3.5',
    'Lemma 3.6', 'Theorem 3.7', 'Lemma 3.8', 'Lemma 3.10', 'Lemma 3.11',
    'Theorem 3.12', 'Theorem 3.13', 'Lemma 3.15', 'Theorem 3.16', 'Theorem 3.17', 'Algorithm 1'
]

found = {}
for page_num in range(len(doc)):
    text = doc[page_num].get_text()
    for t in terms:
        if t in text:
            if t not in found:
                found[t] = []
            found[t].append(page_num + 1)

for t, pages in found.items():
    print(f'{t}: pages {pages}')

# Let's extract pages around Lemma 3.11, 3.12, 3.15, 3.17, Algorithm 1
for p in set(found.get('Lemma 3.11', []) + found.get('Theorem 3.12', []) + found.get('Lemma 3.15', []) + found.get('Theorem 3.17', []) + found.get('Algorithm 1', [])):
    print(f"\n--- PAGE {p} ---")
    print(doc[p-1].get_text())
