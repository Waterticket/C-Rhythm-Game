#define _CRT_SECURE_NO_WARNINGS

//*********************************************//
//**		  RhyThmRinMin Generator		 **//
//**		     리듬린민 생성기			 **//
//**	  	       1307 김영상			     **//
//*********************************************//

// 77번 줄, 선언부에 테스트 맵을 넣어놨습니다
// 맵 바꿔가며 테스트 해주세요!

#include <stdio.h>
#include <Windows.h>
#include <mmsystem.h>
#include <time.h>
#include <math.h>
#include <process.h>
#include "MapReader.h"
#include "resource.h"
#include "Digitalv.h"
#include "Pan.h"
#include "networking.h"

#pragma comment(lib,"Winmm.lib")

#define TRUE 1
#define FALSE 0
#define OEM 2

// 입력 받을 키 선언
#define KEY_A 0x41
#define KEY_B 0x53
#define KEY_C 0x4C
#define KEY_D 0xBA

// 읽을 노트 길이 (ms)
#define READ_NOTE_MIL 580

// 음악 파라메터 선언
MCI_OPEN_PARMS m_mciOpenParms;
MCI_PLAY_PARMS m_mciPlayParms;
DWORD m_dwDeviceID;
MCI_OPEN_PARMS mciOpen;
MCI_PLAY_PARMS mciPlay;
int dwID;

// 사용할 폰트
HFONT font;
HFONT font_combo;
HFONT font_score;
HFONT font_acc;
RECT rt;

// 노트 길이
int note_width = 0;
int note_height = 0;

// 타이머 선언
int PlayTimer = 0;
int map_playing = FALSE;
int beg_time = 0;

// 게임 이미지 위치 설정
int Start_Pos = 150;

// 일시정지 체크 변수
int IsPause = FALSE;

// 키 입력 체크 변수
int KeyDown[4] = { 0 };
int KeyLight[4] = { 0 };

// 콘솔 윈도우 조작을 위한 변수 선언
static HWND hWnd;
static HINSTANCE hInst;

// 맵 이름
// 맵을 골라주세요!
//char* NoteMapName = "Jinjin - pi (Jinjin) [4K Beginner].map";          // 00:33 난이도 최하
//char* NoteMapName = "Halozy - K.O.K.O.R.O (Nepuri) [EZ].map";			// 01:17 난이도 하
//char* NoteMapName = "banshi - Fading Star(banshi-Remix) (erlinadewi-) [Beginner].map"; // 02:33 난이도 중
//char* NoteMapName = "sakuzyo - Scattered Rose (Starry-) [Normal].map"; // 00:57 난이도 상
char* NoteMapName = "sakuzyo - Scattered Rose (Starry-) [Another].map";
//char* NoteMapName = "gems - Gems Pack 13 - LN Master 5th (gemboyong) [69 ^ - Randy Mortimer, Penguin (Pinnacle Remix)].map"; 
																		 // 01:35 난이도 최상, 프레임 테스트용 맵. 막 눌러주세요!

// 스레드 핸들 선언
HANDLE TimerHandle;
HANDLE KeyPressHandle;
HANDLE PanTimeHandle;
HANDLE GameHandle;

//*/////////////////////////////////// = 함수 선언 = ///////////////////////////////////*//

//// 기본 함수 ////
int UserDataLoad();						// 최고 점수 불러오기
void UserDataWrite(int score);			// 최고 점수 기록
void ClearCursor();						// 커서 안뜨게
void* M_Timer(void* a);					// 게임 타이머 (스레드 함수)
void KeyDownProcess(int k);				// 키 입력 처리. k : 키 위치 (0,1,2,3)
void HitNote();							// 노트 클릭 체크
void* CheckKeyPress(void* a);			// 키 눌렀는지 체크 (스레드 함수)
void* Pan_Timer(void* a);				// 판정 표시 시간 타이머
void GameOver();						// 게임 오버 함수 (게임 오버시 실행)
void* GameStatusCheck(void* a);			// 게임 상태 체크 (스레드 함수)
void RT_Change(RECT* rts, int a, int b, int c, int d); // 글자 위치 변경
inline void Render();					// 게임 플레이시 렌더링 함수
void CheckPlayStatus();					//플레이 상태 체크
void LoadMap(char* MapName);			// 맵 불러오기

