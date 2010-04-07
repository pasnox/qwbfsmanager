Wii Backup File System(WBFS) by Kwiirk.

WBFS is a simple file system to store wii backup in an efficient way on a harddrive.

=== Wii Filesystem ===
All Wii Games do not take advantage of the whole 4.7GB DVD. 
A lot of WII dvd sectors are in fact never used, even if there is random encrypted garbage in it.
WBFS knows about the wiidisc filesystem, and will copy on the wbfs partition only the part that are really usefull.
The idea of ignoring unused wiidisc sectors is not new, and is implemented in the WiiScrubber tool.

=== Aknowledgement ===
wbfs is based on negentig, a tool from segher, that allows to verify and decrypt wii discs. a modified version 
is included, that allows to open discs on a wbfs filesystem.

=== Disclaimer ===
WBFS is a filesystem tool that may damage you disc partition if you dont know what you are doing. 
WBFS is in a early beta stage, a number of safety checks are not done, that may cause bugs. 
Please save your data, and report bugs (please be productive, and explain exactly the problem)
The authors of the software decline all responsabilities on damage that can been done by the use of this software.
This tool is not made to encourage piracy, only use your own backups from your own games.

=== Usage ===
Before using wbfs tool, you must initialize the drive. Preferably pre-format your USB drive to FAT32.

'X' in the following examples represents your drive letter, for example, G.

*init the partition:
 wbfs_win.exe X init

*estimate the size of the iso on the USB drive
 wbfs_win.exe X estimate <your_wiidisc.iso>

*add an iso to your partition
 wbfs_win.exe X add <your_wiidisc.iso>

*list the wiidisc that are on the wbfs, you will get the DISCID, game name, number of wide sectors used, and number of GB used.
 wbfs_win.exe X list

*count the number of wide sectors / GB available on your partition
 wbfs_win.exe X info

*build Homebrew Channel directories for all the games in your partition
This will actually make a directory for each game with the DISCID of the game, 
copy the icon.png and boot.dol of the current directory, and make a meta.xml with the name of the game
 wbfs_win.exe X makehbc
Then copy all the directories in the apps directory of your sdcard.

*remove a disc from wbfs
 wbfs_win.exe X remove DISCID

*extract an iso from wbfs
 wbfs_win.exe X extract DISCID




=== Programmers usage ===
wbfs is based on libwbfs, a library to handle wbfs discs.
Its usage is documented inside the header file libwbfs/libwbfs.h
The idea is that you only have to give libwbfs some function pointers that allow access to your partition, and original wii discs, 
and you can then manage a wbfs partition.
Some basic access to wiidiscs are provided by wiidisc.(h|c), allowing to extract files from the wii dvd filesystem, 
to change the partition table, and to find out which part of the wii dvd is used.
