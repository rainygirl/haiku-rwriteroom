#ifndef RWR_EDITOR_VIEW_H
#define RWR_EDITOR_VIEW_H

#include <TextView.h>

class CaretView;

// The page: green text on black, and a green caret.
//
// BTextView draws its caret with InvertRect(), which on a black page is a
// white line. A child view parents cannot draw into covers the caret's spot
// instead: the white line is clipped away and the child is the caret, in
// green. Everything else - typing, input methods, selection, undo,
// clipboard - stays BTextView's own.
class EditorView : public BTextView {
public:
	EditorView();

	void AttachedToWindow() override;
	void Draw(BRect updateRect) override;
	void KeyDown(const char* bytes, int32 numBytes) override;
	void MouseDown(BPoint where) override;
	void MouseMoved(BPoint where, uint32 transit,
		const BMessage* dragMessage) override;
	void MouseUp(BPoint where) override;
	void MessageReceived(BMessage* message) override;
	void MakeFocus(bool focus = true) override;
	void WindowActivated(bool active) override;
	void FrameResized(float width, float height) override;
	void Pulse() override;
	void ScrollTo(BPoint where) override;
	void Select(int32 start, int32 finish) override;

	void SetTextSize(float size);
	float TextSize() const { return fTextSize; }

	bool IsModified() const { return fModified; }
	void SetModified(bool modified);

	// Replaces the text without counting as an edit.
	void SetDocument(const char* text, int32 length);

	int32 WordCount() const;

	static rgb_color Green();
	static rgb_color Black();

protected:
	void InsertText(const char* text, int32 length, int32 offset,
		const text_run_array* runs) override;
	void DeleteText(int32 fromOffset, int32 toOffset) override;

private:
	void _UpdateCaret();
	void _TrackInputMethod(const BMessage* message);
	void _PaintComposing();
	void _ApplyStyle();
	void _NotifyChanged();

	CaretView* fCaret;
	bool fBlinkOn;
	float fTextSize;
	bool fModified;
	bool fLoading;

	// The text an input method is still composing (Korean before the
	// syllable is finished, Japanese before conversion), as byte offsets.
	bool fComposing;
	int32 fComposeStart;
	int32 fComposeEnd;
};

#endif