//// 음악 함수 ////
void MusicLoad();						// 음악 불러오기
void MusicOpen(char* FileName);			// 원하는 음악 불러오기
void MusicPlay(int ID, int repeat);		// 음악 재생
void MusicPause(int ID);				// 음악 일시정지
void MusicResume(int ID);				// 음악 다시 재생
void MusicStop(int ID);					// 음악 완전히 정지
void SetMusicPosition(DWORD dwPosition);// 음악 플레이 위치 지정

//// 게임 진행 함수 ////
void PlayMap();							// 게임 플레이
void ReadyRender();						// 준비화면 렌더링
void ReadyView();						// 준비 뷰
void PauseView();						// 일시정지 뷰
void Restart();							// 재시작 함수
void GameOverView();					// 게임 오버 뷰
void* ScoreIncVisual(int* Score_VS);	// 스코어 부드럽게 올라가게 하기 (스레드 함수)
void Result();							// 결과 출력

//*/////////////////////////////////// = 선언부 종료 = ///////////////////////////////////*//

int UserDataLoad() //최고 점수 불러오기
{
	FILE* fp;
	int index = 0;
	int data;
	
	fp = fopen("score.txt", "r");
	while (fscanf(fp, "%d", &data) != EOF) {
		index = (int)data;
	}
	fclose(fp);
	return index; // 최고점수 리턴
}

void UserDataWrite(int score) // 최고 점수 기록
{
	FILE* fp;

	fp = fopen("score.txt", "w");
	fprintf(fp, "%d\n", score); // 스코어 기록
	fclose(fp);
}

void ClearCursor() { // 커서 안뜨게
	CONSOLE_CURSOR_INFO c = { 0 };
	c.dwSize = 1;
	c.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c);
}

void *M_Timer(void* a) // 게임 타이머 (스레드 함수)
{
	beg_time = GetTickCount64() + 80; // 시작 시간
	while (map_playing) {
		PlayTimer = GetTickCount64() - beg_time; // 플레이 타임 = 현재시간 - 시작시간
		if (IsPause) {  // 일시정지 했을 경우
			int now_time = GetTickCount64(); // 중지된 시간 기록
			while (IsPause)
			{
				continue;
			}
			now_time = GetTickCount64() - now_time;
			beg_time = beg_time + now_time; // 플레이 타임 계산을 위한 시작 시간 수정
		}
	}
	return;
}

void KeyDownProcess(int k) // 키 입력 처리. k : 키 위치 (0,1,2,3)
{
	for (int i = -140; i <= 140; i++) {
		if (PlayTimer + i < 0) continue; //배열 범위 규정을 위한 코드

		if (NotePoints[PlayTimer + i][k] == 1) //노트가 있을 경우
		{
			int abs_v = abs(i);						//오차를 구하기 위해 오차의 절대값

			if (abs_v <= 16) IncKool();				//오차 16ms 이내 : Kool
			else if (abs_v <= 35) IncCool();		//오차 35ms 이내 : Cool
			else if (abs_v <= 85) IncGood();		//오차 85ms 이내 : Good
			else if (abs_v <= 140) IncBad();		//오차 140ms 이내 : Bad
			NotePoints[PlayTimer + i][k] = 0;		//노트 친것으로 표시
			ImagePoints[PlayTimer + i][k] = 0;		//Render 함수에서 해당 노트 표시 안함
			break;
		}
	}
}

