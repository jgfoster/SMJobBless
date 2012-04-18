Read Me About SMJobBless
====================================
This Xcode project is an extension of the SMJobBless project found at http://developer.apple.com/library/mac/#samplecode/SMJobBless/Introduction/Intro.html. (The previous ReadMe.txt is now found at ReadMe2.txt.) The original sample code installs a helper tool but does not actually use it. Setting up inter-process communication is non-trivial, but left as an exercise for the reader. Based on various resources (including https://developer.apple.com/library/mac/#samplecode/SampleD/Introduction/Intro.html), I've expanded SMJobBless so that it actually communicates with the helper.

Communication is done using Unix domain sockets (see http://developer.apple.com/library/mac/#technotes/tn2083/_index.html#//apple_ref/doc/uid/DTS10003794-CH1-SUBSECTION32). The socket is defined in SMJobBlessHelper-Launchd.plist, and launchd will create the file (/var/run/com.apple.SMJobBlessHelper.socket) automatically and monitor it for you. 

Most of the additional code here is to handle the complexities of sockets. The helper acts as a server so needs to set up a listener, wait up to 10 seconds for a connection, then accept the connection. Once we have a connection, we wait for up to one second for data and attempt to read a full message. We parse the message and based on the content execute various commands and return a result. The first command is one that returns a version number. This allows the client (SMJobBlessApp) to query the helper for a version and forego the authorization/install process if the version is current.

SMJobBlessApp has been modified to add a 'Poke' button to MainMenu.xib and link it to a -poke: method in SMJobBlessAppController. This method queries the helper for its PID and adds a message to the log. To see this all in action do the following: open the Console so you can see logging messages, build the project, run the application, authorize the installation of the helper tool, and then click the poke button. The helper should report its PID to the log and respond to the application's request. The application should show the same PID in its log. If you poke again within a few seconds, the same PID should display. If you wait longer than 10 seconds, the helper should have quit and will get restarted with a different PID.

To adapt this to your use you will need to add your own commands. Note that each time you rebuild the helper you need to change the bundle version in SMJobBlessHelper-Info.plist (you should also change it in SMJobBless.h to keep things consistent) before doing a Clean and Build. The problem is that if the version number is not changed then the OS thinks that it already has the current code so does not update anything.

Summary of changes from Apple's SMJobBless 1.1 (of June 2010):

* SMJobBless.h - describes some shared information between the helper and the app
* SMJobBless.c - a public function, readMessage() to handle the complexities of message format and reading on a socket stream (which might break up a message into multiple packets)
* SMJobBlessHelper/SMJobBlessHelper-Info.plist - be sure to update Bundle Version on each build
* SMJobBlessHelper/SMJobBlessHelper-Launchd.plist - add ProgramArguments (which are actually replaced) and Sockets
* SMJobBlessHelper/SMJobBlessHelper.c - figure out which socket to use, listen, connect, respond to request
* SMJobBlessApp/SMJobBlessAppController.h - add -poke: as an action
* SMJobBlessApp/SMJobBlessAppController.m - modify -blessHelperWithLabel:error: to check version, add -poke: to send message and log result
* Resources/MainMenu.xib - add Poke button and link to -poke: method

The request and response messages both have the same format, a struct defined in SMJobBless.h. A macro, initMessage(), is available for initializing a message with a command. 

Version History
---------------------------
1.0 (Apr 2012) Update of Apple's sample code.
