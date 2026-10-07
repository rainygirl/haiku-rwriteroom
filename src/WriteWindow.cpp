#include "WriteWindow.h"

#include "Common.h"
#include "EditorView.h"
#include "Settings.h"
#include "Strings.h"

#include <Alert.h>
#include <Application.h>
#include <File.h>
#include <FilePanel.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <MessageRunner.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Screen.h>

#include <stdio.h>

#include <string>

namespace {

// How far the text column may grow, in characters, and the page's margins.
const float kColumnChars = 72;
const float kSideMargin = 40;
const float kTopMargin = 70;
// The pointer counts as "at the top" this close to the edge.
const float kRevealZone = 4;

// Open menus (their windows), so the bar is not pulled away while one is
// down and the pointer is over its items far below the bar.
int32 sOpenMenus = 0;


class TrackedMenu : public BMenu {
public:
	explicit TrackedMenu(const char* name) : BMenu(name) {}

	void AttachedToWindow() override
	{
		BMenu::AttachedToWindow();
		atomic_add(&sOpenMenus, 1);
	}

	void DetachedFromWindow() override
	{
		BMenu::DetachedFromWindow();
		atomic_add(&sOpenMenus, -1);
	}
};


std::string
WithCommas(int32 value)
{
	char digits[32];
	snprintf(digits, sizeof(digits), "%d", (int)value);
	std::string text = digits;
	for (int i = (int)text.size() - 3; i > 0; i -= 3)
		text.insert(i, ",");
	return text;
}

} // namespace


// The black page under everything; the menu bar is its child.
class PageView : public BView {
public:
	PageView(BRect frame)
		:
		BView(frame, "page", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS)
	{
		SetViewColor(EditorView::Black());
	}

	void FrameResized(float width, float height) override
	{
		Window()->PostMessage('Wlay');
	}

	void MouseDown(BPoint where) override
	{
		// A click in the margins still means "write".
		if (BView* editor = FindView("editor"))
			editor->MakeFocus(true);
	}
};


WriteWindow::WriteWindow()
	:
	BWindow(BScreen().Frame(), APP_NAME, B_NO_BORDER_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL,
		B_NOT_MOVABLE | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE
			| B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE),
	fPointerRunner(NULL),
	fOpenPanel(NULL),
	fSavePanel(NULL),
	fHasRef(false),
	fBarShown(false),
	fPinned(false),
	fPending(kNothing)
{
	Settings settings;
	settings.Load();

	fPage = new PageView(Bounds());
	AddChild(fPage);

	fEditor = new EditorView();
	fEditor->SetTextSize(settings.textSize);
	fPage->AddChild(fEditor);

	fMenuBar = new BMenuBar(BRect(0, 0, Bounds().Width(), 20), "menu bar");
	BMenu* file = new TrackedMenu(T("File"));
	file->AddItem(new BMenuItem(T("New"), new BMessage(kMsgNew), 'N'));
	file->AddItem(new BMenuItem(T("Open" "\xE2\x80\xA6"),
		new BMessage(kMsgOpen), 'O'));
	file->AddItem(new BMenuItem(T("Save"), new BMessage(kMsgSave), 'S'));
	file->AddItem(new BMenuItem(T("Save as" "\xE2\x80\xA6"),
		new BMessage(kMsgSaveAs), 'S', B_SHIFT_KEY));
	file->AddSeparatorItem();
	file->AddItem(new BMenuItem(T("About R WriteRoom"),
		new BMessage(kMsgAbout)));
	file->AddSeparatorItem();
	file->AddItem(new BMenuItem(T("Quit"), new BMessage(B_QUIT_REQUESTED),
		'Q'));
	fMenuBar->AddItem(file);

	BMenu* edit = new TrackedMenu(T("Edit"));
	edit->AddItem(new BMenuItem(T("Undo"), new BMessage(B_UNDO), 'Z'));
	edit->AddSeparatorItem();
	edit->AddItem(new BMenuItem(T("Cut"), new BMessage(B_CUT), 'X'));
	edit->AddItem(new BMenuItem(T("Copy"), new BMessage(B_COPY), 'C'));
	edit->AddItem(new BMenuItem(T("Paste"), new BMessage(B_PASTE), 'V'));
	edit->AddSeparatorItem();
	edit->AddItem(new BMenuItem(T("Select all"), new BMessage(B_SELECT_ALL),
		'A'));
	edit->SetTargetForItems(fEditor);
	fMenuBar->AddItem(edit);

	BMenu* view = new TrackedMenu(T("View"));
	view->AddItem(new BMenuItem(T("Larger text"), new BMessage(kMsgLarger),
		'+'));
	view->AddItem(new BMenuItem(T("Smaller text"), new BMessage(kMsgSmaller),
		'-'));
	view->AddSeparatorItem();
	// Esc toggles it too (EditorView); a menu shortcut always needs Alt.
	fPinItem = new BMenuItem(T("Keep the menu shown"),
		new BMessage(kMsgToggleBar));
	view->AddItem(fPinItem);
	fMenuBar->AddItem(view);

	// The document's name and word count, as a disabled item at the end.
	fStatusItem = new BMenuItem("", NULL);
	fStatusItem->SetEnabled(false);
	fMenuBar->AddItem(fStatusItem);

	fPage->AddChild(fMenuBar);
	fMenuBar->ResizeToPreferred();
	fMenuBar->ResizeTo(Bounds().Width(), fMenuBar->Frame().Height());
	fMenuBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	_ShowBar(false);

	_Layout();
	_UpdateStatus();
	fEditor->MakeFocus(true);

	BMessage check(kMsgCheckPointer);
	fPointerRunner = new BMessageRunner(BMessenger(this), &check, 100000);
	SetPulseRate(500000);
}


