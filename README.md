# PocoWSDumpServer
WebSocket Dump server using Poco C++ libraries

## Build Instructions for Windows (7+)
  * Download/Clone this repository.
  * Open it in Visual Studio 2017.
  * Download Poco C++ libraries from [Poco Downloads](https://pocoproject.org/download.html).
    & install it by following the instructions listed [here](http://www.davidrogers.id.au/wp/?p=1697]).
    * Note that the `buildwin.cmd` file builds/installs the 32-bit version of the Poco libraries by default.
  * Build the WebSocketServer project in Visual Studio.
  * You can either run the executable `$POCO_BASE\Net\samples\WebSocketServer\bin\WebSocketServer.exe`(for 64-bit builds) or 
    `$POCO_BASE\Net\samples\WebSocketServer\bin\WebSocketServerd.exe` (for 32-bit builds).
  * Log files would be stored at `$POCO_BASE\Net\samples\WebSocketServer\bin\logs\` and archived by default.
  * Note: This project has been tested on Windows 7 only.
  
 ## Build Instructions for Linux
   * Download/clone this repository.
   * Download Poco C++ libraries from ([Poco Downloads](https://pocoproject.org/download.html).
    & install it by following the instructions listed [here](https://pocoproject.org/docs/00200-GettingStarted.html).
     * If you have GNU Make version >= 3.80, then you run the following commands to build/install the Poco libraries
        ```bash
        $ cd poco-X.Y
        $ ./configure
        $ make -s
        $ sudo make -s install
        ```
   * Add the location of the downloaded Poco libraries repository to the environment variables as `$POCO_BASE`
   * Configure the rule files and Makefile (if reqd.) by referring [this](https://pocoproject.org/docs/99150-GMakeBuildNotes.html).
   * Run the required executable
     * for 64-bit builds/machine, run `$POCO_BASE/Net/samples/WebSocketServer/bin/Linux/x86-64/WebSocketServer`.
     * for 32-bit (&/or 64-bit as well) builds/machine, run `$POCO_BASE/Net/samples/WebSocketServer/bin/Linux/x86-64/WebSocketServerd`.
   * Log files will be stored at `$POCO_BASE/Net/samples/WebSocketServer/bin/Linux/x86-64/logs/`.
   * Note: This project has been tested on CentOS 7 only.
   
## References
  * [Poco Documentation](https://pocoproject.org/documentation.html)
  * Boilerplate: [Poco samples](https://github.com/pocoproject/poco/tree/develop/Net/samples)
  
