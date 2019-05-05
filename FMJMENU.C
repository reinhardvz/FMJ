//
// File : FMJMENU.C
// Note : Full Metal Jacket�� �޴� ���� �Լ���.
//

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>

#include "keys.h"
#include "modplay.h"

//= Define ===========================================================
#define MENUFONTNUM     99          // FMJ�� ��Ʈ ����.
#define MENUWEAPNUM     40          // FMJ�� ���� ����.

#define _ARROW_ 3
#define _ESC_   4
#define _ENTER_ 5
#define _BAND_  7
#define _SLD_   8

//= Typedef ==========================================================

typedef unsigned char  Byte;
typedef unsigned short Word;
typedef unsigned int   DWord;

//- PCX Header -
typedef struct {
	Byte maker;             // PCX�̸� �׻� 10��.
	Byte version;           // PCX ����.
	Byte code;              // RLE �����̸� 1, �ƴϸ� 0.
	Byte bpp;               // �ȼ��� ��Ʈ��.
	Word x1, y1, x2, y2;    // ȭ�� ��ǥ.
	Word hres, vres;        // ���� �ػ�, ���� �ػ�.
	Byte pal16[48];         // 16����.
	Byte vmode;             // ���� ��� ��ȣ.
	Byte nplanes;           // �÷� �÷����� ����. 256�̸� 8��.
	Word bpl;               // ���δ� ����Ʈ ��.
	Word palinfo;           // �ȷ�Ʈ ����.
	Word shres, svres;      // ��ĳ���� ����, ���� �ػ�.
	Byte unused[54];        // ������� ����.
} PCXHDR;

//- ��������Ʈ ���� ����ü -
typedef struct {
	Byte *PartMem;
	Word TotalSize;
} SpriteMem;

typedef struct {
	Byte *SData;
	Word ex;
	Word ey;
} SpriteMem2;

//- FMJ ���� ���� ����ü.
typedef struct {
	Byte *WeapMem;
	Word SizeX, SizeY;
	Word TotalSize;
	Word WeapCost;
	Word WeapWeight;
} WeaponMem;

//- FMJ ���ΰ��� ������ �ִ� ������.
typedef struct {
	Word ArmsFlag;
	Word ArmsCnt;
} HostWeapon;

//- FMJ�� ����Ÿ�� �ε��ؼ� �����ϴ� ����ü.
typedef struct {
	Byte FName[20];
	int  Mission;
} FMJSaveData;

//= Function Definition ==============================================

extern int   RotateI(int   val, int rot);
extern short RotateS(short val, int rot);
extern Byte  RotateB(Byte  val, int rot);

extern void SoundFX(unsigned number);
extern void Gamma(Byte *pal, int gammano);

void ModeChange(int mode);
int  GetKey(void);
void SaveRange(int sx, int sy, int ex, int ey, Byte *mem);
void RestoreRange(int sx, int sy, int ex, int ey, Byte *mem);
void PcxView(Byte *fname);
void PcxView2(Byte *fname);
void PaletteLoad(void);
void LoadMenuFont(void);
void LoadMenuFont2(void);
void LoadMenuWeap(void);
void SprFW(int sx, int sy, int index, int flag);
void FMJMainMenuRestore(Byte *fmjpal);
void FillEnvironBar(int x, int y, int dist);
void ChangeEnvironBar(int idx, int flag);
void DrawCursor(int x, int y);
int  InputFont(int x, int y);
void DisplayStr(int x, int y, Byte *str);
void ShowWeapon(int idx);
void ShowScore(int idx);
void AdjustWeight(void);

int FMJMenu(void);
void FMJMenuInit(void);

void FMJMainMenuStart(int judg);
void FMJMainMenuRun(void);
void CheckFirstMission(void);

void MissionStart(void);
void MissionCommand(int ptr, int idx);
void SaveFMJData(void);
void BuyWeapon(int idx);
int  BuyWeaponCheck(int idx);
void SellWeapon(int idx);

void MissionLoad(void);
int  FindSaveData(void);
void ShowAllSaveData(void);
void ShowSaveData(int idx);
void LoadFMJData(int idx);

void Environment(void);
void EnvironView(void);
void EnvironUpDown(int old, int new);
void EnvironLeftRight(int bar, int dist);

void Finality(void);

//= Data =============================================================

Byte   *VRam = (Byte *)0xA0000;       // ���� ��.
Byte   *VRam2;       // ���� ��.
PCXHDR PcxHead;                       // Pcx ����ü ����.
Byte   FMP1[768], FMP2[768];          // FMJ Menu �ȷ�Ʈ.
Byte   *PcxMem;                       // Pcx�׸� ���� ���(320 * 200).
Word   CordTable[200];                // Y�� ��ǥ����.
short  MenuNewBar, MenuOldBar;
Word   MenuAxis[4] = { 46, 74, 102, 130 };
Word   CommFlag;
int    EnvironSet[5] = { 0, 0, 0, 0, 0 }; // �ӽ� FMJ ȯ�氪.
Byte   LoadFileName[20];
int    FirstMission;                 // ó�� �ӹ��ΰ�?

SpriteMem  SprM[MENUFONTNUM];        // ��������Ʈ ���� ����ü ����.
SpriteMem2  SprM2[MENUFONTNUM];        // ��������Ʈ ���� ����ü ����.
WeaponMem  ArmsM[MENUWEAPNUM];       // FMJ ���� ���� ����ü ����.
HostWeapon HostW[MENUWEAPNUM / 2];   // FMJ ���ΰ��� ������ �ִ� ����� ����.

FMJSaveData FSave[5];

// FMJ ������� ��.
Word ArmsCost[20] = {
     1800,  100, 1350,   80, 3150, 200, 3800,  300,   60,   95,
       50, 2500,   30, 3200,   30, 900, 1000, 4500, 4700, 5000
};

// FMJ ������� ����.
Word ArmsWeight[20] = {
     300,  250, 210, 150,  300,  150,  400,  200,   50,   25,
      45, 1500,   8, 1500,   8, 1300, 1500, 1500, 1800, 2200
};

// FMj ������� ���� ����.
Word ArmsLimit[20] = {
     1,  800, 1, 500, 1, 150, 1, 30, 10, 10,
     10,   1,  30, 1,  30, 1,   1, 1,  1, 1
};

int SaveFMJCount;                   // ���̺� �� ȭ�� ����.
int MissionNumber;                  // ����� ���� �ε�� �ӹ� ��ȣ.
int SuccessFlag;                    // �ӹ� ���� �÷���.
int LoadNumber;                     // Load�ϰ�� ���° ��ȣ�ΰ�?

