/**
 * @brief WebSocket DumpServer - Dumps WS requests received to console.
 * 
 * @file WebSocketServer.cpp
 * @author Siddhant Shrivastava
 * @date 12-09-2018
 */

/// Header files reqd for WebSocket Server
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Format.h"
#include <iostream>

/// Header files for File/Path handling
#include "Poco/Path.h"
#include "Poco/File.h"

/// Header files required for Date, Time handling
#include "Poco/LocalDateTime.h"
#include "Poco/DateTime.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"

#include "Poco/Foundation.h"

/// Header file reqd. for exception handling
#include "Poco/Exception.h"

/// Header files reqd. for logging
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"

#include "Poco/AutoPtr.h"

#include "Poco/SplitterChannel.h"
#include "Poco/ConsoleChannel.h"


using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;

using Poco::Path;
using Poco::File;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::LocalDateTime;

using Poco::Logger;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::FileChannel;
using Poco::Message;

using Poco::AutoPtr;

using Poco::SplitterChannel;
using Poco::ConsoleChannel;


class PageRequestHandler: public HTTPRequestHandler
	/// Return a HTML document with some JavaScript creating
	/// a WebSocket connection.
{
public:
	/**
	 * @brief Returns a HTML document with some JavaScript creating a WebSocket connection
	 * 
	 * @param[in] request - Incoming WS request
	 * @param[out] response - HTML document is sent as a response
	 */
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		std::ostream& ostr = response.send();
		ostr << "<html>";
		ostr << "<head>";
		ostr << "<title>WebSocketServer</title>";
		ostr << "<script type=\"text/javascript\">";
		ostr << "function WebSocketTest()";
		ostr << "{";
		ostr << "  if (\"WebSocket\" in window)";
		ostr << "  {";
		ostr << "    var ws = new WebSocket(\"ws://" << request.serverAddress().toString() << "/ws\");";
		ostr << "    ws.onopen = function()";
		ostr << "      {";
		ostr << "        ws.send(\"Hello, world!\");";
		ostr << "      };";
		ostr << "    ws.onmessage = function(evt)";
		ostr << "      { ";
		ostr << "        var msg = evt.data;";
		ostr << "        alert(\"Message received: \" + msg);";
		ostr << "        ws.close();";
		ostr << "      };";
		ostr << "    ws.onclose = function()";
		ostr << "      { ";
		ostr << "        alert(\"WebSocket closed.\");";
		ostr << "      };";
		ostr << "  }";
		ostr << "  else";
		ostr << "  {";
		ostr << "     alert(\"This browser does not support WebSockets.\");";
		ostr << "  }";
		ostr << "}";
		ostr << "</script>";
		ostr << "</head>";
		ostr << "<body>";
		ostr << "  <h1>WebSocket Server</h1>";
		ostr << "  <p><a href=\"javascript:WebSocketTest()\">Run WebSocket Script</a></p>";
		ostr << "</body>";
		ostr << "</html>";
	}
};


class WebSocketRequestHandler: public HTTPRequestHandler
	/// Handles a WebSocket connection.
{
public:
	/**
	 * @brief Handles the incoming WS request
	 * 
	 * @param[in] request - request to be handled/processed
	 * @param[out] response - used to sendd back the recvd. frame
	 */
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		Application& app = Application::instance();

		try
		{
			WebSocket ws(request, response);

			/// Create & log current local timestamp
			LocalDateTime oNow;
			std::string sNowTimestamp = DateTimeFormatter::format(oNow, DateTimeFormat::RFC1123_FORMAT);
			app.logger().information(sNowTimestamp);

			app.logger().information("WebSocket connection established.");
			app.logger().information("================================================");

			char buffer[1024];
			int flags;
			int iFrameLength;
			do
			{
				iFrameLength = ws.receiveFrame(buffer, sizeof(buffer), flags);
				app.logger().information(Poco::format("Frame received (length=%d, flags=0x%x).", iFrameLength, unsigned(flags)));
				ws.sendFrame(buffer, iFrameLength, flags);
			}
			while (iFrameLength > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
			app.logger().information("WebSocket connection closed.");
			app.logger().information("================================================\n\n");
		}
		catch (WebSocketException& exc)
		{
			app.logger().log(exc);
			switch (exc.code())
			{
			case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
				response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
				// fallthrough
			case WebSocket::WS_ERR_NO_HANDSHAKE:
			case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
			case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
				response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
				response.setContentLength(0);
				response.send();
				break;
			}
		}
	}
};


class RequestHandlerFactory: public HTTPRequestHandlerFactory
/// Factory class for returning either a new WebSocketRequestHandler or
/// a new PageRequestHandler based on the incoming WS request
{
public:
	/**
	 * @brief Create a [WebSocket, Page]RequestHandler object
	 * 
	 * @param[in] request - WS request for which necessary object is created
	 * @return HTTPRequestHandler* 
	 */
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		Application& app = Application::instance();
		app.logger().information("Request from " 
			+ request.clientAddress().toString()
			+ ": "
			+ request.getMethod()
			+ " "
			+ request.getURI()
			+ " "
			+ request.getVersion());
			
		for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
		{
			app.logger().information(it->first + ": " + it->second);
		}
		
		if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
			return new WebSocketRequestHandler;
		else
			return new PageRequestHandler;
	}
};


