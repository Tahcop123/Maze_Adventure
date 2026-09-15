
#include "event.h"


int is_mousedown = 0;//鼠标是否按下 
int is_edit = 0;//是否编辑 


double mousex,mousey;//鼠标坐标

//鼠标事件

void MouseEventProcess(int x, int y, int button, int event)
{
	if(is_interface){
		DisplayClear();
		uiGetMouse(x,y,button,event); //GUI获取鼠标
		Interfacedisplay(); // 刷新显示
	}
	if(is_maze){
//		DisplayClear(); 
		uiGetMouse(x,y,button,event);

		Solutionbutton();
		Exitbutton(); 
	/*	if(is_showsolution){
			Drawmap(map);
		} else{
			Drawmap(map_change);
		}*/
//		Exitbuttondisplay();
	}
	if(is_help1){
		uiGetMouse(x,y,button,event);
		DisplayClear();
	    DrawHelp();
	    Exitbutton();
	}
	if(is_about){
		uiGetMouse(x,y,button,event);
		DisplayClear();
	    DrawAbout();
	    Exitbutton();
	}
	
	if(is_edit) {
		
		int nx,ny;//当前块的坐标 
		static int ox,oy;//选中块的坐标 
		WallT ptr;

		
		mousex = ScaleXInches(x);/*pixels --> inches*/
		mousey = ScaleYInches(y);/*pixels --> inches*/
//		Drawmap(map_change);
//		curwall = SelectNearestNode(Wall, mousex, mousey);
//		DrawSelectedWall(curwall,map_change[curwall->x0][curwall->y0]);
		switch(event){
			case BUTTON_DOWN:
				if (button == LEFT_BUTTON){
					curwall = SelectNearestNode(Wall, mousex, mousey);
					ox = curwall->x0;
					oy = curwall->y0; 
					nx = ox;
					ny = oy;
					DrawSelectedWall(curwall,map_change[ox][oy]);
					if(nx==X &&ny==Y){
						DrawPeople(curwall);
					}
					
				}
				is_mousedown = 1;
				break;
			
			case BUTTON_UP:
				is_mousedown = 0;
				Drawmap(map_change);
				break;
				
			case MOUSEMOVE:
				if(is_mousedown){
				
				ptr = SelectNearestNode(Wall, mousex, mousey);
				nx = ptr->x0;
				ny = ptr->y0;
				if(ptr == curwall){
					break;
				}else{
					int t;
					//交换两个的数值 
					t = map_change[ox][oy];
					map_change[ox][oy] = map_change[nx][ny];
					map_change[nx][ny] = t;
					if(ox==X && oy==Y){
						X = nx;
						Y = ny;
						
					}
					curwall = ptr;
					Drawmap(map_change);
					DrawSelectedWall(curwall,map_change[nx][ny]);
					if(nx==X &&ny==Y){
						DrawPeople(curwall);
					}
					ox = nx;
					oy = ny; 
				}
				
			}else{
				ptr = SelectNearestNode(Wall, mousex, mousey);
				if(ptr == curwall){
					
				} else{
				
				Drawmap(map_change);
				nx = ptr->x0;
				ny = ptr->y0;
				DrawSelectedWall(ptr,map_change[nx][ny]);
				
				if(nx==X &&ny==Y){
						DrawPeople(ptr);
					}
				
				curwall=ptr;
				
			}
			}
		}
	}
} 



//键盘回调函数 
// Bug 11 fix: unified player movement, replaces 4x duplicated direction code
void MovePlayer(int dx, int dy)
{
	int nx = X + dx, ny = Y + dy;
	// Hit wall: just redraw
	if (map_change[nx][ny] == 3) {
		if (is_showsolution) DrawSolution();
		else Drawmap(map_change);
		return;
	}
	// Reached end without key: prompt and redraw
	if (map_change[nx][ny] == 5 && is_key == 0) {
		if (is_showsolution) DrawSolution();
		else Drawmap(map_change);
		char s;
		InitConsole();
		printf("未获得钥匙!\n");
		scanf("%c",&s);
		if(s>=0){
			fclose(stdin);
			fclose(stdout);
			fclose(stderr);
			FreeConsole();
		}
		return;
	}
	// Valid move
	X = nx;
	Y = ny;
	Judgekey();
	DisplayClear();
	DrawPic("background.bmp",1920*2/3,1080*2/3);
	if (is_showsolution) DrawSolution();
	else Drawmap(map_change);
	Draweditbox(is_edit);
	Judge();
	Hp--;
	step++;
	is_showtip = 0;
	DrawHp(Hp);
}

void KeyboardEventProcess(int key,int event)
{
	char s;
	WallT ptr;
	int oldX, oldY;
	switch(event)
	{
		case KEY_DOWN:
			oldX = X; oldY = Y;
			if(key=='E'){//e随机生成 
				is_interface = 0;
				DisplayClear();
			    DrawPic("background.bmp",1920/3*2,1080/3*2);
			    CreatMap();
			    CreatWalllist();
			    X=Y=2; 
			    Randomkey(); 
			    OptimalSolution();
			    Hp = shortstep*2;
			    Drawmap(map_change);
			    is_maze = 1;
			    Draweditbox(is_edit);
			}else if(key=='H'){//H帮助 
				is_interface = 0;
		    	is_help1 = 1;
		        DisplayClear();
	            DrawHelp();
	            Exitbutton();
			}else if(key=='S'){//s保存 
				Savefile();
			}else if(key=='O'){//o打开 
	            is_interface=0;
				DisplayClear();
			    DrawPic("background.bmp",1920/3*2,1080/3*2);
				CreatMap(); 
	        	CreatWalllist();
				Openfile();
				X=Y=2;
				step = 0;//每次步数清零 
	            is_start = 0;//timer starts on first move 
	            is_key=0;
	            shortstep = 0;
	            OptimalSolution();
			    Hp = shortstep*2;
	        	Drawmap(map_change);
	        	is_maze =1;
	        	Draweditbox(is_edit);
			}else if(key==VK_UP && is_maze==1){
			MovePlayer(-1, 0);
		}else if(key==VK_LEFT && is_maze==1){
			MovePlayer(0, -1);
		}else if(key==VK_DOWN && is_maze==1){
			MovePlayer(1, 0);
		}else if(key==VK_RIGHT && is_maze==1){
			MovePlayer(0, 1);
		}else if(key == 'X'){
				//按下x开始编辑 
				is_edit = !is_edit;
				is_showsolution = 0;//推出编辑 
				is_showtip = 0;
				DisplayClear();
				DrawPic("background.bmp",1920/3*2,1080/3*2);
				Exitbutton();
				Solutionbutton();
				Drawmap(map_change);
				Draweditbox(is_edit);
			}else if(key == 'Q' ){
				if(is_edit){
					ptr = SelectNearestNode(Wall, mousex, mousey);
					map_change[ptr->x0][ptr->y0] = 3; 
					Drawmap(map_change);
				}
			}else if(key == 'W'){
				if(is_edit){
					ptr = SelectNearestNode(Wall, mousex, mousey);
					map_change[ptr->x0][ptr->y0] = 0; 
					Drawmap(map_change);
				}
			} 
			// Bug 6 fix: start timer on first valid directional move
			if((key==VK_UP||key==VK_DOWN||key==VK_LEFT||key==VK_RIGHT) && is_maze==1){
				if((X != oldX || Y != oldY) && !is_start){
					start = clock();
					is_start = 1;
				}
			}
		break;
	}

} 

