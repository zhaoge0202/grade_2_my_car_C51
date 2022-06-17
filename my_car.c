#include <reg52.h>

typedef unsigned char uchar;
typedef unsigned int uint;

uchar Maze[8][8];
/*unsigned char Maze[8][8] = {
  {0xf5,0xf5,0xf9,0xf3,0xf5,0xf5,0xf5,0xfb},
 	{0xf5,0xf1,0xf8,0xf2,0xf5,0xfd,0xfa,0xfa},
 	{0xfb,0xfa,0xf6,0xf4,0xf1,0xf1,0xf4,0xfc},
 	{0xfa,0xfa,0xf7,0xf5,0xf8,0xf6,0xf5,0xf9},
 	{0xf6,0xf4,0xf1,0xf5,0xf0,0xf5,0xfd,0xfa},
 	{0xf3,0xf1,0xf4,0xf5,0xf4,0xf1,0xfd,0xfa},
 	{0xfa,0xfa,0xf3,0xf5,0xf5,0xf0,0xf5,0xfc},
 	{0xfe,0xf6,0xf4,0xf5,0xfd,0xf6,0xf5,0xfd} };	*/
		
		
sfr P4 = 0xe8;
sbit BEEP = P3 ^ 7;
sbit A0 = P4 ^ 0;
sbit A1 = P2 ^ 0;
sbit A2 = P2 ^ 7;
sbit irR1_C = P2 ^ 1;
sbit irR2_LU = P2 ^ 2;
sbit irR3_L = P2 ^ 3;
sbit irR4_R = P2 ^ 4;
sbit irR5_RU = P2 ^ 5;
sbit tube1 = P4 ^ 3;
sbit tube2 = P4 ^ 2;
uchar irL = 0, irLU = 0, irC = 0, irRU = 0, irR = 0;


bit sprint_flag = 0 ;		//冲刺标志
uchar i, j, k, t;
uchar flag2,flag_recall = 0;//遍历时
uchar y_axis = 0, x_axis = 1, y_axis_r = 2, x_axis_r = 3;
uchar x = 0, y = 0, begin = 0;
uchar flag_stack = 0;
uchar ab_dric = 0;
uchar fis, sec; //第一个，第二个
//记录可行路线的个数
uchar num = 0, flag = 0,nums;

//登高表
uchar best_board[8][8];
uint all = 0;
uint now = 0, nowx, nowy,bestx,besty;
uint pathx = 0;
uint pathy = 0;
uchar pathnum;
uchar Direction;
//栈保存①岔路口②坐标
unsigned char stackx[37];
unsigned char stacky[37];
uchar f = 0, r = 1, b = 2, l = 3;
uchar Way_dirc = 0;
#define MOUSE_IR_ON(GROUP_NO) \
    do                        \
    {                         \
        A0 = (GROUP_NO)&0x01; \
        A1 = (GROUP_NO)&0x02; \
        A2 = (GROUP_NO)&0x04; \
    } while (0)

//延迟函数
void delay_ms(uint ms)
{
    while (ms--)
    {
        for (t = 0; t < 114; t++);
    }
}

void initTime2(uint us)
{
    //T2中断允许
    EA = 1;
    ET2 = 1;
    TH2 = RCAP2H = (65536 - us) / 256;
    TL2 = RCAP2L = (65536 - us) % 256;
    TR2 = 1;
}

void amend()//修正函数
{
		uchar code straight[] = {0x11,0x93,0x82,0xc6,0x44,0x6c,0x28,0x39};
		if (!irC && irL && irR)
		{
				if (irLU)
				{
						while(1){
								for (i = 0; i < 8; i++)
								{
										P1 = (straight[i] | 0xf0);
										delay_ms(1);
										if(!irLU)
										break;
								}
								if(!irLU)
										break;
						}
				}
				if (irRU)
				{
						while(1){
								for (i = 0; i < 8; i++)
								{
										P1 = (straight[i] | 0x0f);
										delay_ms(1);
										if(!irRU)
										break;
								}
								if(!irRU)
										break;
						}
				}
		}
}