void HitNote() // 노트 클릭 체크
{
	if (KeyDown[0]) {    // 현재 해당 키를 눌렀을 경우
		KeyDownProcess(0);   // 참이라면 KeyDownProcess함수 실행
		KeyDown[0] = FALSE;     //  KeyDown 변수를 거짓으로 되돌림
								//  꾹 누르는것을 방지하기 위해서
	}
	if (KeyDown[1]) { // 위와 동일함
		KeyDownProcess(1);
		KeyDown[1] = FALSE;
	}
	if (KeyDown[2]) {
		KeyDownProcess(2);
		KeyDown[2] = FALSE;
	}
	if (KeyDown[3]) {
		KeyDownProcess(3);
		KeyDown[3] = FALSE;
	}

	Rate = (floor(Combo / 10) * 0.85) + 1; // 콤보 보너스 수정
}

void* CheckKeyPress(void* a) // 키 눌렀는지 체크 (스레드 함수)
{
	while (map_playing)
	{
	  //0x0000 : 이전에 누른 적이 없고 호출 시점에도 눌려있지 않은 상태
	  //0x0001 : 이전에 누른 적이 있고 호출 시점에는 눌려있지 않은 상태
	  //0x8000 : 이전에 누른 적이 없고 호출 시점에는 눌려있는 상태
	  //0x8001 : 이전에 누른 적이 있고 호출 시점에도 눌려있는 상태

		if (GetAsyncKeyState(KEY_A) & 0x0001) //Key A를 눌렀을 경우
			KeyDown[0] = TRUE;
		if (GetAsyncKeyState(KEY_B) & 0x0001)
			KeyDown[1] = TRUE;
		if (GetAsyncKeyState(KEY_C) & 0x0001)
			KeyDown[2] = TRUE;
		if (GetAsyncKeyState(KEY_D) & 0x0001)
			KeyDown[3] = TRUE;
		HitNote();
	}
	return;
}

void* Pan_Timer(void* a) //판정 표시 시간 타이머
{
	while (map_playing) {
		if (PanVisualTimer > 0) // 판정 타이머가 0이상 (300ms 이내 키를 눌렀을 경우)
		{
			PanVisualTimer--; //타이머를 1 줄인다
		}
		Sleep(1); //1ms 쉬기
		while (IsPause) continue;
	}
}

void GameOver() // 게임 오버 함수 (게임 오버시 실행)
{
	map_playing = FALSE; //게임을 중지시킴
}

void* GameStatusCheck(void* a) // 게임 상태 체크 (스레드 함수)
{
	ClearCursor();
	while (map_playing) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) //[ESC] 키를 누를 경우
			PauseView(); // 게임 일시정지

		if ((HP <= 0)&& (M_NoFail == 0))  //HP가 0이 될경우 (HP가 0보다 작을경우)
			GameOver(); // 게임오버 (게임 중지)

		for (int i = PlayTimer - 130; i >= PlayTimer - 230; i--) // Miss 판정
		{
			if (i < 0) continue; // 배열을 위한 함수 (배열 인덱스에 -가 들어가는걸 방지)
			for (int j = 0; j < 4; j++)
			{
				if (NotePoints[i][j] == 1) // 치지 않은 노트가 있을 경우
				{
					ImagePoints[i][j] = -1; // 노트 치지 않은것으로 표시
					NotePoints[i][j] = -1; // Render에서 표시되지 않게 마크
					IncMiss(); //Miss 증가 (Pan.c 함수)
				}
			}
		}

		while (IsPause) continue;
	}
	return;
}

void RT_Change(RECT* rts, int a, int b, int c, int d) // 글자 위치 변경
{
	// Render 함수에서 글자 표시시에 위치 지정하는 함수
	rts->left = a;
	rts->top = b;

	rts->right = c;
	rts->bottom = d;
}

