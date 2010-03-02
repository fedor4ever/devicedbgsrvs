//
// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/*
 * Copyright (c) 1985, 1988, 1990, 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * FTP server.
 */


#include <netinet/net_types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>

#define	FTP_NAMES
#include <arpa/ftp.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <glob.h>
#include <limits.h> /* for CHAR_BIT */
#include <netdb.h>
#include <pwd.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <spawn.h>

#include "pathnames.h"
#include "extern.h"

// Symbian includes

#include <f32file.h>
#include <in_iface.h>

static char version[]="v1.0.1000";

extern	off_t restart_point;
extern	char cbuf[];

struct	sockaddr_in server_addr;
struct	sockaddr_in ctrl_addr;
struct	sockaddr_in data_source;
struct	sockaddr_in data_dest;
struct	sockaddr_in his_addr;
struct	sockaddr_in pasv_addr;

in_addr server_in_addr;

int	daemon_mode = 0;
int	data;
jmp_buf	errcatch, urgcatch;
int	logged_in;
struct	passwd *pw;
int	debug = 0;
int	timeout = 900;    /* timeout after 15 minutes of inactivity */
int	maxtimeout = 7200;/* don't allow idle time to be set beyond 2 hours */
int	logging = 0;
int	replicate = 0;
int	high_data_ports = 0;
int	anon_only = 0;
int	multihome = 0;
int guest;
int	stats;
int	statfd = -1;
int	portcheck = 1;
int	dochroot;
int	type;
int	form;
int	stru;			/* avoid C keyword */
int	mode;
int	doutmp = 0;		/* update utmp file */
int	usedefault = 1;		/* for data transfers */
int	pdata = -1;		/* for passive mode */
int port = 21;    /* default FTP port */
//sig_atomic_t transflag;
int transflag;
off_t	file_size;
off_t	byte_count;
#if !defined(CMASK) || CMASK == 0
#undef CMASK
#define CMASK 027
#endif
int	defumask = CMASK;		/* default umask value */
char	tmpline[7];
char	hostname[]="Symbian"; 
char*	remotehost=hostname; 
char*	dhostname=hostname;
char	*guestpw;
static char ttyline[20];
char	*tty = ttyline;		/* for klogin */
static struct utmp utmp;	/* for utmp */

TDriveNumber drive;

char	*ident = NULL;

int root = 1;
FILE* logFile;
/*
 * Timeout intervals for retrying connections
 * to hosts that don't accept PORT cmds.  This
 * is a kludge, but given the problems with TCP...
 */
#define	SWAITMAX	90	/* wait at most 90 seconds */
#define	SWAITINT	5	/* interval between retries */

int	swaitmax = SWAITMAX;
int	swaitint = SWAITINT;

#ifdef HASSETPROCTITLE
char	proctitle[BUFSIZ];	/* initial part of title */
#endif /* HASSETPROCTITLE */

#define LOG(message) \
	if (logging/* > 1*/) { \
	    fprintf(logFile, "LOG_INFO %s\r\n", message); \
		fflush(logFile); }

#define LOGCMD(cmd, file) \
	if (logging/* > 1*/) { \
	    fprintf(logFile, "LOG_INFO %s %s\r\n", cmd, file); \
		fflush(logFile); }
#define LOGCMD2(cmd, file1, file2) \
	 if (logging/* > 1*/) { \
	 	fprintf(logFile, "LOG_INFO %s %s %s\r\n", cmd, file1, file2); \
		fflush(logFile); }
#define LOGBYTES(cmd, file, cnt) \
	if (logging/* > 1*/) { \
		if (cnt == (off_t)-1) \
		    fprintf(logFile, "LOG_INFO %s %s\r\n", cmd, file); \
		else \
		    fprintf(logFile, "LOG_INFO %s %s = %qd bytes\r\n", cmd, file, (quad_t)(cnt)); \
		fflush(logFile) ; \
	}

static void	 ack __P((const char *));
static FILE	*dataconn __P((const char *, off_t, const char *));
static void	 dolog __P((struct sockaddr_in *));
static void	 end_login __P((void));
static FILE	*getdatasock __P((const char *));
static int	guniquefd __P((const char *, char **));
static int	 receive_data __P((FILE *, FILE *));
static void	 replydirname __P((const char *, const char *));
static void	 send_data __P((FILE *, FILE *, off_t, off_t, int));

#if defined(TCPWRAPPERS)
static int	 check_host __P((struct sockaddr_in *));
#endif

in_addr getServerAddress();
TDriveNumber getSystemDrive();

void logxfer __P((const char *, off_t, time_t));

#define MAXUSERNAME 64
char * username[MAXUSERNAME];
char * userhome[MAXPATHLEN];

#undef IP_PORTRANGE