WriteWindow::~WriteWindow()
{
	delete fPointerRunner;
	delete fOpenPanel;
	delete fSavePanel;
}


void
WriteWindow::_Layout()
{
	BRect bounds = fPage->Bounds();
	float charWidth = fEditor->StringWidth("M");
	float width = charWidth * kColumnChars;
	if (width > bounds.Width() - kSideMargin * 2)
		width = bounds.Width() - kSideMargin * 2;
	float left = floorf((bounds.Width() - width) / 2);
	// Full height, so the text scrolls from edge to edge; the insets keep
	// the first and last lines away from the edges.
	fEditor->MoveTo(left, 0);
	fEditor->ResizeTo(width, bounds.Height());
	fEditor->SetInsets(0, kTopMargin, 0, kTopMargin);
}


void
WriteWindow::_ShowBar(bool show)
{
	// IsHidden(view) is the bar's own state; plain IsHidden() is also true
	// while the window has not been shown, which skipped the first Hide().
	bool hidden = fMenuBar->IsHidden(fMenuBar);
	fBarShown = show;
	if (show && hidden)
		fMenuBar->Show();
	else if (!show && !hidden)
		fMenuBar->Hide();
}


void
WriteWindow::_CheckPointer()
{
	if (!IsActive() || fPinned)
		return;
	BPoint where;
	uint32 buttons;
	fPage->GetMouse(&where, &buttons, false);
	if (!fBarShown) {
		if (where.y <= kRevealZone && where.y >= -1)
			_ShowBar(true);
		return;
	}
	float barBottom = fMenuBar->Frame().bottom;
	if (where.y > barBottom + 30 && atomic_get(&sOpenMenus) == 0
		&& buttons == 0)
		_ShowBar(false);
}


void
WriteWindow::_UpdateStatus()
{
	std::string status = _DocumentName();
	if (fEditor->IsModified())
		status += std::string(" (") + T("edited") + ")";
	status += "   " + TF("%1 words", WithCommas(fEditor->WordCount()));
	fStatusItem->SetLabel(status.c_str());
	SetTitle((_DocumentName() + " - " APP_NAME).c_str());
}


std::string
WriteWindow::_DocumentName() const
{
	return fHasRef ? std::string(fRef.name) : std::string(T("Untitled"));
}