int FMJTotalScore;                  // FMJ ����.(�ܺο��� �����)
int FMJTotalBaseWeight;             // FMJ�� �⺻ ����.
int FMJTotalAppendWeight;           // FMJ�� �߰� ����.

int ResolutionAdjust;               // �ػ� ���� ����.(�ܺο��� �����)
int ScreenSizeAdjust;               // ȭ�� ũ�� ���� ����.(�ܺο��� �����)
int BrightAdjust;                   // ��� ���� ����.(�ܺο��� �����)
int EffectAdjust;                   // ȿ���� ���� ����.(�ܺο��� �����)
int MusicAdjust;                    // ���� ���� ����.(�ܺο��� �����)

int PLUSMINUS[5] = {1, 4, 1, 8, 8};           // ȯ�� ���� ������ ����.
int MAXVALUE[5] = {2, 64, 7, 255, 255};
//= Code =============================================================

//- Base Code ----------------------------------------------

// ȭ���� ��ȯ�Ѵ�.
void ModeChange(int mode)
{
    union REGS r;

    r.h.ah = 0;
    r.h.al = mode;
    int386(0x10, &r, &r);
}

// Ű�� ����Ѵ�.
int GetKey(void)
{
    int key;

    key = getch();
    // Ȯ��Ű�� 256�� ���Ѵ�.
    if(key == 0) key = getch() + 256;

    return (key);
}

// ȭ���� ������� �Ѵ�.
void FadeIn(Byte *pal)
{
    int i, j;

    outp(0x3c8, 0);
    for(j = 0; j < 64; j += 1)
    {
	for(i = 0; i < 768; i += 3)
	{
	    outp(0x3c9, (pal[i]   * j) >> 6);
	    outp(0x3c9, (pal[i+1] * j) >> 6);
	    outp(0x3c9, (pal[i+2] * j) >> 6);
	}
    }
    Gamma(pal,BrightAdjust);
}

// ȭ���� ��ο����� �Ѵ�.
void FadeOut(Byte *pal)
{
    int i, j;

    outp(0x3c8, 0);
    for(j = 63; j >= 0; j -= 1)
    {
	for(i = 0; i < 768; i += 3)
	{
	    outp(0x3c9, (pal[i]   * j) >> 6);
	    outp(0x3c9, (pal[i+1] * j) >> 6);
	    outp(0x3c9, (pal[i+2] * j) >> 6);
	}
    }
}

// ������ ���� ��Ҹ� �����Ѵ�.
void SaveRange(int sx, int sy, int ex, int ey, Byte *mem)
{
    Word i, j, index;

    for(i=sy; i < ey + 1; i++)
    {
       for(j=sx; j < ex + 1; j++)
       {
	    index = CordTable[i] + j;
	    mem[index] = *(VRam + index);
       }
    }
}

// ������ ���� ����� ����Ÿ�� �����Ѵ�.
void RestoreRange(int sx, int sy, int ex, int ey, Byte *mem)
{
    Word i, j, index;

    for(i=sy; i < ey + 1; i++)
    {
       for(j=sx; j < ex + 1; j++)
       {
	    index = CordTable[i] + j;
	    *(VRam + index) = mem[index];
       }
    }
}

// PCX ȭ���� �����ش�.
// ���� �� -> 0 : PCX ȭ���� �ƴϰų� ȭ���� ����. 1 : ����.
void PcxView(Byte *fname)
{
    FILE *fp;
    int  rc, si, di, x, y, xsize, ysize;
    Byte ch, data;

    fp = fopen(fname, "rb");

    fread(&PcxHead, sizeof(PcxHead), 1, fp);

    if(PcxHead.maker != 10)
    {
	fclose(fp);
	return;
    }

    fseek(fp, 128, SEEK_SET);
    xsize = PcxHead.x2 - PcxHead.x1 + 1;
    ysize = PcxHead.y2 - PcxHead.y1 + 1;

    fread(PcxMem, xsize * ysize, 1, fp);
    fclose(fp);

    di = si = x = y = 0;
    while(1)
    {
	ch = PcxMem[si++];
	if((ch & 0xC0) == 0xC0)
	{
	    rc = ch & 0x3F;
	    data = PcxMem[si++];
	}
	else
	{
	    rc = 1;
	    data = ch;
	}
	while(rc--)
	{
	    *(VRam + (CordTable[y] + x)) = data;
	    x++;
	}

	if(x >= xsize)
	{
	    x = 0;
	    y++;
	}

	if(y >= ysize) break;
    }
}

void PcxView2(Byte *fname)
{
    FILE *fp;
    int  rc, si, di, x, y, xsize, ysize;
    Byte ch, data;

    fp = fopen(fname, "rb");

    fread(&PcxHead, sizeof(PcxHead), 1, fp);

    if(PcxHead.maker != 10)
    {
	fclose(fp);
	return;
    }

    fseek(fp, 128, SEEK_SET);
    xsize = PcxHead.x2 - PcxHead.x1 + 1;
    ysize = PcxHead.y2 - PcxHead.y1 + 1;

    fread(PcxMem, xsize * ysize, 1, fp);
    fclose(fp);

    di = si = x = y = 0;
    while(1)
    {
	ch = PcxMem[si++];
	if((ch & 0xC0) == 0xC0)
	{
	    rc = ch & 0x3F;
	    data = PcxMem[si++];
	}
	else
	{
	    rc = 1;
	    data = ch;
	}
	while(rc--)
	{
	    *(VRam2 + (CordTable[y] + x)) = data;
	    x++;
	}

	if(x >= xsize)
	{
	    x = 0;
	    y++;
	}

	if(y >= ysize) break;
    }
}

// Fmj Menu Palette�� �ε��Ͽ� �ʱ�ȭ�Ѵ�.
// ���ϰ� -> 0 : ȭ���� ���ų� �޸� �Ҵ翡 ������. 1 : ����.
void PaletteLoad(void)
{
    FILE *fp;
    int  i;

    fp = fopen("FMJP.P", "rb");
    fread(FMP1, 768, 1, fp);
    fclose(fp);

    fp = fopen("FMJPP.P", "rb");
    fread(FMP2, 768, 1, fp);
    fclose(fp);
}

