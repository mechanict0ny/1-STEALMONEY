//compile: cc miro.c -o miro -lcurses -lpthread

#include <stdio.h>
#include <unistd.h>
#include <termio.h>
#include <curses.h>
#include <stdlib.h>
#include <pthread.h>

#define WALL 'X'
#define TREASURE '$'
#define STAR 'O'
#define BLANK ' '

#define OFFSET_ROW 2
#define OFFSET_COL 3

#define fROW 21
#define fCOL 40

#define MROW 4
#define MCOL 5

#define ROW 15
#define COL 24

#define TROW 3
#define TCOL 27

#define TIMER_ROW 5
#define TIMER_COL 5

#define LIMIT 30
#define PENALTY 3

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

int direction[4][2]={{1,0},{-1,0},{0,-1},{0,1}};
char field[fROW][fCOL];
char miro[ROW][COL];
char start[ROW][COL];
int limit;

int cur_row;
int cur_col;

int stage=1;

int pallete[3][5][3];
int play();
void* show_timer(void* m);
void draw_start(char* start_name);
void draw_map(char* map_name);
void draw_field(char* field_name);


int main(){
	char* map_list[3]={"map1.txt","map2.txt","map3.txt"};
	int result;

	initscr();
	noecho();
	curs_set(0);

	start_color();
	init_pair(1,COLOR_RED,COLOR_WHITE);
	init_pair(2,COLOR_RED,COLOR_BLACK);
	init_pair(3,COLOR_WHITE,COLOR_BLACK);

	// gradation 색상정보를 가져온다.
	// gradation.txt는 5개의 레벨을 가진 색상조합 3개가 합쳐진 형태이다.
	// 색상의 format은 RGB를 사용한다.
	FILE* fp=fopen("gradation.txt","r");
	for(int i=0;i<3;i++)
		for(int j=0;j<5;j++)
			fscanf(fp,"rgb(%d,%d,%d)\n",&pallete[i][j][0],&pallete[i][j][1],&pallete[i][j][2]);
	fclose(fp);

	draw_start("start.txt");
	if(getchar()=='q'){
	  endwin();
	  return 0;
	}
	clear();

	for(int i=0;i<3;i++){
		clear();
		draw_field("field.txt");
		draw_map(map_list[i]);

		result=play();

	//game over
	if(result==0){
		clear();
		addstr("GAME OVER");
		refresh();
		sleep(3);
		break;
	}

	//treasure!
	else if(result==1){
		move(cur_row,cur_col);
		addstr(" < YEAH!!!!!\n");
		refresh();
		sleep(1);
		limit=0;
		clear();
		if(i==2){
			addstr("success!!!\nclear all stage!!!");
	    refresh();
			sleep(3);
			break;
		}
		addstr("go to next stage");
		stage++;
		refresh();
		sleep(3);
	}

	//quit
	else{
		clear();
		addstr("you gave up TT");
		refresh();
		sleep(3);
		break;
	}
}

	endwin();
	return 0;
}

void* show_timer(void* m)
	{
		limit=LIMIT;
		for(limit;limit>0;limit--){
			move(TROW,TCOL);
			if(limit%2==0){
				attron(COLOR_PAIR(1));
				attroff(1);
			}
			else if(limit%2==1){
			 attron(COLOR_PAIR(2));
			 attroff(2);
			}
			addch('0'+limit/10);
			addch('0'+limit%10);
			attron(COLOR_PAIR(3));
			refresh();
			sleep(1);
		}
		return NULL;
	}

void draw_field(char* field_name){
	FILE*fp=fopen(field_name,"r");
	if(!fp){
		printf("FILE cannot be opened\n");
		exit(1);
	}

	for(int i=0;i<fROW;i++){
		fgets(field[i],fCOL,fp);
		move(i+OFFSET_ROW,0+OFFSET_COL);
		addstr(field[i]);
	}
	refresh();
	fclose(fp);
}

void draw_map(char* map_name){
		FILE*fp=fopen(map_name,"r");
		if(!fp){
			printf("FILE cannot be opened\n");
			exit(1);
		}

		for(int i=0;i<ROW;i++){
			fgets(miro[i],COL+2,fp);
			move(i+MROW+OFFSET_ROW,0+MCOL+OFFSET_COL);
			for(int j=0;j<COL;j++)
				addch(miro[i][j]);
		}
		refresh();
		fclose(fp);
}

void draw_start(char* start_name){
	FILE*fp=fopen(start_name,"r");
	if(!fp){
		printf("FILE cannot be opened\n");
		exit(1);
	}

	for(int i=0;i<fROW;i++){
		fgets(start[i],fCOL,fp);
		move(i+OFFSET_ROW,0+OFFSET_COL);
		addstr(start[i]);
		}
	refresh();
	fclose(fp);
}

int play(){
	char input;

	int arr_row=0;
	int arr_col=1;
	cur_row=OFFSET_ROW+MROW;
	cur_col=OFFSET_COL+MCOL+1;
	pthread_t t1;

	int dir;
	int pair=4,index=10,offset=0, increase=1;

	attron(COLOR_PAIR(3));
	move(cur_row,cur_col);
	addch(STAR);
	refresh();

	pthread_create(&t1,NULL,show_timer,NULL);


	while((input=getch())!='q'){

		if(input==27){
			getch();
			input=getch();
			if(input==65) dir=DOWN;
			else if(input==66) dir=UP;
			else if(input==68) dir=LEFT;
			else if(input==67) dir=RIGHT;
		}
		else continue;

		if(miro[arr_row+direction[dir][0]][arr_col+direction[dir][1]]==WALL){
			limit-=PENALTY; //시간 깎기
			continue;
		}

		init_color(index,(pallete[stage-1][offset][0]-5)*4, (pallete[stage-1][offset][1]-5)*4, (pallete[stage-1][offset][2]-5)*4);
		init_pair(pair,COLOR_WHITE,index++);

		attron(COLOR_PAIR(pair));
		move(cur_row,cur_col);
		addch(BLANK);
		cur_row+=direction[dir][0];
		cur_col+=direction[dir][1];
		move(cur_row,cur_col);
		addch(STAR);
		attroff(COLOR_PAIR(pair));
		arr_row=cur_row-(OFFSET_ROW+MROW);
		arr_col=cur_col-(OFFSET_COL+MCOL);

		if(miro[arr_row][arr_col]==TREASURE) return 1;
		if(limit<=0)return 0; //game over
		refresh();

		// color level change
		// 0->1->2->3->4->3->2->1->0
		if(increase) offset++;
		else offset--;

		if(offset==5) {
			increase=0;
			offset=3;
		}
		else if(offset==-1) {
			increase=2	;
			offset++;
		}
		pair++;

	}
	return 2; //quit
}