inline void Render() // 게임 플레이시 렌더링 함수
{
	hWnd = GetConsoleWindow();		// 자신의 콘솔 윈도우를 가져옴
	hInst = GetModuleHandle(NULL);	// 콘솔 핸들러 가져옴
	HDC hDC, hMemDC;		//표시할 메모리 할당
	static HDC hBackDC;		//표시전 렌더링할 함수 할당 (더블 버퍼링)
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap; //표시할 비트맵
	BITMAP Bitmap;	//비트맵 선언

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1000, 500);		//렌더링 할 팔레트 크기 선언
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);	//표시되는 비트맵

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1)); //노트 이미지 로드
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);

	note_width = Bitmap.bmWidth;
	note_height = Bitmap.bmHeight;
	for (int i = PlayTimer; i < PlayTimer + READ_NOTE_MIL; i++) // 화면에 그릴 노트 범위
	{
		if (PlayTimer < 0) //타이머가 작동하지 않으면 나가기
			break;
		for (int j = 0; j < 4; j++) // 각 키마다 확인
		{
			if (ImagePoints[i][j] == 1) // 위치에 노트가 있을경우
			{
				/*if (i == TimingPoints[0].time + (int)(floor(TimingPoints[0].beatLength / 4.0 * k))) k++; // 최적화 코드였으나 구현 귀찮음으로..
				else continue;*/
				GdiTransparentBlt(hBackDC, j* Bitmap.bmWidth + Start_Pos, (i - PlayTimer - 500)*(-0.9), Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
				// 단노트 표시
			}
			else if (ImagePoints[i][j] == 2) {
				GdiTransparentBlt(hBackDC, j * Bitmap.bmWidth + Start_Pos, (i - PlayTimer - 500) * (-0.9), Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
				// 롱노트 표시
			}
		}
	}
	DeleteObject(hNewBitmap); // 노트 오브젝트 삭제

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3)); // 판정선 로드
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, Start_Pos, 450, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228)); // 그리기
	DeleteObject(hNewBitmap);

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4)); // 노트 양쪽 구분선 로드
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, Start_Pos, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228)); // 왼쪽 그리기
	GdiTransparentBlt(hBackDC, Start_Pos+(note_width*4), 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228)); // 오른쪽 그리기
	DeleteObject(hNewBitmap);

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_HP_BAR)); // HP바 로드
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, Start_Pos + (note_width * 4) + 10, 450-floor((double)((__int64)Bitmap.bmHeight * (HP)) / 1.5), Bitmap.bmWidth, floor(((__int64)Bitmap.bmHeight*(HP))/1.5), hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
			// 5*5 비트맵을 HP에 따라서 늘리고 줄인다.
			// 시작 지점과 이미지 길이를 조절해서 최대한 자연스럽게 꾸민다
	DeleteObject(hNewBitmap);

	if (PanVisualTimer > 0) { // 판정 이미지을 표시해야 할경우 (300ms 이내에 노트를 쳤을경우)
		if (LastPan == KOOL) { // 최근에 친 노트가 Kool 인경우
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_KOOL)); // Kool 이미지 로드
			GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, 235, 350, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228)); // 그리기
			DeleteObject(hNewBitmap);
		}
		else if (LastPan == COOL) { // 이하 같음
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_COOL));
			GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, 235, 350, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
			DeleteObject(hNewBitmap);
		}
		else if (LastPan == GOOD) {
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_GOOD));
			GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, 235, 350, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
			DeleteObject(hNewBitmap);
		}
		else if (LastPan == BAD) {
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_BAD));
			GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, 235, 350, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
			DeleteObject(hNewBitmap);
		}
		else if (LastPan == MISS) {
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_MISS));
			GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, 235, 350, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
			DeleteObject(hNewBitmap);
		}
	}

	char buf[100]; // 표시할 버퍼 생성
	SetTextColor(hBackDC, RGB(255, 255, 255)); // 텍스트 색상 설정
	SetBkMode(hBackDC, TRANSPARENT); // 글자 배경색 투명

	if (Combo > 0) { // 콤보가 0 이상일경우 (콤보가 0일 경우 표시 안하기 위해)
		(HFONT)SelectObject(hBackDC, font_combo); // 폰트를 font_combo로 설정

		sprintf(buf, "%d", Combo); //콤보 buffer에 삽입
		RT_Change(&rt, 10, 110, 500, 160); // 글자 위치 변경
		DrawTextA(hBackDC, buf, -1, &rt, DT_CENTER); // 글자 드로우
	}

	(HFONT)SelectObject(hBackDC, font_score); // 스코어 폰트로 변경

	//이하 동일
	sprintf(buf, "%08d", Score); // 점수 표시
	RT_Change(&rt, 10, 10, 950, 160);
	DrawTextA(hBackDC, buf, -1, &rt, DT_RIGHT);

	(HFONT)SelectObject(hBackDC, font_acc);

	sprintf(buf, "%.2lf%%\n", Accurate); // 정확도 표시
	RT_Change(&rt, 10, 40, 950, 160);
	DrawTextA(hBackDC, buf, -1, &rt, DT_RIGHT);

	BitBlt(hDC, 0, 0, 1000, 500, hBackDC, 0, 0, SRCCOPY); // 백그라운드에서 그린 이미지 콘솔에 그리기

	DeleteObject(SelectObject(hBackDC, hBackBitmap)); // 사용한 오브젝트 정리
	DeleteObject(hNewBitmap);
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC); // 윈도우 할당 해제
}