// 4����Ʈ ������ �ؼ� ������ �� ��Ʈ ȭ���� �ε��Ѵ�.
void LoadMenuFont(void)
{
    FILE *fp;
    int i, j, size, tsize;

    fp = fopen("FMJF.P", "rb");

    for(i=0; i < MENUFONTNUM; i++)
    {
	tsize = 0;
	fseek(fp, 10, SEEK_SET);

	for(j=0; j < i; j++)
	{
	    fread(&size, 4, 1, fp);
	    tsize += size;
	}
	fseek(fp, 1034 + tsize + 8, SEEK_SET);

	fread(&tsize, 4, 1, fp);
	SprM[i].TotalSize = tsize;
	SprM[i].PartMem = (Byte *)malloc(tsize);

	fread(SprM[i].PartMem, tsize, 1, fp);
    }
    fclose(fp);

//    PcxView("FMJA.PCX");

}

void LoadMenuFont2(void)
{
    PcxView2("FMJA.PCX");
    CutSprF(64,46,190,22,0);
    CutSprF(64,74,190,22,1);
    CutSprF(64,102,190,22,2);
    CutSprF(64,130,190,22,3);

    PcxView2("FMJA-1.PCX");
    CutSprF(64,46,190,22,4);
    CutSprF(64,74,190,22,5);
    CutSprF(64,102,190,22,6);
    CutSprF(64,130,190,22,7);
//
    PcxView2("FMJC.PCX");
    CutSprF(80,88,72,22,8);
    CutSprF(151,88,72,22,9);

    PcxView2("FMJC-1.PCX");
    CutSprF(80,88,72,22,10);
    CutSprF(151,88,72,22,11);
//
    PcxView2("FMJB.PCX");
    CutSprF(20,32,281,22,12);
    CutSprF(20,60,281,22,13);
    CutSprF(20,88,281,22,14);
    CutSprF(20,116,281,22,15);
    CutSprF(20,144,281,22,16);

    PcxView2("FMJB-1.PCX");
    CutSprF(20,32,281,22,17);
    CutSprF(20,60,281,22,18);
    CutSprF(20,88,281,22,19);
    CutSprF(20,116,281,22,20);
    CutSprF(20,144,281,22,21);
//
    PcxView2("FMJG.PCX");
    CutSprF(84,70,150,40,24);
//
    PcxView2("FMJD-1.PCX");
//    CutSprF(176,14,38,11,35);
//    CutSprF(81,15,38,11,36);
    CutSprF(152,7,131,21,35);
    CutSprF(14,7,131,21,36);
    CutSprF(191,33,94,11,41);
    CutSprF(191,50,94,11,42);
    CutSprF(191,67,94,11,43);
    CutSprF(191,84,94,11,44);

    PcxView2("FMJD.PCX");
    CutSprF(191,33,94,11,37);
    CutSprF(191,50,94,11,38);
    CutSprF(191,67,94,11,39);
    CutSprF(191,84,94,11,40);
//
    PcxView2("FMJH-1.PCX");
    CutSprF(23,3,266,29,83);
    CutSprF(23,35,266,29,84);
    CutSprF(23,67,266,29,85);
    CutSprF(23,99,266,29,86);
    CutSprF(23,131,266,29,87);
    CutSprF(23,163,266,29,88);

    PcxView2("FMJH-2.PCX");
    CutSprF(23,3,266,29,89);
    CutSprF(23,35,266,29,90);
    CutSprF(23,67,266,29,91);
    CutSprF(23,99,266,29,92);
    CutSprF(23,131,266,29,93);
    CutSprF(23,163,266,29,94);

    PcxView2("FMJH-3.PCX");
    CutSprF(23,3,266,29,95);
    CutSprF(23,35,266,29,96);
    CutSprF(23,67,266,29,97);

    PcxView2("FMJH.PCX");
    CutSprF(18,54,276,57,98);
}

// FMJ�� ���⸦ �ε��ϴ� �Լ�.
void LoadMenuWeap(void)
{
    FILE *fp;
    int i, j, size, tsize;

    fp = fopen("FMJW.P", "rb");

    for(i=0; i < MENUWEAPNUM; i++)
    {
	tsize = 0;
	fseek(fp, 10, SEEK_SET);

	for(j=0; j < i; j++)
	{
	    fread(&size, 4, 1, fp);
	    tsize += size;
	}
	fseek(fp, 1034 + tsize, SEEK_SET);

	fread(&ArmsM[i].SizeX, 4, 1, fp);
	fread(&ArmsM[i].SizeY, 4, 1, fp);
	fread(&tsize, 4, 1, fp);
	ArmsM[i].TotalSize = tsize;
	ArmsM[i].WeapMem = (Byte *)malloc(tsize);

	fread(ArmsM[i].WeapMem, tsize, 1, fp);
    }

    for(i=0; i < (MENUWEAPNUM >> 1); i++)
    {
	ArmsM[i].WeapCost   = ArmsCost[i];
	ArmsM[i].WeapWeight = ArmsWeight[i];
    }

    fclose(fp);
}

// 4����Ʈ ������ �� ��Ʈ�� Ǯ� ȭ�鿡 �����ش�.
// flag : 0 -> ��Ʈ, flag : 1 -> ����.
void SprFW(int sx, int sy, int index, int flag)
{
    int   i, oldsx, srcount, tsize;
    short count;
    Byte  data1, *buf;

    if(flag)
    {
	tsize = ArmsM[index].TotalSize;
	buf   = ArmsM[index].WeapMem;
	if(index < 20)
	{
	    sx = 80 - ArmsM[index].SizeX / 2;
	    sy = 50 - ArmsM[index].SizeY / 2;
	}
    }
    else
    {
	tsize = SprM[index].TotalSize;
	buf   = SprM[index].PartMem;
    }
    srcount=0, oldsx = sx;

    for(i=0; i < tsize; )
    {
	data1 = *(buf + srcount);
	i++;
	srcount++;

	if(data1==0x0A)
	{
	    count = *((short *)(buf + srcount));
	    srcount += 2;
	    i += 2;
	    sx += count;
	}

	if(data1 == 0x0B)
	{
	    count = *((short *)(buf + srcount));
	    srcount += 2;
	    i += 2;
	    memcpy((VRam + sx + CordTable[sy]), (buf+srcount), count);
	    sx += count;
	    i += count;
	    srcount += count;
	}

	if(data1 == 0x0C)
	{
	    count = *((short *)(buf + srcount));
	    count = count << 2;
	    srcount += 2;
	    i += 2;
	    memcpy((VRam + sx + CordTable[sy]), (buf+srcount), count);
	    sx += count;
	    i += count;
	    srcount += count;
	}

	if(data1==0x0D)
	{
	    sx = oldsx;
	    sy++;
	}
    }
}