int
main(int argc, char *argv[], char **envp)
{
	int ch, on = 1;
	socklen_t addrlen;
	char *cp, line[LINE_MAX];
	FILE *fd;
	const char *argstr = "RAdDhlMSt:T:u:UvP:";
	
	tzset();	/* in case no timezone database in ~ftp */

	/* set this here so klogin can use it... */
	(void)snprintf(ttyline, sizeof(ttyline), "ftp%d", getpid());

	while ((ch = getopt(argc, argv, argstr)) != -1) {
		switch (ch) {
		case 'A':
			anon_only = 1;
			break;

		case 'd':
			debug = 1;
			break;

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
			
		case 'h':
			high_data_ports = 1;
			break;

		case 'l':
			logging++;	/* > 1 == extra logging */
			break;

		case 'M':
			multihome = 1;
			break;

		case 'S':
			stats = 1;
			break;

		case 't':
			timeout = atoi(optarg);
			if (maxtimeout < timeout)
				maxtimeout = timeout;
			break;

		case 'T':
			maxtimeout = atoi(optarg);
			if (timeout > maxtimeout)
				timeout = maxtimeout;
			break;

		case 'u':
		    {
			long val = 0;

			val = strtol(optarg, &optarg, 8);
			if (*optarg != '\0' || val < 0 || (val & ~ACCESSPERMS))
				printf("bad value for -u");
			else
				defumask = val;
			break;
		    }

		case 'U':
			doutmp = 1;
			break;

		case 'v':
			debug = 1;
			break;

		default:
			printf("unknown flag -%c ignored", optopt);
			scanf("");
			break;
		}
	}

	(void) freopen(_PATH_DEVNULL, "w", stderr);

	/*
	 * LOG_NDELAY sets up the logging connection immediately,
	 * necessary for anonymous ftp's that chroot and can't do it later.
	 */
#ifndef LOG_FTP
#define LOG_FTP LOG_DAEMON
#endif
	
		int ctl_sock, fd2;
	
		server_in_addr = getServerAddress();
		
		/*
		 * Open a socket, bind it to the FTP port, and start
		 * listening.
		 */
		ctl_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (ctl_sock < 0) {
            User::Panic(_L("socket error"),0);
			LOG("socket error");
			exit(1);
		}
		
		setsockopt(ctl_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	
		server_addr.sin_family = 0;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = port; 
		if (bind(ctl_sock, (struct sockaddr *)&server_addr,
			 sizeof(server_addr))) {
			 User::Panic(_L("bind error"),0);
            LOG("bind error");
			exit(1);
		}
		if (listen(ctl_sock, 32) < 0) {
		    User::Panic(_L("listen error"),0);
			LOG("listen error");
			exit(1);
		}
			addrlen = sizeof(his_addr);
			fd2 = accept(ctl_sock, (struct sockaddr *)&his_addr,
				    &addrlen);
			
			(void) dup2(fd2, 0);
			(void) dup2(fd2, 1);
			close(ctl_sock);
			close(fd2);


	drive = getSystemDrive();
	
	if(logging)
	{
		char logFileName[MAXPATHLEN];
		
		char driveLetter = 'A'+drive;
		
		sprintf(logFileName,"%c:\\ftpd_%d.log",driveLetter,getpid());
		logFile = fopen(logFileName,"w");
		
		fprintf(logFile, "** Log file for the FTP instance with PID %d **\r\n",getpid());
		fflush(logFile);
	}
	
	
	addrlen = sizeof(ctrl_addr);
	if (getsockname(0, (struct sockaddr *)&ctrl_addr, &addrlen) < 0) {
		User::Panic(_L("getsockname error"),0);
		exit(1);
	}

	//clone to accept new connections
	if(replicate)
		posix_spawn(NULL,argv[0],NULL,NULL,argv,envp);
	
	data_source.sin_port = htons(ntohs(ctrl_addr.sin_port) - 1);

	dolog(&his_addr);
	/*
	 * Set up default state
	 */
	data = -1;
	type = TYPE_A;
	form = FORM_N;
	stru = STRU_F;
	mode = MODE_S;
	tmpline[0] = '\0';

	/* If logins are disabled, print out the message. */
	if ((fd = fopen(_PATH_NOLOGIN,"r")) != NULL) {
		while (fgets(line, sizeof(line), fd) != NULL) {
			if ((cp = strchr(line, '\n')) != NULL)
				*cp = '\0';
			lreply(530, "%s", line);
		}
		(void) fflush(stdout);
		(void) fclose(fd);
		reply(530, "System not available.");
		exit(0);
	}
	if ((fd = fopen(_PATH_FTPWELCOME, "r")) != NULL) {
		while (fgets(line, sizeof(line), fd) != NULL) {
			if ((cp = strchr(line, '\n')) != NULL)
				*cp = '\0';
			lreply(220, "%s", line);
		}
		(void) fflush(stdout);
		(void) fclose(fd);
		/* reply(220,) must follow */
	}
	
	reply(220, "%s FTP server (%s) ready.",
			hostname, version);
	(void) setjmp(errcatch);
	for (;;)
		(void) yyparse();
	/* NOTREACHED */
}

static int login_attempts;	/* number of failed login attempts */
static int askpasswd;		/* had user command, ask for passwd */
static char curname[16];	/* current USER name */

/*
 * USER command.
 * Sets global passwd pointer pw if named account exists and is acceptable;
 * sets askpasswd if a PASS command is expected.  If logged in previously,
 * need to reset state.  If name is "ftp" or "anonymous", the name is not in
 * _PATH_FTPUSERS, and ftp account exists, set guest and pw, then just return.
 * If account doesn't exist, ask for passwd anyway.  Otherwise, check user
 * requesting login privileges.  Disallow anyone who does not have a standard
 * shell as returned by getusershell().  Disallow anyone mentioned in the file
 * _PATH_FTPUSERS to allow people such as root and uucp to be avoided.
 */
void user(char *name)
{
	//const char *cp, *shell;

	if (logged_in) {
		if (guest) {
			reply(530, "Can't change user from guest login.");
			return;
		} else if (dochroot) {
			reply(530, "Can't change user from chroot user.");
			return;
		}
		end_login();
	}

	guest = 0;

	// check password
	
	pw = (passwd *)malloc(sizeof(pw));
	
	strcpy((char*)username,name);
	pw->pw_name = (char*)username;
	
	strcpy((char*)userhome,"/");
	pw->pw_dir = (char*)userhome;
	
	if (logging) {
		strncpy(curname, name, sizeof(curname)-1);
		curname[sizeof(curname)-1] = '\0';
	}
	
	if (strcmp(name, "ftp") == 0 || strcmp(name, "anonymous") == 0) 
		reply(331,"Guest login ok, type your name as password.");
	else
		reply(331, "Password required for %s.", name);

	askpasswd = 1;
	/*
	 * Delay before reading passwd after first failed
	 * attempt to slow down passwd-guessing programs.
	 */
	if (login_attempts)
		sleep((unsigned) login_attempts);
}


/*
 * Terminate login as previous user, if any, resetting state;
 * used when USER command is given or login fails.
 */
static void end_login(void)
{
	(void) seteuid((uid_t)0);
	if (logged_in) {
		if (doutmp)
			logout(utmp.ut_line);
	}
	pw = NULL;
	logged_in = 0;
	guest = 0;
	dochroot = 0;
}

void pass(char *passwd)
{
	int rval;
	FILE *fd;
	
	if (logged_in || askpasswd == 0) {
		reply(503, "Login with USER first.");
		return;
	}
	askpasswd = 0;
	
	if (!guest) {		/* "ftp" is only account allowed no password */
	
		/*
		 * Authenticate password...
		 */

		rval = 0;

		/*
		 * If rval == 1, the user failed the authentication check
		 * above.  If rval == 0, either Kerberos or local authentication
		 * succeeded.
		 */
		if (rval) {
			reply(530, "Login incorrect.");
			pw = NULL;
			return;
		}
	} else {
		/* Save anonymous' password. */
		guestpw = strdup(passwd);
		if (guestpw == (char *)NULL)
			fatal("Out of memory");
	}
	login_attempts = 0;		/* this time successful */
	
	if (setegid((gid_t)pw->pw_gid) < 0) {
		reply(550, "Can't set gid.");
		return;
	}
	(void) initgroups(pw->pw_name, pw->pw_gid);

	/* open utmp before chroot */
	if (doutmp) {
		memset((void *)&utmp, 0, sizeof(utmp));
		(void)time(&utmp.ut_time);
		(void)strncpy(utmp.ut_name, pw->pw_name, sizeof(utmp.ut_name));
		(void)strncpy(utmp.ut_host, remotehost, sizeof(utmp.ut_host));
		(void)strncpy(utmp.ut_line, ttyline, sizeof(utmp.ut_line));
		login(&utmp);
	}

	/* open stats file before chroot */
	if (guest && (stats == 1) && (statfd < 0))
		if ((statfd = open(_PATH_FTPDSTATFILE, O_WRONLY|O_APPEND)) < 0)
			stats = 0;

	logged_in = 1;
	dochroot = 0; 
	
	/*
	 * Set home directory to root
	 */
	chdir("/"); //root

	/*
	 * Display a login message, if it exists.
	 * N.B. reply(230,) must follow the message.
	 */
	if ((fd = fopen(_PATH_FTPLOGINMESG, "r")) != NULL) {
		char *cp, line[LINE_MAX];

		while (fgets(line, sizeof(line), fd) != NULL) {
			if ((cp = strchr(line, '\n')) != NULL)
				*cp = '\0';
			lreply(230, "%s", line);
		}
		(void) fflush(stdout);
		(void) fclose(fd);
	}
	if (guest) {
		if (ident != NULL)
			free(ident);
		ident = strdup(passwd);
		if (ident == (char *)NULL)
			fatal("Ran out of memory.");
		reply(230, "Guest login ok, access restrictions apply.");
	} else {
		reply(230, "User %s logged in.", pw->pw_name);
	}
	(void) umask(defumask);
	return;
}

void internalName(char* output, const char *input)
	{
		//from /c/aaa.txt to c:\aaa.txt
		
		strcpy(output,input);
		
		//root is a special case
		if(strcmp(output,"/")==0)
			{
			return;
			}
		
		for(unsigned int i=0 ; i<strlen(output); i++)
			{
			if(output[i]=='/')
				output[i]= '\\';
			}
		
		//relative path, do not modify
		if((output[0]!='\\' && strlen(output)>1) || strcmp(output,".")==0)
			{
			return;
			}
		
		if(strlen(output)==1 && root)
			{
			//it's a drive: C\0 -> C:\0
			output[2]=output[1];
			output[1]=':';
			
			}
		else
			{
			
			// /C/system\n -> C:/system\n
			output[0]=output[1];
			output[1]=':';
			}
	}

void externalName(char* output, const char *input)
	{
		//from c:/aaa.txt to /c/aaa.txt 

		strcpy(output,input);
		
		for(unsigned int i=0 ; i<strlen(output); i++)
			{
			if(output[i]=='\\')
				output[i]= '/';
			}
		
		output[1]=output[0];
		output[0]='/';
	}

void retrieve(const char *cmd, const char *name)
{
	FILE *fin, *dout;
	struct stat st;
	int (*closefunc) __P((FILE *));
	time_t start;

	char adaptedName[MAXPATHLEN];
	internalName(adaptedName,name);
	
	LOGCMD("RETR", adaptedName);
	
	if (cmd == 0) {
		fin = fopen(adaptedName, "r"); closefunc = fclose;
		st.st_size = 0;
	} else {
		char line[BUFSIZ];
		
		(void) snprintf(line, sizeof(line), cmd, adaptedName);
		name = line;
		fin = ftpd_popen(line, "r"), closefunc = ftpd_pclose;
		st.st_size = -1;
		st.st_blksize = BUFSIZ;
	}
	if (fin == NULL) {
		if (errno != 0) {
			perror_reply(550, name);
			if (cmd == 0) {
				LOGCMD("get", adaptedName);
			}
		}
		return;
	}
	byte_count = -1;
	if (cmd == 0 && (fstat(fileno(fin), &st) < 0 || !S_ISREG(st.st_mode))) {
		reply(550, "%s: not a plain file.", name);
		goto done;
	}
	
	
	if (restart_point) {
		if (type == TYPE_A) {
			off_t i, n;
			int c;

			n = restart_point;
			i = 0;
			while (i++ < n) {
				if ((c=getc(fin)) == EOF) {
					perror_reply(550, name);
					goto done;
				}
				if (c == '\n')
					i++;
			}
		} else if (lseek(fileno(fin), restart_point, SEEK_SET) < 0) {
			perror_reply(550, name);
			goto done;
		}
	}
	
	dout = dataconn(name, st.st_size, "w");
	if (dout == NULL)
		goto done;
	time(&start);
	send_data(fin, dout, st.st_blksize, st.st_size,
		  (restart_point == 0 && cmd == 0 && S_ISREG(st.st_mode)));
	if ((cmd == 0) && stats)
		logxfer(name, st.st_size, start);
	(void) fclose(dout);
	data = -1;
	pdata = -1;
done:
	if (cmd == 0)
		LOGBYTES("get", name, byte_count);
	(*closefunc)(fin);
}

void store(const char *name, const char *mode, int unique)
{

	FILE *fout, *din;
	int (*closefunc) __P((FILE *));
	struct stat st;
	int fd;

	char adaptedName[MAXPATHLEN];
	internalName(adaptedName,name);
	
	LOGCMD("STOR", adaptedName);
	
	if (unique && stat(adaptedName, &st) == 0) {
		char *nam;

		fd = guniquefd(adaptedName, &nam);
		if (fd == -1) {
			LOGCMD(*mode == 'w' ? "put" : "append", name);
			return;
		}
		name = nam;
		if (restart_point)
			mode = "r+";
		fout = fdopen(fd, mode);
	} else
		fout = fopen(adaptedName, mode);

	closefunc = fclose;
	if (fout == NULL) {
		perror_reply(553, name);
		LOGCMD(*mode == 'w' ? "put" : "append", name);
		return;
	}
	byte_count = -1;
	if (restart_point) {
		if (type == TYPE_A) {
			off_t i, n;
			int c;

			n = restart_point;
			i = 0;
			while (i++ < n) {
				if ((c=getc(fout)) == EOF) {
					perror_reply(550, name);
					goto done;
				}
				if (c == '\n')
					i++;
			}
			/*
			 * We must do this seek to "current" position
			 * because we are changing from reading to
			 * writing.
			 */
			if (fseek(fout, 0L, SEEK_CUR) < 0) {
				perror_reply(550, name);
				goto done;
			}
		} else if (lseek(fileno(fout), restart_point, SEEK_SET) < 0) {
			perror_reply(550, name);
			goto done;
		}
	}
	din = dataconn(adaptedName, (off_t)-1, "r");
	if (din == NULL)
		goto done;
	if (receive_data(din, fout) == 0) {
		if (unique)
			reply(226, "Transfer complete (unique file name:%s).",
			    name);
		else
			reply(226, "Transfer complete.");
	}
	(void) fclose(din);
	data = -1;
	pdata = -1;
done:
	LOGBYTES(*mode == 'w' ? "put" : "append", name, byte_count);
	(*closefunc)(fout);
}

static FILE * getdatasock(const char *mode)
{
	int on = 1, s, t, tries;
	
	if (data >= 0)
		return (fdopen(data, mode));
	(void) seteuid((uid_t)0);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		goto bad;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
	    (char *) &on, sizeof(on)) < 0)
		goto bad;
	/* anchor socket to avoid multi-homing problems */

	data_source.sin_family = AF_INET;
	data_source.sin_addr = ctrl_addr.sin_addr;
	for (tries = 1; ; tries++) {
		if (bind(s, (struct sockaddr *)&data_source,
		    sizeof(data_source)) >= 0)
			break;
		if (errno != EADDRINUSE || tries > 10)
			goto bad;
		sleep(tries);
	}
	(void) seteuid((uid_t)pw->pw_uid);

	return (fdopen(s, mode));
