#include "Settings.h"

#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>

namespace {

bool
SettingsPath(BPath& path)
{
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path, true) != B_OK)
		return false;
	return path.Append("RWriteRoom") == B_OK;
}

} // namespace


void
Settings::Load()
{
	BPath path;
	if (!SettingsPath(path))
		return;
	BFile file(path.Path(), B_READ_ONLY);
	BMessage message;
	if (file.InitCheck() != B_OK || message.Unflatten(&file) != B_OK)
		return;
	float size;
	if (message.FindFloat("textSize", &size) == B_OK)
		textSize = size;
}


void
Settings::Save() const
{
	BPath path;
	if (!SettingsPath(path))
		return;
	BMessage message;
	message.AddFloat("textSize", textSize);
	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() == B_OK)
		message.Flatten(&file);
}
