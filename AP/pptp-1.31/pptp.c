/* pptp.c ... client shell to launch call managers, data handlers, and
 *            the pppd from the command line.
 *            C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp.c,v 1.1 2005/03/20 05:06:37 rex Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#if defined(__FreeBSD__)
#include <libutil.h>
#elif defined(__NetBSD__)
#include <util.h>
#elif defined(__APPLE__)
#include <util.h>
#else
#include <pty.h>
#endif
#ifdef USER_PPP
#include <fcntl.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/wait.h>
#if defined(__APPLE__)
#include "getopt.h"
#else
#include <getopt.h>
#endif
#include <limits.h>
#include "config.h"
#include "pptp_callmgr.h"
#include "pptp_gre.h"
#include "version.h"
#include "inststr.h"
#include "util.h"
#include "pptp_quirks.h"
#include "pqueue.h"
#ifdef NO_LD_SHARELIB
extern int getpt (void);
extern int grantpt (int fd);
extern int ptsname_r (int fd, char *buf, size_t buflen);
extern int unlockpt (int fd);
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pty.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#endif

#ifndef PPPD_BINARY
#define PPPD_BINARY "pppd"
#endif

int syncppp = 0;

struct in_addr get_ip_address(char *name);
int open_callmgr(struct in_addr inetaddr, char *phonenr, int argc,char **argv,char **envp, int pty_fd);
void launch_callmgr(struct in_addr inetaddr, char *phonenr, int argc,char **argv,char **envp);
int get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id);
void launch_pppd(char *ttydev, int argc, char **argv);

void usage(char *progname) {
  fprintf(stderr,
	  "%s\n"
	  "Usage:\n"
	  "  %s <hostname> [<pptp options> -- ][pppd <pppd options>]\n"
	  "\n"
	  "Or using pppd option pty: \n"
	  "  pppd pty \"%s <hostname> --nolaunchpppd <pptp options>\"\n"
	  "\n"
	  "Available pptp options:\n"
          "  --phone <number>	Pass <number> to remote host as phone number\n"
	  "  --nolaunchpppd	Do not launch pppd, for use as a pppd pty\n"
          "  --quirks <quirk>	Work around a buggy PPTP implementation\n"
	  "			Currently recognised values are BEZEQ_ISRAEL only\n"
	  "  --debug		Run in foreground (for debugging with gdb)\n"
	  "  --sync		Enable Synchronous HDLC (pppd must use it too)\n"
	  "  --timeout <secs>	Time to wait for reordered packets (0.01 to 10 secs)\n"
	  "  --logstring <name>	Use <name> instead of 'anon' in syslog messages\n"
          "  --localbind <addr>	Bind to specified IP address instead of wildcard\n",
	  version, progname, progname);
  log("%s called with wrong arguments, program not started.", progname);
  
  exit(1);
}

struct in_addr localbind = { INADDR_NONE };
static int signaled = 0;

void do_nothing(int sig) { 
    /* do nothing signal handler. Better than SIG_IGN. */
    signaled = 1;
}

sigjmp_buf env;
void sighandler(int sig) {
  siglongjmp(env, 1);
}

void sigstats(int sig) {
  syslog(LOG_NOTICE, "GRE statistics:\n");
  #define LOG(name,value) syslog(LOG_NOTICE, name "\n", stats .value)
  LOG("rx accepted  = %d", rx_accepted);
  LOG("rx lost      = %d", rx_lost);
  LOG("rx under win = %d", rx_underwin);
  LOG("rx over  win = %d", rx_overwin);
  LOG("rx buffered  = %d", rx_buffered);
  LOG("rx OS errors = %d", rx_errors);
  LOG("rx truncated = %d", rx_truncated);
  LOG("rx invalid   = %d", rx_invalid);
  LOG("rx acks      = %d", rx_acks);
  LOG("tx sent      = %d", tx_sent);
  LOG("tx failed    = %d", tx_failed);
  LOG("tx short     = %d", tx_short);
  LOG("tx acks      = %d", tx_acks);
  LOG("tx oversize  = %d", tx_oversize);
  LOG("round trip   = %d usecs", rtt);
  #undef LOG
}

/* TODO: redesign to avoid longjmp/setjmp.  Several variables here
   have a volatile qualifier to silence warnings from gcc < 3.0.
   Remove the volatile qualifiers if longjmp/setjmp are removed. */

