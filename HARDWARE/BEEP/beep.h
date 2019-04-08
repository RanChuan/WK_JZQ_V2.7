
#ifndef __BEEP_H
#define __BEEP_H

#ifdef  __cplusplus
	extern "C" {
#endif



#define BEEP PCout(13)

typedef struct 
{
	char tone[4];//������1_������1��������1~������ 0����Ъ
	char time[4];//1/8��8��֮һ����1/4���ķ�֮һ��
} jianpu;


void BEEP_Init(void);

void Beep_Play (jianpu *jianpu_);
void Beep_End (void);

void set_jianpu(jianpu *j,char *tone_,char *time_);
void jianpu_end(jianpu *j);

void Load_song(void);
void Load_song1(void);
void Load_song2(void);
void Load_song3(void);

void Load_up(void);
void Load_down(void);




#ifdef  __cplusplus
	}
#endif





#endif


