#!/bin/sh

for f in `grep -lR "int main(void)" ../test/` ; do  export TESTMAINNAME=`echo $f | sed -e "s/\.\.\/test\/test_\(.*\).c$/\1/"` && sed -e "s/int main(void)/int "$TESTMAINNAME"_main(void)/" $f > app/src/main/cpp/tests/test_$TESTMAINNAME.c ; done

