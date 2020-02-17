#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MapReader.h"

int NotePoints[MAX_TSTAMP][M_ROW] = { 0 };		// 노트 클릭 표시
int ImagePoints[MAX_TSTAMP][M_ROW] = { 0 };		// 노트 표시용
int TPoint_array_section = 0;					// 타이밍 포인트 위치
int Last_Note_pos = 0;							// 마지막 노트의 시간
int _KEY_COUNT_ = 4;							// 키 카운트 (4키)

void TPoint(char* TStr)		// 타이밍 포인트 읽어오기
{
	int row[6] = { 0 };
	char buff[200] = {NULL};
	Trim(TStr, buff);		// 양끝 빈 공간 지우기

	char* ptr = strtok(buff, ","); // ','를 기준으로 자르기
	char strs[200];
	int i = 0, key = 0;
	while (ptr != NULL)               // 자른 문자열이 나오지 않을 때까지 반복
	{
		Trim(ptr, strs);			  // 좌우 빈칸 제거
		row[i] = atoi(strs);          // 값 추가
		ptr = strtok(NULL, ",");      // 다음 문자열을 잘라서 포인터를 반환
		i++;
	}

	switch (row[0])					  // 0번째 값 (키 위치)
	{
	case 64:	// 0번째 키
		key = 0;
		break;
	case 192:	// 1번째 키
		key = 1;
		break;
	case 320:	// 2번째 키
		key = 2;
		break;
	case 448:	//3번째 키
		key = 3;
		break;
	}

	NotePoints[row[2]][key] = 1;	  // 노트 추가
	Last_Note_pos = row[2];			  // 마지막 노트 위치 갱신
	if (row[3] == 128) {			  // 롱노트 판정 (0: 단노트, 128: 롱노트)
		for (int n = row[2]; n <= row[5]; n++) // 롱노트 끝날때까지 채우기 : row[5] 까지
			ImagePoints[n][key] = 2;  // 롱노트 채우기
	}
	else {
		ImagePoints[row[2]][key] = 1; // 단노트 채우기
	}
}

int Trim(char* line, char line2[]) // 빈칸 자르기
{
	int len = 0;
	char cpTrim[MAX_STR_LEN];
	int xMan = 0;
	int i;

	len = strlen(line);				// 원본 문자열 길이
	if (len >= MAX_STR_LEN)
	{
		puts("string too long");
		return -1;
	}

	strcpy(cpTrim, line);			// 문자열 복사

	// 앞에거 잘라내기
	for (i = 0; i < len; i++)
	{
		if (cpTrim[i] == ' ' || cpTrim[i] == '\t')
			xMan++;
		else
			break;
	}

	// 뒤에거 잘라내기
	for (i = len - 1; i >= 0; i--)
	{
		if (cpTrim[i] == ' ' || cpTrim[i] == '\t' || cpTrim[i] == '\n')
			cpTrim[i] = '\0';
		else
			break;
	}

	strcpy(line2, cpTrim + xMan);		// line2에 붙여 넣기

	return strlen(line);
}

void ReadProperty_General(char* str)    // General Section 읽기
{
	char nstr[200] = { NULL };
	char* ptr = strtok(str, ":");
	int i = 0;
	Trim(ptr, nstr);
	if (strcmp(nstr, "AudioFilename") == 0) {  // 오디오 파일 이름
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_General.AudioFilename, nstr);
	}
	else if (strcmp(nstr, "AudioLeadIn") == 0) { // 오디오 시작 위치
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		M_General.AudioLeadIn = atoi(ptr);
	}
	else if (strcmp(nstr, "PreviewTime") == 0) { // 미리보기 시작 위치
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		M_General.PreviewTime = atoi(ptr);
	}
	else if (strcmp(nstr, "Countdown") == 0) { // 초반 카운트 다운 여부
		Trim(ptr, nstr);
		ptr = strtok(NULL, ":");
		M_General.Countdown = atoi(ptr);
	}
	else if (strcmp(nstr, "StackLeniency") == 0) { // 플레이 래이턴시
		Trim(ptr, nstr);
		ptr = strtok(NULL, ":");
		M_General.StackLeniency = atof(ptr);
	}
}

void ReadProperty_MetaData(char* str)    // MetaData Section 읽기
{
	char nstr[200] = { NULL };
	char* ptr = strtok(str, ":");
	int i = 0;
	Trim(ptr, nstr);
	if (strcmp(nstr, "Title") == 0) {				// 타이틀 영어
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.Title, nstr);
	}
	else if (strcmp(nstr, "TitleUnicode") == 0) {	// 타이틀 원어
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.TitleUnicode, nstr);
	}
	else if (strcmp(nstr, "Artist") == 0) {			// 아티스트 영어
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.Artist, nstr);
	}
	else if (strcmp(nstr, "ArtistUnicode") == 0) {	// 아티스트 원어
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.ArtistUnicode, nstr);
	}
	else if (strcmp(nstr, "Creator") == 0) {		// 제작자
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.Creator, nstr);
	}
	else if (strcmp(nstr, "Version") == 0) {		// 세부 맵 이름
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.Version, nstr);
	}
	else if (strcmp(nstr, "Source") == 0) {			// 맵 소스
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.Source, nstr);
	}
	else if (strcmp(nstr, "Tags") == 0) {			// 맵 태그
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		strcpy(M_MetaData.Tags, nstr);
	}
	else if (strcmp(nstr, "BeatmapID") == 0) {		// 비트맵 ID
		ptr = strtok(NULL, ":");
		M_MetaData.BeatmapID = atoi(ptr);
	}
	else if (strcmp(nstr, "BeatmapSetID") == 0) {	// 비트맵 셋 ID
		ptr = strtok(NULL, ":");
		M_MetaData.BeatmapSetID = atoi(ptr);
	}
}

