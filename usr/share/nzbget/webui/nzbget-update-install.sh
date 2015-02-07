#!/bin/sh
mkdir -p /tmp/nzbget
chmod 775 /tmp/nzbget
cd /tmp/nzbget

URL="http://jamied.pwp.blueyonder.co.uk/nzbget/nzbget.${NZBUP_BRANCH}.tar.gz"

echo "Installing ${NZBUP_BRANCH}"

echo "Downloading new version..."
wget -q --no-check-certificate "${URL}" || echo "[ERROR] Download failed" | exit 1
echo "Downloading new version...OK";
tar -zxf "/tmp/nzbget/nzbget.${NZBUP_BRANCH}.tar.gz"
chmod 775 /tmp/nzbget -R
chown nzbget:users /tmp/nzbget -R
cd /

# Installing the update
arch=$(uname -m | sed 's/x86_//;s/i[3-6]86/32/')
if [ $arch -eq 32 ]; then
    echo "Installing binary file for a 32 bit systems"
    mv /tmp/nzbget/nzbget_i386 /opt/nzbget/
elif [ $arch -eq 64 ]; then
    echo "Installing binary file for a 64 bit systems"
    mv /tmp/nzbget/nzbget_amd64 /opt/nzbget/
else
    echo "Installing binary file for a arm type systems"
    mv /tmp/nzbget/nzbget_arm /opt/nzbget/
fi

if [ -f "/opt/nzbget/nzbget" ]; then
    rm /opt/nzbget/nzbget
fi

echo "Restarting NzbGet..."
mv /opt/nzbget/nzbget_* /opt/nzbget/nzbget
chown nzbget:users /opt/nzbget/nzbget
rm -Rf /usr/share/nzbget/webui
mv /tmp/nzbget/webui /usr/share/nzbget/

rm -Rf /tmp/nzbget
setsid /etc/init.d/nzbget restart
echo "Update Completed, close window and refresh the page."
exit 0