void CheckPlayStatus() //플레이 상태 체크
{
	if (PlayTimer >= Last_Note_pos + 5000) { // 노트가 끝났을 경우
		map_playing = FALSE; // 게임 끝내기
	}
}

void LoadMap(char * MapName) // 맵 불러오기
{
	char buf[500] = { NULL }; // 버퍼 선언
	if (LoadMapFile(MapName) == 0)
	{
		printf("Load Failed.");
	}
	sprintf(buf, "%s - %s (%s) [%s]", M_MetaData.Artist, M_MetaData.Title, M_MetaData.Creator, M_MetaData.Version); 
			// 음악 제작자, 맵 이름, 맵 제작자, 버전 buffer에 쓰기
	SetConsoleTitle(buf); //버퍼 제목에 쓰기
}

void MusicLoad() // 음악 불러오기
{
	mciOpen.lpstrElementName = M_General.AudioFilename; // 맵에서 선택한 오디오 파일 불러오기
	mciOpen.lpstrDeviceType = "mpegvideo";

	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
		(DWORD)(LPVOID)&mciOpen); // 재생 커맨드 보내기

	dwID = mciOpen.wDeviceID; // 재생 디바이스 ID 설정
}

void MusicOpen(char * FileName) // 원하는 음악 불러오기
{
	mciOpen.lpstrElementName = FileName; // 파일 경로
	mciOpen.lpstrDeviceType = "mpegvideo";

	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
		(DWORD)(LPVOID)&mciOpen); // 재생 커맨드 보내기

	dwID = mciOpen.wDeviceID;
}

void MusicPlay(int ID, int repeat) // 음악 재생
{
	if(repeat == 0) // 반복하지 않을경우
		mciSendCommand(ID, MCI_PLAY, NULL, (DWORD)(LPVOID)&m_mciPlayParms); // 반복하지 않고 재생
	else
		mciSendCommand(ID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&m_mciPlayParms); // 음악이 끝나면 반복됨
}

void MusicPause(int ID) // 음악 일시정지
{
	mciSendCommand(ID, MCI_PAUSE, MCI_NOTIFY, (DWORD)(LPVOID)&mciPlay);
}

void MusicResume(int ID) // 음악 다시 재생
{
	mciSendCommandW(ID, MCI_RESUME, 0, NULL);
}

void MusicStop(int ID) // 음악 완전히 정지
{
	mciSendCommandW(ID, MCI_CLOSE, 0, NULL);
}

void SetMusicPosition(DWORD dwPosition) // 음악 플레이 위치 지정
{
	MCI_SEEK_PARMS mciSeekParams;
	mciSeekParams.dwTo = dwPosition;
	mciSendCommand(dwID, MCI_SEEK, MCI_TO, (DWORD)(LPVOID)&mciSeekParams); // 음악 위치 지정
}

