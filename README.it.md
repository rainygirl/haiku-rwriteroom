<img src="icon.png" width="64" align="left" alt="">

# R WriteRoom per Haiku

[English](README.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [Italiano](README.it.md) · [Français](README.fr.md)

Scrittura a schermo intero, senza distrazioni: lettere verdi e un cursore verde su una pagina nera, e nient'altro. Il menu resta nascosto finché non porti il puntatore in cima allo schermo.

![Scrivere con R WriteRoom](screenshots/writing.png)

![Il menu, visibile mentre il puntatore è in cima](screenshots/menu.png)

- Porta il puntatore sul **bordo superiore** per far scendere il menu: nuovo, apri, salva, salva come, testo più grande o più piccolo, esci. Allontanalo e il menu sparisce di nuovo. **Esc** lo tiene visibile o lo nasconde.
- La barra dei menu mostra anche il nome del file, se ci sono modifiche non salvate e il numero di parole.
- Le scorciatoie funzionano anche con il menu nascosto: **Alt+N** nuovo, **Alt+O** apri, **Alt+S** salva, **Shift+Alt+S** salva come, **Alt+Q** esci, **Alt+Più/Meno** dimensione del testo.
- I file sono testo semplice UTF-8. Se esci con modifiche non salvate, te lo chiede prima.
- Menu in inglese, italiano, coreano e giapponese, secondo le preferenze Locale.

## Installazione

| Haiku | Comandi |
| --- | --- |
| x86 a 32 bit (x86_gcc2) | `pkgman add-repo https://pkgman.rainygirl.com/x86_gcc2`<br>`pkgman install rwriteroom_x86` |
| x86_64 | `pkgman add-repo https://pkgman.rainygirl.com/x86_64`<br>`pkgman install rwriteroom` |
| arm64 (RENKU) | `pkgman install rwriteroom` |

Poi avvia **R WriteRoom** da Deskbar → Applications, oppure apri un file di testo con il menu **Open with** di Tracker.

## Licenza

MIT

## Uso dell'IA

Parti di R WriteRoom sono state sviluppate con l'aiuto di strumenti di programmazione basati sull'IA (Claude di Anthropic). Tutto il codice è stato rivisto e testato dall'autore.
