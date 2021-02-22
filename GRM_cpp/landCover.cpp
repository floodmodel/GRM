
#include "grm.h"

extern cvAtt* cvs;

void calPET_PriestleyTaylor(int i, int dtsec, double dy_m)
{
	//=== �Ʒ� ������� �� ���� �ʿ�
	double satVPG = 0.7;//saturated vapor press gradient
	double a = 1.3;// coefficient
	double g = 1.0; // coefficient of psychrometer
	double rn = 1.0;// net radiation flux
	double lv = 1.0;//Latent Heat Of Vaporization
	double rhow = 999.0; // kg/m3  --> �̰� define ����� �ٲ���
	double kc = 0.7; //�۹����. ���߻� ��� 0.6~0.8
	//====================
	double er = rn / lv / rhow;// evporation ratiio
	double pet_m = er * a * satVPG / (satVPG + g);
	cvs[i].aet_m = pet_m* kc;
}

void calPET_Hargreaves(int i, int dtsec, double dy_m)
{	
	//=== �Ʒ� ������� �� ���� �ʿ�
	double ra = 3.0;// ���¾纹�緮(extraterrestrial radiation) (mm / day)
	double t_max = 15; //���ְ� ���(��)
	double t_min = 8; //������ ���(��)
	double ht = 17.8; //�µ����	
	double kc = 0.7; //�۹����. ���߻� ��� 0.6~0.8
	//====================
	double krs = 0.0023;//���纸�����	
	double he = 0.5;//  Hargreaves ����
	double tm = t_max - t_min;
	double tp = t_max + t_min;	
	double pet_m = krs*ra * pow(tm, he) * (tp /2+ht);
	cvs[i].aet_m = pet_m* kc;
}