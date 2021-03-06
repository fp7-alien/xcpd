# Set application version based on the git version

#Default
XDPD_VERSION="Unknown (no GIT repository detected)"

AC_CHECK_PROG(ff_git,git,yes,no)

if test $ff_git = no
then
	AC_MSG_RESULT([git not found!])
else

	AC_MSG_CHECKING([the build number(version)])
	
	if test -d $srcdir/.git ; then
		#Try to retrieve the build number
		_XDPD_GIT_BUILD=`git log -1 --pretty=%H`
		_XDPD_GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`
		XDPD_VERSION=`git describe --abbrev=0`
		_XDPD_GIT_DESCRIBE=`git describe --abbrev=40`

		AC_DEFINE_UNQUOTED([XCPD_BUILD],["$_XCPD_GIT_BUILD"])
		AC_DEFINE_UNQUOTED([XCPD_BRANCH],["$_XCPD_GIT_BRANCH"])
		AC_DEFINE_UNQUOTED([XCPD_DESCRIBE],["$_XCPD_GIT_DESCRIBE"])

	fi

	AC_MSG_RESULT($XCPD_VERSION)
fi

AC_DEFINE_UNQUOTED([XCPD_VERSION],["$XCPD_VERSION"])
