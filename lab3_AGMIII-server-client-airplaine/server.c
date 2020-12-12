#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 8080 
// a struckt to hold data when starting threads,this will enshure that each thread is aware of the dimentions of the plane and the socket descripter. 
struct argument_struct {
	int socket;
	int row,column;
};
//----
void *preform(void *arg);
void *PlaneManager(void *arg);
//----
// global variales for th list of thread ids and the plane 2d array
int** plane;
pthread_t tid[20]; 
//validation method for the selected seat
int seatIsValid(int row,int col){
	
	if (plane[row][col]==0){
		return 0;
	}
	return -1;
}
//validate if plane is full or not 
int IfPlaneIsFull(int row,int col){
	
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			if(plane[i][j]==0){
				return -1;
			}
		}
	}
	return 0;
}
//------main-------
int main(int argc, char const *argv[]) 
{ 
	//set variables
	int row=10,column =10;
	int server_fd, new_socket; 
    	struct sockaddr_in address;
	
    	int opt = 1; 
    	int addrlen = sizeof(address); 
	
	if(argc>2){
		row=atoi(argv[1]);
		column =atoi(argv[2]);
		
	}	
	
	printf("%s%d%s%d\n","the plane's size is: ",row," x ",column);
	//----------allocating memory for the plane---------
	plane = malloc( row*sizeof(int *) );// N is the number of the rows
	if (plane == NULL)
		return -1;
	for (int i = 0 ; i < row ; i++)
	{
		//for each element of the plane array allocate another array.
		plane[i] = malloc( column*sizeof(int) );// M is the number of the columns
		if (plane[i] == NULL)
			return -1;
	}
	//--------------------------------------------------
	struct argument_struct managerInfo ={new_socket,row,column}; 
	pthread_t managerID;
	//start the managaer thread
	if(pthread_create(&managerID, NULL, PlaneManager, &managerInfo)!=0){
			printf("thread failed");
		} 
		printf("%s\n","thread started");
	//--------------------------
    	// Creating socket file descriptor 
    	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    	{ 
    	    perror("socket failed"); 
    	    exit(EXIT_FAILURE); 
    	} 
       
    	// Forcefully attaching socket to the port 8080 
    	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
    	                                              &opt, sizeof(opt))) 
    	{ 
    	    perror("setsockopt"); 
    	    exit(EXIT_FAILURE); 
    	} 
	//assine the family port and adress to bind to.
    	address.sin_family = AF_INET; 
    	address.sin_addr.s_addr = INADDR_ANY; 
    	address.sin_port = htons( PORT ); 
       
    	// Forcefully attaching socket to the port 8080 
    	if (bind(server_fd, (struct sockaddr *)&address,  
    	                             sizeof(address))<0) 
    	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
    	if (listen(server_fd, 20) < 0) 
    	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	int count=0;
	//starting to listen for clients 
	printf("waiting for clients...\n");
	while(new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen)){

		
		printf("connection accepted\n");		
 		//set the arguments for the thread to handle the client 
		struct argument_struct args={new_socket,row,column};
		
		//launch the thread and varify if it started or not. 
		if(pthread_create(&tid[count], NULL, preform, &args)!=0){
			printf("thread failed");
		} 
		printf("%s\n","thread started");
		if(count>=5){
			count=0;
        	}	
		
 	   	if(new_socket<0){	
		 perror("accept"); 
		exit(EXIT_FAILURE);
		} 	
	
	}
	//disconnect the threads that end.
	for(count;count>0;count--){
		pthread_join(tid[count],NULL);
		printf("%s\n","thread ended");
	}
	return 0;
} 
//thread function
void *PlaneManager(void *arg){
	//take the arguemtns-----
	struct argument_struct *args = arg;
	int col = args->column;
	int row = args->row;
	//------------------------
	while(1){
		if(IfPlaneIsFull(row,col)==0){
			printf("%s\n","the plane is full and is leaving now");
			for(int i=0;i<row;i++){
				for(int j=0;j<col;j++){
					plane[i][j]=0;
				}
			}
			printf("%s\n","there is a new plane waiting to be boarded");
		}
	
	}
		
}
void *preform(void *arg){ // preforms the actions for the client
	//take the arguemtns-----
	struct argument_struct *args = arg;
	int column = args->column;
	int row = args->row;
	//------------------------
	//send the client the dimentions of the plane
	send(args->socket,&args->row,sizeof(int),0); 
	send(args->socket,&args->column,sizeof(int),0); 
	//-------------------------
	// reads the number of tickets the client wants
 	int numberOfTickets;
	if(read( args->socket ,(void *)&numberOfTickets, sizeof(int))==-1){
		printf("error");
	}
	//------------------------
	//get the thread id to print it with the numbe rof tickets they ordered
	pthread_t   tid;
	tid = pthread_self();
	printf("%s %lu %s %d\n","number of tickets for order", tid ,"is: ",numberOfTickets);
	//------------------------
	int break_x1;
	int responceCode;
	int rowread=0,columnread=0;
	for(int i=0;i<numberOfTickets;i++){
		//read the user responce and act accordingly
		if(read( args->socket ,(void *)&rowread, sizeof(int))!=-1&&
		read( args->socket ,(void *)&columnread, sizeof(int))!=-1){
			//if automatic give the first avlible seat.
			if(rowread==-1&&columnread==-1){
				break_x1=0;
				for (int j = 0; j < row; j++) {
        				for (int k = 0; k < column; k++) {
         					if(plane[j][k]==0){
							break_x1=1;
							plane[j][k]=1;
							break;
   						}
					if (break_x1) break;        				
					}
				if (break_x1) break;
        			}
			//send the responce code to the client to tell if the ticket was sucksessful or not.
			responceCode =1;
			send(args->socket,&responceCode,sizeof(int),0);
			//if manual check if selected seat is avalible and assine if if possible.
			}else{
				//
				if(seatIsValid(rowread,columnread)==0){
					plane[rowread][columnread] =1;
					responceCode =1;
					send(args->socket,&responceCode,sizeof(int),0);			
				}else if(seatIsValid(rowread,columnread)==-1){
					responceCode=0;
					i--;
					send(args->socket,&responceCode,sizeof(int),0);				
				}				
			}
		
		}
	}

//print plane after order is complete 
	for (int i = 0; i < row; i++) {
	       	for (int j = 0; j < column; j++) {
         	 
			printf("%d ", plane[i][j]);
	       	}
        	printf("\n");
    	}
	pthread_exit((void*)NULL);
	return NULL;
}