void turn(uchar l)//转弯函数
{		
		uchar code right[] = {0x11, 0x33, 0x22, 0x66, 0x44, 0xcc, 0x88, 0x99};
		uchar code left[] = {0x11, 0x99, 0x88, 0xcc, 0x44, 0x66, 0x22, 0x33};
    if (l == 3)
    {
        for (j = 0; j < 50; j++)
        {
            for (i = 0; i < 8; i++)
            {
                P1 = left[i];
                delay_ms(1);
            }
        }
        ab_dric = (ab_dric + 3) % 4;
    }
    else if (l == 1)
    {
        for (j = 0; j < 50; j++)
        {
            for (i = 0; i < 8; i++)
            {
                P1 = right[i];
                delay_ms(1);
            }
        }
        ab_dric = (ab_dric + 1) % 4;
    }
    else if (l == 2)
    {
        for (j = 0; j < 50; j++)
        {
            for (i = 0; i < 16; i++)
            {
                P1 = right[i % 8];
                delay_ms(1);
            }
        }
        ab_dric = (ab_dric + 2) % 4;
    }
}
void modify_position()//修改坐标
{
		if(ab_dric == 0)
				y++;
		else if(ab_dric == 1)
				x++;
		else if(ab_dric == 2)
				y--;
		else if(ab_dric == 3)
				x--;
}

void go_straight()//直走&修正&记录坐标
{
		uchar code straight[] = {0x11,0x93,0x82,0xc6,0x44,0x6c,0x28,0x39};
    j = 0;
    while (j < 104)
    {
        for (i = 0; i < 8; i++)
        {
						amend();
            P1 = straight[i];
            delay_ms(2);
        }
        j++;
    }
		delay_ms(10);
		modify_position();
}

//传入小车的相对方向是否走过
uchar boolean_road(uint pos)
{
    uint temp;
    //转弯后的绝对方向 =（转弯前的绝对方向(ab_dric) + 转弯数值(pos)）%4
    pos = (ab_dric + pos) % 4;
		if(pos == 0){
				return temp = (Maze[x][y+1]>>4) == 0x0f;}
		if(pos == 1){
				return temp = (Maze[x+1][y]>>4) == 0x0f;}
		if(pos == 2){
				return temp = (Maze[x][y-1]>>4) == 0x0f;}
		if(pos == 3){
				return temp = (Maze[x-1][y]>>4) == 0x0f;}
		return 0;
}
void info_dirc()//记录来的方向
{
		if (ab_dric == 0)
				Maze[x][y] = Maze[x][y] & 0xdf;
		if (ab_dric == 1)
				Maze[x][y] = Maze[x][y] & 0xef;
		if (ab_dric == 2)
				Maze[x][y] = Maze[x][y] & 0x7f;
		if (ab_dric == 3)
				Maze[x][y] = Maze[x][y] & 0xbf;
}
void save()
{
    num = 0;
    //记录当前位置有几条路可以走
    if (!irL && boolean_road(l))
    {
        num++;
        Way_dirc = 3;
    }
    if (!irR && boolean_road(r))
    {
        num++;
        Way_dirc = 1;
    }
    if (!irC && boolean_road(f))
    {
        num++;
        Way_dirc = 0;
    }

    if (Maze[x][y] == 0xff)
    {
				// 记录低四位
				if(ab_dric==0)
				{
						if(irC==0)
						{
								Maze[x][y]&=0xf5;
						}
						if(irR==0)
						{
								Maze[x][y]&=0xf9;
						}
						if(irL==0)
						{
								Maze[x][y]&=0xfc;
						}
						if(irC&&irR&&irL)
						{
								Maze[x][y]&=0xfd;
						}
				}
				else if(ab_dric==1)
				{
						if(irC==0)
						{
								Maze[x][y]&=0xfa;
						}
						if(irR==0)
						{
								Maze[x][y]&=0xf9;
						}
						if(irL==0)
						{
								Maze[x][y]&=0xf6;
						}
						if(irC&&irR&&irL)
						{
								Maze[x][y]&=0xfe;
						}
				}
				else if(ab_dric==2)
				{
						if(irC==0)
						{
								Maze[x][y]&=0xf5;
						}
						if(irR==0)
						{
								Maze[x][y]&=0xf6;
						}
						if(irL==0)
						{
								Maze[x][y]&=0xf3;
						}
						if(irC&&irR&&irL)
						{
								Maze[x][y]&=0xf7;
						}
				}
				else if(ab_dric==3)
				{
						if(irC==0)
						{
								Maze[x][y]&=0xfa;
						}
						if(irR==0)
						{
								Maze[x][y]&=0xf3;
						}
						if(irL==0)
						{
								Maze[x][y]&=0xf9;
						}
						if(irC&&irR&&irL)
						{
								Maze[x][y]&=0xfb;
						}
				}
				info_dirc();
    }
}
void fork()//记录岔路口
{		
		if(num > 1){
				stackx[flag_stack] = x;
				stacky[flag_stack] = y;
				flag_stack++;
		}
}
//回溯函数
void recall_back()
{
		uchar pos1 = 0;
		uchar pos2 = 0;
		while(stackx[flag_stack-1] != x || stacky[flag_stack-1] != y)
		{
				if (Maze[x][y] >> 4 == 0x7)
						pos1 = 0;
				else if (Maze[x][y] >> 4 == 0xb)
						pos1 = 1;
				else if (Maze[x][y] >> 4 == 0xd)
						pos1 = 2;
				else if (Maze[x][y] >> 4 == 0xe)
						pos1 = 3;
				pos2 = (pos1 + 4 - ab_dric) % 4;
				if(x == 0 && y == 0){
						flag_recall = 1;
						break;
				}
				if (pos2 == 0)
				{
						go_straight();
				}
				else
				{
						turn(pos2);
						go_straight();
				}
		}
}
//初始化队列
void initStack()
{
		for (i = 0; i < 50; i++)
		{
				stackx[i] = 0;
				stacky[i] = 0;
		}
}
//初始化等高表
void initbest()
{
		for (i = 0; i < 8; i++)
		{
				for (j = 0; j < 8; j++)
				{
						best_board[i][j] = 0xff;
				}
		}
}
//创建等高表, 遍历完成后, 计算最短路径
void path()
{
		initStack();
		initbest();
		best_board[0][0] = 1;
		stackx[0] = 0;
		stacky[0] = 0;
		all+=1;
		while (now != all)
		{
				pathx = stackx[now];
				pathy = stacky[now];
				if(pathx + 1 < 8)
						if ((Maze[pathx][pathy] & 0x04) == 0x00 && (best_board[pathx + 1][pathy]) == 0xff)
						{
								best_board[pathx + 1][pathy] = best_board[pathx][pathy] + 1;
								stackx[all] = pathx + 1;
								stacky[all] = pathy;
								all++;
						}
				if(pathy + 1 < 8)
						if ((Maze[pathx][pathy] & 0x08) == 0x00 && (best_board[pathx][pathy + 1]) == 0xff)
						{
								best_board[pathx][pathy + 1] = best_board[pathx][pathy] + 1;
								stackx[all] = pathx;
								stacky[all] = pathy + 1;
								all++;
						}
				if(pathx - 1 > - 1)
						if ((Maze[pathx][pathy] & 0x01) == 0x00 && (best_board[pathx - 1][pathy]) == 0xff)
						{
								best_board[pathx - 1][pathy] = best_board[pathx][pathy] + 1;
								stackx[all] = pathx - 1;
								stacky[all] = pathy;
								all++;
						}
				if(pathy - 1 > - 1)
						if ((Maze[pathx][pathy] & 0x02) == 0x00 && (best_board[pathx][pathy - 1]) == 0xff)
						{
								best_board[pathx][pathy - 1] = best_board[pathx][pathy] + 1;
								stacky[all] = pathx;
								stacky[all] = pathy - 1;
								all++;
						}
				now++;
		}
}

