# XLRest

This projects reads the swagger.json file from a web api to create excel UDFs (User Defined Functions).

### Prequesite

* Install Visual Studio 2015 (https://www.visualstudio.com/downloads/)
* Install Excel SDK (https://msdn.microsoft.com/en-us/library/office/bb687883.aspx)

### Config

The config file is lib/xlrest.json but should be copied to your home directory (%USERPROFILE%)

* active: "true"  
* host: "localhost", server name
* port: 8301, port name
* context: "/kernel", context used by the service
* key: "francois", key used by nginx to choose a server (default: username)
* use_https: false, 
* use_proxy: false, uses (or not) the system proxy
* category: "sds", category used by excel associated to the functions
* prefix: "rms_", prefix added to all the functions
* tags: ["reports"], tags used to select the function in the swagger.json
* methods: ["GET"], the http methods allowed

### Test

Executes lib/launch.bat to start Excel
The test project contains tests to be executed in Debug mode only.
The release mode exports only what is necessary for global optimization. 

### Log

The log file is in %TEMP%\xlrest.log
The info/warning/error (no debug) logs are sent to Splunk in Release mode

### HTTP headers

All the requests send those http headers for monitoring

* X-CLIENT-HOSTNAME : the client host name
* X-CLIENT-USERNAME : the client user name
* X-CLIENT-SHEETNAME : excel sheet name
* X-CLIENT-SESSION : when excel was started
* X-CLIENT-KEY : the key specified in the config file
* X-CLIENT-REQUEST : the request number