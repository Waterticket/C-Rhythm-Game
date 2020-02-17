#pragma once

#define MAX_STR_LEN 500

#define S_GENERAL 1
#define S_EDITOR 2
#define S_METADATA 3
#define S_DIFFICULTY 4
#define S_EVENTS 5
#define S_TIMINGPOINT 6
#define S_HOBJECT 7

#define MAX_TSTAMP 300000
#define M_ROW 4
#define MAX_PSTAMP 2000

struct General_Set {
	char AudioFilename[100];
	int AudioLeadIn;
	int PreviewTime;
	int Countdown;
	float StackLeniency;
}M_General;

struct MetaData_Set {
	char Title[300];
	char TitleUnicode[300];
	char Artist[100];
	char ArtistUnicode[100];
	char Creator[100];
	char Version[100];
	char Source[200];
	char Tags[200];
	int BeatmapID;
	int BeatmapSetID;
}M_MetaData;

struct Difficulty_Set {
	float HPDrainRate;
	int CircleSize;
	float OverallDifficulty;
	int ApproachRate;
}M_Difficulty;

struct TimingPoint_Set {
	int time;
	double beatLength;
	int meter;
	int Volume;
	int uninherited;
	int effects;
}TimingPoints[MAX_PSTAMP];

extern int NotePoints[MAX_TSTAMP][M_ROW];
extern int ImagePoints[MAX_TSTAMP][M_ROW];
extern int TPoint_array_section;
extern int Last_Note_pos;
extern int _KEY_COUNT_;

void TPoint(char* TStr);
int Trim(char* line, char line2[]);
void ReadProperty_General(char* str);
void ReadProperty_MetaData(char* str);
void ReadProperty_TimingPoint(char* str);
void ReadProperty_Difficulty(char* str);
void ReadLine_Check(char* str, int section);
int LoadMapFile(char* beatmap);