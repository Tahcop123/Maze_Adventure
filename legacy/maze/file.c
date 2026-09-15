#include"file.h"

void Openfile()
{
	FILE *fp;
	int ch;
	int i=0,j=0;
	char filename[1024] = { 0 };
	bool b = OpenFileDialog("text file(*.txt)\0*.txt", filename);
	if(b){
		fp=fopen(filename,"r");
		if(fp == NULL){
			MessageBoxA(NULL, "Failed to open file!", "Error", MB_ICONERROR);
			return;
		}
	    while((ch = fgetc(fp)) != EOF){
	    	if(ch != '\n' && ch != '\r' && ch != '\0'){
	    		// validate digit: only '0'..'5' and '-' (for boundary -1) are valid
	    		if((ch >= '0' && ch <= '5') || ch == '-'){
			    	map_change[i][j]=ch-'0';
				}
		        j++;
		        if(j >= Col){
		        	i++;
		        	j=0;
		        	if(i >= Row) break; // prevent overflow
		    	}
		    }
	    }
	    fclose(fp);
	}

}
void Savefile()
{
	FILE *fp;
	char ch;
	int i=0,j=0;
	char filename[1024] = { 0 };
	int b = SaveFileDialog("text file(*.txt)\0*.txt", filename);
	if(b){
		fp=fopen(filename,"w");
		if(fp == NULL){
			MessageBoxA(NULL, "Failed to save file!", "Error", MB_ICONERROR);
			return;
		}
	    for(i=0;i<Row;i++){
	    	for(j=0;j<Col;j++){
	    		ch=map_change[i][j]+'0';
	    		fputc(ch,fp);
	    	}
	    	fputc('\n',fp);
	    }
    	fclose(fp);
	}

}
