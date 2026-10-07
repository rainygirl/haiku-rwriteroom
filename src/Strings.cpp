#include "Strings.h"

#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>

#include <stdlib.h>
#include <string.h>

#include <map>

namespace {

struct Translation {
	const char* en;
	const char* it;
	const char* ko;
	const char* ja;
};

// Korean is written in 합니다체; Japanese in です/ます.
const Translation kTranslations[] = {
	{ "File", "File", "파일", "ファイル" },
	{ "New", "Nuovo", "새 문서", "新規" },
	{ "Open" "\xE2\x80\xA6", "Apri" "\xE2\x80\xA6", "열기" "\xE2\x80\xA6",
		"開く" "\xE2\x80\xA6" },
	{ "Save", "Salva", "저장", "保存" },
	{ "Save as" "\xE2\x80\xA6", "Salva come" "\xE2\x80\xA6",
		"다른 이름으로 저장" "\xE2\x80\xA6", "名前を付けて保存" "\xE2\x80\xA6" },
	{ "About R WriteRoom", "Informazioni su R WriteRoom", "R WriteRoom 정보",
		"R WriteRoom について" },
	{ "Quit", "Esci", "종료", "終了" },
	{ "Edit", "Modifica", "편집", "編集" },
	{ "Undo", "Annulla", "실행 취소", "元に戻す" },
	{ "Cut", "Taglia", "잘라내기", "切り取り" },
	{ "Copy", "Copia", "복사", "コピー" },
	{ "Paste", "Incolla", "붙여넣기", "貼り付け" },
	{ "Select all", "Seleziona tutto", "모두 선택", "すべて選択" },
	{ "View", "Visualizza", "보기", "表示" },
	{ "Larger text", "Testo più grande", "글자 크게", "文字を大きく" },
	{ "Smaller text", "Testo più piccolo", "글자 작게", "文字を小さく" },
	{ "Keep the menu shown", "Mostra sempre il menu", "메뉴 항상 보이기",
		"メニューを常に表示" },
	{ "Untitled", "Senza titolo", "제목 없음", "無題" },
	{ "%1 words", "%1 parole", "%1단어", "%1 語" },
	{ "edited", "modificato", "수정됨", "変更あり" },
	{ "Save the changes to \"%1\"?", "Salvare le modifiche a \"%1\"?",
		"\"%1\"의 바뀐 내용을 저장하시겠습니까?",
		"\"%1\" の変更を保存しますか?" },
	{ "Cancel", "Annulla", "취소", "キャンセル" },
	{ "Don't save", "Non salvare", "저장 안 함", "保存しない" },
	{ "Could not open \"%1\".", "Impossibile aprire \"%1\".",
		"\"%1\"을(를) 열 수 없습니다.", "\"%1\" を開けませんでした。" },
	{ "Could not save \"%1\".", "Impossibile salvare \"%1\".",
		"\"%1\"을(를) 저장할 수 없습니다.", "\"%1\" を保存できませんでした。" },
	{ "OK", "OK", "확인", "OK" },
	{ "A black page, green letters and nothing else.\n\n"
		"Move the pointer to the top of the screen (or press Esc) for the "
		"menu.",
		"Una pagina nera, lettere verdi e nient'altro.\n\n"
		"Porta il puntatore in cima allo schermo (o premi Esc) per il menu.",
		"까만 종이에 초록 글자, 그 밖에는 아무것도 없습니다.\n\n"
		"마우스를 화면 맨 위로 올리면(또는 Esc를 누르면) 메뉴가 나타납니다.",
		"黒いページに緑の文字、ほかには何もありません。\n\n"
		"ポインターを画面のいちばん上に動かす (または Esc を押す) と"
		"メニューが出ます。" },
};


std::string
DetectLanguage()
{
	const char* forced = getenv("RWR_LANG");
	if (forced != NULL && forced[0] != '\0')
		return std::string(forced, 0, 2);

	// The Locale preferences' own settings file. BLocaleRoster would read the
	// same list, but it lives in liblocale on older Haiku and in libbe on
	// newer, and one binary should run on both.
	BPath path;
	BMessage languages;
	BFile file;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK
		&& path.Append("Locale settings") == B_OK
		&& file.SetTo(path.Path(), B_READ_ONLY) == B_OK
		&& languages.Unflatten(&file) == B_OK) {
		const char* language;
		for (int32 i = 0; languages.FindString("language", i, &language) == B_OK;
				i++) {
			std::string code(language, 0, 2);
			if (code == "en" || code == "it" || code == "ko" || code == "ja")
				return code;
		}
	}
	return "en";
}


const std::string&
Language()
{
	static const std::string language = DetectLanguage();
	return language;
}


const std::map<std::string, const char*>&
Table()
{
	static std::map<std::string, const char*> table;
	if (!table.empty() || Language() == "en")
		return table;
	for (size_t i = 0; i < sizeof(kTranslations) / sizeof(kTranslations[0]);
			i++) {
		const Translation& t = kTranslations[i];
		const char* text = Language() == "it" ? t.it
			: Language() == "ko" ? t.ko : t.ja;
		table[t.en] = text;
	}
	return table;
}

} // namespace


const char*
T(const char* english)
{
	const std::map<std::string, const char*>& table = Table();
	std::map<std::string, const char*>::const_iterator found
		= table.find(english);
	return found == table.end() ? english : found->second;
}


std::string
TF(const char* english, const std::string& a1, const std::string& a2,
	const std::string& a3)
{
	std::string text = T(english);
	const std::string* args[] = { &a1, &a2, &a3 };
	for (size_t pos = 0; (pos = text.find('%', pos)) != std::string::npos;) {
		if (pos + 1 < text.size() && text[pos + 1] >= '1'
			&& text[pos + 1] <= '3') {
			const std::string& value = *args[text[pos + 1] - '1'];
			text.replace(pos, 2, value);
			pos += value.size();
		} else
			pos++;
	}
	return text;
}


const char*
CurrentLanguage()
{
	return Language().c_str();
}
