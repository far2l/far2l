#!/bin/bash
pkgname="$1"
git tag -l v_* | sort -V | tail -5 | while read tag
do
	tagx="v${tag:2}"
    (echo "$pkgname (${tagx#v}-1) unstable; urgency=low\n"; \
    	git log --pretty=format:'  * %s' $prevtag..$tag; \
    	git log --pretty='format:%n%n -- %aN <%aE>  %aD%n%n' $tag^..$tag) \
    	| cat - debian-changelog | sponge debian-changelog
        prevtag=$tag
done

# VK: remove it et all
#tag=`git tag -l v* | sort -V | tail -1`
#[ `git log --exit-code $tag..HEAD | wc -l` -ne 0 ] && gbp dch -s $tag -S --no-multimaint --nmu --ignore-branch --snapshot-number="'{:%Y%m%d%H%M%S}'.format(__import__('datetime').datetime.fromtimestamp(`git log -1 --pretty=format:%at`))"

sed -i 's/UNRELEASED/unstable/' debian-changelog
sed -i 's/\\n/\n/' debian-changelog
sed -i 's/\(_2\./\(2\./' debian-changelog