class WebSocketServer: public Poco::Util::ServerApplication
	/// The main application class.
	///
	/// This class handles command-line arguments and
	/// configuration files.
	/// Start the WebSocketServer executable with the help
	/// option (/help on Windows, --help on Unix) for
	/// the available command line options.
	///
	/// To use the sample configuration file (WebSocketServer.properties),
	/// copy the file to the directory where the WebSocketServer executable
	/// resides. If you start the debug version of the WebSocketServer
	/// (WebSocketServerd[.exe]), you must also create a copy of the configuration
	/// file named WebSocketServerd.properties. In the configuration file, you
	/// can specify the port on which the server is listening (default
	/// 9980) and the format of the date/time string sent back to the client.
	///
	/// To test the WebSocketServer you can use any web browser (http://localhost:9980/).
{
public:
	WebSocketServer(): _helpRequested(false)
	{
	}
	
	~WebSocketServer()
	{
	}

protected:
	/**
	 * @brief Loads configuration file & initializes the application
	 * 
	 * @param[out] self - application to be initialized
	 */
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present

		/// Configures the application's root logger
		/// to log to both console & specified log file

		/// Create & configure the channels
		/// one for console channel & other for pattern formatted file channel
		/// Merge both to a Splitter Channel

		// Creates a ConsoleChannel
		AutoPtr<ConsoleChannel> pConsoleChannel(new ConsoleChannel);

		// Create log file path & corresponding directories
		Path pLogFilePath(false);
		pLogFilePath.pushDirectory("log");
		File pLogFile(pLogFilePath);
		pLogFile.createDirectories();
		// Set the Log File name
		pLogFilePath.setFileName("log.raw");

		/// Create log file path based on the OS
#if defined(POCO_OS_FAMILY_WINDOWS)
		std::string sLogFileName(pLogFilePath.toString(Path::PATH_WINDOWS));
#elif defined(POCO_OS_FAMILY_UNIX)
		std::string sLogFileName(pLogFilePath.toString(Path::PATH_UNIX));
#endif

		// Creates a Pattern Formatter - formats each log message
		AutoPtr<PatternFormatter> pPF(new PatternFormatter("%Y-%m-%d %H:%M:%S.%c %N[%P]:%s:%q:%t"));
		// Creates a FileChannel & configures/sets up the same
		AutoPtr<FileChannel> pFileChannel(new FileChannel);
		// pFileChannel->setProperty("path", "log.raw");
		pFileChannel->setProperty("path", sLogFileName);
		pFileChannel->setProperty("rotation", "5 K");
		pFileChannel->setProperty("archive", "timestamp");
		// Merge both the PatternFormatter & FileChannel
		// to a resultant FormattingChannel
		AutoPtr<FormattingChannel> pFCFile(new FormattingChannel(pPF, pFileChannel));

		// Creates & configures a SplitterChannel with the channels specified above
		AutoPtr<SplitterChannel> pSplitterChannel(new SplitterChannel);
		pSplitterChannel->addChannel(pFCFile);
		pSplitterChannel->addChannel(pConsoleChannel);

		// Sets app's root logger's channel to the above SplitterChannel
		Logger::root().setChannel(pSplitterChannel);

		// Logger::root().information("This is a test log!");
		// Logger::root().information(pLogFilePath.toString(Path::PATH_WINDOWS));
		Logger::root().information("Log files stored at: " + sLogFileName);

		ServerApplication::initialize(self);
	}
		
	/**
	 * @brief Unintializes the Poco application
	 * 
	 */
	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	/**
	 * @brief Defines the available Poco application command-line options
	 * 
	 * @param[out] options - OptionSet which is updated
	 */
	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);
		
		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false));
	}

	/**
	 * @brief Sets the flag corresponding to the CLI options provided
	 * 
	 * @param[in] name - option name
	 * @param[in] value option value
	 */
	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
	}

	/**
	 * @brief Handles the "help" option
	 * 
	 */
	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A sample HTTP server supporting the WebSocket protocol.");
		helpFormatter.format(std::cout);
	}

	/**
	 * @brief Main/Driver function
	 * 
	 * @param[in] args - CLI arguments provided
	 * @return int - exit status 
	 */
	int main(const std::vector<std::string>& args)
	{
		if (_helpRequested)
		{
			displayHelp();
		}
		else
		{
			// get parameters from configuration file
			unsigned short port = (unsigned short) config().getInt("WebSocketServer.port", 9980);
			
			// set-up a server socket
			ServerSocket svs(port);
			// set-up a HTTPServer instance
			HTTPServer srv(new RequestHandlerFactory, svs, new HTTPServerParams);
			// start the HTTPServer
			srv.start();
			// wait for CTRL-C or kill
			waitForTerminationRequest();
			// Stop the HTTPServer
			srv.stop();
		}
		return Application::EXIT_OK;
	}
	
private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(WebSocketServer)