bad:
	/* Return the real value of errno (close may change it) */
	t = errno;
	(void) seteuid((uid_t)pw->pw_uid);
	(void) close(s);
	errno = t;
	return (NULL);
}

static FILE * dataconn(const char *name, off_t size, const char *mode)
{
	char sizebuf[32];
	FILE *file;
	int retry = 0;

	file_size = size;
	byte_count = 0;
	if (size != (off_t) -1) {
		(void) snprintf(sizebuf, sizeof(sizebuf), " (%lld bytes)", 
				(quad_t) size);
	} else
		sizebuf[0] = '\0';
	
	if (pdata >= 0) {
		struct sockaddr_in from;
		int s;
		socklen_t fromlen = sizeof(from);

		s = accept(pdata, (struct sockaddr *)&from, &fromlen);
		
		//TODO: OE bug ? sizebuf seems to be changed to 0x18 after accept
		//reassigning sizebuf
		if (size != (off_t) -1) {
		(void) snprintf(sizebuf, sizeof(sizebuf), " (%lld bytes)", 
				(quad_t) size);
		} else
			sizebuf[0] = '\0';
		
		
		if (s < 0) {
			reply(425, "Can't open data connection.");
			(void) close(pdata);
			pdata = -1;
			return (NULL);
		}
		if (ntohs(from.sin_port) < IPPORT_RESERVED) {
			perror_reply(425, "Can't build data connection");
			(void) close(pdata);
			(void) close(s);
			pdata = -1;
			return (NULL);
		}
		if (from.sin_addr.s_addr != his_addr.sin_addr.s_addr) {
			perror_reply(435, "Can't build data connection"); 
			(void) close(pdata);
			(void) close(s);
			pdata = -1;
			return (NULL);
		}
		(void) close(pdata);
		pdata = s;

		if (size != (off_t) -1) {
		(void) snprintf(sizebuf, sizeof(sizebuf), " (%lld bytes)", 
				(quad_t) size);
		} else
			sizebuf[0] = '\0';
		
		reply(150, "Opening %s mode data connection for '%s'%s.",
		     type == TYPE_A ? "ASCII" : "BINARY", name, sizebuf);
		return (fdopen(pdata, mode));
	}
	if (data >= 0) {
		reply(125, "Using existing data connection for '%s'%s.",
		    name, sizebuf);
		usedefault = 1;
		return (fdopen(data, mode));
	}
	if (usedefault)
		data_dest = his_addr;
	usedefault = 1;
	file = getdatasock(mode);
	if (file == NULL) {
		reply(425, "Can't create data socket (%s,%d): %s.",
		    inet_ntoa(data_source.sin_addr),
		    ntohs(data_source.sin_port), strerror(errno));
		return (NULL);
	}
	data = fileno(file);

	/*
	 * attempt to connect to reserved port on client machine;
	 * this looks like an attack
	 */
	if (ntohs(data_dest.sin_port) < IPPORT_RESERVED ||
	    ntohs(data_dest.sin_port) == 2049) {		/* XXX */
		perror_reply(425, "Can't build data connection");
		(void) fclose(file);
		data = -1;
		return NULL;
	}
	if (data_dest.sin_addr.s_addr != his_addr.sin_addr.s_addr) {
		perror_reply(435, "Can't build data connection");
		(void) fclose(file);
		data = -1;
		return NULL;
	}
	while (connect(data, (struct sockaddr *)&data_dest,
	    sizeof(data_dest)) < 0) {
		if (errno == EADDRINUSE && retry < swaitmax) {
			sleep((unsigned) swaitint);
			retry += swaitint;
			continue;
		}
		perror_reply(425, "Can't build data connection");
		(void) fclose(file);
		data = -1;
		return (NULL);
	}
	reply(150, "Opening %s mode data connection for '%s'%s.",
	     type == TYPE_A ? "ASCII" : "BINARY", name, sizebuf);
	return (file);
}

