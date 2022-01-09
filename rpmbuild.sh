#!/bin/sh

rpmbuild --define "_topdir `pwd`/rpm" --target x86_64 -bb rpmbuild-jerry-0.3.0.spec
mv rpm/RPMS/x86_64/*.rpm .
rm -rf rpm
