#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define PORT 8080 
//method for user input
void userInput(int* userInputArray, int planesSizeRow,int PlaneSizeCol){
	int row, column;
	//primt the user and offer some details 
	printf("%s%d%s%d\n","Hello. please enter your seat numbers seperated with a space, seats start at 1 1 and go to",planesSizeRow,"x",PlaneSizeCol);
	printf("%s\n","an example would be '1 2' this will reserve the second seat in the first row ");
	fflush(stdout);
	//--------select a seat as long as no valid entry has been made ---------
	do{
    		scanf("%d %d", &row,&column);
		if(row>planesSizeRow||column>PlaneSizeCol||row<0||column<0){
			printf("%s\n","im sorry you have sellected an invalid seat, please try again");
		}
	}while(row>planesSizeRow||column>PlaneSizeCol||row<0||column<0);
	//------------------------------------------------------------------------
	//asign the values to be used
	userInputArray[0]=row-1;
	userInputArray[1]=column-1;
}//end user input
//main method 
int main(int argc, char const *argv[]) {
	//variables  
	struct sockaddr_in serv_addr;
	char message[20];
	int sock = 0;  
	int* userInputArray =(int*) malloc(2 * sizeof(int));
	int rowread,columnread,row,column;
	// set up the socket id and check for errors
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
   	//set the conecting struct to all zeros
	memset(&serv_addr, '0', sizeof(serv_addr)); 
   	//initulize the family and port of the adress
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
       
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
	        return -1; 
	} 
	
	// attempt to conect wit the server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
	        return -1; 
	} 
	//read the dimentions of the plane
	read( sock ,&rowread, sizeof(int));//row
	read( sock ,&columnread, sizeof(int));//col
	
	setvbuf(stdout, NULL, _IONBF, 0);
	// greet the use and ask how many thickets they want
	printf("%s\n","hello and thank you for choosing sky high airlines");
	tickets:
	printf("%s\n","how manny tickets would you like?");
	int x,y=(rowread*columnread);	
	scanf("%d",&x);
	//send number of tickets to the server
	if(x<=y){
		send(sock,(void *)&x,sizeof(int),0);
	}else{
		printf("%s","thats too many tickets for this plane");
		goto tickets;		
	}
	//the value to send if the user chouse auto. 
	int autosend;
	for(int i=0;i<x;i++){

		//goto tag for the input if the user enterd invalid selection 
		input:
		//promp the user for the way they want to select thier seat.
		printf("%s\n","how would you like to choose your seat?,by manual(m) or automatic(a) selection?");
		//take in ther choice and make sure its always lowercase
		scanf("%s",message);
		for(int j = 0; message[j]; j++){
  			message[j] = tolower(message[j]);
		}


		// test to see if the input is valid, and deturmine what to do based on user responce.
		if(strcmp(message,"automatic")==0||strcmp(message,"a")==0){
			// set the variable to send and send it
			autosend = -1;
			send(sock,(void *)&autosend,sizeof(int),0);
			send(sock,(void *)&autosend,sizeof(int),0);
			
		}else	if(strcmp(message,"manual")==0||strcmp(message,"m")==0){
			//get input from user and send it
			userInput(userInputArray,rowread,columnread);
			send(sock,(void *)&userInputArray[0],sizeof(int),0);
			send(sock,(void *)&userInputArray[1],sizeof(int),0);
		
		}else{
			//if the user gave invalid input go back to the start without incrimenting the loop variable 
			printf("%s\n","error, invalid input");
			goto input;
		}
		// the return code for succsesfully reserving a seat or not
		int returnVal;
		read( sock ,&returnVal, sizeof(int));
		// tell the user what happend based on the return value
		if(returnVal==1){
			printf("%s\n","your ticket is confirmed");
		}else if(returnVal==0){
			i--;
			printf("%s\n","sorry that seat is taken");		
		}
		
	}
	return 0;
} 