void PutSprF(int sx, int sy, int index, int flag)
{
    int i, j, sp, tp;
    int Dx, Dy;
    Byte  data, *buf;

    if(flag == 0)
    {
	Dx = SprM2[index].ex;
	Dy = SprM2[index].ey;
	buf = SprM2[index].SData;

	sp = 0;
	tp = sx + sy * 320;

	for(j=0; j<Dy; j++)
	{
		for(i=0; i<Dx; i++)
		{
			if(buf[sp]!=0) VRam[tp] = buf[sp];
			sp++;
			tp++;
		}
		tp = tp + (320 - Dx);
	}
    }
}

void CutSprF(int sx, int sy, int dx, int dy, int index)
{
    int i, j, sp, tp;
    int size;
    Byte  data, *buf;

    SprM2[index].ex = (Word)dx;
    SprM2[index].ey = (Word)dy;
    size = dx * dy;
    SprM2[index].SData = (Byte *)malloc(size);
    buf = SprM2[index].SData;

    sp = sx + sy * 320;
    tp = 0;

    for(j=0; j<dy; j++)
    {
	for(i=0; i<dx; i++)
	{
	    buf[tp] = VRam2[sp];
	    sp++;
	    tp++;
	}
	sp = sp + (320 - dx);
    }
}

// FMJ ���� �޴��� �����Ѵ�.
void FMJMainMenuRestore(Byte *fmjpal)
{
    FadeOut(fmjpal);
    PcxView("FMJA.PCX");
    SaveRange(85, 78, 85 + 150, 78 + 40, PcxMem);
//    SprFW(64, MenuAxis[MenuNewBar], 4 + MenuNewBar, 0);
    PutSprF(64, MenuAxis[MenuNewBar], 4 + MenuNewBar, 0);
    FadeIn(FMP1);

    CommFlag = 0;
    MenuOldBar = MenuNewBar;
}

// FMJ ȯ��ٸ� ������ �Ÿ���ŭ ĥ�Ѵ�.
void FillEnvironBar(int x, int y, int dist)
{
    int i, j;

    for(i=y; i < (y + 10); i++)
	for(j=x; j < (x + dist); j++)
	   *(VRam + (CordTable[i] + j)) = 181;
}

// FMJ ȯ��ٸ� �����Ѵ�.
void ChangeEnvironBar(int idx, int dist)
{
    int y, diff;

    diff = EnvironSet[idx];
    diff += dist;

    if(diff > MAXVALUE[idx]) diff = MAXVALUE[idx];
    if(diff < 0)  diff = 0;
    y = 39 + (idx * 28);

    FillEnvironBar(191, y, diff * 97 / MAXVALUE[idx] );
    EnvironSet[idx] = diff;
}

// Ŀ���� �׸��� �Լ�.
void DrawCursor(int x, int y)
{
    int i, imsi;

    imsi = CordTable[y + 6];
    for(i=x; i < (x + 8); i++) *(VRam + imsi + i) = 29;
}

// ���ڸ� �Է¹޴´�.
int InputFont(int x, int y)
{
    int key, loop, ret, sx, dist;

    loop = 1, ret = dist = 0;

    for(sx=0; sx < 20; sx++) LoadFileName[sx] = 0;

    sx = x;
    while(loop)
    {
	DrawCursor(sx, y);
	key = GetKey();

	switch(key)
	{
	    case BSPACE : if(dist > 0)
			  {
			       LoadFileName[--dist] = 0;
			       sx -= 8;
			  }
			  break;
	    case ENTER  : key = strlen(LoadFileName);
			  if(key) ret = 1;
			  else    ret = 0;
			  loop = 0;
			  SoundFX(_ENTER_);
			  break;
	    case ESC    : loop = 0;
			  SoundFX(_ESC_);
			  break;
	    default     : if(dist >= 9) break;

			  key = toupper(key);
			  if((key >= 48) && (key <= 57))
			  {
			       LoadFileName[dist++] = key;
			       sx += 8;
			  }
			  else if((key >= 65) && (key <= 90))
			  {
				LoadFileName[dist++] = key;
				sx += 8;
			  }
			  break;
	}
	SprFW(90, 100, 46, 0);
	DisplayStr(x, y, LoadFileName);
    }
    return (ret);
}

// ���ڿ��� ����Ѵ�.
void DisplayStr(int x, int y, Byte *str)
{
    int i, len, idx, chr;

    len = strlen(str);
    for(i=0; i < len; i++)
    {
	chr = toupper(str[i]);

	if((chr >= 48) && (chr <= 57))      SprFW(x, y, chr - 1, 0);
	else if((chr >= 65) && (chr <= 90)) SprFW(x, y, chr - 8, 0);

	x += 8;
    }
}

// FMJ ���⸦ �����ش�.
void ShowWeapon(int idx)
{
    int i, len;
    Byte num[20];

    RestoreRange(36, 27, 125, 74, PcxMem);
    SprFW(141, 11, idx, 1);
    RestoreRange(32, 142, 130, 176, PcxMem);
    SprFW(20, 142, 20 + idx, 1);

//    RestoreRange(182, 14, 230, 28, PcxMem);
    RestoreRange(167, 9, 167+131, 9+21, PcxMem);
    if(idx > 7)
    {
	itoa(FMJTotalAppendWeight, num, 10);
//    SprFW(182, 14, 36, 0);
//    PutSprF(182, 14, 36, 0);
    PutSprF(167, 9, 36, 0);
    }
    else
    {
	itoa(FMJTotalBaseWeight, num, 10);
//    SprFW(182, 14, 35, 0);
//    PutSprF(182, 14, 35, 0);
    PutSprF(167, 9, 35, 0);
    }
    len = strlen(num);
    RestoreRange(194, 121, 259, 128, PcxMem);
    for(i=0; i < len; i++) SprFW(259 - ((len - i) << 3), 121, num[i] - 1, 0);

    RestoreRange(24, 102, 60, 110, PcxMem);
    if(HostW[idx].ArmsFlag)
    {
	SprFW(54, 30, 45, 0);
	itoa(HostW[idx].ArmsCnt, num, 10);
	len = strlen(num);
	for(i=0; i < len; i++) SprFW(54 - ((len - i) << 3), 102, num[i] - 1, 0);
    }
}

