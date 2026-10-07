<img src="icon.png" width="64" align="left" alt="">

# Haiku용 R WriteRoom

[English](README.md) · [日本語](README.ja.md)

글쓰기에만 집중하는 전체 화면 편집기입니다. 까만 종이 위에 초록 글자와 초록 커서만 나오고, 메뉴는 마우스를 화면 맨 위로 올릴 때까지 숨어 있습니다.

![R WriteRoom에서 글을 쓰는 화면](screenshots/writing.png)

![마우스를 맨 위로 올렸을 때 나타나는 메뉴](screenshots/menu.png)

- 마우스를 **화면 맨 위**로 올리면 메뉴가 내려옵니다. 열기, 저장, 다른 이름으로 저장, 새 문서, 글자 크기, 종료를 고를 수 있습니다. 마우스를 내리면 메뉴가 다시 숨습니다. **Esc**를 누르면 메뉴를 계속 보이게 하거나 다시 숨깁니다.
- 메뉴 막대에는 파일 이름, 저장하지 않은 변경 여부, 단어 수도 나옵니다.
- 메뉴가 숨어 있어도 단축키가 동작합니다. **Alt+N** 새 문서, **Alt+O** 열기, **Alt+S** 저장, **Shift+Alt+S** 다른 이름으로 저장, **Alt+Q** 종료, **Alt+Plus/Minus** 글자 크기.
- 파일은 UTF-8 일반 텍스트입니다. 저장하지 않은 채 종료하면 먼저 묻습니다.
- 메뉴는 영어, 이탈리아어, 한국어, 일본어를 지원하며 Locale 환경설정을 따릅니다.

## pkgman으로 설치

| Haiku | 명령 |
| --- | --- |
| 32비트 x86 (x86_gcc2) | `pkgman add-repo https://pkgman.rainygirl.com/x86_gcc2`<br>`pkgman install rwriteroom_x86` |
| x86_64 | `pkgman add-repo https://pkgman.rainygirl.com/x86_64`<br>`pkgman install rwriteroom` |
| arm64 | `pkgman install rwriteroom` (RENKU 이미지에는 저장소가 이미 등록되어 있습니다) |

설치한 뒤 Deskbar → Applications에서 **R WriteRoom**을 실행하거나, Tracker의 **Open with** 메뉴로 텍스트 파일을 여십시오.

## 소스에서 빌드

Haiku에서 빌드합니다.

```sh
make                # x86_64, arm64
setarch x86 make    # 32비트 x86_gcc2 시스템
```

다른 시스템에서 크로스 빌드하는 방법은 `tools/cross-build.sh`에 있습니다.

## 라이선스

MIT

## AI 사용 고지

R WriteRoom의 일부는 AI 코딩 도구(Anthropic Claude)의 도움을 받아 개발했습니다. 모든 코드는 작성자가 검토하고 테스트했습니다.
