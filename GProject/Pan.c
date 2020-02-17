#include <Windows.h>
#include "Pan.h"
#include <process.h>

int Score = 0; 
int PanVisualTimer = 0;

int TargetPass[5] = { 0 };
int LastPan = -1;

int M_NoFail = 0;

int HP = 100;
int Combo = 0;
int MaxCombo = 0;
double Rate = 1.0;
double HPUpRate = 1.0;
double HPDownRate = 1.0;
double Accurate = 100.0;

void CalcAcc() // 정확도 계산
{
	Accurate = (((__int64)((__int64)70 * TargetPass[BAD]) + ((__int64)130 * TargetPass[GOOD]) + ((__int64)200 * ((__int64)TargetPass[COOL] + (__int64)TargetPass[KOOL]))) /
		(((__int64)TargetPass[MISS] + TargetPass[BAD] + TargetPass[GOOD] + TargetPass[COOL] + TargetPass[KOOL]) * 2.0));
	// 100.00% 만점
}

void* ScoreIncSoft(int a) // 스코어 부드럽게 증가 (스레드 함수)
{
	for (int i = 0; i <= a; i++)
	{
		Score++;
		if ((i % ((a / 47)+1)) == 0) Sleep(1); // 스코어 부드럽게 증가시키기
	}
	return 0;
}

void IncScore(int s) // 스코어 증가
{
	int sc = (s * Rate); // 스코어 계산
	HANDLE IncHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ScoreIncSoft, sc, 0, NULL); // 부드럽게 증가시키는 스레드 실행
}

void IncKool() // 쿨 증가
{
	TargetPass[KOOL] ++;    // 쿨 입력 
	LastPan = KOOL;			// 판정 표시할것 함수 출력
	PanVisualTimer = MaxVisualPanTime; // 판정 표시 시간 초기화
	Combo++;				// 콤보 ++
	if (Combo >= MaxCombo) MaxCombo++;	// Combo가 MaxCombo보다 크면 MaxCombo 추가
	if (HP + (2.5 * HPUpRate) <= 100) HP += (2.5 * HPUpRate); // HP 회복
	else HP = 100;
	HPUpRate *= 1.15; // HP 상승 가중치
	HPDownRate = 1.0; // HP 하락 가중치 제거
	IncScore(55);	  // 스코어 증가
	CalcAcc();
}

void IncCool() // Cool 증가
{
	// 위 함수와 같은부분 생략
	TargetPass[COOL]++;
	LastPan = COOL;
	PanVisualTimer = MaxVisualPanTime;
	Combo++;
	if (Combo >= MaxCombo) MaxCombo++;
	if (HP + (2 * HPUpRate) <= 100) HP += (2 * HPUpRate);
	else HP = 100;
	HPUpRate *= 1.1;
	HPDownRate = 1.0;
	IncScore(50);
	CalcAcc();
}

void IncGood()
{
	// 위 함수와 같은부분 생략
	TargetPass[GOOD]++;
	LastPan = GOOD;
	PanVisualTimer = MaxVisualPanTime;
	Combo++;
	if (Combo >= MaxCombo) MaxCombo++;
	if (HP + (1 * HPUpRate) <= 100) HP += (1 * HPUpRate);
	else HP = 100;
	HPUpRate *= 1.05;
	HPDownRate = 1.0;
	IncScore(35);
	CalcAcc();
}

void IncBad()
{
	// 위 함수와 같은부분 생략
	TargetPass[BAD]++;
	LastPan = BAD;
	PanVisualTimer = MaxVisualPanTime;
	Combo++;
	if (Combo >= MaxCombo) MaxCombo++;
	HP--;
	HPDownRate *= 1.1;
	IncScore(15);
	CalcAcc();
}

void IncMiss()
{
	// 위 함수와 같은부분 생략
	TargetPass[MISS]++;
	LastPan = MISS;
	PanVisualTimer = MaxVisualPanTime;
	Combo = 0;		// 콤보 초기화
	HP -= (3 * HPDownRate); // HP 차감
	HPDownRate *= 1.1; // HP 차감 가중치
	HPUpRate = 1.0;
	CalcAcc();
}