/*
 * Tranfer the contents of "instr" to "outstr" peer using the appropriate
 * encapsulation of the data subject to Mode, Structure, and Type.
 *
 * NB: Form isn't handled.
 */
static void send_data(FILE *instr, FILE *outstr, off_t blksize, off_t filesize, int isreg)
{
	int c, cnt, filefd, netfd;
	char *buf, *bp;
	size_t len,size;

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return;
	}
	switch (type) {

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			(void) putc(c, outstr);
		}
		fflush(outstr);
		transflag = 0;
		if (ferror(instr))
			goto file_err;
		if (ferror(outstr))
			goto data_err;
		reply(226, "Transfer complete.");
		return;

	case TYPE_I:
	case TYPE_L:
		/*
		 * isreg is only set if we are not doing restart and we
		 * are sending a regular file
		 */
		netfd = fileno(outstr);
		filefd = fileno(instr);

		if (isreg && filesize < (off_t)16 * 1024 * 1024) {
			buf = (char *)mmap(0, filesize, PROT_READ, MAP_SHARED, filefd, (off_t)0);
			if (buf==MAP_FAILED || buf==NULL) {
				goto oldway;
			}
			bp = buf;
			len = filesize;
			do {
				cnt = write(netfd, bp, len);
				len -= cnt;
				bp += cnt;
				if (cnt > 0) byte_count += cnt;
			} while(cnt > 0 && len > 0);

			transflag = 0;
			munmap(buf, (size_t)filesize);
			if (cnt < 0)
				goto data_err;
			reply(226, "Transfer complete.");
			return;
		}

