#ifndef RWR_COMMON_H
#define RWR_COMMON_H

#include <SupportDefs.h>

#define APP_SIGNATURE "application/x-vnd.rainygirl-RWriteRoom"
#define APP_NAME "R WriteRoom"

enum {
	kMsgNew = 'Wnew',
	kMsgOpen = 'Wopn',
	kMsgSave = 'Wsav',
	kMsgSaveAs = 'Wsas',
	kMsgSaveTo = 'Wsto',		// from the save panel
	kMsgLarger = 'Wlrg',
	kMsgSmaller = 'Wsml',
	kMsgToggleBar = 'Wbar',
	kMsgAbout = 'Wabt',
	kMsgCheckPointer = 'Wptr',
	kMsgTextChanged = 'Wtxt',
	kMsgModifiedChanged = 'Wmod'
};

#endif
