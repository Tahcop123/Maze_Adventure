#include "solution.h"

int is_showsolution = 0; // whether to show solution
// BFS data
struct location entry,destination,current[10000];  // current stores BFS nodes
int pre[10000];                          // predecessor of each node
int visit[100][100]={0};				// visited flag
int shortstep=1;                        // shortest path step count
int is_showtip = 0;
int xx,yy;// tip target cell

// Check whether cell (x,y) is passable: road(0), key(2), start(4), end(5)
int can_go(int x,int y)
{
    if (x>0 && x<=Row && y>0 && y<=Col)
        if ((map[x][y]==0 || map[x][y]==2 || map[x][y]==5||map[x][y]==4) && !visit[x][y])
            return 1;
    return 0;
}

/*
 * Generic BFS from (sx,sy) to (tx,ty).
 * Marks path cells on map[] with value 1 (skipping start and end cells).
 * Returns step count (number of moves), or -1 if unreachable.
 * If next_x/next_y are non-NULL, stores the first move's target cell.
 */
int BFSPath(int sx, int sy, int tx, int ty, int *next_x, int *next_y)
{
    int i;
    int head=0, tail=0;
    current[head].x = sx;
    current[head].y = sy;
    pre[head] = -1;
    visit[sx][sy] = 1;  // Bug 1 fix: mark start as visited

    while (head<=tail)
    {
        for (i=0;i<4;i++)
        {
            int x=current[head].x+direction[i][0];
            int y=current[head].y+direction[i][1];
            if (can_go(x,y))
            {
                tail++;
                current[tail].x=x;
                current[tail].y=y;
                pre[tail]=head;
                visit[x][y]=1;
                if (x==tx && y==ty)
                {
                    // Backtrack path: mark cells, count steps, find first move
                    int t = tail;
                    int node_count = 0;
                    int first_t = -1;
                    while (t != -1)
                    {
                        if (pre[t] != -1)  // not the start node
                        {
                            // skip end cell so its flag (5 / key 2) is preserved
                            if (!(current[t].x==tx && current[t].y==ty))
                            {
                                map[current[t].x][current[t].y] = 1;
                            }
                            // pre[t]==0 means direct successor of start = first move
                            if (pre[t] == 0) first_t = t;
                        }
                        node_count++;
                        t = pre[t];
                    }
                    if (first_t != -1 && next_x && next_y)
                    {
                        *next_x = current[first_t].x;
                        *next_y = current[first_t].y;
                    }
                    return node_count - 1;  // step count = edges
                }
            }
        }
        head++;
    }
    return -1;
}

void OptimalSolution()
{
    int i,j,m,n;
    // Sync working map to BFS map (clear old solution marks)
    for (i=1;i<=Row;i++)
    {
    	for (j=1;j<=Col;j++)
		{
			if (map_change[i][j]!=1)
				map[i][j]=map_change[i][j];
			else map[i][j]=0;
		 }
	}

    // Locate start (player) and end
    entry.x=X;
    entry.y=Y;
    for (i=1;i<=Row;i++)
        for(j=1;j<=Col;j++)
        {
            if (map[i][j]==5)
            {
                destination.x=i;
                destination.y=j;
            }
        }

    // Locate key (if any)
    int kx=-1, ky=-1;
    for (i=1;i<=Row;i++)
        for(j=1;j<=Col;j++)
        {
            if (map[i][j]==2)
            {
                kx=i;
                ky=j;
            }
        }

    shortstep = 0;
    xx = entry.x;
    yy = entry.y;

    if (!is_key && kx > 0)
    {
        // Bug 2 fix: two-segment BFS — player -> key -> end
        int nx, ny;
        int s1 = BFSPath(entry.x, entry.y, kx, ky, &nx, &ny);
        if (s1 >= 0)
        {
            xx = nx;
            yy = ny;  // tip is the first move of segment 1
            // Reset visited array for second BFS
            for(m=0;m<100;m++)
                for(n=0;n<100;n++)
                    visit[m][n]=0;
            int s2 = BFSPath(kx, ky, destination.x, destination.y, NULL, NULL);
            if (s2 >= 0)
            {
                shortstep = s1 + s2;  // total steps through key
            }
        }
    }
    else
    {
        // Key already picked up (or no key): direct BFS player -> end
        int s = BFSPath(entry.x, entry.y, destination.x, destination.y, &xx, &yy);
        if (s >= 0) shortstep = s;
    }

    // Reset visited array
    for(m=0;m<100;m++){
   		for(n=0;n<100;n++){
		   	visit[m][n]=0;
		}
	}
}

