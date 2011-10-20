#!/bin/sh

TARGET=$1
PREFIX=$2

exec >&2

SHA=$(git show --pretty=format:'%H' HEAD)
TEMP=$(mktemp -t $TARGET.XXXXXXXXXX)

echo "char *${PREFIX}_sha = \"$SHA\";" > $TEMP
echo -n "char *${PREFIX}_diff = \"" >> $TEMP
git diff $SHA | perl -npe 's/([\\\"])/\\\1/g; s/\n/\\n/g; s/\t/\\t/g;' >> $TEMP
echo "\";" >> $TEMP

diff -Nq $TEMP $TARGET > /dev/null || mv $TEMP $TARGET
rm -f $TEMP