oldway:
		size = blksize * 16; 
	
		if ((buf = (char *)malloc(size)) == NULL) {
			transflag = 0;
			perror_reply(451, "Local resource failure: malloc");
			return;
		}

		while ((cnt = read(filefd, buf, size)) > 0 &&
		    write(netfd, buf, cnt) == cnt)
			byte_count += cnt;

		transflag = 0;
		(void)free(buf);
		if (cnt != 0) {
			if (cnt < 0)
				goto file_err;
			goto data_err;
		}
		reply(226, "Transfer complete.");
		return;
	default:
		transflag = 0;
		reply(550, "Unimplemented TYPE %d in send_data", type);
		return;
	}

data_err:
	transflag = 0;
	perror_reply(426, "Data connection");
	return;

file_err:
	transflag = 0;
	perror_reply(551, "Error on input file");
}

/*
 * Transfer data from peer to "outstr" using the appropriate encapulation of
 * the data subject to Mode, Structure, and Type.
 *
 * N.B.: Form isn't handled.
 */
static int receive_data(FILE *instr, FILE *outstr)
{
	int c;
	int cnt;
	volatile int bare_lfs = 0;
	char buf[BUFSIZ];

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return (-1);
	}
	switch (type) {

	case TYPE_I:
	case TYPE_L:
		
		do {
			cnt = read(fileno(instr), buf, sizeof(buf));
		
			if (cnt > 0) {
				if (write(fileno(outstr), buf, cnt) != cnt)
					goto file_err;
				byte_count += cnt;
			}
		} while (cnt > 0);
		transflag = 0;
		return (0);

	case TYPE_E:
		reply(553, "TYPE E not implemented.");
		transflag = 0;
		return (-1);

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			(void) putc(c, outstr);
		}
		fflush(outstr);
		if (ferror(outstr))
			goto file_err;
		transflag = 0;
		if (bare_lfs) {
			lreply(226,
		"WARNING! %d bare linefeeds received in ASCII mode",
			    bare_lfs);
		(void)printf("   File may not have transferred correctly.\r\n");
		}
		return (0);
	default:
		reply(550, "Unimplemented TYPE %d in receive_data", type);
		transflag = 0;
		return (-1);
	}
	
file_err:
	transflag = 0;
	perror_reply(452, "Error writing file");
	return (-1);
}

void statfilecmd(char *filename)
{
	FILE *fin;
	int c;
	char line[LINE_MAX];

	char adaptedName[MAXPATHLEN];
	internalName(adaptedName,filename);
	
	(void)snprintf(line, sizeof(line), "/bin/ls -lgA %s", filename);
	fin = ftpd_popen(line, "r");
	
	lreply(211, "status of %s:", filename);
	while ((c = getc(fin)) != EOF) {
		if (c == '\n') {
			if (ferror(stdout)){
				perror_reply(421, "control connection");
				(void) ftpd_pclose(fin);
				dologout(1);
				/* NOTREACHED */
			}
			if (ferror(fin)) {
				perror_reply(551, filename);
				(void) ftpd_pclose(fin);
				return;
			}
			(void) putc('\r', stdout);
		}
		(void) putc(c, stdout);
	}
	(void) ftpd_pclose(fin);
	
	
	reply(211, "End of Status");
}

void statcmd(void)
{
	struct sockaddr_in *sn;
	u_char *a, *p;

	lreply(211, "%s FTP server status:", hostname, version);
	printf("     %s\r\n", version);
	printf("     Connected to %s", remotehost);
	if (!isdigit(remotehost[0]))
		printf(" (%s)", inet_ntoa(server_in_addr));
	printf("\r\n");
	if (logged_in) {
		if (guest)
			printf("     Logged in anonymously\r\n");
		else
			printf("     Logged in as %s\r\n", pw->pw_name);
	} else if (askpasswd)
		printf("     Waiting for password\r\n");
	else
		printf("     Waiting for user name\r\n");
	printf("     TYPE: %s", typenames[type]);
	if (type == TYPE_A || type == TYPE_E)
		printf(", FORM: %s", formnames[form]);
	if (type == TYPE_L)
#if CHAR_BIT == 8
		printf(" %d", CHAR_BIT);
#else
		printf(" %d", bytesize);	/* need definition! */
#endif
	printf("; STRUcture: %s; transfer MODE: %s\r\n",
	    strunames[stru], modenames[mode]);
	if (data != -1)
		printf("     Data connection open\r\n");
	else if (pdata != -1) {
		printf("     in Passive mode");
		sn = &pasv_addr;
		goto printaddr;
	} else if (usedefault == 0) {
		printf("     PORT");
		sn = &data_dest;
printaddr:
		a = (u_char *) &sn->sin_addr;
		p = (u_char *) &sn->sin_port;
#define UC(b) (((int) b) & 0xff)
		printf(" (%d,%d,%d,%d,%d,%d)\r\n", UC(a[0]),
			UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
#undef UC
	} else
		printf("     No data connection\r\n");
	
	
	printf("     System Drive: %c\r\n",'A'+drive);
	
	reply(211, "End of status");
}

void fatal(const char *s)
{

	reply(451, "Error in server: %s\n", s);
	reply(221, "Closing connection due to server error.");
	dologout(0);
	/* NOTREACHED */
}

void
#ifdef __STDC__
reply(int n, const char *fmt, ...)
#else
reply(int n, char *fmt, va_dcl va_alist)
#endif
{
	va_list ap;
#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)printf("%d ", n);
	(void)vprintf(fmt, ap);
	(void)printf("\r\n");
	(void)fflush(stdout);
}

void
#ifdef __STDC__
lreply(int n, const char *fmt, ...)
#else
lreply(n, fmt, va_alist)
	int n;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)printf("%d- ", n);
	(void)vprintf(fmt, ap);
	(void)printf("\r\n");
	(void)fflush(stdout);
}

static void ack(const char *s)
{

	reply(250, "%s command successful.", s);
}

void nack(const char *s)
{

	reply(502, "%s command not implemented.", s);
}

/* ARGSUSED */

