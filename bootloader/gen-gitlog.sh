#!/bin/sh

TARGET=$1
PREFIX=$2

exec >&2

SHA=$(git show --pretty=format:'%H' HEAD | head -1)
TEMP=$(mktemp -t $TARGET.XXXXXXXXXX)

(echo "#include \"auto-dnc-gitlog.h\"";
echo "char *${PREFIX}_sha = \"$SHA\";";
echo -n "char *${PREFIX}_diff = \"";
git diff $SHA | perl -npe 's/([\\\"])/\\\1/g; s/\n/\\n/g; s/\t/\\t/g;';
echo "\";") > $TEMP

diff -Nq $TEMP $TARGET > /dev/null || mv $TEMP $TARGET
rm -f $TEMP

