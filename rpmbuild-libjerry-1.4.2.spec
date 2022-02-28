##########################################
# Spec file for Jerry application server #
##########################################

Summary: Jerry library to beild embedded application server apps
Name: libjerry
Version: 1.4.2
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

mkdir -p $RPM_BUILD_ROOT/usr/lib64
cp -a ../../build/jerry/1.4.2/default/architecture/linux-gcc/link-dynamic/libjerry.so $RPM_BUILD_ROOT/usr/lib64/libjerry.so.1.4.2
ln -s libjerry.so.1.4.2 $RPM_BUILD_ROOT/usr/lib64/libjerry.so

exit

%files
%attr(0755, root, root) /usr/lib64/libjerry.so.1.4.2
%attr(0777, root, root) /usr/lib64/libjerry.so

%pre

%post

%postun

%clean

%changelog
* Sun Feb 27 2022 Sven Lukas <sven.lukas@gmail.com>
  - First prebuild RPM