void yyerror(char *s)
{
	char *cp;
        
	(void)s; /* ignore argument */

	if ((cp = strchr(cbuf,'\n'))!=NULL)
		*cp = '\0';
	reply(500, "'%s': command not understood.", cbuf);
}

void dele(char *name)
{
	struct stat st;
	
	char internalPath[MAXPATHLEN];
	internalName(internalPath,name);

	LOGCMD("delete", name);
	if (stat(internalPath, &st) < 0) {
		perror_reply(550, name);
		return;
	}
	if ((st.st_mode&S_IFMT) == S_IFDIR) {
		if (rmdir(internalPath) < 0) {
			perror_reply(550, name);
			return;
		}
		goto done;
	}
	if (unlink(internalPath) < 0) {
		perror_reply(550, name);
		return;
	}
done:
	ack("DELE");
}

void cwd(const char *path)
{
	FILE *message;
	
	LOGCMD("cwd", path);
	
	char internalPath[MAXPATHLEN];
	
	char cwd_path[MAXPATHLEN];
	getcwd(cwd_path, sizeof cwd_path);
	
	
	if( (strlen(path)==1 && path[0]=='/') || (strlen(cwd_path)==2 && strcmp(path,"..")==0))
	{
		root=1;
		ack("CWD");
	}
	else
	{
		
		internalName(internalPath, path);

		if (chdir(internalPath) < 0)
			perror_reply(550, path);
		else {
		
			if ((message = fopen(_PATH_CWDMESG, "r")) != NULL) {
				char *cp, line[LINE_MAX];
	
				while (fgets(line, sizeof(line), message) != NULL) {
					if ((cp = strchr(line, '\n')) != NULL)
						*cp = '\0';
					lreply(250, "%s", line);
				}
				(void) fflush(stdout);
				(void) fclose(message);
			}
			root = 0;
			ack("CWD");
		}
	}
}

void replydirname(const char *name, const char *message)
{
	char npath[MAXPATHLEN];
	int i;

	for (i = 0; *name != '\0' && i < (int)sizeof(npath) - 1; i++, name++) {
		npath[i] = *name;
		if (*name == '"')
			npath[++i] = '"';
	}
	npath[i] = '\0';
	reply(257, "\"%s\" %s", npath, message);
}

void makedir(char *name)
{

	LOGCMD("mkdir", name);

	char path[MAXPATHLEN];

	internalName(path, name);
	
	if (mkdir(path, 0777) < 0)
		perror_reply(550, name);
	else
		replydirname(name, "directory created.");
}

void removedir(char *name)
{

	char path[MAXPATHLEN];

	internalName(path, name);

	LOGCMD("rmdir", name);
	if (rmdir(path) < 0)
		perror_reply(550, name);
	else
		ack("RMD");
}
void pwd(void)
{
	char path[MAXPATHLEN];
	
	if(root)
		{
		replydirname("/", "is current directory.");
		}
	else
		if (getcwd(path, sizeof path) == (char *)NULL)
		{	
		    externalName(path, path);
			reply(550, "%s.", path);
		}
		else
		{
			externalName(path, path);
			replydirname(path, "is current directory.");
		}
}

char * renamefrom(char *name)
{
	struct stat st;
	
	char internal[MAXPATHLEN];
	
	internalName(internal,name);

	if (stat(internal, &st) < 0) {
		perror_reply(550, name);
		return ((char *)0);
	}
	reply(350, "File exists, ready for destination name");
	return (name);
}

void renamecmd(char *from, char *to)
{

	char internalFrom[MAXPATHLEN];
	char internalTo[MAXPATHLEN];

	internalName(internalFrom,from);
	internalName(internalTo,to);
	
	LOGCMD2("rename", from, to);
	if (rename(internalFrom, internalTo) < 0)
		perror_reply(550, "rename");
	else
		ack("RNTO");
}

static void dolog(struct sockaddr_in *sn)
{
	struct hostent *hp = gethostbyaddr((char *)&sn->sin_addr,
		sizeof(struct in_addr), AF_INET);

	if (hp)
		(void) strncpy(remotehost, hp->h_name, sizeof(remotehost)-1);
	else
		(void) strncpy(remotehost, inet_ntoa(sn->sin_addr),
		    sizeof(remotehost)-1);
	remotehost[sizeof(remotehost)-1] = '\0';
}

/*
 * Record logout in wtmp file
 * and exit with supplied status.
 */
void dologout(int status)
{
	transflag = 0;

	if(logging)
	{
		fclose(logFile);
	}
		
	if (logged_in) {
		(void) seteuid((uid_t)0);
		if (doutmp)
			logout(utmp.ut_line);
	}
	/* beware of flushing buffers after a SIGPIPE */
	_exit(status);
}

/*
 * Note: a response of 425 is not mentioned as a possible response to
 *	the PASV command in RFC959. However, it has been blessed as
 *	a legitimate response by Jon Postel in a telephone conversation
 *	with Rick Adams on 25 Jan 89.
 */
void passive(void)
{
	socklen_t len;
#ifdef IP_PORTRANGE
	int on;
#else
	u_short port;
#endif
	char *p, *a;

	if (pw == NULL) {
		reply(530, "Please login with USER and PASS");
		return;
	}
	if (pdata >= 0)
		close(pdata);
	pdata = socket(AF_INET, SOCK_STREAM, 0);
	if (pdata < 0) {
		perror_reply(425, "Can't open passive connection");
		return;
	}

#ifdef IP_PORTRANGE
	on = high_data_ports ? IP_PORTRANGE_HIGH : IP_PORTRANGE_DEFAULT;
	if (setsockopt(pdata, IPPROTO_IP, IP_PORTRANGE,
		       (char *)&on, sizeof(on)) < 0)
		goto pasv_error;
#else
#define FTP_DATA_BOTTOM 40000
#define FTP_DATA_TOP    44999
	if (high_data_ports) {
		for (port = FTP_DATA_BOTTOM; port <= FTP_DATA_TOP; port++) {
			pasv_addr = ctrl_addr;
			pasv_addr.sin_port = htons(port);
			if (bind(pdata, (struct sockaddr *) &pasv_addr,
				 sizeof(pasv_addr)) == 0)
				break;
			if (errno != EADDRINUSE)
				goto pasv_error;
		}
		if (port > FTP_DATA_TOP)
			goto pasv_error;
	}
	else
#endif
	{
		pasv_addr = ctrl_addr;
		pasv_addr.sin_port = 0;
		if (bind(pdata, (struct sockaddr *)&pasv_addr,
			 sizeof(pasv_addr)) < 0)
			goto pasv_error;
	}

	len = sizeof(pasv_addr);
	if (getsockname(pdata, (struct sockaddr *) &pasv_addr, &len) < 0)
		goto pasv_error;
	if (listen(pdata, 1) < 0)
		goto pasv_error;
	
	in_addr addr = getServerAddress();
	
	a = (char *) &addr;
	p = (char *) &pasv_addr.sin_port;

#define UC(b) (((int) b) & 0xff)

	reply(227, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)", UC(a[0]),
		UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
	return;

pasv_error:
	(void) close(pdata);
	pdata = -1;
	perror_reply(425, "Can't open passive connection");
	return;
}

