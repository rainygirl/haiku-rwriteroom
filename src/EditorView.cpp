#include "EditorView.h"

#include "Common.h"

#include <Application.h>
#include <Font.h>
#include <Window.h>

#include <ctype.h>
#include <math.h>
#include <string.h>


// The caret. Clicks on it belong to the text under it.
class CaretView : public BView {
public:
	CaretView()
		:
		BView(BRect(0, 0, 1, 10), "caret", B_FOLLOW_NONE, B_WILL_DRAW)
	{
		SetViewColor(EditorView::Green());
	}

	void SetOn(bool on)
	{
		SetViewColor(on ? EditorView::Green() : EditorView::Black());
		Invalidate();
	}

	void MouseDown(BPoint where) override
	{
		Parent()->MouseDown(ConvertToParent(where));
	}
};


rgb_color
EditorView::Green()
{
	// The phosphor green of the classic terminals WriteRoom borrowed it from.
	rgb_color color = { 0x33, 0xff, 0x33, 255 };
	return color;
}


rgb_color
EditorView::Black()
{
	rgb_color color = { 0, 0, 0, 255 };
	return color;
}


EditorView::EditorView()
	:
	BTextView("editor", B_WILL_DRAW | B_FRAME_EVENTS | B_PULSE_NEEDED
		| B_NAVIGABLE),
	fCaret(NULL),
	fBlinkOn(true),
	fTextSize(18),
	fModified(false),
	fLoading(false)
{
	SetStylable(false);
	SetWordWrap(true);
	SetDoesUndo(true);
}


void
EditorView::AttachedToWindow()
{
	BTextView::AttachedToWindow();
	SetViewColor(Black());
	SetLowColor(Black());
	_ApplyStyle();

	fCaret = new CaretView();
	AddChild(fCaret);
	_UpdateCaret();
}


void
EditorView::_ApplyStyle()
{
	BFont font(be_fixed_font);
	font.SetSize(fTextSize);
	rgb_color green = Green();
	SetFontAndColor(&font, B_FONT_ALL, &green);
	SetHighColor(green);
	Invalidate();
	_UpdateCaret();
}


void
EditorView::SetTextSize(float size)
{
	if (size < 9)
		size = 9;
	if (size > 48)
		size = 48;
	fTextSize = size;
	_ApplyStyle();
	ScrollToSelection();
}


void
EditorView::KeyDown(const char* bytes, int32 numBytes)
{
	if (numBytes == 1 && bytes[0] == B_ESCAPE) {
		Window()->PostMessage(kMsgToggleBar);
		return;
	}
	// Typing hides the mouse pointer until it moves, as WriteRoom does.
	be_app->ObscureCursor();
	BTextView::KeyDown(bytes, numBytes);
	_UpdateCaret();
}


void
EditorView::MouseDown(BPoint where)
{
	BTextView::MouseDown(where);
	_UpdateCaret();
}


void
EditorView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	BTextView::MouseMoved(where, transit, dragMessage);
	_UpdateCaret();
}


void
EditorView::MouseUp(BPoint where)
{
	BTextView::MouseUp(where);
	_UpdateCaret();
}


void
EditorView::MessageReceived(BMessage* message)
{
	if (message->what == B_MOUSE_WHEEL_CHANGED) {
		// There is no scroll bar to take the wheel, so the page does.
		float delta = message->GetFloat("be:wheel_delta_y", 0);
		font_height fh;
		GetFontHeight(&fh);
		float line = ceilf(fh.ascent + fh.descent + fh.leading);
		float maximum = TextHeight(0, CountLines() - 1)
			+ TextRect().top - Bounds().Height() + 60;
		float y = Bounds().top + delta * line * 3;
		if (y > maximum)
			y = maximum;
		if (y < 0)
			y = 0;
		ScrollTo(BPoint(Bounds().left, y));
		return;
	}
	BTextView::MessageReceived(message);
	_UpdateCaret();
}


void
EditorView::MakeFocus(bool focus)
{
	BTextView::MakeFocus(focus);
	_UpdateCaret();
}


void
EditorView::WindowActivated(bool active)
{
	BTextView::WindowActivated(active);
	_UpdateCaret();
}


void
EditorView::FrameResized(float width, float height)
{
	BTextView::FrameResized(width, height);
	_UpdateCaret();
}


void
EditorView::Pulse()
{
	BTextView::Pulse();
	if (fCaret == NULL || fCaret->IsHidden(fCaret))
		return;
	fBlinkOn = !fBlinkOn;
	fCaret->SetOn(fBlinkOn);
}


void
EditorView::ScrollTo(BPoint where)
{
	BTextView::ScrollTo(where);
	_UpdateCaret();
}


void
EditorView::Select(int32 start, int32 finish)
{
	BTextView::Select(start, finish);
	_UpdateCaret();
}


void
EditorView::InsertText(const char* text, int32 length, int32 offset,
	const text_run_array* runs)
{
	BTextView::InsertText(text, length, offset, runs);
	_NotifyChanged();
}


void
EditorView::DeleteText(int32 fromOffset, int32 toOffset)
{
	BTextView::DeleteText(fromOffset, toOffset);
	_NotifyChanged();
}


void
EditorView::_NotifyChanged()
{
	if (!fLoading && !fModified) {
		fModified = true;
		if (Window() != NULL)
			Window()->PostMessage(kMsgModifiedChanged);
	}
	if (Window() != NULL)
		Window()->PostMessage(kMsgTextChanged);
}


void
EditorView::SetModified(bool modified)
{
	if (fModified == modified)
		return;
	fModified = modified;
	if (Window() != NULL)
		Window()->PostMessage(kMsgModifiedChanged);
}


void
EditorView::SetDocument(const char* text, int32 length)
{
	fLoading = true;
	SetText(text, length);
	fLoading = false;
	Select(0, 0);
	ScrollToSelection();
	SetModified(false);
	if (Window() != NULL)
		Window()->PostMessage(kMsgTextChanged);
}


int32
EditorView::WordCount() const
{
	const char* text = Text();
	int32 length = TextLength();
	int32 words = 0;
	bool inWord = false;
	for (int32 i = 0; i < length; i++) {
		unsigned char c = text[i];
		// Any byte that is not ASCII white space belongs to a word, so
		// Korean and Japanese text counts too (by runs between spaces).
		bool space = c == ' ' || c == '\t' || c == '\n' || c == '\r';
		if (!space && !inWord)
			words++;
		inWord = !space;
	}
	return words;
}


void
EditorView::_UpdateCaret()
{
	if (fCaret == NULL || Window() == NULL)
		return;
	int32 start, end;
	GetSelection(&start, &end);
	bool show = start == end && IsFocus() && Window()->IsActive()
		&& IsEditable();
	if (!show) {
		if (!fCaret->IsHidden(fCaret))
			fCaret->Hide();
		return;
	}
	float height;
	BPoint point = PointAt(start, &height);
	// BTextView's own caret is the 1-pixel column at point.x; this covers it
	// and one more, so the green bar is easy to find.
	fCaret->MoveTo(point.x, point.y);
	fCaret->ResizeTo(1, height - 1);
	fBlinkOn = true;
	fCaret->SetOn(true);
	if (fCaret->IsHidden(fCaret))
		fCaret->Show();
}