void
WriteWindow::OpenRef(const entry_ref& ref)
{
	BEntry entry(&ref, true);
	entry_ref resolved;
	entry.GetRef(&resolved);
	BFile file(&resolved, B_READ_ONLY);
	off_t size = 0;
	if (file.InitCheck() != B_OK || file.GetSize(&size) != B_OK
		|| size > 64 * 1024 * 1024) {
		BAlert* alert = new BAlert(APP_NAME,
			TF("Could not open \"%1\".", resolved.name).c_str(), T("OK"),
			NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go(NULL);
		return;
	}
	std::string text((size_t)size, '\0');
	ssize_t got = size > 0 ? file.Read(&text[0], size) : 0;
	if (got < 0)
		got = 0;
	text.resize((size_t)got);
	fEditor->SetDocument(text.c_str(), (int32)text.size());
	fRef = resolved;
	fHasRef = true;
	_UpdateStatus();
	fEditor->MakeFocus(true);
}


bool
WriteWindow::_Save(const entry_ref* ref)
{
	BFile file(ref, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	ssize_t length = fEditor->TextLength();
	if (file.InitCheck() != B_OK
		|| file.Write(fEditor->Text(), length) != length) {
		BAlert* alert = new BAlert(APP_NAME,
			TF("Could not save \"%1\".", ref->name).c_str(), T("OK"), NULL,
			NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go(NULL);
		return false;
	}
	// A new file gets a type, so Tracker opens it as text; an existing
	// file keeps whatever it had.
	BNodeInfo info(&file);
	char type[B_MIME_TYPE_LENGTH];
	if (info.GetType(type) != B_OK)
		info.SetType("text/plain");
	fRef = *ref;
	fHasRef = true;
	fEditor->SetModified(false);
	_UpdateStatus();
	return true;
}


void
WriteWindow::_ShowSavePanel()
{
	if (fSavePanel == NULL) {
		BMessenger self(this);
		fSavePanel = new BFilePanel(B_SAVE_PANEL, &self, NULL, 0, false,
			new BMessage(kMsgSaveTo));
	}
	if (fHasRef) {
		BEntry entry(&fRef);
		BEntry parent;
		entry_ref parentRef;
		if (entry.GetParent(&parent) == B_OK
			&& parent.GetRef(&parentRef) == B_OK)
			fSavePanel->SetPanelDirectory(&parentRef);
		fSavePanel->SetSaveText(fRef.name);
	} else
		fSavePanel->SetSaveText((std::string(T("Untitled")) + ".txt").c_str());
	fSavePanel->Show();
}


bool
WriteWindow::_ConfirmDiscard(PendingAction then)
{
	if (!fEditor->IsModified())
		return true;
	BAlert* alert = new BAlert(APP_NAME,
		TF("Save the changes to \"%1\"?", _DocumentName()).c_str(),
		T("Cancel"), T("Don't save"), T("Save"), B_WIDTH_AS_USUAL,
		B_OFFSET_SPACING, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	int32 choice = alert->Go();
	if (choice == 0)
		return false;
	if (choice == 1)
		return true;
	if (fHasRef)
		return _Save(&fRef);
	fPending = then;
	_ShowSavePanel();
	return false;
}


void
WriteWindow::_RunPending()
{
	PendingAction action = fPending;
	fPending = kNothing;
	switch (action) {
		case kQuitAfterSave:
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		case kNewAfterSave:
			_New();
			break;
		case kOpenAfterSave:
			OpenRef(fPendingRef);
			break;
		case kNothing:
			break;
	}
}


void
WriteWindow::_New()
{
	fEditor->SetDocument("", 0);
	fHasRef = false;
	_UpdateStatus();
	fEditor->MakeFocus(true);
}


void
WriteWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case 'Wlay':
			_Layout();
			break;

		case kMsgCheckPointer:
			_CheckPointer();
			break;

		case kMsgToggleBar:
			fPinned = !fPinned;
			fPinItem->SetMarked(fPinned);
			_ShowBar(fPinned);
			break;

		case kMsgNew:
			if (_ConfirmDiscard(kNewAfterSave))
				_New();
			break;

		case kMsgOpen:
			if (fOpenPanel == NULL) {
				BMessenger self(this);
				fOpenPanel = new BFilePanel(B_OPEN_PANEL, &self, NULL,
					B_FILE_NODE, false);
			}
			fOpenPanel->Show();
			break;

		case B_REFS_RECEIVED:
		case B_SIMPLE_DATA:
		{
			entry_ref ref;
			if (message->FindRef("refs", &ref) != B_OK)
				break;
			if (_ConfirmDiscard(kOpenAfterSave))
				OpenRef(ref);
			else
				fPendingRef = ref;
			break;
		}

		case kMsgSave:
			if (fHasRef)
				_Save(&fRef);
			else
				_ShowSavePanel();
			break;

		case kMsgSaveAs:
			_ShowSavePanel();
			break;

		case kMsgSaveTo:
		{
			entry_ref directory;
			const char* name;
			if (message->FindRef("directory", &directory) != B_OK
				|| message->FindString("name", &name) != B_OK)
				break;
			BPath path(&directory);
			path.Append(name);
			entry_ref ref;
			if (get_ref_for_path(path.Path(), &ref) == B_OK && _Save(&ref))
				_RunPending();
			break;
		}

		case B_CANCEL:
			// A panel closed; a save that was asked for did not happen.
			fPending = kNothing;
			break;

		case kMsgLarger:
		case kMsgSmaller:
		{
			float size = fEditor->TextSize()
				+ (message->what == kMsgLarger ? 2 : -2);
			fEditor->SetTextSize(size);
			_Layout();
			Settings settings;
			settings.Load();
			settings.textSize = fEditor->TextSize();
			settings.Save();
			break;
		}

		case kMsgAbout:
		{
			BAlert* alert = new BAlert(APP_NAME, (std::string(APP_NAME "\n\n")
				+ T("A black page, green letters and nothing else.\n\n"
					"Move the pointer to the top of the screen (or press "
					"Esc) for the menu.")
				+ "\n\nCopyright 2026 Lee JunHaeng. MIT License.").c_str(),
				T("OK"));
			alert->Go(NULL);
			break;
		}

		case kMsgTextChanged:
		case kMsgModifiedChanged:
			_UpdateStatus();
			break;

		default:
			BWindow::MessageReceived(message);
	}
}


bool
WriteWindow::QuitRequested()
{
	if (fPending == kQuitAfterSave)
		return false;
	if (!_ConfirmDiscard(kQuitAfterSave))
		return false;
	return true;
}


void
WriteWindow::WindowActivated(bool active)
{
	BWindow::WindowActivated(active);
	if (!active && !fPinned && atomic_get(&sOpenMenus) == 0)
		_ShowBar(false);
}


void
WriteWindow::ScreenChanged(BRect frame, color_space mode)
{
	// Resolution changes: stay a full-screen page.
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(), frame.Height());
}
