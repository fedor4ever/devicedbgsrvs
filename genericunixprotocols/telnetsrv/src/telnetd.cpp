/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

//  Include Files

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/telnet.h>

//For connect
#include <sys/socket.h>
#include <netinet/in.h>

//For posix_spawn
#include <spawn.h>

//for pipe
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/param.h>

//Symbian headers
#include <e32std.h>
#include <es_sock.h>
#include <f32file.h>

void add_return(char* source, char* dest);
int startConnection();
TDriveNumber getSystemDrive();

int main(int argc, char *argv[], char **envp)
	{

	int net;
	int fds[3];
	int pid;
	int ctl_sock;
	int on = 1;
	int replicate = 0;
	int ch = 1;

	char ayt_response[]={IAC,NOP};

	socklen_t addrlen;

	int port = 23;

	sockaddr_in server_addr;
	sockaddr_in his_addr;

	const char *argstr = "RDP:";

	while ((ch = getopt(argc, argv, argstr)) != -1) {
		switch (ch) {

		case 'D':

			for(int i=1 /*no name*/; i<argc; i++)
				{
				for(unsigned int j=0;j<strlen(argv[i]); j++)
					if(argv[i][j]=='D')
						{
						//Daemon token found, change it to allow replication to new background processes
						argv[i][j]='R';
						}
				}
			posix_spawn(NULL,argv[0],NULL,NULL,argv,envp);
			exit(0);
			break;

		case 'R':
			replicate = 1;
			break;

		case 'P':
			port = atoi(optarg);
			break;

		default:
			printf("unknown flag -%c ignored", optopt);
			scanf("");
			break;
		}
	}

	if(startConnection() != KErrNone)
		{
		exit(1);
		}

	ctl_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (ctl_sock < 0)
		{
		exit(1);
		}

	if (setsockopt(ctl_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
		{
		exit(1);
		}


	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(ctl_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		{
		exit(1);
		}

	if (listen(ctl_sock, 32) < 0)
		{
		exit(1);
		}

	addrlen = sizeof(his_addr);

	net = accept(ctl_sock, (struct sockaddr *)&his_addr, &addrlen);

	if(net<0)
		{
		exit(1);
		}

	close(ctl_sock);

	//replicate server
	if(replicate)
		posix_spawn(NULL,argv[0],NULL,NULL,argv,envp);

	//launch shell and redirect the commands...
	pid=popen3("zsh.exe", NULL, NULL, fds);


	if(pid>0)
		{

		char read_zsh[256];
		char read_zsh_crlf[512];
		int sock_cnt=0, zsh_cnt=0, ret;
		int max=0;
	    fd_set read_fds;

	    //create fifo pipe
		char pipeName[MAXPATHLEN];
		sprintf(pipeName,"%c:\\telnetd_%d_pipe",getSystemDrive()+'A',getpid());

		if (mkfifo(pipeName, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH) < 0)
			{
			exit(1);
			}

		int pipe = open(pipeName, O_RDONLY | O_NONBLOCK);


	    max =  fds[1] > fds[2] ? fds[1] : fds[2];
		max =  max > net ? max : net;
		max =  max > pipe ? max : pipe;

		char exportVariable[MAXPATHLEN];
		sprintf(exportVariable,"PIPE='%s'\n",pipeName);

		//read first prompt and write PIPE variable
		read(fds[1], read_zsh, 11); //"localhost# "
		write(fds[0], exportVariable, strlen(exportVariable));

		for(;;)
			{
			int status=-1;

			FD_ZERO(&read_fds);
			FD_SET(net, &read_fds);
			FD_SET(fds[1], &read_fds);
			FD_SET(fds[2], &read_fds);
			FD_SET(pipe, &read_fds);

			//is child terminated...

			//Fix for defect to prevent the process from taking too much of the processor
			sleep(1);

			int wait_pid=waitpid(pid, &status, WNOHANG);
			if(pid == wait_pid || wait_pid == -1) //dont wait for the child to terminate
				{
				break;
				}

			//is any of the fds ready for read...
			ret=select(max+1, &read_fds, NULL, NULL, NULL);

			if(ret==-1)
				{
				break;
				}

   		    //is there any data to be written onto socket..
			if(FD_ISSET(fds[1], &read_fds))
				{
				memset(&read_zsh[0], 0, 256);

				zsh_cnt=read(fds[1], read_zsh, 255);
				if(zsh_cnt>0)
					{
					//reading from the socket can have more than one \n
					//so replace all \n with \r\n.
					add_return(read_zsh,read_zsh_crlf);
					write(net, read_zsh_crlf, strlen(read_zsh_crlf));
					}
				}

   		    //is there any data to be written onto socket..
			if (FD_ISSET(fds[2], &read_fds))
				{
				memset(&read_zsh[0], 0, 256);

				zsh_cnt=read(fds[2], read_zsh, 255);
				if(zsh_cnt>0)
					{
					//reading from the socket can have more than one \n
					//so replace all \n with \r\n.
					add_return(read_zsh,read_zsh_crlf);
					write(net, read_zsh_crlf, strlen(read_zsh_crlf));
					}
				}


			//is there any data to be written on to pipe
			if(FD_ISSET(net, &read_fds))
				{
				//char* ptr=0;
				char temp,temp2;

				sock_cnt=0;

				sock_cnt=read(net, &temp, 1);
				if(sock_cnt > 0)
					{
					//echo the command..
					//write(net, &temp, 1);

					switch(temp & 0xFF)
						{
						case '\r':
								sock_cnt=read(net, &temp, 1); //expect \n here...
								//write(net, &temp, 1);
								if(sock_cnt && !(write(fds[0], &temp, 1) > 0))
									printf("1 telnetd: zsh is not waiting for data.\n");
								break;

						case IAC: //telnet command
							{
								read(net, &temp2, 1);
								switch(temp2 & 0xFF)
									{
									case AYT:
										write(net,&ayt_response,2);
										break;
									}
								break;
							}

						default:
							if(!(write(fds[0], &temp, 1) > 0))
								printf("2 telnetd: zsh is not waiting for data.\n");
						}

					}
   		    	}


			if(FD_ISSET(pipe, &read_fds))
				{
				memset(&read_zsh[0], 0, 256);
				zsh_cnt=read(pipe, read_zsh, 255);

				if(zsh_cnt>0)
					{
					write(net, read_zsh, strlen(read_zsh));
					}
				}


			}

		close(net);
		close(fds[0]);
		close(fds[1]);
		close(fds[2]);
		close(pipe);

		//delete pipe file
		unlink(pipeName);

		exit(0);//exit server here...



		}
	else
		{

		char errorMessage[]="\n\rShell (zsh.exe) not launched\n\r";

		write(net, errorMessage, strlen(errorMessage));
		sleep(5);
		exit(-1); //exit here
		}

	}


void add_return(char* source, char* dest)
	{

	int sourceLen = strlen(source);
	int destIndex = 0;
	int sourceIndex = 0;

	for(; sourceIndex<sourceLen; sourceIndex++, destIndex++)
		{
		if(source[sourceIndex]=='\n')
			{
			dest[destIndex]='\r';
			destIndex++;
			}
		dest[destIndex] = source[sourceIndex];
		}

	dest[destIndex] = '\0';

	}


int startConnection()
	{

	int error;

	RSocketServ rSockServer;

	error = rSockServer.Connect();

	if(error == KErrNone)
    {
    RConnection rConnect;
    error = rConnect.Open(rSockServer);
    if(error == KErrNone)
        {
        TRequestStatus status;
        rConnect.Start(status);
        User::WaitForRequest(status);
        }
    rConnect.Close();
    }

	rSockServer.Close();

	return error;

	}


TDriveNumber getSystemDrive()
	{
	_LIT(KFileSrvDll, "efsrv.dll");

	TDriveNumber defaultSysDrive(EDriveC);
	RLibrary pluginLibrary;
	TInt pluginErr = pluginLibrary.Load(KFileSrvDll);

	if (pluginErr == KErrNone)
		{
		typedef TDriveNumber(*fun1)();
		fun1 sysdrive;

	#ifdef __EABI__
		sysdrive = (fun1)pluginLibrary.Lookup(336);
	#else
		sysdrive = (fun1)pluginLibrary.Lookup(304);
	#endif

		if(sysdrive!=NULL)
			{
			defaultSysDrive = sysdrive();
			}
		}
	pluginLibrary.Close();
	return defaultSysDrive;
	}



