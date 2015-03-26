# Introduction #

Information about use Google API, testing platforms, Qt versions.

# Details #

Project **qt-google-tasks** uses Google Tasks API. Project contains  version for desktop.

### Google API ###

Project **qt-google-tasks** uses Google Tasks API.

How it works:
Work with Google-API perfomed by send https-request with using QNetworkAccessManager (see files _tasks\_data\_manager.h_ and _tasks\_data\_manager.cpp_)

API features used in this project:
|get tasks lists|
|:--------------|
|create list|
|delete list|
|update list|
|clear tasks from list|
|get tasks by list|
|create task|
|delete task|
|update task|
|move task in list (up or down)|
|get user E-mail by token|


File **HowToRegisterYourAppIicationInGoogle** describes how register your own application on Google.

### Tested platforms ###
Project was tested on:
| **OS** | **Qt version** |
|:-------|:---------------|
|Linux 64bit|Qt 4.7.4, 4.8.1|
|Windows 7 64bit|Qt 4.8.0|
|Windows XP SP3 32bit|Qt 4.8.0|

# Various comments #

This application can be compiled on Linux, MacOS, Windows. For Windows you can use either MingW compiler or MSVC compiler.