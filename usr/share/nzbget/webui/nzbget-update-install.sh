#!/bin/sh
#
mkdir -p /tmp/nzbget
cd /tmp/nzbget
echo "Installing ${NZBUP_BRANCH}"
echo "Downloading new version...";
wget -q "http://jamied.pwp.blueyonder.co.uk/nzbget/nzbget.${NZBUP_BRANCH}.tar.gz"

if [ -f "/tmp/nzbget/nzbget.${NZBUP_BRANCH}.tar.gz" ]; then
    echo "Downloaded new version...OK"
    echo "Installing Updated Version."
    tar -zxf "/tmp/nzbget/nzbget.${NZBUP_BRANCH}.tar.gz"
    arch=$(uname -m | sed 's/x86_//;s/i[3-6]86/32/')
    if [ $arch -eq 32 ]; then
        echo "Installing binary file for a 32 bit systems"
        mv nzbget_i386 /opt/nzbget/
    elif [ $arch -eq 64 ]; then
        echo "Installing binary file for a 64 bit systems"
        mv nzbget_amd64 /opt/nzbget/
    else
        echo "Installing binary file for a arm type systems"
        mv nzbget_arm /opt/nzbget/
    fi

    if [ -f "/opt/nzbget/nzbget" ]; then
        rm /opt/nzbget/nzbget
    fi
	
    mv /opt/nzbget/nzbget_* /opt/nzbget/nzbget

    chown nzbget:users /opt/nzbget/nzbget
    rm -Rf /usr/share/nzbget/webui
    mv webui /usr/share/nzbget/
    sleep 2
    rm -R /tmp/nzbget
else
    rm -R /tmp/nzbget
    echo "[ERROR] Download failed, try again later"
    sleep 10
    exit 0
fi
#Stop NZBGet Daemon
echo -n "Stoping NZBGet Daemon : "
service nzbget stop
sleep 2
echo "Starting NZBGet..."
service nzbget start
#$NZBOP_APPBIN -c $NZBOP_CONFIGFILE -D
exit 0