// ���� �ڱ��� ��, ���� �� ������ ����, ���� �ڱⰡ ������ �� �ִ� ����.
void ShowScore(int idx)
{
    int i, len;
    Byte num[20];

    itoa(FMJTotalScore, num, 10);
    len = strlen(num);
    RestoreRange(177, 168, 240, 176, PcxMem);
    for(i=0; i < len; i++) SprFW(240 - ((len - i) << 3), 168, num[i] - 1, 0);

    if(idx > 7) itoa(FMJTotalAppendWeight, num, 10);
    else        itoa(FMJTotalBaseWeight, num, 10);
    len = strlen(num);
    RestoreRange(194, 121, 259, 128, PcxMem);
    for(i=0; i < len; i++) SprFW(259 - ((len - i) << 3), 121, num[i] - 1, 0);

    RestoreRange(24, 102, 60, 110, PcxMem);
    if(HostW[idx].ArmsFlag)
    {
	SprFW(54, 30, 45, 0);
	itoa(HostW[idx].ArmsCnt, num, 10);
	len = strlen(num);
	for(i=0; i < len; i++) SprFW(54 - ((len - i) << 3), 102, num[i] - 1, 0);
    }
}

void AdjustWeight(void)
{
    int i, j, cnt, wet;

    FMJTotalBaseWeight = 2300;
    FMJTotalAppendWeight = 3800;

    for(i=0; i < 8; i++)
    {
	if(HostW[i].ArmsFlag)
	{
	    cnt = HostW[i].ArmsCnt;
	    wet = ArmsWeight[i];
	    for(j=0; j < cnt; j++) FMJTotalBaseWeight -= wet;
	}
    }

    for(i=8; i < 20; i++)
    {
	if(HostW[i].ArmsFlag)
	{
	    cnt = HostW[i].ArmsCnt;
	    wet = ArmsWeight[i];
	    for(j=0; j < cnt; j++) FMJTotalAppendWeight -= wet;
	}
    }
}

int FMJMenu(void)
{
    int i, ret;

    if( SuccessFlag != 1 )
    FMJMenuInit();

    //- �޸𸮸� �Ҵ��Ѵ�.
    PcxMem = (Byte *)malloc(64000);
    if(! PcxMem) return(0);

    VRam2 = (Byte *)malloc(64000);
    if(! VRam2) return(0);

    //- ��Ʈ �޸� �Ҵ�.
    LoadMenuFont();
    LoadMenuFont2();

    //- FMJ ���� �޸� �Ҵ�.
    LoadMenuWeap();

    MenuNewBar = MenuOldBar = CommFlag = 0;
    ModeChange(0x13);

    switch(SuccessFlag)
    {
	case -1 : FMJTotalScore        = 12000;     // �ӹ��� ���.
		  FMJTotalBaseWeight   = 2300;      // lost in mission
		  FMJTotalAppendWeight = 3800;
		  MissionNumber        = 0;
		  FirstMission         = 0;
		  SuccessFlag          = 0;

		  for(i=0; i < (MENUWEAPNUM >> 1); i++)
		  {
		      HostW[i].ArmsFlag = 0;
		      HostW[i].ArmsCnt  = 0;
		  }
		  FMJMainMenuStart(0);   // �ӹ��� ESC ����.
		  break;                // abort mission
	case  0 :
		  FMJMainMenuStart(0);   // �ӹ��� ESC ����.
		  break;                // abort mission
	case  1 : MissionNumber++;
		  AdjustWeight();
		  SaveFMJData();
		  MissionStart();       // �ӹ� ����.
		  if(CommFlag != 2) FMJMainMenuStart(1);
		  SuccessFlag = 0;
		  break;                // mission completed
    }

    if(CommFlag == 2) FadeOut(FMP2);

    free(PcxMem);

    for(i=0; i < MENUFONTNUM; i++) free(SprM[i].PartMem);
    for(i=0; i < MENUWEAPNUM; i++) free(ArmsM[i].WeapMem);

//    ModeChange(0x03);
    return (CommFlag);
}

// FMJ �޴��� �ʿ��� ����Ÿ�� �ʱ�ȭ�Ѵ�.(���� ó���� ����)
void FMJMenuInit(void)
{
    int i;

    //- ��ǥ ���̺� �Ҵ�.
    for(i=0; i < 200; i++) CordTable[i] = i * 320;

    //- ���� �ڽ��� ���⸦ �ʱ�ȭ�Ѵ�.
    for(i=0; i < (MENUWEAPNUM >> 1); i++)
    {
	HostW[i].ArmsFlag = 0;
	HostW[i].ArmsCnt  = 0;
    }

    FMJTotalScore        = 12000;
    FMJTotalBaseWeight   = 2300;
    FMJTotalAppendWeight = 3800;

    EnvironSet[0] = ResolutionAdjust;
    EnvironSet[1] = ScreenSizeAdjust;
    EnvironSet[2] = BrightAdjust;
    EnvironSet[3] = EffectAdjust;
    EnvironSet[4] = MusicAdjust;

    FirstMission = 0;
    PaletteLoad();
}

//- FMJ Menu Code ------------------------------------------

// FMJ ���� �޴��� �����ϴ� �Լ�.
void FMJMainMenuStart(int judg)
{
    int key;

    if(judg == 0)
    {
	FadeOut(FMP1);
	PcxView("FMJA.PCX");
//	SprFW(64, 46, 4, 0);
	PutSprF(64, 46, 4, 0);
	SaveRange(85, 78, 85 + 150, 78 + 40, PcxMem);
	FadeIn(FMP1);
    }

    while((CommFlag == 0) || (CommFlag == 3))
    {
	CommFlag = 0;
	MenuOldBar = MenuNewBar;

	key = GetKey();

	switch(key)
	{
	    case UP    : MenuNewBar--;
			 if(MenuNewBar < 0) MenuNewBar = 3;
			 SoundFX(_ARROW_);
			 break;
	    case DOWN  : MenuNewBar++;
			 if(MenuNewBar > 3) MenuNewBar = 0;
			 SoundFX(_ARROW_);
			 break;
	    case ENTER : SoundFX(_ENTER_);
			 FMJMainMenuRun();
			 if(CommFlag == 3) MissionStart();
			 break;
	}
	if(MenuNewBar != MenuOldBar)
	{
//	    SprFW(64, MenuAxis[MenuOldBar], MenuOldBar, 0);
//	    SprFW(64, MenuAxis[MenuNewBar], 4 + MenuNewBar, 0);
	    PutSprF(64, MenuAxis[MenuOldBar], MenuOldBar, 0);
	    PutSprF(64, MenuAxis[MenuNewBar], 4 + MenuNewBar, 0);
	}
    }
}

void FMJMainMenuRun(void)
{
    switch(MenuNewBar)
    {
	case 0 : if(FirstMission == 0) CheckFirstMission();
		 if(FirstMission) MissionStart();
		 break;
	case 1 : MissionLoad();
		 break;
	case 2 : Environment();
		 break;
	case 3 : Finality();
		 break;
    }
}