void ReadProperty_Difficulty(char* str)    // Difficulty Section 읽기
{
	char nstr[200] = { NULL };
	char* ptr = strtok(str, ":");
	int i = 0;
	Trim(ptr, nstr);
	if (strcmp(nstr, "HPDrainRate") == 0) {				// HP 피통
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		M_Difficulty.HPDrainRate = atof(nstr);
	}
	else if (strcmp(nstr, "CircleSize") == 0) {			// 키 개수
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		M_Difficulty.CircleSize = atof(nstr);
	}
	else if (strcmp(nstr, "OverallDifficulty") == 0) {	// 판정 난이도1
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		M_Difficulty.OverallDifficulty = atof(nstr);
	}
	else if (strcmp(nstr, "ApproachRate") == 0) {		// 판정 난이도2
		ptr = strtok(NULL, ":");
		Trim(ptr, nstr);
		M_Difficulty.ApproachRate = atof(nstr);
	}
}

void ReadProperty_TimingPoint(char* str)    // TimingPoint Section 읽기
{
	//Structure [time,beatLength,meter,sampleSet,sampleIndex,volume,uninherited,effects]
	char* ptr = strtok(str, ",");
	char strs[200];
	int i = 0, key = 0;
	while (ptr != NULL)               // 자른 문자열이 나오지 않을 때까지 반복
	{
		Trim(ptr, strs);			  // 좌우 빈칸 제거
		switch (i)
		{
		case 0:
			TimingPoints[TPoint_array_section].time = atoi(strs);			// 변속 출현 시간
			break;
		case 2:
			TimingPoints[TPoint_array_section].meter = atoi(strs);			// 변속 길이
			break;
		case 5:
			TimingPoints[TPoint_array_section].Volume = atoi(strs);			// 볼륨 조절
			break;
		case 6:
			TimingPoints[TPoint_array_section].uninherited = atoi(strs);	// 상속 여부 (BPM 조절 or 스크롤 속도 조절)
			break;
		case 7:
			TimingPoints[TPoint_array_section].effects = atoi(strs);		// 효과
			break;
		case 1:
			TimingPoints[TPoint_array_section].beatLength = atof(strs);		// 비트 길이
			break;
		}
		ptr = strtok(NULL, ",");      // 다음 문자열을 잘라서 포인터를 반환
		i++;
	}
	TPoint_array_section++;
}

void ReadLine_Check(char* str, int section) {
	// Section 검사

	switch (section)
	{
	case S_GENERAL:
		ReadProperty_General(str);		// 기본 설정
		break;
	case S_EDITOR:
		// Editor (Non-Use)
		break;
	case S_METADATA:
		ReadProperty_MetaData(str);		// 메타 데이터 (제목, 이름 등)
		break;
	case S_DIFFICULTY:
		ReadProperty_Difficulty(str);	// 난이도 설정
		break;
	case S_EVENTS:
		// Events(ex. BG)
		break;
	case S_TIMINGPOINT:
		ReadProperty_TimingPoint(str);	// 타이밍 설정
		break;
	case S_HOBJECT:
		TPoint(str);					// 노트 설정
		break;
	}
}

int LoadMapFile(char* beatmap) // 맵 파일 불러오기
{
	FILE* pFile = NULL;
	pFile = fopen(beatmap, "r");
	if (pFile != NULL)
	{
		char strTemp[MAX_STR_LEN];
		int section = 0;

		while (!feof(pFile))
		{
			fgets(strTemp, sizeof(strTemp), pFile);
			if (strcmp(strTemp, "[General]\n") == 0) { // 기본설정
				section = S_GENERAL;
			}
			else if (strcmp(strTemp, "[Editor]\n") == 0) { // 에디터
				section = S_EDITOR;
			}
			else if (strcmp(strTemp, "[Metadata]\n") == 0) { // 메타 데이터
				section = S_METADATA;
			}
			else if (strcmp(strTemp, "[Difficulty]\n") == 0) { // 난이도
				section = S_DIFFICULTY;
			}
			else if (strcmp(strTemp, "[Events]\n") == 0) { // 이벤트
				section = S_EVENTS;
			}
			else if (strcmp(strTemp, "[TimingPoints]\n") == 0) { // 타이밍 포인트
				section = S_TIMINGPOINT;
			}
			else if (strcmp(strTemp, "[HitObjects]\n") == 0) { // 노트
				section = S_HOBJECT;
			}
			else {
				ReadLine_Check(strTemp, section); // 설정 불러오기
			}
		}
		if (M_General.PreviewTime == -1)
			M_General.PreviewTime = 0;
		fclose(pFile); // 파일 닫기
		return 1;
	}
	else {
		return 0;
	}
}