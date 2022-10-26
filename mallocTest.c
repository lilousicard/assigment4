#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ROW 10
#define COL 100
int main(void) {
  size_t buff = 100;
  char** stringArr = (char**)malloc(sizeof(char*)*ROW);
  char *input = (char*)malloc(sizeof(char)*COL);
  for (int i = 0; i<ROW; i++){
	getline(&input,&buff,stdin);
  	stringArr[i] = (char*) malloc (sizeof(char)*strlen(input)+1);
	input[strlen(input)-1]=0;
	for(int j = 0; j<strlen(input); j++){
		stringArr[i][j] =input[j];
	}
  }

  
  stringArr = (char**)realloc(stringArr,sizeof(char*)*20);

  for(int i = 10; i<20; i++){
	  getline(&input,&buff,stdin);
	  stringArr[i] = (char*) malloc(sizeof(char)*strlen(input)+1);
	  input[strlen(input)-1]=0;
	  for(int j=0; j<strlen(input);j++){
		  stringArr[i][j] = input[j];
	  }
  }
  
  free(input);
  for (int i = 0; i< ROW+10; i++){
    printf("%s\n",stringArr[i]);
    free((void*)stringArr[i]);
  }
  
  free((void*)stringArr);
  
  return 0;
}
