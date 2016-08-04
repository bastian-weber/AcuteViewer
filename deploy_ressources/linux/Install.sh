#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root. Use the sudo command."
   exit 1
fi

#get script path
script=$(readlink -f "$0")
scriptPath=$(dirname "$script")

echo "This script will copy the Acute Viewer application to a location of you choice and create a desktop file."
read -e -p "Installation path: " -i "/opt" path
if [ -d "$path" ]; then
	installPath="$path/AcuteViewer"
	uninstallScript="$installPath/Uninstall.sh"
	if mkdir -p $installPath; then
		#copy files
		cp -r "$scriptPath/"* "$installPath"
		#add symbolic link for use from command line
		sudo ln -s $installPath/Run_AcuteViewer.sh /usr/bin/acuteviewer
		#write desktop file
		cat <<- _EOF_ > "/usr/share/applications/AcuteViewer.desktop"
		[Desktop Entry]
		Version=1.0
		Name=Acute Viewer
		GenericName=Acute Viewer

		Exec=$installPath/Run_AcuteViewer.sh
		Terminal=false
		Icon=$installPath/data/icon_128.png
		Type=Application
		Categories=Unity;Application;
		_EOF_
		#write uninstall script
		cat <<- _EOF_ > "$uninstallScript"
		#!/bin/bash
		
		if [[ \$EUID -ne 0 ]]; then
   			echo "This script must be run as root. Use the sudo command."
   			exit 1
		fi
		rm -r "$installPath"
		rm "/usr/share/applications/AcuteViewer.desktop"
		rm "/usr/bin/acuteviewer"
		echo "Uninstallation was successful."
		_EOF_
		chmod +x "$uninstallScript" 
		echo "Installation was successful."
	else
		echo "Could not create SimpleViewer directory. Make sure you have sufficient permissions to write to the folder you specified."
	fi
else
	echo "The path you entered is not valid. Installation unsuccessful."
fi