int main(int argc, char **argv, char **envp) {
  struct in_addr inetaddr;
  volatile int callmgr_sock = -1;
  char ttydev[255];
  int pty_fd, tty_fd, gre_fd, rc;
  volatile pid_t parent_pid, child_pid;
  u_int16_t call_id, peer_call_id;
  int pppdargc;
  char **pppdargv;
  char phonenrbuf[65]; /* maximum length of field plus one for the trailing
                        * '\0' */
  char * volatile phonenr = NULL;
  volatile int launchpppd = 1, debug = 0;
  
  if (argc < 2)
    usage(argv[0]);

  /* Step 1a: Get IP address for the hostname in argv[1] */
  inetaddr = get_ip_address(argv[1]);

  /* step 1b: Find the ppp options, extract phone number */
  argc--;
  argv++;
  while(1){ 
      /* structure with all recognised options for pptp */
      static struct option long_options[] = {
          {"phone", 1, 0, 0},  
          {"nolaunchpppd", 0, 0, 0},  
	  {"quirks", 1, 0, 0},
	  {"debug", 0, 0, 0},
	  {"sync", 0, 0, 0},
	  {"timeout", 1, 0, 0},
	  {"logstring", 1, 0, 0},
          {"localbind", 1, 0, 0},
          {0, 0, 0, 0}
      };
      int option_index = 0;
      int c;
      opterr=0; /* suppress "unrecognised option" message, here
                 * we assume that it is a pppd option */
      c = getopt_long (argc, argv, "", long_options, &option_index);
      if( c==-1) break;  /* no more options */
      switch (c) {
        case 0: 
	  if (option_index == 0) { /* --phone specified */
	    /* copy it to a buffer, as the argv's will be overwritten by 
	     * inststr() */
	    strncpy(phonenrbuf,optarg,sizeof(phonenrbuf));
	    phonenrbuf[sizeof(phonenrbuf)-1]='\0';
	    phonenr=phonenrbuf;
	  } else if (option_index == 1) {/* --nolaunchpppd specified */
	    launchpppd=0;
	  } else if (option_index == 2) {/* --quirks specified */
	    if (set_quirk_index(find_quirk(optarg)))
	      usage(argv[0]);
	  } else if (option_index == 3) {/* --debug */
	    debug = 1;
          } else if (option_index == 4) {/* --sync specified */
            syncppp=1;
	  } else if (option_index == 5) {/* --timeout */
		  
	    /*
	    float new_packet_timeout = atof(optarg);
	    if (new_packet_timeout < 0.0099 ||
		new_packet_timeout > 10) {
	      fprintf(stderr, "Packet timeout %s (%f) out of range: "
		      "should be between 0.01 and 10 seconds\n", optarg,
		      new_packet_timeout);
	      log("Packet timeout %s (%f) out of range: should be between "
		  "0.01 and 10 seconds", optarg, new_packet_timeout);
	      exit(2);
	    } else {
	    
	      packet_timeout_usecs = new_packet_timeout * 1000000;
	    }
	    */
	    int  new_packet_timeout = atoi(optarg);
	    packet_timeout_usecs = new_packet_timeout * 1000000;
	   
	   
	  } else if (option_index == 6) {/* --logstring */
	    log_string = strdup(optarg);
	  } else if (option_index == 7) {/* --localbind */ 
	    if (inet_pton(AF_INET, optarg, (void *) &localbind) < 1) {
	      fprintf(stderr, "Local bind address %s invalid\n", optarg);
	      log("Local bind address %s invalid\n", optarg);
	      exit(2);
	    }
          }
	  break;
        case '?': /* unrecognised option, treat it as the first pppd option */
            /* fall through */
        default:
            c = -1;
            break;
      }
      if (c == -1) break;  /* no more options for pptp */
    }
  pppdargc = argc - optind;
  pppdargv = argv + optind;

  log("The synchronous pptp option is %sactivated\n", syncppp ? "" : "NOT ");

  /* Step 2a: Now we have the peer address, bind the GRE socket early,
     before starting pppd. This prevents the ICMP Unreachable bug
     documented in <1026868263.2855.67.camel@jander> */

  gre_fd = pptp_gre_bind(inetaddr);
  if (gre_fd < 0) {
      close(callmgr_sock);
      fatal("Cannot bind GRE socket, aborting.");
  }

  /* Step 3: Find an open pty/tty pair. */
  if(launchpppd){
      rc = openpty (&pty_fd, &tty_fd, ttydev, NULL, NULL);
      if (rc < 0) { 
          close(callmgr_sock); 
          fatal("Could not find free pty.");
      }
  
      /* Step 4: fork and wait. */
      signal(SIGUSR1, do_nothing); /* don't die */
      parent_pid = getpid();
      switch (child_pid = fork()) {
      case -1:
        fatal("Could not fork pppd process");

      case 0: /* I'm the child! */
        close (tty_fd);
        signal(SIGUSR1, SIG_DFL);
        child_pid = getpid();
        break;
      default: /* parent */
        close (pty_fd);
        /*
         * There is still a very small race condition here.  If a signal
         * occurs after signaled is checked but before pause is called,
         * things will hang.
         */
        if (!signaled) {
            pause(); /* wait for the signal */
        }
        launch_pppd(ttydev, pppdargc, pppdargv); /* launch pppd */
        perror("Error");
        fatal("Could not launch pppd");
      }
  } else { /* ! launchpppd */
      pty_fd = tty_fd = STDIN_FILENO;
      close(STDOUT_FILENO); /* close unused file descriptor, that is redirected to the pty */
      child_pid=getpid();
      parent_pid=0; /* don't kill pppd */
  }

  do {
    /*
     * Step 2: Open connection to call manager
     *         (Launch call manager if necessary.)
     */
    callmgr_sock = open_callmgr(inetaddr, phonenr, argc, argv, envp, pty_fd);

  /* Step 5: Exchange PIDs, get call ID */
  } while (get_call_id(callmgr_sock, parent_pid, child_pid, 
	               &call_id, &peer_call_id) < 0);

  /* Step 5b: Send signal to wake up pppd task */
  if (launchpppd){
    kill(parent_pid, SIGUSR1);
    sleep(2);
    /* become a daemon */
    if (!debug && daemon(0, 0) != 0) {
      perror("daemon");
    }
  } else {
    /* re-open stderr as /dev/null to release it */
    file2fd("/dev/null", "wb", STDERR_FILENO);
  }
  
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "pptp: GRE-to-PPP gateway on %s", 
	     ttyname(tty_fd));
    inststr(argc,argv,envp, buf);
  }

  if (sigsetjmp(env, 1)!=0) goto shutdown;
  signal(SIGINT,  sighandler);
  signal(SIGTERM, sighandler);
  signal(SIGKILL, sighandler);
  signal(SIGCHLD, sighandler);
  signal(SIGUSR1, sigstats);
 
  /* Step 6: Do GRE copy until close. */
  pptp_gre_copy(call_id, peer_call_id, pty_fd, gre_fd);

