###############################################################################
# Spec file for Jerry application server
################################################################################

Summary: Jerry application server
Name: jerry
Version: 1.4.1
Release: 1
License: Freeware
URL: http://www.sven-lukas.de
Group: System
Packager: Sven Lukas
BuildRoot: ./rpmbuild/

%description
Jerry application server is a reference application server for C++ apps using ESL.

%prep
#echo "BUILDROOT = $RPM_BUILD_ROOT"

mkdir -p $RPM_BUILD_ROOT/usr/bin
cp -a ../../build/jerry/1.4.1/default/architecture/linux-gcc/link-executable/jerry $RPM_BUILD_ROOT/usr/bin

exit

%files
%attr(0755, root, root) /usr/bin/jerry

%pre

%post

%postun

%clean

%changelog
* Sun Feb 6 2022 Sven Lukas <sven.lukas@gmail.com>
  - Added private context usage for apps
  - Allow apps to specify their libraries to load
  - Added eslx, so there is no external esl-library needed anymore
* Sun Jan 16 2022 Sven Lukas <sven.lukas@gmail.com>
  - First prebuild RPM
