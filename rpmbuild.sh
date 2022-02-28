#!/bin/sh

rpmbuild --define "_topdir `pwd`/rpm" --target x86_64 -bb rpmbuild-jerry-1.4.2.spec
mv rpm/RPMS/x86_64/*.rpm .
rm -rf rpm

rpmbuild --define "_topdir `pwd`/rpm" --target x86_64 -bb rpmbuild-libjerry-1.4.2.spec
mv rpm/RPMS/x86_64/*.rpm .
rm -rf rpm
