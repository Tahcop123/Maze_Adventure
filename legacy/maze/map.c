#include "map.h"

linkedlistADT Wall = NULL;//墙体链表

WallT curwall = NULL;// 当前墙 

int Row=20,Col=30;   //迷宫尺寸 

int map[100][100];           /*存储用随机生成的原始迷宫*/

int map_change[100][100];    /*存储走迷宫、设计迷宫中的改变后的迷宫*/

int direction[4][2]={{0,-1},{0,1},{1,0},{-1,0}};  /*人移动的方向数组*/

int is_key = 0;//是否拿到钥匙 

int X = 2,Y = 2;//人的坐标

int xk,yk;//钥匙坐标
 
int can_set(int x,int y)
{
    /*判断此墙上下左右四个方向是否只有一个位置是路*/
    int i,count=0;
    for (i=0;i<4;i++){
        int x1=x + direction[i][0];
        int y1=y + direction[i][1];
        if(map[x1][y1]==0||map[x1][y1]==-1){
        	count++;
		}
    }
    if(count<=1){
    	return 1;
	}else{
		return 0;
	}
}

//随机生成地图   3-墙 0-路 4-起点 5-终点 1-解 2-钥匙 
void CreatMap()
{
    int i,j;
    struct location road[10000];
    /*先将迷宫所有位置都设置为墙,即障碍物*/
    for (i=1;i<=Row;i++)
    {
        for (j=1;j<=Col;j++)
        {
            map[i][j]=3;
        }
    }
    /*在迷宫外围设置一层保护层,避免挖出界*/
    for (i=0;i<=Row+1;i++)
    {
        map[i][0]=-1;
        map[i][Col+1]=-1;
    }
    for (j=0;j<=Col+1;j++)
    {
        map[0][j]=-1;
        map[Row+1][j]=-1;
    }

    /*参考随机prim算法生成迷宫,运用队列road*/
    int head=0,tail=0;  /*队头队尾初始化为0*/
    road[head].x=2; road[tail].y=2;    /*选择初始值(2,2)加入队列*/

    while (head<=tail)  /*若墙队列不为空*/
    {
        /*在墙队列中任取一点*/
        srand(time(0));
	    int r=rand() % (tail-head+1)+head;
        int x=road[r].x;
        int y=road[r].y;

        /*判断此墙是否满足设置为路的要求*/
        if (can_set(x,y))
        {
            map[x][y]=0;
            /*若设置成路,则将该位置周围的所有墙插入队列*/
            for (i=0;i<4;i++)
   			{
       			int x_next=x+direction[i][0];
        		int y_next=y+direction[i][1];
        		if (map[x_next][y_next]==3) 
        		{
        			tail++;
        			road[tail].x=x_next;
        			road[tail].y=y_next; 
				}
    		}
        } 

        /*从墙队列中删除当前位置所在节点*/
        struct location t;
    	t=road[head]; road[head]=road[r]; road[r]=t;
        head++;
    }

    /*设置起点与终点*/
    map[2][2]=4; 
    for (i=Row-1;i>=0;i--)
    	if (map[i][Col-1]==0){
    		map[i][Col-1]=5;
    		break;
		}

    /*将生成的初始地图拷贝一份*/
    for (i=1;i<=Row;i++)
        for (j=1;j<=Col;j++)
        {
            map_change[i][j]=map[i][j];
        }
}

//建立墙的链表 
void CreatWalllist(){
	double Wallx,Wally;//墙体的长宽 
	
	int i,j;
	
	Wallx = winwidth/Col;
	Wally = 5.0*winheight/7/Row;
	
	// Bug 10 fix: free old wall list before creating new one
	if (Wall != NULL) {
		FreeLinkedList(Wall);
		Wall = NULL;
	}
	Wall = NewLinkedList();//建立强的链表 
	for(i = 1; i <=Row; i++ ){
		for(j = 1; j <= Col;j++){ 
			WallT rptr;
			rptr = GetBlock(sizeof(*rptr));
			rptr->wx = Wallx*1.0/2 + (j-1) * Wallx;
			rptr->wy = winheight / 7 + (Row-i+1.0/2) * Wally;
			rptr->x0 = i;
			rptr->y0 = j;
			
			rptr->isSelected=FALSE;
			InsertNode(Wall, NULL, rptr);
		}
	}
}




