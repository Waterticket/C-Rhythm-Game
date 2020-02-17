#pragma once

#define MISS 0
#define BAD 1
#define GOOD 2
#define COOL 3
#define KOOL 4

#define MaxVisualPanTime 300

extern int Score;
extern int PanVisualTimer;
extern int TargetPass[5];
extern int LastPan;

extern int M_NoFail;

extern int HP;
extern int Combo;
extern int MaxCombo;
extern double Rate;

extern double Accurate;

double HPUpRate;
double HPDownRate;

void* ScoreIncSoft(int a);
void IncScore(int s);
void IncKool();
void IncCool();
void IncGood();
void IncBad();
void IncMiss();