// ó�� �ӹ� �����̸� �̸��� �Է¹޴´�.
void CheckFirstMission(void)
{
//    SprFW(85, 78, 24, 0);
    PutSprF(85, 78, 24, 0);

    FirstMission = InputFont(92, 105);
    RestoreRange(85, 78, 85 + 150, 78 + 40, PcxMem);
}

// FMJ �ӹ��� �����Ѵ�.
void MissionStart(void)
{
    int loop, key, bar, old, idx;
    int axis[4] = { 33, 50, 67, 84 };
//    int axis[4] = { 31, 48, 65, 82 };

    loop = 1, bar = idx = 0;

    FadeOut(FMP1);
    PcxView("FMJD.PCX");

    SaveRange(36, 27, 125, 74, PcxMem);
    SaveRange(32, 142, 130, 176, PcxMem);
//  SaveRange(24, 102, 60, 110, PcxMem);
//  SaveRange(24, 75, 27 + 272, 75 + 41, PcxMem);
    SaveRange(24, 75, 24 + 280, 75 + 60, PcxMem);

    SaveRange(194, 121, 259, 128, PcxMem);
    SaveRange(177, 168, 240, 176, PcxMem);
//    SaveRange(182, 14, 230, 28, PcxMem);
    SaveRange(167, 9, 167+131, 9+21, PcxMem);

//    SprFW(191, 33, 41, 0);
    PutSprF(191, 33, 41, 0);

    SprFW(141, 11, 0, 1);
    SprFW(20, 142, 20, 1);
//    SprFW(182, 14, 35, 0);
//    PutSprF(182, 14, 35, 0);
    PutSprF(167, 9, 35, 0);
    ShowScore(0);

//    SprFW(27, 75, 98, 0);
//    SprFW(27, 75 + 23, MissionNumber + 83, 0);
    PutSprF(27, 75, 98, 0);
    PutSprF(27 + 3, 75 + 23, MissionNumber + 83, 0);
    FadeIn(FMP2);

    getch();

    RestoreRange(24, 75, 24 + 280, 75 + 60, PcxMem);
    ShowScore(0);

    while(loop)
    {
	old = bar;
	key = GetKey();

	switch(key)
	{
	    case UP     : bar--;
			  if(bar < 0) bar = 3;
			  SoundFX(_ARROW_);
			  break;
	    case DOWN   : bar++;
			  if(bar > 3) bar = 0;
			  SoundFX(_ARROW_);
			  break;
	    case RIGHT  : if((bar == 0) || (bar == 1))
			  {
			    idx++;
			    if(idx > 19) idx = 0;
			    ShowWeapon(idx);
			    SoundFX(_BAND_);
			  }
			  break;
	    case LEFT   : if((bar == 0) || (bar == 1))
			  {
			    idx--;
			    if(idx < 0) idx = 19;
			    ShowWeapon(idx);
			    SoundFX(_BAND_);
			  }
			  break;
	    case ENTER  : if((bar == 0) || (bar == 1)) SoundFX(_SLD_);
			  else SoundFX(_ENTER_);
			  MissionCommand(bar, idx);
			  if((bar == 2) || (bar == 3)) loop = 0;
			  break;
	}
	if(bar != old)
	{
//	    SprFW(191, axis[old], 37 + old, 0);
//	    SprFW(191, axis[bar], 41 + bar, 0);
	    PutSprF(191, axis[old], 37 + old, 0);
	    PutSprF(191, axis[bar], 41 + bar, 0);
	}
    }
}

void MissionCommand(int ptr, int idx)
{
    switch(ptr)
    {
	case 0 : BuyWeapon(idx);
		 break;
	case 1 : SellWeapon(idx);
		 break;
	case 2 : FMJMainMenuRestore(FMP2);
		 break;
	case 3 : CommFlag = 2;
		 SaveFMJData();
		 break;
    }
}

// FMJ ����Ÿ�� �����Ѵ�.
void SaveFMJData(void)
{
    FILE  *fp;
    int   imsi, i;
    short temp;
    Byte  buff[120];

    fp = fopen("FMJS.P", "rb+");
    fread(&SaveFMJCount, 1, 4, fp);
    SaveFMJCount = RotateI(SaveFMJCount, 25);

    if(FirstMission == 1)          // �Է�(�߰�)
    {
	imsi = SaveFMJCount;
	SaveFMJCount++;
	if(SaveFMJCount >= 5) SaveFMJCount = 5;
    }

    i = RotateI(SaveFMJCount, 7);
    fseek(fp, 0, SEEK_SET);
    fwrite(&i, 1, 4, fp);

    if(FirstMission == 1)
    {
	if(imsi == 5)
	{
	    for(i=1; i < 5; i++)
	    {
		fseek(fp, (i * 112) + 4, SEEK_SET);
		fread(buff, 1, 112, fp);
		fseek(fp, ((i - 1) * 112) + 4, SEEK_SET);
		fwrite(buff, 1, 112, fp);
	    }
	    LoadNumber = 4;
	}
	else
	{
	    fseek(fp, (imsi * 112) + 4, SEEK_SET);
	    LoadNumber = imsi;
	}
    }
    else fseek(fp, (LoadNumber * 112) + 4, SEEK_SET);

    i = RotateI(FMJTotalScore, 7);
    fwrite(&i, 1, 4, fp);
    i = RotateI(FMJTotalBaseWeight, 7);
    fwrite(&i, 1, 4, fp);
    i = RotateI(FMJTotalAppendWeight, 7);
    fwrite(&i, 1, 4, fp);

    for(i=0; i < 16; i++) buff[i] = RotateB(LoadFileName[i], 5);
    fwrite(buff, 1, 16, fp);

    i = RotateI(MissionNumber, 7);
    fwrite(&i, 1, 4, fp);

    for(i=0; i < 20; i++)
    {
	temp = RotateS(HostW[i].ArmsFlag, 9);
	fwrite(&temp, 1, 2, fp);
	temp = RotateS(HostW[i].ArmsCnt, 9);
	fwrite(&temp, 1, 2, fp);
    }
    fclose(fp);
    FirstMission = 2;       // OverWrite
}

// FMJ ���⸦ ���.
void BuyWeapon(int idx)
{
    int weight, cost, limit, flag;

    weight = ArmsWeight[idx];
    cost   = ArmsCost[idx];
    limit  = ArmsLimit[idx];

    flag = BuyWeaponCheck(idx);
    if(flag) return;

    if((FMJTotalScore - cost) < 0) return;
    if((idx > 7) && ((FMJTotalAppendWeight - weight) < 0)) return;
    if((idx < 8) && ((FMJTotalBaseWeight  -  weight) < 0)) return;
    if(HostW[idx].ArmsCnt == limit) return;

    FMJTotalScore -= cost;
    HostW[idx].ArmsCnt++;
    HostW[idx].ArmsFlag = 1;
    if(idx > 7) FMJTotalAppendWeight -= weight;
    else        FMJTotalBaseWeight   -= weight;

    ShowScore(idx);
}