/*
 * Generate unique name for file with basename "local".
 * The file named "local" is already known to exist.
 * Generates failure reply on error.
 */
static int guniquefd(const char *local, char **nam)
{
	static char newch[MAXPATHLEN];
	struct stat st;
	int count, len, fd;
	char *cp;

	cp = strrchr(local, '/');
	if (cp)
		*cp = '\0';
	if (stat(cp ? local : ".", &st) < 0) {
		perror_reply(553, cp ? local : ".");
		return (-1);
	}
	if (cp)
		*cp = '/';
	(void) strncpy(newch, local, sizeof(newch)-1);
	newch[sizeof(newch)-1] = '\0';
	len = strlen(newch);
	if (len+2+1 >= (int)sizeof(newch)-1)
		return (-1);
	cp = newch + len;
	*cp++ = '.';
	for (count = 1; count < 100; count++) {
		(void)snprintf(cp, sizeof(newch) - (cp - newch), "%d", count);
		fd = open(newch, O_RDWR|O_CREAT|O_EXCL, 0666);
		if (fd == -1)
			continue;
		if (nam)
			*nam = newch;
		return (fd);
	}
	reply(452, "Unique file name cannot be created.");
	return (-1);
}

/*
 * Format and send reply containing system error number.
 */
void perror_reply(int code, const char *string)
{

	reply(code, "%s: %s.", string, strerror(errno));
}

static const char *onefile[] = {
	"",
	0
};


void send_file_list(const char *whichf, int simple)
{
	struct stat st;
	DIR *dirp = NULL;
	struct dirent *dir;
	FILE *volatile dout = NULL;
	char const *const *volatile dirlist;
	const char *dirname;
	//volatile int simple = 0;
	volatile int freeglob = 0;
	glob_t gl;
	char time[26];

	char internal_whichf[MAXPATHLEN];
	
	//remove modifier for hidden files "-a" 
	if(whichf[0]=='-' && whichf[1]=='a')
		{
		strcpy(internal_whichf,whichf+2);
		
		//if target location is empty, set "."
		if(internal_whichf[0]==0)
			strcpy(internal_whichf,".");
		
		internalName(internal_whichf,internal_whichf);
		}
	else
		{
		internalName(internal_whichf,whichf);
		}

	
	LOGCMD("send_file_list",internal_whichf);
	
	/* XXX: should the { go away if __linux__? */
	
	if((root && strcmp(internal_whichf, ".")==0) || strcmp(internal_whichf, "/")==0  )
		{
		if (dout == NULL) {
		dout = dataconn("file list", (off_t)-1,
			"w");
		if (dout == NULL)
			goto out;
		transflag++;
		}
	
		//get the drives list
		RFs fileSystem;
		fileSystem.Connect();
		
		TDriveList	driveList;
		fileSystem.DriveList(driveList);
	
		
		if (simple)
			{
			for(int i=0; i<driveList.Length(); i++)
				{
				if(driveList[i]!=0)
					{
					fprintf(dout,"%c%s\n", 'A'+i ,type == TYPE_A ? "\r" : "");
					}
				}
			}
		else
			{
			
			for(int i=0; i<driveList.Length(); i++)
				{
				if(driveList[i]!=0)
					{
					fprintf(dout,"drw-------   1 ftp ftp         0 Jan 01  2008 %c%s\n", 'A'+i ,type == TYPE_A ? "\r" : "");
					}
				}
			}
		
		fileSystem.Close();
			
		}
	else
		{
		
		if (strpbrk(whichf, "~{[*?") != NULL) {	
	#ifdef __linux__
		        /* see popen.c */
			int flags = GLOB_NOCHECK;
	#else
			int flags = GLOB_BRACE|GLOB_NOCHECK|GLOB_QUOTE|GLOB_TILDE;
	#endif
	
			memset(&gl, 0, sizeof(gl));
			freeglob = 1;
			if (glob(internal_whichf, flags, 0, &gl)) {
				reply(550, "not found");
				goto out;
			} else if (gl.gl_pathc == 0) {
				errno = ENOENT;
				perror_reply(550, whichf);
				goto out;
			}
			/* The cast is necessary because of bugs in C's type system */
			dirlist = (char const *const *) gl.gl_pathv;
		} else {
			onefile[0] = internal_whichf;
			dirlist = onefile;
		}
	
		if (setjmp(urgcatch)) {
			transflag = 0;
			goto out;
		}
		
		 while ((dirname = *dirlist++)!=NULL) {
			if (stat(dirname, &st) < 0) {
				perror_reply(550, whichf);
				if (dout != NULL) {
					(void) fclose(dout);
					transflag = 0;
					data = -1;
					pdata = -1;
				}
				goto out;
			}
	
			if (S_ISREG(st.st_mode)) {
				if (dout == NULL) {
					dout = dataconn("file list", (off_t)-1, "w");
					if (dout == NULL)
						goto out;
					transflag++;
				}
			} else if (!S_ISDIR(st.st_mode))
				continue;
	
			if ((dirp = opendir(dirname)) == NULL)
				continue;
	
			while ((dir = readdir(dirp)) != NULL) {
				char nbuf[MAXPATHLEN];

				if (dir->d_name[0] == '.' && dir->d_namlen == 1)
					continue;
				if (dir->d_name[0] == '.' && dir->d_name[1] == '.' &&
				    dir->d_namlen == 2)
					continue;

				snprintf(nbuf, sizeof(nbuf), "%s/%s", dirname,
				dir->d_name);
	
				/*
				 * We have to do a stat to insure it's
				 * not a directory or special file.
				 */
				stat(nbuf, &st);
				
				if (simple){
					if (dout == NULL) {
						dout = dataconn("file list", (off_t)-1,
							"w");
						if (dout == NULL)
							goto out;
						transflag++;
					}
					
					fprintf(dout, "%s%s\n", dir->d_name, type == TYPE_A ? "\r" : "");
					
					byte_count += strlen(nbuf) + 1;
				}
				else //!simple
					{
					
					if (dout == NULL) {
					dout = dataconn("file list", (off_t)-1,
						"w");
					if (dout == NULL)
						goto out;
					transflag++;
					}
					
					ctime_r(&st.st_atime,time);
				    //"Mon Dec 10 17:35:44 2007"
				    
				    char month[4];
				    month[0] = time[4]; //'D'
				    month[1] = time[5]; //'e'
				    month[2] = time[6]; //'c'
				    month[3] = '\0';			    
				    
				    char day[3];
				    day[0] = time[8]; //'1'
				    day[1] = time[9]; //'0'
				    day[2] = '\0';
				    
				    char year[5];
				    year[0] = time[20]; //'2'
	  			    year[1] = time[21]; //'0'			    
	 			    year[2] = time[22]; //'0'
	                year[3] = time[23]; //'7'
	 			    year[4] = '\0';               
				    
				    
				    fprintf(dout, "%c%c%c%c%c%c%c%c%c%c   1 ftp ftp %9jd %s %s  %s %s%s\n",
				    	S_ISDIR(st.st_mode) ? 'd' : '-',
			            st.st_mode & S_IRUSR ? 'r' : '-',
			            st.st_mode & S_IWUSR ? 'w' : '-',
	            		st.st_mode & S_IXUSR ? 'x' : '-',
	            		st.st_mode & S_IRGRP ? 'r' : '-',
	            		st.st_mode & S_IWGRP ? 'w' : '-',
	            		st.st_mode & S_IXGRP ? 'x' : '-',
	            		st.st_mode & S_IROTH ? 'r' : '-',
	            		st.st_mode & S_IWOTH ? 'w' : '-',
	            		st.st_mode & S_IXOTH ? 'x' : '-',
			            st.st_size,
			            month,
			            day,
			            year,
			            dir->d_name,
			            type == TYPE_A ? "\r" : ""
			           );
						
					}
			}
			(void) closedir(dirp);
		}
	}
	if (dout == NULL)
		reply(550, "No files found.");
	else if (ferror(dout) != 0)
		perror_reply(550, "Data connection");
	else
		reply(226, "Transfer complete.");

	transflag = 0;
	if (dout != NULL)
		(void) fclose(dout);
	data = -1;
	pdata = -1;
out:
	if (freeglob) {
		freeglob = 0;
		globfree(&gl);
	}
}