shutdown:
  /* on close, kill all. */
  if(launchpppd)
      kill(parent_pid, SIGTERM);
  close(pty_fd);
  close(callmgr_sock);
  sleep(3);     /* give ctrl manager a chance to exit */
  exit(0);
}

struct in_addr get_ip_address(char *name) {
  struct in_addr retval;
  struct hostent *host = gethostbyname(name);
  if (host==NULL) {
    if (h_errno == HOST_NOT_FOUND)
      fatal("gethostbyname '%s': HOST NOT FOUND", name);
    else if (h_errno == NO_ADDRESS)
      fatal("gethostbyname '%s': NO IP ADDRESS", name);
    else
      fatal("gethostbyname '%s': name server error", name);
  }
  
  if (host->h_addrtype != AF_INET)
    fatal("Host '%s' has non-internet address", name);
  
  memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
  return retval;
}

int open_callmgr(struct in_addr inetaddr, char *phonenr, int argc, char **argv, char **envp, int pty_fd)
{
  /* Try to open unix domain socket to call manager. */
  struct sockaddr_un where;
  const int NUM_TRIES = 3;
  int i, fd;
  pid_t pid;
  int status;

  /* Open socket */
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    fatal("Could not create unix domain socket: %s", strerror(errno));
  }

  /* Make address */
  callmgr_name_unixsock(&where, inetaddr, localbind);

  for (i=0; i<NUM_TRIES; i++) {
    if (connect(fd, (struct sockaddr *) &where, sizeof(where)) < 0) {
      /* couldn't connect.  We'll have to launch this guy. */

      unlink (where.sun_path);	

      /* fork and launch call manager process */
      switch (pid=fork()) {
      case -1: /* failure */
	  fatal("fork() to launch call manager failed.");
      case 0: /* child */
	  {
	      close (fd);
	      /* close the pty in the call manager */
	      close(pty_fd);
	      launch_callmgr(inetaddr, phonenr, argc, argv, envp);
	  }
      default: /* parent */
	  waitpid(pid, &status, 0);
	  if (status!=0)
	      fatal("Call manager exited with error %d", status);
	  break;
      }
      sleep(1);
    }
    else return fd;
  }
  close(fd);
  fatal("Could not launch call manager after %d tries.", i);
  return -1;   /* make gcc happy */
}

void launch_callmgr(struct in_addr inetaddr, char *phonenr, int argc,
        char**argv,char**envp) 
{
      char *my_argv[3] = { argv[0], inet_ntoa(inetaddr), phonenr };
      char buf[128];
      snprintf(buf, sizeof(buf), "pptp: call manager for %s", my_argv[1]);
      inststr(argc,argv,envp,buf);
      exit(callmgr_main(3, my_argv, envp));
}

