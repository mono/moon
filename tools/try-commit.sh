#!/bin/bash -e

URL=http://moon.sublimeintervention.com/
COMMIT=HEAD
LANE=try-trunk
ACTION=None
BRANCH=master
USER=
PASSWORD=

ACTION_VALID=0
VALID=0

usage ()
{
	echo ""
	echo "Usage : mw-try-commit --websiteurl=url --commit=sha1 --lane=name-of-try-lane --action=none|cherry-pick|merge --branch=target-branch --user=user --password=password"
	echo ""
	echo "This script allows selecting and configuring a set of MonoDevelop"
	echo "modules to be included in an integrated build."
	echo ""
	echo "Options:"
	echo ""
	echo "--websiteurl=url"
	echo ""
	echo "  The url to the website where MonkeyWrench is running."
	echo ""
	echo "--commit=sha1"
	echo ""
	echo "  The commit to try out. You must have pushed the commit first."
	echo ""
	echo "--lane=name-of-try-lane"
	echo ""
	echo "  The name of the lane you want to use to try the commit."
	echo ""
	echo "--action=none|cherry-pick|merge"
	echo ""
	echo "  What to do if the commit succeeds."
	echo "  In any case an e-mail will be sent to the committer."
	echo ""
	echo "--branch=target-branch"
	echo ""
	echo "  Specifies which branch to cherry-pick or merge the commit (according to"
	echo "  the action you specify) if the commit succeeds. The default is 'master'."
	echo ""
	echo "--user=user"
	echo ""
	echo "  The user to use to login on the MonkeyWrench server."
	echo ""
	echo "--password=password"
	echo ""
	echo "  User's password."
	echo ""
}

validate_action ()
{
	# change to lowercase
	ACTION=${ACTION,,}
	if [[ "x$ACTION" == "xnone" || "x$ACTION" == "x0" ]]; then 
		ACTION_VALID=0
		ACTION=none
	elif [[ "x$ACTION" == "xcherry-pick" || "x$ACTION" == "xcherrypick" || "x$ACTION" == "x1" ]]; then
		ACTION_VALID=0
		ACTION=cherry-pick
	elif [[ "x$ACTION" == "xmerge" || "x$ACTION" == "x2" ]]; then
		ACTION_VALID=0
		ACTION=merge
	else
		ACTION_VALID=1
	fi
}

while test x$1 != x; do
	case $1 in
		--websiteurl=*)
			URL=`echo $1 | sed 's/--websiteurl=//'`
			;;
		--commit=*)
			COMMIT=`echo $1 | sed 's/--commit=//'`
			;;
		--lane=*)
			LANE=`echo $1 | sed 's/--lane=//'`
			;;
		--action=*)
			ACTION=`echo $1 | sed 's/--action=//'`
			;;
		--branch=*)
			BRANCH=`echo $1 | sed 's/--branch=//'`
			;;
		--user=*)
			USER=`echo $1 | sed 's/--user=//'`
			;;
		--password=*)
			PASSWORD=`echo $1 | sed 's/--password=//'`
			;;
		--help)
			usage
			exit
			;;
		*)
			echo Unknown argument $1 >&2
			usage
			exit 1
			;;
	esac
	shift
done

get_data ()
{
	read -a data -p"Enter $1 or (q) quit: "
}

while [[ 1 ]]
	do
		VALID=1
		echo Configure the try commit
		echo
		echo "1) Url: $URL"
		if [[ "x$URL" == "x" ]]; then
			echo "     You need to enter the website url"
			VALID=0
		fi
		COMMIT=`git log -1 $data --format=format:%H`
		echo "2) Commit: $COMMIT"
		if [[ "x$COMMIT" != "x" ]]; then
			echo "     Subject: `git log -1 $COMMIT --format=format:%s`"
			echo "     Author : `git log -1 $COMMIT --format=format:\"%aN <%aE>\"`"
		else
			echo "     You need to enter the commit"
			VALID=0
		fi
		echo "3) Lane: $LANE"
		if [[ "x$LANE" == "x" ]]; then
			echo "     You need to enter the lane"
			VALID=0
		fi
		echo "4) Action: $ACTION"
		validate_action
		if [[ "x$ACTION_VALID" == "x1" ]]; then
			echo "     The action '$ACTION' is not valid. Choose between 'None', 'Cherry-pick' and 'Merge'."
			VALID=0
		fi
		if  [[ "x$ACTION" != "xnone" ]]; then
			echo "5) Branch: $BRANCH"
		fi
		
		echo " $MSG"
		echo "Enter the number to edit"
		if [[ "x$VALID" == "x1" ]]; then
			read -a response  -p"(q) quit, or ENTER to continue:  "
		else
			read -a response  -p"(q) quit:  "
		fi
		echo

		if [[ "x$VALID" == "x1" ]]; then
			if [ -z $response ] ; then
				break
			fi
		fi

		case $response in
			"1")
				get_data websiteurl
				URL=$data
				;;
			"2")
				get_data commit
				COMMIT=$data
				;;
			"3")
				get_data lane
				LANE=$data
				;;
			"4")
				get_data action
				ACTION=$data
				;;
			"5")
				get_data branch
				BRANCH=$data
				;;
			"q" | "Q")
				exit 1
				;;
			"h" | "H")
				usage
				;;
			*)
				MSG="Unknown option '$response'"
				;;
		esac
done

if [[ "x$USER" == "x" ]]; then
	read -a USER -p"User: "
	read -a PASSWORD -p"Password: "
fi

echo Contacting server, this may take a little while...
curl "$URL/TryCommit.aspx?lane=$LANE&action=$ACTION&branch=$BRANCH&commit=$COMMIT&output=text&user=$USER&password=$PASSWORD"
echo