void logxfer(const char *name, off_t size, time_t start)
{
	char buf[400 + MAXHOSTNAMELEN*4 + MAXPATHLEN*4];
	char dir[MAXPATHLEN], path[MAXPATHLEN], rpath[MAXPATHLEN];
	char vremotehost[MAXHOSTNAMELEN*4], vpath[MAXPATHLEN*4];
	char *vpw;
	time_t now;

	if ((statfd >= 0) && (getcwd(dir, sizeof(dir)) != NULL)) {
		time(&now);

		vpw = (char *)malloc(strlen((guest) ? guestpw : pw->pw_name)*4+1);
		if (vpw == NULL)
			return;

		snprintf(path, sizeof path, "%s/%s", dir, name);
		if (realpath(path, rpath) == NULL) {
			strncpy(rpath, path, sizeof rpath-1);
			rpath[sizeof rpath-1] = '\0';
		}

		snprintf(buf, sizeof(buf),
		    "%.24s %ld %s %qd %s %c %s %c %c %s ftp %d %s %s\n",
		    ctime(&now), (long)(now - start + (now == start)),
		    vremotehost, (long long) size, vpath,
		    ((type == TYPE_A) ? 'a' : 'b'), "*" /* none yet */,
		    'o', ((guest) ? 'a' : 'r'),
		    vpw, 0 /* none yet */,
		    ((guest) ? "*" : pw->pw_name),
		    dhostname);
		write(statfd, buf, strlen(buf));
		free(vpw);
	}
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

in_addr getServerAddress()
	{
	
	in_addr address;
	
	//default adddress to be returned if failure
	inet_aton("0.0.0.0", &address);
	
	RSocketServ rSockServer;
	if(KErrNone==rSockServer.Connect())
    {
    RConnection rConnect;
    if(KErrNone==rConnect.Open(rSockServer))
        {
        TRequestStatus status; 
        rConnect.Start(status);
            
        User::WaitForRequest(status); 
        }
    }
	
	TAutoClose<RSocketServ> ss;
    User::LeaveIfError(ss.iObj.Connect());
    ss.PushL();

    TAutoClose<RSocket> sock;
    User::LeaveIfError(sock.iObj.Open(ss.iObj, _L("udp")));
    sock.PushL();

    User::LeaveIfError(sock.iObj.SetOpt(KSoInetEnumInterfaces, KSolInetIfCtrl));

    TProtocolDesc in;
    User::LeaveIfError(sock.iObj.Info(in));
    TPckgBuf<TSoInetInterfaceInfo> info, next;

    TInt res=sock.iObj.GetOpt(KSoInetNextInterface, KSolInetIfCtrl, info);
	
    while(res==KErrNone)
        {
		
        res=sock.iObj.GetOpt(KSoInetNextInterface, KSolInetIfCtrl, next);
		if(info().iState != EIfUp || info().iFeatures&KIfIsLoopback || info().iName.Left(4) == _L("eth6")) 
		{
		info = next; continue;	
		}
		
		TName address_descriptor;
	    char address_string[16]; //xxx.xxx.xxx.xxx\0
	    
        info().iAddress.Output(address_descriptor);
        
        int i=0;
        
        for(i=0; i<address_descriptor.Length(); i++)
        	{
	        address_string[i]=address_descriptor[i];
        	}
        
        address_string[i]='\0';
        
        inet_aton(address_string, &address);
        
        if(res==KErrNone)
            {
            info = next;
         	}
        }
	
    sock.Pop();
    ss.Pop();
    
    rSockServer.Close();
    
    return address;
	}


void sizecmd(char *filename)
{

	char internalFilename[MAXPATHLEN];

	internalName(internalFilename,filename);

	switch (type) {
	case TYPE_L:
	case TYPE_I: 
	case TYPE_A: {
		struct stat stbuf;
		if (stat(internalFilename, &stbuf) < 0 || !S_ISREG(stbuf.st_mode))
			reply(550, "%s: not a plain file.", filename);
		else
			reply(213, "%qu", (quad_t) stbuf.st_size);
		break; }
	default:
		reply(504, "SIZE not implemented for Type %c.", "?AEIL"[type]);
	}
}

