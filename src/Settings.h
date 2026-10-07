#ifndef RWR_SETTINGS_H
#define RWR_SETTINGS_H

struct Settings {
	float textSize = 18;

	void Load();
	void Save() const;
};

#endif
