<img src="icon.png" width="64" align="left" alt="">

# R WriteRoom for Haiku

[한국어](README.ko.md) · [日本語](README.ja.md)

Full-screen, distraction-free writing: green letters and a green cursor on a black page, and nothing else. The menu stays out of sight until you move the pointer to the top of the screen.

![Writing in R WriteRoom](screenshots/writing.png)

![The menu, shown while the pointer is at the top](screenshots/menu.png)

- Move the pointer to the **top edge** to bring the menu down: open, save, save as, new, larger or smaller text, quit. Move it away and the menu goes again. **Esc** keeps it shown or hides it.
- The menu bar also shows the file's name, whether it has unsaved changes and the word count.
- Shortcuts work while the menu is hidden: **Alt+N** new, **Alt+O** open, **Alt+S** save, **Shift+Alt+S** save as, **Alt+Q** quit, **Alt+Plus/Minus** text size.
- Files are plain UTF-8 text. Quitting with unsaved changes asks first.
- Menus in English, Italian, Korean and Japanese, following the Locale preferences.

## Install with pkgman

| Haiku | Commands |
| --- | --- |
| 32-bit x86 (x86_gcc2) | `pkgman add-repo https://pkgman.rainygirl.com/x86_gcc2`<br>`pkgman install rwriteroom_x86` |
| x86_64 | `pkgman add-repo https://pkgman.rainygirl.com/x86_64`<br>`pkgman install rwriteroom` |
| arm64 | `pkgman install rwriteroom` (RENKU images have the repository already) |

Then start **R WriteRoom** from Deskbar → Applications, or open a text file with it from Tracker's **Open with** menu.

## Build from source

On Haiku:

```sh
make                # x86_64, arm64
setarch x86 make    # 32-bit x86_gcc2 systems
```

Cross-building from another system is described in `tools/cross-build.sh`.

## License

MIT

## AI disclosure

Parts of R WriteRoom were developed with the assistance of AI coding tools (Anthropic's Claude). All code has been reviewed and tested by the author.
