dnl> Do not add anything above
AC_INIT([Makefile.in], 1.2.0)
dnl> Do not add anything above

AC_PROG_CPP

dnl> Add /usr/local/ /opt/local
CFLAGS=""
NTOPNG_VERSION=`cat ./configure.ac | head -2 | grep 'C_INIT' | cut -d " " -f 2|cut -d ')' -f 1`
CFLAGS="${CFLAGS} -I/usr/local/include -I/opt/local/include"
CPPFLAGS="${CPPFLAGS} -I/usr/local/include -I/opt/local/include"
LDFLAGS="${LDFLAGS} -L/usr/local/lib -L/opt/local/lib"

SVN_RELEASE=`svn info . | grep "^Revision"|cut -d " " -f 2`
AC_DEFINE_UNQUOTED(NTOPNG_SVN_RELEASE, "r${SVN_RELEASE}", [SVN Release])
SVN_DATE=`svn info . | grep "^Last Changed Date"|cut -d " " -f 4-`
AC_DEFINE_UNQUOTED(NTOPNG_SVN_DATE, "${SVN_DATE}", [Last SVN change])
PACKAGE_VERSION="${NTOPNG_VERSION}_${SVN_RELEASE}"

if test -z `which pkg-config`; then
   echo "#!/bin/sh\n" > pkg-config
fi

if test -f ".svn/wc.db"; then
   SVN_WC_DB=".svn/wc.db"
fi

MACHINE=`uname -m`
SYSTEM=`uname -s`

if test $SYSTEM = "Linux"; then
   OSNAME=`./linux.guess`
else
   OSNAME=`./config.guess`
fi
AC_DEFINE_UNQUOTED(PACKAGE_OSNAME, "${OSNAME}", [OS name])

if test $MACHINE = "x86_64"; then
   EXTN="amd64"
else
   EXTN="i386"
fi

DATE=`date +"%Y-%m-%d"`

if test -d "/usr/include/hiredis"; then
 AC_DEFINE_UNQUOTED(HAVE_HIREDIS, 1, [Local hiredis package present])
 HIREDIS_INC="`pkg-config --cflags hiredis` -I/usr/include/hiredis"
 HIREDIS_LIB="`pkg-config --libs hiredis` -lhiredis"
else
 HIREDIS_INC="-I ./third-party/hiredis"
fi

PF_RING_HOME=${HOME}/PF_RING
if test -d ${PF_RING_HOME}; then
  AC_MSG_CHECKING(for PF_RING source installed in $PF_RING_HOME)
fi

if test -f "/usr/local/lib/libpfring.a" || test -f "/opt/pfring/lib/libpfring.a" || test -f "${PF_RING_HOME}/userland/lib/libpfring.a"; then
  AC_DEFINE_UNQUOTED(HAVE_PF_RING, 1, [Native PF_RING support])
  if test -f "${PF_RING_HOME}/userland/lib/libpfring.a"; then
  echo "${PF_RING_HOME}/userland/libpcap/libpcap.a"
     if test -f "${PF_RING_HOME}/userland/libpcap/libpcap.a"; then
       LDFLAGS="${LDFLAGS} ${PF_RING_HOME}/userland/lib/libpfring.a -L${PF_RING_HOME}/userland/libpcap/ -lnuma"
       CPPFLAGS="${CPPFLAGS} -I${PF_RING_HOME}/kernel -I${PF_RING_HOME}/userland/lib -I${PF_RING_HOME}/userland/libpcap"
       echo "Using PF_RING installed in ${PF_RING_HOME}"
     else
       echo "Found PF_RING but the userland libs have not been compiled"
       exit
     fi
  else
    if test -d "/opt/pfring"; then
      if test -f "/opt/pfring/lib/libpfring.a"; then
         echo "/opt/pfring/lib/libpfring.a"
         LDFLAGS="${LDFLAGS} /opt/pfring/lib/libpfring.a -L/opt/pfring/lib"
         CPPFLAGS="${CPPFLAGS} -I/opt/pfring/include"
         echo "Using PF_RING installed in /opt/pfring"
      else
       if test -f "/usr/local/lib/libpfring.a"; then
        if test -f "/usr/local/lib/libpcap.a"; then
    	  LDFLAGS="${LDFLAGS} /usr/local/lib/libpfring.a"
          echo "Using PF_RING installed in /usr/local/lib"
	  AC_CHECK_LIB([pcap], [pcap_open_live], pcap=true)
	  if test x$pcap = x
	  then
	    echo "Found /usr/local/lib/libpfring.a but missing header files."
	    exit
	  fi
	else
	  echo "Found /usr/local/lib/libpfring.a  but missing libpcap.a in the same directory"
	  echo "Please make sure you have installed your PF_RING libraries and try again"
	  exit
        fi
      fi
    fi
  fi
 fi