void PlayMap() // 게임 플레이
{
	// 폰트 지정
	font = CreateFont(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Aller");
	font_combo = CreateFont(48, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Slant");
	font_score = CreateFont(36, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Slant");
	font_acc = CreateFont(32, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Cunia");

	char buf[500] = { NULL };

	// 스레드 생성
	TimerHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)M_Timer, 0, 0, NULL); // 타이머 스레드
	KeyPressHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)CheckKeyPress, 0, 0, NULL); // 키입력 스레드
	PanTimeHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)Pan_Timer, 0, 0, NULL); // 판정 표시 타이머 스레드
	GameHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)GameStatusCheck, 0, 0, NULL); // 전반적 게임 관리 스레드

	map_playing = TRUE; // 게임 플레이 시작
	while (map_playing) {
		CheckPlayStatus(); // 게임 상태 체크
		Render(); // 게임 화면 렌더링 시작

		// 게임이 일시정지 됬을경우
		if (IsPause) MusicPause(dwID); //음악 일시정지
		else MusicResume(dwID);        //음악 재생
		while (IsPause) continue;    // 일시정지시 함수 일시정지
	}
}

void ReadyRender() // 준비화면 렌더링
{
	// Render 함수와 중복되는 주석은 달지 않음
	hWnd = GetConsoleWindow();
	hInst = GetModuleHandle(NULL);
	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1280, 720);
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

	char buf[200];
	(HFONT)SelectObject(hBackDC, font_score); // 폰트 설정

	SetTextColor(hBackDC, RGB(255, 255, 255)); // 색 설정
	SetBkMode(hBackDC, TRANSPARENT);

	sprintf(buf, "Map: %s [%s]", M_MetaData.Title, M_MetaData.Version); // 맵 이름, 버전 출력
	RT_Change(&rt, 20, 30, 900, 90);
	DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

	sprintf(buf, "Artist: %s", M_MetaData.Artist); // 음악 제작자 출력
	RT_Change(&rt, 20, 90, 700, 120);
	DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

	sprintf(buf, "Press [ENTER] To Start"); // 엔터누르면 시작 출력
	RT_Change(&rt, 20, 180, 700, 240);
	DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

	sprintf(buf, "Your Best Score : %d", UserDataLoad()); // 기존 최고 스코어 출력
	RT_Change(&rt, 20, 400, 700, 430);
	DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

	BitBlt(hDC, 0, 0, 1280, 720, hBackDC, 0, 0, SRCCOPY); // 이미지 콘솔에 그리기

	DeleteObject(SelectObject(hBackDC, hBackBitmap));
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC);
}

void ReadyView() // 준비 뷰
{
	// 폰트 정의
	font_score = CreateFont(36, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Slant");

	SetMusicPosition(M_General.PreviewTime); //음악 시작시간을 프리뷰 타임에 맞춤
	MusicPlay(dwID, 1); // 음악 재생, 루프
	ReadyRender(); // 준비 렌더링 표시
	while (1)
	{
		if (GetAsyncKeyState(VK_RETURN) & 0x0001) { // 엔터키를 누르면 나가기
			break;
		}
	}

	MusicPause(dwID); // 프리뷰 음악 재생 일시정지
	// 이 함수 이후 본 게임 렌더링 시작
}

void PauseView() // 일시정지 뷰
{
	IsPause = 1; // 일시정지 활성화
	// Render 함수와 중복되는 주석은 달지 않음

	hWnd = GetConsoleWindow();
	hInst = GetModuleHandle(NULL);
	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	while (1) {
		hDC = GetDC(hWnd);

		hMemDC = CreateCompatibleDC(hDC);
		hBackDC = CreateCompatibleDC(hDC);

		hBackBitmap = CreateCompatibleBitmap(hDC, 1280, 720);
		hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

		char buf[200];
		(HFONT)SelectObject(hBackDC, font_score);

		SetTextColor(hBackDC, RGB(255, 255, 255));
		SetBkMode(hBackDC, TRANSPARENT);

		sprintf(buf, "Pause..");
		RT_Change(&rt, 20, 30, 900, 90);
		DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

		sprintf(buf, "Press [ESC] to Resume");
		RT_Change(&rt, 20, 90, 900, 150);
		DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

		BitBlt(hDC, 0, 0, 1280, 720, hBackDC, 0, 0, SRCCOPY);

		DeleteObject(SelectObject(hBackDC, hBackBitmap));
		DeleteDC(hBackDC);
		DeleteDC(hMemDC);

		ReleaseDC(hWnd, hDC);

		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) { // [ESC] 키를 눌렀을 경우
			IsPause = 0; // 일시정지 해제
			break; //함수 나가기
		}
	}
	return; // 다시 원래 함수로 돌아감
}