// ���� ���� ������ �˻��Ѵ�.
// return : 1 -> ���⸦ �����Ҽ��� �ִ�, return : 0 -> ���⸦ �����Ҽ��� ����.
int BuyWeaponCheck(int idx)
{
    int ret;

    ret = 0;
    switch(idx)
    {
	case 1  : if(! HostW[0].ArmsFlag) ret = 1;
		  break;
	case 3  : if(! HostW[2].ArmsFlag) ret = 1;
		  break;
	case 5  : if(! HostW[4].ArmsFlag) ret = 1;
		  break;
	case 7  : if(! HostW[6].ArmsFlag) ret = 1;
		  break;
	case 11 : ret = HostW[13].ArmsFlag;
		  break;
	case 12 : if(! HostW[11].ArmsFlag) ret = 1;
		  break;
	case 13 : ret = HostW[11].ArmsFlag;
		  break;
	case 14 : if(! HostW[13].ArmsFlag) ret = 1;
		  break;
	case 15 : ret = HostW[16].ArmsFlag;
		  break;
	case 16 : ret = HostW[15].ArmsFlag;
		  break;
	case 17 : if(HostW[18].ArmsFlag || HostW[19].ArmsFlag) ret = 1;
		  break;
	case 18 : if(HostW[17].ArmsFlag || HostW[19].ArmsFlag) ret = 1;
		  break;
	case 19 : if(HostW[17].ArmsFlag || HostW[18].ArmsFlag) ret = 1;
		  break;
    }
    return (ret);
}

// ���� �ڱⰡ ������ ������ �Ǵ�.
void SellWeapon(int idx)
{
    int weight, cost;

    if(! HostW[idx].ArmsFlag) return;

    FMJTotalScore += ArmsCost[idx];
    if(idx > 7) FMJTotalAppendWeight += ArmsWeight[idx];
    else        FMJTotalBaseWeight   += ArmsWeight[idx];

    HostW[idx].ArmsCnt--;
    if(HostW[idx].ArmsCnt == 0)
    {
	RestoreRange(36, 27, 125, 74, PcxMem);
	SprFW(141, 11, idx, 1);
	HostW[idx].ArmsFlag = 0;
    }

    ShowScore(idx);
}

// ����� �ӹ��� �ҷ��´�.
void MissionLoad(void)
{
    int loop, key, bar, old;
    int axis[5] = { 62, 81, 100, 119, 138 };

    loop = 1, bar = 0;

    key = FindSaveData();
    if(! key) return;

    FadeOut(FMP1);
    PcxView("FMJE.PCX");
    ShowAllSaveData();
    FadeIn(FMP1);

    while(loop)
    {
	old = bar;
	key = GetKey();

	switch(key)
	{
	    case UP    : bar--;
			 if(bar < 0) bar = SaveFMJCount - 1;
			 SoundFX(_ARROW_);
			 break;
	    case DOWN  : bar++;
			 if(bar >= SaveFMJCount) bar = 0;
			 SoundFX(_ARROW_);
			 break;
	    case ENTER : SoundFX(_ENTER_);
			 LoadFMJData(bar);
			 loop = 0;
			 break;
	    case ESC   : loop = 0;
			 SoundFX(_ESC_);
			 break;
	}
	if(bar != old)
	{
	    SprFW(81, axis[old], 25 + old, 0);
	    ShowSaveData(old);
	    SprFW(81, axis[bar], 30 + bar, 0);
	    ShowSaveData(bar);
	}
    }
    if(CommFlag == 0) FMJMainMenuRestore(FMP1);
}

// ����� FMJ ����Ÿ�� �ִ°��� �����Ѵ�.
int FindSaveData(void)
{
    FILE *fp;
    int i, j;

    fp = fopen("FMJS.P", "rb");

    fread(&SaveFMJCount, 1, 4, fp);
    SaveFMJCount = RotateI(SaveFMJCount, 25);

    if(SaveFMJCount == 0)
    {
	fclose(fp);
	return (0);
    }

    for(i=0; i < SaveFMJCount; i++)
    {
	fseek(fp, 12, SEEK_CUR);
	fread(FSave[i].FName, 1, 16, fp);

	for(j=0; j < 16; j++)
	   FSave[i].FName[j] = RotateB(FSave[i].FName[j], 3);

	fread(&FSave[i].Mission, 1, 4, fp);
	FSave[i].Mission = RotateI(FSave[i].Mission, 25);
	fseek(fp, 80, SEEK_CUR);
    }
    fclose(fp);
    return (1);
}

// ����� ��� FMJ ����Ÿ�� �����ش�.
void ShowAllSaveData(void)
{
    int i, imsi;
    int axis[5] = { 62, 81, 100, 119, 138 };
    Byte num[10];

    SprFW(81, 62, 30, 0);
    for(i=0; i < SaveFMJCount; i++)
    {
	imsi = axis[i] + 4;
	DisplayStr(100, imsi, FSave[i].FName);
	itoa(FSave[i].Mission, num, 10);
	DisplayStr(215, imsi, num);
    }
}

// ������ ��ȣ�� FMJ ����Ÿ�� �����ش�.
void ShowSaveData(int idx)
{
    int i, imsi;
    int axis[5] = { 62, 81, 100, 119, 138 };
    Byte num[10];

    imsi = axis[idx] + 4;
    DisplayStr(100, imsi, FSave[idx].FName);
    itoa(FSave[idx].Mission, num, 10);
    DisplayStr(215, imsi, num);
}

// ����� FMJ ����Ÿ�� �ε��Ѵ�.
void LoadFMJData(int idx)
{
    FILE *fp;
    int  i;

    fp = fopen("FMJS.P", "rb");
    fseek(fp, (idx * 112) + 4, SEEK_SET);

    CommFlag = 3;
    FirstMission = 2;
    LoadNumber   = idx;
    MenuNewBar--;
    MenuOldBar   = MenuNewBar;
    strcpy(LoadFileName, FSave[idx].FName);
    MissionNumber = FSave[idx].Mission;

    fread(&i, 1, 4, fp);
    FMJTotalScore = RotateI(i, 25);
    fread(&i, 1, 4, fp);
    FMJTotalBaseWeight = RotateI(i, 25);
    fread(&i, 1, 4, fp);
    FMJTotalAppendWeight = RotateI(i, 25);
    fseek(fp, 20, SEEK_CUR);

    for(i=0; i < 20; i++)
    {
	fread(&HostW[i].ArmsFlag, 1, 2, fp);
	HostW[i].ArmsFlag = RotateS(HostW[i].ArmsFlag, 7);
	fread(&HostW[i].ArmsCnt,  1, 2, fp);
	HostW[i].ArmsCnt = RotateS(HostW[i].ArmsCnt, 7);
    }
    fclose(fp);
}

