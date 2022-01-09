###############################################################################
# Spec file for Jerry application server
################################################################################

Summary: Jerry application server
Name: jerry
Version: 0.3.0
Release: 1
License: Freeware
URL: http://www.sven-lukas.de
Group: System
Packager: Sven Lukas
Requires: libeslx = 0.3.0
BuildRoot: ./rpmbuild/

%description
Jerry application server is a reference application server for C++ apps using ESL.

%prep
#echo "BUILDROOT = $RPM_BUILD_ROOT"

mkdir -p $RPM_BUILD_ROOT/usr/bin
cp -a ../../build/jerry/0.3.0/default/architecture/linux-gcc/link-executable/jerry $RPM_BUILD_ROOT/usr/bin

exit

%files
%attr(0755, root, root) /usr/bin/jerry

%pre

%post

%postun

%clean

%changelog
* Sun Jan 9 2022 Sven Lukas <sven.lukas@gmail.com>
  - First prebuild RPM
