#include "Common.h"
#include "WriteWindow.h"

#include <Application.h>
#include <Entry.h>


class App : public BApplication {
public:
	App()
		:
		BApplication(APP_SIGNATURE)
	{
		// Made here, not in ReadyToRun: a file opened with the app arrives
		// as RefsReceived before that.
		fWindow = new WriteWindow();
		fWindow->Show();
	}

	void ArgvReceived(int32 argc, char** argv) override
	{
		entry_ref ref;
		if (argc > 1 && get_ref_for_path(argv[1], &ref) == B_OK) {
			BMessage message(B_REFS_RECEIVED);
			message.AddRef("refs", &ref);
			fWindow->PostMessage(&message);
		}
	}

	void RefsReceived(BMessage* message) override
	{
		fWindow->PostMessage(message);
	}

private:
	WriteWindow* fWindow;
};


int
main()
{
	App app;
	app.Run();
	return 0;
}