// FMJ ȯ���� �����Ѵ�.
void Environment(void)
{
    int loop, key, bar, old;
//    int axis[5] = { 35, 63, 91, 119, 147 };
    int axis[5] = { 32, 60, 88, 116, 144 };

    loop = 1, bar = 0;

    FadeOut(FMP1);
    PcxView("FMJB.PCX");
//    SprFW(20, 32, 17, 0);
    PutSprF(20, 32, 17, 0);
    EnvironView();
    FadeIn(FMP1);

    while(loop)
    {
	old = bar;
	key = GetKey();

	switch(key)
	{
	    case UP    : bar--;
			 if(bar < 0) bar = 4;
//	     SprFW(20, axis[old], 12 + old, 0);
//	     SprFW(20, axis[bar], 17 + bar, 0);
	     PutSprF(20, axis[old], 12 + old, 0);
	     PutSprF(20, axis[bar], 17 + bar, 0);
			 EnvironUpDown(old, bar);
			 SoundFX(_ARROW_);
			 break;
	    case DOWN  : bar++;
			 if(bar > 4) bar = 0;
//	     SprFW(20, axis[old], 12 + old, 0);
//	     SprFW(20, axis[bar], 17 + bar, 0);
	     PutSprF(20, axis[old], 12 + old, 0);
	     PutSprF(20, axis[bar], 17 + bar, 0);
			 EnvironUpDown(old, bar);
			 SoundFX(_ARROW_);
			 break;
	case RIGHT : if(bar)
			SprFW(188, axis[bar] + 3, 23, 0);
	     else
			SprFW(188, axis[bar] + 3, 22, 0);
			 EnvironLeftRight(bar, PLUSMINUS[bar]);
			 SoundFX(_ARROW_);
			 break;
	case LEFT  : if(bar)
			SprFW(188, axis[bar] + 3, 23, 0);
	     else
			SprFW(188, axis[bar] + 3, 22, 0);
			 EnvironLeftRight(bar, -PLUSMINUS[bar]);
			 SoundFX(_ARROW_);
			 break;

	    case ESC   : loop = 0;
			 SoundFX(_ESC_);
			 break;
	}
    }
    FMJMainMenuRestore(FMP1);
}

// FMJ�� ȯ�� �������� �����ش�.
void EnvironView(void)
{
    int var;
    int axis[3] = { 191, 225, 259 };

    var = EnvironSet[0];
    FillEnvironBar(axis[var], 39, 29);

    var = EnvironSet[1];
    FillEnvironBar(191, 67, var * 97 / MAXVALUE[1]);

    var = EnvironSet[2];
    FillEnvironBar(191, 95, var * 97 / MAXVALUE[2]);

    var = EnvironSet[3];
    FillEnvironBar(191, 123, var * 97 / MAXVALUE[3]);

    var = EnvironSet[4];
    FillEnvironBar(191, 151, var * 97 / MAXVALUE[4]);
}

// ��, �Ʒ��� �����̸鼭 ȯ�� �������� �����ִ� �Լ�.
void EnvironUpDown(int old, int new)
{
    int oldx, oldy, newx, newy;
    int axis[3] = { 191, 225, 259 };

    oldx = EnvironSet[old], newx = EnvironSet[new];
    oldy = 39 + (old * 28), newy = 39 + (new * 28);

    if(old == 0) FillEnvironBar(axis[oldx], oldy, 29);
    else         FillEnvironBar(191, oldy, oldx * 97 / MAXVALUE[old]);

    if(new == 0) FillEnvironBar(axis[newx], newy, 29);
    else         FillEnvironBar(191, newy, newx * 97 / MAXVALUE[new]);
}

// ȯ�� �������� ����, ���� ��Ű�� �Լ�.
void EnvironLeftRight(int bar, int dist)
{
    int x, y, idx;
    int axis[3] = { 191, 225, 259 };

    y = 39 + (bar * 28);
    switch(bar)
    {
	case 0 : idx = EnvironSet[0];
		 idx += dist;
		 if(idx < 0)      idx = 2;
		 else if(idx > 2) idx = 0;
		 FillEnvironBar(axis[idx], y, 29);
		 ResolutionAdjust = idx;
		 EnvironSet[0] = idx;
		 break;
	case 1 : ChangeEnvironBar(1, dist);
		 ScreenSizeAdjust = EnvironSet[1];
		 break;
	case 2 : ChangeEnvironBar(2, dist);
		 BrightAdjust = EnvironSet[2];
		 Gamma(FMP1,BrightAdjust);
		 break;
	case 3 : ChangeEnvironBar(3, dist);
		 EffectAdjust = EnvironSet[3];
		 MODSetSampleVolume(EffectAdjust);
		 break;
	case 4 : ChangeEnvironBar(4, dist);
		 MusicAdjust = EnvironSet[4];
		 MODSetMusicVolume(MusicAdjust);
		 break;
    }
}

// FMJ �����⸦ �����Ѵ�.
void Finality(void)
{
    int loop, key, old, bar;
    int axis[2] = { 80, 151 };

    loop = 1, bar = 0;

    FadeOut(FMP1);
    PcxView("FMJC.PCX");
//    SprFW(91, 88, 10, 0);
    PutSprF(80, 88, 10, 0);
    FadeIn(FMP1);

    while(loop)
    {
	old = bar;
	key = GetKey();

	switch(key)
	{
	    case RIGHT :
	    case LEFT  : bar = 1 - bar;
			 SoundFX(_ARROW_);
			 break;
	    case ENTER : loop = 0;
			 SoundFX(_ENTER_);
			 break;
	}
	if(bar != old)
	{
//	    SprFW(axis[old], 88, 8  + old, 0);
//	    SprFW(axis[bar], 88, 10 + bar, 0);
	    PutSprF(axis[old], 88, 8  + old, 0);
	    PutSprF(axis[bar], 88, 10 + bar, 0);
	}
    }
    CommFlag = 1 - bar;
    if(CommFlag == 0) FMJMainMenuRestore(FMP1);
}

//void main(void)
//{
//    FMJMenuInit();
//    FMJMenu();
//    printf("CommFlag is %d\n", CommFlag);
//}
