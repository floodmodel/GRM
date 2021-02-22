
#include "grm.h"

extern cvAtt* cvs;

void calPET_PriestleyTaylor(int i, int dtsec, double dy_m)
{
	//=== 아래 계수들은 값 수정 필요
	double satVPG = 0.7;//saturated vapor press gradient
	double a = 1.3;// coefficient
	double g = 1.0; // coefficient of psychrometer
	double rn = 1.0;// net radiation flux
	double lv = 1.0;//Latent Heat Of Vaporization
	double rhow = 999.0; // kg/m3  --> 이건 define 상수로 바꾸자
	double kc = 0.7; //작물계수. 증발산 계수 0.6~0.8
	//====================
	double er = rn / lv / rhow;// evporation ratiio
	double pet_m = er * a * satVPG / (satVPG + g);
	cvs[i].aet_m = pet_m* kc;
}

void calPET_Hargreaves(int i, int dtsec, double dy_m)
{	
	//=== 아래 계수들은 값 수정 필요
	double ra = 3.0;// 일태양복사량(extraterrestrial radiation) (mm / day)
	double t_max = 15; //일최고 기온(℃)
	double t_min = 8; //일최저 기온(℃)
	double ht = 17.8; //온도계수	
	double kc = 0.7; //작물계수. 증발산 계수 0.6~0.8
	//====================
	double krs = 0.0023;//복사보정계수	
	double he = 0.5;//  Hargreaves 지수
	double tm = t_max - t_min;
	double tp = t_max + t_min;	
	double pet_m = krs*ra * pow(tm, he) * (tp /2+ht);
	cvs[i].aet_m = pet_m* kc;
}