//寻优
void find_best()
{
		initStack();
		pathx = pathy = 7;
		i = best_board[7][7];
		while( i >= 1)
		{
				stackx[i] = pathx;
				stacky[i] = pathy;
				i--;
				if (pathy - 1 >= 0 && best_board[pathx][pathy - 1] == i)
				{
						pathy--;
				}
				else if (pathx - 1 >= 0 && best_board[pathx-1][pathy ] == i)
				{
						pathx--;
				}
				else if (pathx + 1 <= 7 && best_board[pathx+1][pathy] == i)
				{
						pathx++;
				}
				else if (pathy + 1 <= 7 && best_board[pathx][pathy +1] == i)
				{
						pathy++;
				}
		}
}
//广度优先，进行冲刺
void sp()
{		
		path();
		find_best();
		pathx = pathy = 0;
		Direction = 0;
		pathnum = 0;
		while(pathx != 7 || pathy != 7)//判断终止条件，是否到达终点
		{
				pathnum++;
				switch (Direction)
				{
					case 0: {
							if (stackx[pathnum] == pathx + 1)
							{
									pathx += 1;
									Direction=(Direction+1)%4;
									turn(1);
									go_straight();
							}
							if (stackx[pathnum] == pathx - 1)
							{
									pathx -= 1;
									Direction=(Direction+3)%4;
									turn(3);
									go_straight();
							}
							if (stacky[pathnum] == pathy + 1)
							{
									pathy += 1;
									go_straight();
							}
							if (stacky[pathnum] == pathy - 1)
							{
									pathy -= 1;
									turn(2);
									Direction=(Direction+2)%4;
									go_straight();
							}
					}
					break;
					case 1: {
							if (stackx[pathnum] == pathx + 1)
							{
									pathx += 1;
									go_straight();
							}
							if (stackx[pathnum] == pathx -1)
							{
									pathx -= 1;
									turn(2);
									Direction=(Direction+2)%4;
									go_straight();
							}
							if (stacky[pathnum] == pathy + 1)
							{
									pathy += 1;
									turn(3);
									Direction=(Direction+3)%4;
									go_straight();
							}
							if (stacky[pathnum] == pathy - 1)
							{
									pathy -= 1;
									turn(1);
									Direction=(Direction+1)%4;
									go_straight();
							}
					}
					break;
					case 2: {
							if (stackx[pathnum] == pathx + 1)
							{
									pathx += 1;
									turn(3);
									Direction=(Direction+3)%4;
									go_straight();
							}
							if (stackx[pathnum] == pathx - 1)
							{
									pathx -= 1;
									turn(1);
									Direction=(Direction+1)%4;
									go_straight();
							}
							if (stacky[pathnum] == pathy + 1)
							{
									pathy += 1;
									turn(2);
									Direction=(Direction+2)%4;
									go_straight();
							}
							if (stacky[pathnum] == pathy - 1)
							{
									pathy -= 1;
									go_straight();
							}
					}
					break;
					case 3: {
							if (stackx[pathnum] == pathx + 1)
							{
									pathx += 1;
									turn(2);
									Direction=(Direction+2)%4;
									go_straight();
							}
							if (stackx[pathnum] == pathx - 1)
							{
									pathx -= 1;
									go_straight();
							}
							if (stacky[pathnum] == pathy + 1)
							{
									pathy += 1;
									turn(1);
									Direction=(Direction+1)%4;
									go_straight();
							}
							if (stacky[pathnum] == pathy - 1)
							{
									pathy -= 1;
									turn(3);
									Direction=(Direction+3)%4;
									go_straight();
							}
					}
					break;
			}
		}
		while(1);
}
//右手法则
void right_hand()
{
		if (!irR && boolean_road(r))
		{
				turn(r);
				go_straight();
		}
		else
		{
				if (!irC && boolean_road(f))
				{
						go_straight();
				}
				else
				{
						if (!irL && boolean_road(l))
						{
								turn(l);
								go_straight();
						}
				}
		}
}
//开始移动
void move_start()
{
    if(x == 0 && y == 0 && begin == 0)
    {
        begin = 1;
        go_straight();
    }
		else if (begin == 1)
    {
				flag2 = 1;
				//到达终点滴
        if (x == 7 && y == 7)
        {
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
						BEEP = 0;
            delay_ms(150);
            BEEP = 1;
        }
				save();//记录信息
				fork();
        if (num >= 2)
        {
						//当有岔路口时响两声
            BEEP = 0;
            delay_ms(10);
            BEEP = 1;
						delay_ms(10);
						BEEP = 0;
            delay_ms(10);
            BEEP = 1;
						//右手法则
            right_hand();
        }
        else if (num == 1)
        {
            if (Way_dirc == 0)
                go_straight();
            else
            {
                turn(Way_dirc);
                go_straight();
            }
        }
        //遇到死胡同时调用回溯
        else if (num == 0)
        {
						flag2 = 0;
						if(stackx[flag_stack-1] == x && stacky[flag_stack-1] == y)
								flag_stack--;
						recall_back();
						//跳出循环
						if(flag_recall == 1)
								return;
				}
    }
}
//红外传感器
void time2() interrupt 5
{
    static uint ir = 0, lastIr = 1;
    TF2 = 0;
    if (ir != lastIr)
    {
        MOUSE_IR_ON(ir % 5);
        ir++;
    }
    else
    {
        lastIr++;
        switch ((ir - 1) % 5)
        {
        case 0:
            irC = irR1_C == 0 ? 1 : 0;
            break;
        case 1:
            irLU = irR2_LU == 0 ? 2 : 0;
            break;
        case 2:
            irL = irR3_L == 0 ? 3 : 0;
            break;
        case 3:
            irR = irR4_R == 0 ? 4 : 0;
            break;
        case 4:
            irRU = irR5_RU == 0 ? 5 : 0;
            break;
        }
    }
}

//初始化
void init_variable()
{
		for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
            Maze[i][j] = 0xff;
        }
    }
		Maze[0][0] = 0xf5;
		stackx[0] = 0;
		stacky[0] = 0;
		flag_stack = 1;
}
void main()
{
    initTime2(5000);
		init_variable();
    while (1)
    {
        delay_ms(100);
        move_start();
				//跳出
				if(flag_recall == 1)
						break;
    }
		turn(2);
		//建立登高表，寻找最短路径，并进行冲刺
		sp();
		while(1);
}