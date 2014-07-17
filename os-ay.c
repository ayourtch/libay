
/**************************************************************************
*
*  Copyright © 2008-2009 Andrew Yourtchenko, ayourtch@gmail.com.
*
*  Permission is hereby granted, free of charge, to any person obtaining 
* a copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation 
* the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom 
* the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included 
* in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
* OR OTHER DEALINGS IN THE SOFTWARE. 
*
*****************************************************************************/

/**
 * @defgroup osfunc OS-interaction functions (signals and execution)
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#ifdef WITH_TUNTAP
#ifdef __linux__
#include <linux/if.h>
#include <linux/if_tun.h>
#endif
#ifdef __APPLE__
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include <net/if_utun.h>
#endif
#endif


#include "os-ay.h"
#include "debug-ay.h"

/*@{*/


signal_func *
set_signal_handler(int signo, signal_func * func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if(sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}


void
makedaemon(char *logname)
{
  int uid = 32767;
  int gid = 32767;
  char *user = "nobody";
  struct passwd *pwd;
  int logfd;

  logfd = open(logname, O_WRONLY | O_CREAT | O_APPEND);
  if(logfd < 0) {
    debug(DBG_GLOBAL, 0, "Could not open logfile '%s', exiting", logname);
    exit(255);
  }

  if(getuid() == 0) {
    pwd = getpwnam(user);
    if(pwd) {
      uid = pwd->pw_uid;
      gid = pwd->pw_gid;
    }
    debug(DBG_GLOBAL, 0,
          "Launched as root, trying to become %s (uid %d, gid %d)..",
          user, uid, gid);
    notminus(setgroups(0, (const gid_t *) 0), "setgroups");
    initgroups(user, gid);      // not critical if fails
    notminus(setgid(gid), "setgid");
    notminus(setegid(gid), "setegid");
    notminus(setuid(uid), "setuid");
    notminus(seteuid(gid), "seteuid");
    debug(DBG_GLOBAL, 0, "now forking");
  }


  if(fork() != 0)
    exit(0);

  setsid();
  set_signal_handler(SIGHUP, SIG_IGN);
  set_signal_handler(SIGPIPE, SIG_IGN);

  if(fork() != 0)
    exit(0);

  //chdir("/tmp");
  //chroot("/tmp");
  //umask(077);


  close(0);
  close(1);
  close(2);
  dup2(logfd, 1);
  dup2(logfd, 2);
  debug(DBG_GLOBAL, 0, "Started as a daemon");
}

time_t get_file_mtime(char *fname)
{
  struct stat st;
  if (0 == stat(fname, &st)) {
    return st.st_mtime;
  } else {
    return -1;
  }

}

#ifdef WITH_TUNTAP

#ifdef __linux__

int tuntap_alloc(char *dev, int kind) 
{
  struct ifreq ifr;
  int fd, err;

  if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
    return -1;

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = kind;

  if(dev && *dev )
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
    close(fd);
    return err;
  }
  return fd;
}


int tap_alloc(char *dev) 
{
  return tuntap_alloc(dev, IFF_TAP | IFF_NO_PI);
}

int tun_alloc(char *dev) 
{
  return tuntap_alloc(dev, IFF_TUN);
}

#endif /* linux */

#ifdef __APPLE__

int tap_alloc(char *dev) 
{
  fprintf(stderr, "No tap interfaces on MacOS X, sorry !\n");
  return -1;
}

/* 
 * The initial code is from 
 * http://www.newosxbook.com/src.jl?tree=listings&file=17-15-utun.c
 * by Jonathan Levin.
 *
 * Thank you Jonathan!
 */


int tun_alloc(char *dev) 
{
  struct sockaddr_ctl sc;
  struct ctl_info ctlInfo;
  int fd;
  int utun_unit;

  memset(&ctlInfo, 0, sizeof(ctlInfo));
  if (strlcpy(ctlInfo.ctl_name, UTUN_CONTROL_NAME, sizeof(ctlInfo.ctl_name)) >=
      sizeof(ctlInfo.ctl_name)) {
          fprintf(stderr,"UTUN_CONTROL_NAME too long");
          return -1;
  }
  fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);

  if (fd == -1) {
          perror ("socket(SYSPROTO_CONTROL)");
          return -1;
  }
  if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
          perror ("ioctl(CTLIOCGINFO)");
          close(fd);
          return -1;
  }

  sc.sc_id = ctlInfo.ctl_id;
  sc.sc_len = sizeof(sc);
  sc.sc_family = AF_SYSTEM;
  sc.ss_sysaddr = AF_SYS_CONTROL;
  for(sc.sc_unit = 1; ; sc.sc_unit++) {
    /*
    * If the connect is successful,
    * a tun%d device will be created, where "%d"
    * is our unit number -1
    */
    if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) == 0) {
      if(dev) {
        sprintf(dev, "utun%d", sc.sc_unit - 1);
      }
      return fd;
    } else if (EBUSY == errno) {
	/* fall through to retry the next unit */
    } else {
      perror ("connect(AF_SYS_CONTROL)");
      close(fd);
      return -1;
    }
  }
  return fd;
}

#endif /* APPLE */

#endif

/*@}*/