void Restart() // 재시작 함수
{
	// 변수 초기화
	HP = 100;
	PlayTimer = 0;
	Score = 0;
	PanVisualTimer = 0;
	LastPan = -1;
	Combo = 0;
	MaxCombo = 0;
	Rate = 1.0;
	HPUpRate = 1.0;
	HPDownRate = 1.0;
	Accurate = 100.0;

	Start_Pos = 150;

	for (int i = 0; i < 5; i++)
		TargetPass[i] = 0;

	ClearCursor();
	LoadMap(NoteMapName);   // 맵 재로드
	MusicLoad();			// 음악 재로드

	SetMusicPosition(0);	// 음악 위치 원점으로
	MusicPlay(dwID, 0);     // 루프 없이 음악 재생
	map_playing = TRUE;		// 플레이 중 함수 활성화
	PlayMap();				// 맵 플레이
	Result();				// 결과 출력
}

void GameOverView() // 게임 오버 뷰
{
	// Render 함수와 중복되는 주석은 달지 않음
	hWnd = GetConsoleWindow();
	hInst = GetModuleHandle(NULL);
	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1280, 720);
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

	char buf[200];
	(HFONT)SelectObject(hBackDC, font_score);

	SetTextColor(hBackDC, RGB(255, 255, 255));
	SetBkMode(hBackDC, TRANSPARENT);

	sprintf(buf, "You are Dead..");
	RT_Change(&rt, 20, 30, 900, 90);
	DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

	sprintf(buf, "Press [R] to retry");
	RT_Change(&rt, 20, 90, 900, 150);
	DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

	BitBlt(hDC, 0, 0, 1280, 720, hBackDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hBackDC, hBackBitmap));
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC);
	
	while (1) {
		if (GetAsyncKeyState(0x52) & 0x0001) {  // [R] 키를 누를경우
			Restart();							// 재시작
			break;
		}
	}
}

void* ScoreIncVisual(int* Score_VS) // 스코어 부드럽게 올라가게 하기 (스레드 함수)
{
	for (int i = 0; i < Score; i++) // 스코어 만큼 반복
	{
		(*Score_VS)++;	// 스코어 ++
		if(i%((Score/1400)+1) == 0) Sleep(2); //일정 초에만 Score_VS 출력
			// 곧, 일정 초가 되야만 글자가 반영이 되어 부드럽게 올라가는 연출이 된다
	}
}

