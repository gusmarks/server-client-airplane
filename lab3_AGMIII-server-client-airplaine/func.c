void *listen(void *arg){
	
	while(1){	
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  	                       (socklen_t*)&addrlen))<0) { 
			perror("accept"); 
			exit(EXIT_FAILURE); 
    		} 	
			valread = read( new_socket , buffer, 1024); 
    		printf("%s\n",buffer ); 
    		send(new_socket , hello , strlen(hello) , 0 ); 
    		printf("Hello message sent\n");
	} 

}
void perform(){
	printf("%s\n","hello");
}