void DrawSolution(){
	OptimalSolution();
	Drawmap(map);
}
void Solutionbutton(){
	double fH = GetFontHeight();
	double h = fH*2;  // button height
	double x = 0;
	double y = h;
	double w = winwidth/9; // button width
	double p,q;// tip position



	if( button(GenUIID(0), x, y, w, h, "Solution") ){
	//		DisplayClear();
	//		DrawSolution();
	//		Exitbutton();
		is_showsolution = !is_showsolution;

	    if(is_showsolution){
			if(is_showtip==1){
				is_showtip = 0;
			}
			DisplayClear();
//			DrawSolution();
			DrawPic("background.bmp",1920/3*2,1080/3*2);
			OptimalSolution();
			Drawmap(map);
			Draweditbox(is_edit);

		}else{
			DisplayClear();
			DrawPic("background.bmp",1920/3*2,1080/3*2);
			Drawmap(map_change);
			Draweditbox(is_edit);
		}
	}
	if( button(GenUIID(0), x+w, y, w, h, "tip")) {

		is_showtip = !is_showtip;
		if(is_showtip){
			OptimalSolution();
			if(is_showsolution == 1){
				is_showsolution = 0;
			}
			DisplayClear();
			DrawPic("background.bmp",1920/3*2,1080/3*2);
			Drawmap(map_change);
			Draweditbox(is_edit);
			p= ww*1.0/2 + (yy-1) * ww;
			q = winheight / 7 + (Row-xx+1.0/2) * wh;
			SetPenColor("Cyan");
			StartFilledRegion(0.6);
			MovePen(p-ww/2,q+wh/2);
			DrawLine(ww,0);
			DrawLine(0,-wh);
			DrawLine(-ww,0);
			DrawLine(0,wh);
			EndFilledRegion();
		}else{
			DisplayClear();
			DrawPic("background.bmp",1920/3*2,1080/3*2);
			Drawmap(map_change);
			Draweditbox(is_edit);
		}
	}
}
//edit box
void Draweditbox(int n){
	double fH = GetFontHeight();
	double h = fH*2;  // button height
	double x = 0;
	double y = 6*winheight/7+fH;
	double w = winwidth/9; // button width
	char a[100]="edit(Press x)";
	if(n==0){
		SetPenColor("Blue");
		MovePen(x,y);
		SetPenSize(1);
		DrawLine(w,0);
		DrawLine(0,h);
		DrawLine(-w,0);
		DrawLine(0,-h);
		MovePen(x+w/10,y+3*fH/5);
		DrawTextString(a);
	}else if(n==1){

		SetPenColor("Red");
		StartFilledRegion(1.0);
		MovePen(x,y);
		SetPenSize(1);
		DrawLine(w,0);
		DrawLine(0,h);
		DrawLine(-w,0);
		DrawLine(0,-h);
		EndFilledRegion();

		SetPenColor("Blue");
		MovePen(x,y);
		SetPenSize(1);
		DrawLine(w,0);
		DrawLine(0,h);
		DrawLine(-w,0);
		DrawLine(0,-h);
		MovePen(x+w/10,y+3*fH/5);
		DrawTextString(a);
	}

}