else
  AC_CHECK_LIB([pcap], [pcap_open_live], pcap=true)
  if test x$pcap = x
  then
    echo "Please install libpcap(-dev) (http://tcpdump.org)"
    exit
  fi
fi

CFLAGS="-Wall $CFLAGS"

dnl> http://luajit.org/install.html
if test $SYSTEM = "Darwin"; then
   if test $MACHINE = "x86_64"; then
      LINK_OPTS="-pagezero_size 10000 -image_base 100000000"
      RES=`g++ -Wall -fno-color-diagnostics Prefs.cpp 2>&1 | grep "unrecognized command line option "|wc -l`
      if test $RES -eq 0; then
            CFLAGS="-fno-color-diagnostics $CFLAGS"
      fi
   fi
fi

AC_MSG_CHECKING(for gnutls)
AC_CHECK_LIB([gnutls], [gnutls_priority_set], gnutls=true)

AC_DEFINE_UNQUOTED([HAVE_GNUTLS], $gnutls, [We have gnutls])
if test x$gnutls = xtrue
 then
  LIBGNUTLS="-lgnutls"

  AC_MSG_CHECKING(for gcrypt)
  AC_CHECK_LIB([gcrypt], [gcry_check_version], gcrypt=true)
  if test x$gcrypt = xtrue
  then
    LIBGNUTLS="${LIBGNUTLS} -lgcrypt"
  fi
fi

echo "Checking RRD prerequisites..."
AC_CHECK_LIB([xml2], [xmlParseFile], xml2=true)
if test x$xml2 = x
then
echo "Please install libxml2(-devel) package (RRD prerequisite)"
exit
fi

AC_CHECK_LIB([glib-2.0], [glib_check_version], glib2=true)
if test x$glib2 = x
then
echo "Please install libglib-2.0 (glib2-devel/libglib2.0-dev) package (RRD prerequisite)"
exit
fi

dnl> zlib (used by edjdb)
Z_LIB=""
AC_CHECK_LIB([z], [zlibVersion], zlib=true)
if test x$zlib = xtrue
then
  Z_LIB="-lz"
fi

dnl> ldl (used by edjdb)
DL_LIB=""
AC_CHECK_LIB([dl], [dlopen], ldl=true)
if test x$ldl = xtrue
then
  DL_LIB="-ldl"
fi

AC_CHECK_LIB([numa], [numa_available], AC_DEFINE_UNQUOTED(HAVE_LIBNUMA, 1, [libnuma is installed]))
AC_MSG_CHECKING([if libnuma is present])
if test "x$ac_cv_lib_numa_numa_available" = xyes; then :
   NUMA_LIB="-lnuma"
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT(no)
fi

dnl> Check for package install
DOWNLOAD_GEOIP=0
AC_CHECK_LIB([GeoIP], [GeoIP_record_by_ipnum])
AC_HAVE_HEADERS(GeoIP.h)
if test ".${ac_cv_lib_GeoIP_GeoIP_record_by_ipnum}" = ".yes" &&
   test ".${ac_cv_header_GeoIP_h}" = ".yes"; then

   AC_CHECK_LIB([GeoIP], [GeoIP_lib_version])
   if test ".${ac_cv_lib_GeoIP_GeoIP_lib_version}" = ".yes"; then
     AC_DEFINE_UNQUOTED(HAVE_GEOIP_IPv6, 1, [GeoIP IPv6 support is present])
     dnl> LIBS="$LIBS -lGeoIP"
     AC_DEFINE_UNQUOTED(HAVE_GEOIP, 1, [GeoIP support is present])
     GEOIP_LIB="-lGeoIP"
     dnl> Download databases
     DOWNLOAD_GEOIP=1
   else
     AC_DEFINE_UNQUOTED(HAVE_GEOIP_IPv6, 1, [GeoIP not supported: library too old, plase upgrade first])
   fi
