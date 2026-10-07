#ifndef RWR_WRITE_WINDOW_H
#define RWR_WRITE_WINDOW_H

#include <Entry.h>
#include <Window.h>

#include <string>

class BFilePanel;
class BMenuBar;
class BMenuItem;
class BMessageRunner;
class EditorView;
class PageView;

// The whole screen: a black page with a column of green text in the middle.
// The menu bar lives above the screen's top edge and comes down while the
// pointer is at the top (or after Esc).
class WriteWindow : public BWindow {
public:
	WriteWindow();
	~WriteWindow() override;

	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;
	void WindowActivated(bool active) override;
	void ScreenChanged(BRect frame, color_space mode) override;

	void OpenRef(const entry_ref& ref);

private:
	enum PendingAction {
		kNothing,
		kQuitAfterSave,
		kNewAfterSave,
		kOpenAfterSave
	};

	void _Layout();
	void _ShowBar(bool show);
	void _CheckPointer();
	void _UpdateStatus();
	bool _Save(const entry_ref* ref);
	void _ShowSavePanel();
	// Asks about unsaved changes. True if it is fine to go on now; false if
	// the user cancelled or a save panel is open (then `then` runs after it).
	bool _ConfirmDiscard(PendingAction then);
	void _RunPending();
	void _New();
	std::string _DocumentName() const;

	PageView* fPage;
	EditorView* fEditor;
	BMenuBar* fMenuBar;
	BMenuItem* fStatusItem;
	BMenuItem* fPinItem;
	BMessageRunner* fPointerRunner;
	BFilePanel* fOpenPanel;
	BFilePanel* fSavePanel;

	entry_ref fRef;
	bool fHasRef;
	bool fBarShown;
	bool fPinned;
	PendingAction fPending;
	entry_ref fPendingRef;
};

#endif