void Result() // 결과 출력
{
	if (HP <= 0) { // HP가 0 이하, 죽어서 끝났을 경우
		MusicPause(dwID);	// 음악 종료
		GameOverView();		// 게임오버 뷰 출력
	}
	else { // 성공해서 깼을 경우
		if(UserDataLoad() < Score) UserDataWrite(Score); // 최고점수면 점수 기록
		char* buffs[200] = {NULL};
		sprintf(buffs, "{\"Kool\":%d,\"Cool\":%d,\"Good\":%d,\"Bad\":%d,\"Miss\":%d,\"MaxCombo\":%d,\"Acc\":%.2lf}"
			, TargetPass[KOOL], TargetPass[COOL], TargetPass[GOOD], TargetPass[BAD], TargetPass[MISS], MaxCombo, (double)Accurate);
		send_score("player2", Score, M_MetaData.BeatmapID, buffs);
		MusicOpen("applause.mp3");		// 결과화면 BGM 로드
		MusicPlay(dwID, 0);		// 음악 재생

		int Score_View = 0; // 보이는 스코어 선언
		HANDLE IncHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ScoreIncVisual, &Score_View, 0, NULL);
				// 스코어 부드럽게 연출하기 위해 스레드 실행

		// 폰트 로드
		HFONT fnt_score = CreateFont(36, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Cunia");
		HFONT fnt_result = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Cunia");
		HFONT fnt_combo = CreateFont(36, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Cunia");

		while (1) {
			// Render 함수와 중복되는 주석은 달지 않음
			hWnd = GetConsoleWindow();
			hInst = GetModuleHandle(NULL);
			HDC hDC, hMemDC;
			static HDC hBackDC;
			HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
			BITMAP Bitmap;

			hDC = GetDC(hWnd);

			hMemDC = CreateCompatibleDC(hDC);
			hBackDC = CreateCompatibleDC(hDC);

			hBackBitmap = CreateCompatibleBitmap(hDC, 1280, 720);
			hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

			char buf[200];
			(HFONT)SelectObject(hBackDC, fnt_score);

			SetTextColor(hBackDC, RGB(174, 189, 242));
			SetBkMode(hBackDC, TRANSPARENT);

			if (Score == Score_View) {
				if(Accurate >= 80.0) hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_SCORE_A)); // A 판정
				else if (Accurate >= 70.0) hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_SCORE_B)); // B 판정
				else if (Accurate >= 60.0) hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_SCORE_C)); // C 판정
				else hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_SCORE_D)); // D 판정
				GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
				SelectObject(hMemDC, hNewBitmap);
				GdiTransparentBlt(hBackDC, 580, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228)); // 그리기
				DeleteObject(hNewBitmap);
			}

			sprintf(buf, "Map: %s [%s]", M_MetaData.Title, M_MetaData.Version);
			RT_Change(&rt, 20, 30, 900, 90);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

			sprintf(buf, "Score: %07d", Score_View); // 스코어 출력 (부드럽게)
			RT_Change(&rt, 20, 90, 700, 120);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

			(HFONT)SelectObject(hBackDC, fnt_result);

			SetTextColor(hBackDC, RGB(255,255,255));
			SetBkMode(hBackDC, TRANSPARENT);

			sprintf(buf, "Kool: %d\n", TargetPass[KOOL]); //Kool, Cool... 출력
			RT_Change(&rt, 75, 150, 500, 200);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);
			sprintf(buf, "Cool: %d\n", TargetPass[COOL]);
			RT_Change(&rt, 300, 150, 500, 200);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);
			sprintf(buf, "Good: %d\n", TargetPass[GOOD]);
			RT_Change(&rt, 75, 190, 500, 240);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);
			sprintf(buf, "Bad: %d\n", TargetPass[BAD]);
			RT_Change(&rt, 300, 190, 500, 240);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);
			sprintf(buf, "Miss: %d\n", TargetPass[MISS]);
			RT_Change(&rt, 75, 230, 500, 270);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

			(HFONT)SelectObject(hBackDC, fnt_combo);

			SetTextColor(hBackDC, RGB(255, 255, 255));
			SetBkMode(hBackDC, TRANSPARENT);

			sprintf(buf, "MaxCombo: %d\n", MaxCombo);	// 최대 콤보
			RT_Change(&rt, 75, 300, 500, 370);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

			sprintf(buf, "Accuracy: %.2lf%%\n", (double)Accurate); // 정확도 출력
			RT_Change(&rt, 75, 350, 500, 400);
			DrawTextA(hBackDC, buf, -1, &rt, DT_LEFT);

			BitBlt(hDC, 0, 0, 1280, 720, hBackDC, 0, 0, SRCCOPY); // 이미지 표시

			DeleteObject(SelectObject(hBackDC, hBackBitmap));
			DeleteDC(hBackDC);
			DeleteDC(hMemDC);

			ReleaseDC(hWnd, hDC);
		}
	}
}

int main() {
	//* 선작업 *//
	ClearCursor();					// 커서 안보이게
	LoadMap(NoteMapName);			// 맵 로드
	MusicLoad();					// 음악 로드
	ReadyView();					// 준비 뷰
	
	//* 게임 플레이 *//
	SetMusicPosition(0);			// 음악 위치 처음으로
	MusicPlay(dwID, 0);				// 음악 재생
	PlayMap();						// 게임 플레이

	//* 결과 출력 *//
	Result();						// 결과 화면

	return 0;
}