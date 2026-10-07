# R WriteRoom -- development notes

A WriteRoom for Haiku: one borderless window the size of the screen, a black
page, green text in a centred column, a green caret, and a menu bar that is
hidden until the pointer reaches the top edge. The README is installation
only; everything else lives here.

## Layout

```
src/App.cpp          BApplication; argv and Tracker "Open with" refs
src/WriteWindow.*    the full-screen window, hidden menu bar, open/save/quit
src/EditorView.*     BTextView subclass: colours, green caret, wheel, Esc
src/Strings.*        en/it/ko/ja table (same scheme as R InstallWizard)
src/Settings.*       text size, ~/config/settings/RWriteRoom
src/app.rdef         signature, version, file_types (text), vector icon
tools/make_icon.py   writes the vector_icon block (rworldradio's box)
tools/cross-build.sh arm64 / x86 / x86_64 cross builds
```

## How things are done, and why

- **Full screen** is a `B_NO_BORDER_WINDOW_LOOK` window with the screen's
  frame, not movable or resizable; `ScreenChanged()` follows resolution
  changes. Deskbar is an ordinary window unless set "always on top", so the
  page covers it.
- **The green caret.** BTextView draws its caret with `InvertRect()`, which
  on black is white, and `_DrawCaret` is private. A child view (`CaretView`)
  sits where the caret is: a parent cannot draw into a child's area, so the
  white line is clipped away and the child, painted green, is the caret. It
  blinks by changing colour (green/black), never by hiding, because hiding
  it would uncover BTextView's own caret for half a blink. It is hidden only
  when BTextView shows no caret either (a selection, no focus, inactive
  window). Everything else - typing, input methods (Korean, Japanese),
  undo, selection, clipboard - stays BTextView's. `_UpdateCaret()` runs
  after every key, mouse event, select, scroll and resize.
- **Colours** are set after `BTextView::AttachedToWindow()`; BTextView only
  re-adopts system colours while it still has them, so explicit black/green
  stay.
- **Hidden menu.** A 100 ms BMessageRunner asks `GetMouse()`: within 4 px of
  the top the bar is shown; more than 30 px below it, with no button down
  and no menu open, it is hidden again. Open menus are counted by a BMenu
  subclass (`AttachedToWindow`/`DetachedFromWindow` of the menu's window), so
  the bar is not pulled away while the pointer is over a menu's items.
  Esc (handled in EditorView: a BMenuItem shortcut always needs Alt) pins it.
  Menu shortcuts work while the bar is hidden.
- **`IsHidden(view)` versus `IsHidden()`.** Plain `IsHidden()` is also true
  while the window has not been shown yet, so the first `Hide()` of the bar
  was skipped and it started out visible. Both the bar and the caret check
  `IsHidden(self)`.
- **Status** (file name, "edited", word count) is a disabled menu item at the
  end of the bar. Words are runs between ASCII white space, so Korean and
  Japanese count by spacing.
- **Saving** writes UTF-8 and gives a new file `text/plain`; an existing
  file keeps its type. Unsaved changes are asked about on New, Open and
  Quit; when the document is untitled the save panel opens and the pending
  action runs after the save (`fPending`).
- Typing calls `be_app->ObscureCursor()`, as WriteRoom hides the pointer.

## Building

`make` on Haiku (`setarch x86 make` on a gcc2 hybrid). Cross builds, each
leaving `dist/<arch>/RWriteRoom` with resources:

```sh
tools/cross-build.sh arm64            # this Mac's haiku-builder container
tools/cross-build.sh x86_64           # haiku/cross-compiler:x86_64-r1beta4
tools/cross-build.sh x86              # MacMiniM4, vaio-p-builder
X86_VIA=image tools/cross-build.sh x86   # or haiku/cross-compiler:x86_gcc2h-r1beta4 here
```

The arm64 build uses the tree's unpacked `haiku_devel` when it is there.
When the tree is between builds (2026-10-07 it was being rebuilt by another
session and `packages_build/minimum` held no devel package), the script
makes its own sysroot: headers from the source tree, libbe/libroot/libtracker
and the crt glue from the object tree, and `start_dyn.o` compiled from
`src/system/glue/start_dyn.c` with the private system headers.

## Testing

QEMU, never the user's machines:
- arm64: MacMiniM4's `~/Workspace/renku-arm64/test-media.image` with
  `snapshot=on` (another session's VM was using `renku-arm64-media.image`).
  That image has no monospace or CJK font, so text there is proportional and
  Korean is blank; that is the image, not the app.
- x86_64: `haiku-master-hrev60177-x86_64-anyboot.iso`, TCG.
- x86: `haiku-vaio-p-patched-v156.iso` (gcc2 hybrid), `qemu-system-i386`.
Test files go in on a read-only `fat:` USB drive (Deskbar > Mount).

Verified (2026-10-07): full screen over Deskbar, menu hidden at start, shown
at the top edge and hidden again below it, File menu open, green blinking
caret with no white one, typing and Korean text, Alt+S saving (status loses
"edited"), quit asking to save, save panel for an untitled document, the
saved bytes. arm64, x86_64 and x86.

## AI disclosure

Parts of this program and these notes were produced with Claude.
