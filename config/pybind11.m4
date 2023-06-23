# X_LIB_PYBIND11([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------------
AC_DEFUN([X_LIB_PYBIND11],
[
  AC_PROVIDE([X_LIB_PYBIND11])
  AC_MSG_CHECKING([pybind11 installation])
  if test x$PYTHON_BIN != x; then
    pybind11_include=`$PYTHON_BIN -c "import pybind11; print(pybind11.get_include())"`
    AC_MSG_RESULT([$pybind11_include])
    if test -z "$pybind11_include" ; then
      AC_MSG_ERROR([cannot find pybind11 include path])
    else
      CPPFLAGS="-I$pybind11_include $CPPFLAGS"
      have_pybind11=true
      AC_DEFINE([HAVE_PYBIND11],[1],[Define if the pybind11 is present $PYBIND11_INCLUDE])
    fi
  fi
  AM_CONDITIONAL([HAVE_PYBIND11], [test x$have_pybind11 = xtrue])
])