/* XXX need better error checking XXX */
int get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id)
{
  u_int16_t m_call_id, m_peer_call_id;
  /* write pid's to socket */
  /* don't bother with network byte order, because pid's are meaningless
   * outside the local host.
   */
  int rc;
  rc = write(sock, &gre, sizeof(gre));
  if (rc != sizeof(gre))
      return -1;
  rc = write(sock, &pppd, sizeof(pppd));
  if (rc != sizeof(pppd))
      return -1;
  rc = read(sock,  &m_call_id, sizeof(m_call_id));
  if (rc != sizeof(m_call_id))
      return -1;
  rc = read(sock,  &m_peer_call_id, sizeof(m_peer_call_id));
  if (rc != sizeof(m_peer_call_id))
      return -1;
  /*
   * XXX FIX ME ... DO ERROR CHECKING & TIME-OUTS XXX
   * (Rhialto: I am assuming for now that timeouts are not relevant
   * here, because the read and write calls would return -1 (fail) when
   * the peer goes away during the process. We know it is (or was)
   * running because the connect() call succeeded.)
   * (James: on the other hand, if the route to the peer goes away, we
   * wouldn't get told by read() or write() for quite some time.)
   */
  *call_id = m_call_id;
  *peer_call_id = m_peer_call_id;

  return 0;
}

void launch_pppd(char *ttydev, int argc, char **argv) {
  char *new_argv[argc+4]; /* XXX if not using GCC, hard code a limit here. */
  int i = 0, j;

  new_argv[i++] = PPPD_BINARY;
#ifdef USER_PPP
  new_argv[i++] = "-direct";
  /* ppp expects to have stdin connected to ttydev */
  if ((j = open(ttydev, O_RDWR)) == -1)
    fatal("Cannot open %s: %s", ttydev, strerror(errno));
  if (dup2(j, 0) == -1)
    fatal("dup2 failed: %s", strerror(errno));
  close(j);
#else
  new_argv[i++] = ttydev;
  new_argv[i++] = "38400";
#endif
  for (j=0; j<argc; j++)
    new_argv[i++] = argv[j];
  new_argv[i] = NULL;
  execvp(new_argv[0], new_argv);
}

#ifdef NO_LD_SHARELIB
#include <sys/stat.h>
#define ENABLE_FEATURE_DEVPTS 0
int xgetpty(char *line)
{
        int p;
#if ENABLE_FEATURE_DEVPTS
        p = open("/dev/ptmx", O_RDWR);
        if (p > 0) {
                const char *name;
                grantpt(p);
                unlockpt(p);
                name = ptsname(p);
                if (!name) {
                        printf("ptsname error (is /dev/pts mounted?)");
                        goto fail;
                }
                strncpy(line, name, 20);
                return p;
        }
#else
        struct stat stb;
        int i;
        int j;

        strcpy(line, "/dev/ptyXX");

        for (i = 0; i < 16; i++) {
                line[8] = "pqrstuvwxyzabcde"[i];
                line[9] = '0';
                if (stat(line, &stb) < 0) {
                        continue;
                }
                for (j = 0; j < 16; j++) {
                        line[9] = j < 10 ? j + '0' : j - 10 + 'a';
        //                        fprintf(stderr, "Trying to open device: %s\n", line);
                        p = open(line, O_RDWR | O_NOCTTY);
                        if (p >= 0) {
                                line[5] = 't';
                                return p;
                        }
                }
        }
#endif /* FEATURE_DEVPTS */
fail:
        printf("open pty fail!");
        return -1; /* never get here */
}


/* Create pseudo tty master slave pair and set terminal attributes
   according to TERMP and WINP.  Return handles for both ends in
   AMASTER and ASLAVE, and return the name of the slave end in NAME.  */
int
openpty (int *amaster, int *aslave, char *name, struct termios *termp,
	 struct winsize *winp)
{
#ifdef PATH_MAX
  char buf[PATH_MAX];
#else
  char buf[512];
#endif
  int master, slave;

//  master = getpt ();
  master = xgetpty (buf);
  if (master == -1)
    return -1;

  if (grantpt (master))
    goto fail;

  if (unlockpt (master))
    goto fail;
#if 0
  if (pts_name (master, &buf, sizeof (_buf)))
#else
//  if (ptsname_r (master, buf, sizeof buf))
//
//	printf("1444\n");
//#endif
//    goto fail;
//}
  slave = open (buf, O_RDWR | O_NOCTTY);
  if (slave == -1)
    {
#if 0
      if (buf != _buf)
	free (buf);
#endif
      goto fail;
    }

  /* XXX Should we ignore errors here?  */
  if(termp)
    tcsetattr (slave, TCSAFLUSH, termp);
  if (winp)
    ioctl (slave, TIOCSWINSZ, winp);

  *amaster = master;
  *aslave = slave;
  if (name != NULL)
    strcpy (name, buf);

#if 0
  if (buf != _buf)
    free (buf);
#endif
  return 0;

 fail:
  close (master);
  return -1;
#endif
}
#endif
