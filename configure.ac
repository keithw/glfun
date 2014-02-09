#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([glfun], [0.1], [keithw@mit.edu])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Add picky CXXFLAGS
CPPFLAGS="-std=c++11"
PICKY_CXXFLAGS="-pedantic -Wall -Wextra -Weffc++ -Werror"
AC_SUBST([PICKY_CXXFLAGS])
AC_LANG_PUSH(C++)

# Checks for libraries.
PKG_CHECK_MODULES([GL], [gl])
PKG_CHECK_MODULES([GLFW], [glfw3])
PKG_CHECK_MODULES([GLEW], [glew])
PKG_CHECK_MODULES([GLU], [glu])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.

AC_LANG_POP(C++)

AC_CONFIG_FILES([Makefile
		 src/Makefile])

AC_OUTPUT