//根据数字不同画墙 
void DrawWall(void *rect, int n)
{
	WallT r = (WallT)rect;
	int pensize = GetPenSize();/*保存当前系统笔画粗细*/
	string color = GetPenColor();/*保存当前系统颜色*/

	double mx,my;
		mx=r->wx;
		my=r->wy;
		
	if(n == 3 || n == -1){
		
		SetPenSize(2); 
		SetPenColor("Brown");
		StartFilledRegion(1.0);
		MovePen(mx-ww/2,my+wh/2);
		DrawLine(ww,0);
		DrawLine(0,-wh);
		DrawLine(-ww,0);
		DrawLine(0,wh);
		EndFilledRegion();

		SetPenColor("Black");
		MovePen(mx-ww/2,my+wh/6);
		DrawLine(ww,0);
		MovePen(mx-ww/2,my-wh/6);
		DrawLine(ww,0);
		MovePen(mx,my+wh/2);
		DrawLine(0,-wh/3);
		MovePen(mx,my-wh/2);
		DrawLine(0,wh/3);	
		MovePen(mx-ww/4,my+wh/6);
		DrawLine(0,-wh/3);
		MovePen(mx+ww/4,my+wh/6);
		DrawLine(0,-wh/3);
		
		MovePen(mx-ww/2,my+wh/2);
		SetPenColor("Gray");
		DrawLine(ww,0);
		DrawLine(0,-wh);
		DrawLine(-ww,0);
		DrawLine(0,wh);
		
	}else if(n == 1){//画路 
		SetPenColor("Cyan");
		StartFilledRegion(0.6);
		MovePen(mx-ww/2,my+wh/2);
		DrawLine(ww,0);
		DrawLine(0,-wh); 
		DrawLine(-ww,0);
		DrawLine(0,wh);
		EndFilledRegion();
		
	}else {
		//画解 
		SetPenColor("Light Gray");
		StartFilledRegion(1.0);
		MovePen(mx-ww/2,my+wh/2);
		DrawLine(ww,0);
		DrawLine(0,-wh); 
		DrawLine(-ww,0);
		DrawLine(0,wh);
		EndFilledRegion();
	}
	//画起点 
	if(n == 4){
		SetPenColor("Red");
		StartFilledRegion(1.0);
		MovePen( mx+ww/7  , my );
		DrawArc(wh/4, 0, 360);
		EndFilledRegion();
	}
	//画终点
	if(n == 5){
		SetPenSize(2);
		SetPenColor("Red");
		MovePen( mx - ww / 4 , my+11*wh/32 );
		DrawLine(0,-7*wh/8);
		StartFilledRegion(1.0);
		MovePen( mx - ww / 4 , my+7*wh/16 );
		DrawLine(0,-7*wh/16);
		DrawLine( ww/2,9*wh/32);
		DrawLine( -ww/2,9*wh/32);
		EndFilledRegion();
	}
	//画钥匙 
	if(n == 2){
		SetPenSize(4);
		SetPenColor("Yellow");
		MovePen( mx+ww*2/5,my-wh/6);
		DrawArc(wh/4, 0, 360);
		SetPenSize(4);
		MovePen( mx,my);
		DrawLine(-ww/4,wh/4); 
		SetPenSize(3);
		DrawLine(ww/8,wh/8); 
		MovePen( mx-ww/8,my+wh/8);
		DrawLine(ww/8,wh/8); 
	}
	SetPenSize(pensize); /*恢复粗细*/
	SetPenColor(color);/*恢复颜色*/
}
//画被选中的块 
void DrawSelectedWall(void *rect, int n){
	WallT r = (WallT)rect;
	int pensize = GetPenSize();/*保存当前系统笔画粗细*/
	string color = GetPenColor();/*保存当前系统颜色*/
	double mx,my;
	mx=r->wx;
	my=r->wy;
	if(n == 3 || n == -1){
		SetPenSize(2); 
		SetPenColor("Brown");
		StartFilledRegion(1.0);
		MovePen(mx-ww/2,my+wh/2);
		DrawLine(ww,0);
		DrawLine(0,-wh);
		DrawLine(-ww,0);
		DrawLine(0,wh);
		EndFilledRegion();

		SetPenColor("Black");
		MovePen(mx-ww/2,my+wh/6);
		DrawLine(ww,0);
		MovePen(mx-ww/2,my-wh/6);
		DrawLine(ww,0);
		MovePen(mx,my+wh/2);
		DrawLine(0,-wh/3);
		MovePen(mx,my-wh/2);
		DrawLine(0,wh/3);	
		MovePen(mx-ww/4,my+wh/6);
		DrawLine(0,-wh/3);
		MovePen(mx+ww/4,my+wh/6);
		DrawLine(0,-wh/3);
		
		MovePen(mx-ww/2,my+wh/2);
		SetPenColor("Red");
		DrawLine(ww,0);
		DrawLine(0,-wh);
		DrawLine(-ww,0);
		DrawLine(0,wh);
	}else if(n == 1){//画路 
		SetPenColor("Cyan");
		StartFilledRegion(1.0);
		MovePen(mx-ww/2,my+wh/2);
		DrawLine(ww,0);
		DrawLine(0,-wh); 
		DrawLine(-ww,0);
		DrawLine(0,wh);
		EndFilledRegion();
		
		SetPenColor("Red");
		DrawLine(ww,0);
		DrawLine(0,-wh);
		DrawLine(-ww,0);
		DrawLine(0,wh);
		
	}else {//画解 
		SetPenColor("Light Gray");
		StartFilledRegion(1.0);
		MovePen(mx-ww/2,my+wh/2);
		DrawLine(ww,0);
		DrawLine(0,-wh); 
		DrawLine(-ww,0);
		DrawLine(0,wh);
		EndFilledRegion();
		
		SetPenColor("Red");
		DrawLine(ww,0);
		DrawLine(0,-wh);
		DrawLine(-ww,0);
		DrawLine(0,wh);
	}
	//画起点 
	if(n == 4){
		SetPenColor("Red");
		StartFilledRegion(1.0);
		MovePen( mx+ww/7  , my );
		DrawArc(wh/4, 0, 360);
		EndFilledRegion();
	}
	//画终点
	if(n == 5){
		SetPenSize(2);
		SetPenColor("Red");
		MovePen( mx - ww / 4 , my+11*wh/32 );
		DrawLine(0,-7*wh/8);
		StartFilledRegion(1.0);
		MovePen( mx - ww / 4 , my+7*wh/16 );
		DrawLine(0,-7*wh/16);
		DrawLine( ww/2,9*wh/32);
		DrawLine( -ww/2,9*wh/32);
		EndFilledRegion();
	} 

	SetPenSize(pensize); /*恢复粗细*/
	SetPenColor(color);/*恢复颜色*/
}
//画人 
void DrawPeople(WallT rect){
	//记录人的坐标 
	X = rect->x0;
	Y = rect->y0; 
	int pensize = GetPenSize();/*保存当前系统笔画粗细*/
	string color = GetPenColor();/*保存当前系统颜色*/
	
	double mx,my;
	mx=rect->wx;
	my=rect->wy;
	
	SetPenColor("Black");
	StartFilledRegion(1.0);
	MovePen( mx+ww/15, my + wh/4  );
	DrawArc(3*wh/16, 0, 360);
	EndFilledRegion();
	SetPenSize(3);
	MovePen(mx-ww/20, my + wh/4);
	DrawLine(0,-wh/2);
	DrawLine(ww/4,-wh/8);
	DrawLine(-ww/4,wh/8);
	DrawLine(-ww/4,-wh/8);
	MovePen(mx-ww/20,my);
	DrawLine(ww/4,-wh/8);
	DrawLine(-ww/4,wh/8);
	DrawLine(-ww/4,-wh/8);
	SetPenSize(pensize); /*恢复粗细*/
	SetPenColor(color);/*恢复颜色*/
}
//画地图 
void Drawmap(int a[][100]){
	double Wallx,Wally;//墙体的长宽 
	int i=1,j=0;
	WallT rptr;
	
	Wallx = winwidth/Col;
	Wally = 5.0*winheight/7/Row;

	linkedlistADT ptr;
	
	ptr = NextNode(Wall, Wall);
	if (ptr == NULL) return NULL;

	while (ptr != NULL){
		rptr = (WallT)NodeObj(Wall, ptr) ;
		DrawWall(rptr,a[rptr->x0][rptr->y0]);
		if( rptr->x0 == X && rptr->y0 == Y){
				DrawPeople(rptr);
		} 
		ptr = NextNode(Wall, ptr);
  	    
	}
	// Bug 7 fix: draw hint text only once per frame
    MovePen(winwidth/5*2,8*winheight/9);
	SetPointSize(20);
	SetPenColor("Yellow"); 
	DrawTextString("提示:获得钥匙后才可以通关!");
	SetPointSize(1);
	
}
//与墙的距离 
double distWall(double x, double y, WallT rect)
{
	double x0, y0;
	x0 = rect->wx;
	y0 = rect->wy;
	return fabs(x-x0)+fabs(y-y0);
}
//选取最近的块 
WallT SelectNearestNode(linkedlistADT Wall , double mx, double my)
{
	linkedlistADT nearestnode = NULL, ptr;
	double mindistance, dist;
	ptr = NextNode(Wall, Wall);
	if (ptr == NULL) return NULL;
    nearestnode = ptr;
  	mindistance = distWall(mx, my, (WallT)NodeObj(Wall, ptr));
	while(NextNode(Wall, ptr) != NULL){
		ptr = NextNode(Wall, ptr);
  	    dist = distWall(mx, my, (WallT)NodeObj(Wall, ptr));
		if(dist < mindistance){
			nearestnode = ptr;
			mindistance = dist;
		}
	}
	return (WallT)NodeObj(Wall, nearestnode);
}
//判断是否到达终点或者生命值是否降低为0 
void Judge()
{
	if(Hp <=0 && is_maze == 1){
		char s;
		InitConsole();
		printf("Game Over!\n");
		scanf("%c",&s);
		if(s>=0){
			fclose(stdin);
			fclose(stdout);
			fclose(stderr);
			FreeConsole();
			DisplayClear();	
		    is_interface = 1;
		    is_maze = 0;
		    drawInterface();
		}
	}
	if(map_change[X][Y] == 5){
		if(is_key == 1){
			char s;
		    end = clock();
	    	InitConsole();
		    printf("您用时%f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
		    printf("您共用%d步\n",step);
		    printf("按任意键继续\n");
		    scanf("%c",&s);
		    if(s>=0){
			    fclose(stdin);
			    fclose(stdout);
			    fclose(stderr);
			    FreeConsole();
		    }
		    is_interface = 1;
		    is_maze = 0;
		    drawInterface();
		}
	}
} 
//判断是否拿到钥匙 
void Judgekey()
{
	if(map_change[X][Y]==2){
		map_change[X][Y]=0;
		is_key = 1;
	}
}