else
AC_MSG_RESULT(Build ntop without geolocation - GeoIP (http://www.maxmind.com/))
GEOIP_DEP=""
fi

AC_CHECK_LIB([nl], [nl_handle_alloc], [NL_LIB="-lnl"])

AC_CHECK_LIB([sqlite3], [sqlite3_open], sqlite=true)
if test x$sqlite = xtrue
then
  AC_DEFINE_UNQUOTED([HAVE_SQLITE], 1, [We have sqlite])
  SQLITE_LIB="-lsqlite3"
else
  echo "SQLite 3.x missing (libsqlite3-dev): please install it and try again"
  exit
fi

AC_DEFINE_UNQUOTED(_CRT_SECURE_NO_WARNINGS, 1, [Disable warning on windows])

GMAKE=`which gmake`
if test x$GMAKE = x
then
	GMAKE="make"
fi

GPP=`which clang++`
if test x$GPP = x
then
	GPP="g++"
fi

SVN=`which svn`
if test x$SVN = x
then
	AC_MSG_RESULT(FATAL ERROR: svn is not installed on your host)
	exit
fi

WGET=`which wget`
if test x$WGET = x
then
	AC_MSG_RESULT(ERROR. wget is not installed on your host: you cannot download GeoIP data)
fi

umask 002

INSTALL_DIR=${prefix}
if test $INSTALL_DIR = "NONE"
then
 INSTALL_DIR=${ac_default_prefix}
fi

if test $SYSTEM = "Darwin"; then
  MAN_DIR=$INSTALL_DIR/share
else
  MAN_DIR=$INSTALL_DIR
fi

AC_DEFINE_UNQUOTED(PACKAGE_RELEASE, "r$SVN_RELEASE", [SVN release of this package])
AC_DEFINE_UNQUOTED(PACKAGE_MACHINE, "$MACHINE", [Architecture of this host])
AC_SUBST(MACHINE)
AC_SUBST(SYSTEM)
AC_SUBST(EXTN)
AC_SUBST(DATE)
AC_SUBST(KERNEL)
AC_SUBST(SVN_RELEASE)
AC_SUBST(LINK_OPTS)
AC_SUBST(LIBGNUTLS)
AC_SUBST(Z_LIB)
AC_SUBST(GEOIP_LIB)
AC_SUBST(SQLITE_LIB)
AC_SUBST(GMAKE)
AC_SUBST(GPP)
AC_SUBST(SVN)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(DL_LIB)
AC_SUBST(NUMA_LIB)
AC_SUBST(NL_LIB)
AC_SUBST(LDFLAGS)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(NTOPNG_VERSION)
AC_SUBST(SVN_RELEASE)
AC_SUBST(SVN_DATE)
AC_SUBST(SVN_WC_DB)
AC_SUBST(INSTALL_DIR)
AC_SUBST(MAN_DIR)
AC_SUBST(HIREDIS_INC)
AC_SUBST(HIREDIS_LIB)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES(packages/ntopng.spec)
AC_CONFIG_FILES(packages/ntopng-data.spec)
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(doc/doxygen.conf)

AC_OUTPUT

echo ""
echo "You are now ready to compile typing $GMAKE"

if test $DOWNLOAD_GEOIP = "1"
then
  echo "Please do not forget to download GeoIP databases doing: $GMAKE geoip"
fi

dnl>
dnl> ntopng release history
dnl>
dnl> 0.1    	r6089	2013-03-25
dnl> 1.0	r6533	2013-06-30
dnl> 1.1	r6929	2013-11-03
dnl> 1.2	r8110	2014-08-13
