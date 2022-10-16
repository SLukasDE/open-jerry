# jerry ![](https://img.shields.io/github/license/SLukasDE/jerry.svg)

Jerry is a free reference HTTP application server for C++ like Apache Tomcat for JAVA. Is is based on the ESL framework. It also needs TinyXML2 to load the server configuration file (e.g. server.xml).

There is just the ESL framework needed to build this code without any other ESL bindings. ESL bindings will be configured in the server configuration file with \"<library file=\"...\" module=\"esl\">\" and loaded as dynamic library. So, you can change your binding, for example the HTTP server implementation just by modifing the file server.xml. Bindings are needed for "esl-http-server" and "esl-system". So you can use mhd4esl and zsystem4esl.
Usefull, but not necessary bindings are "esl-logging" and "esl-stacktrace". "logbook4esl" and "boostst4esl" provide implementations for "esl-logging" and "esl-stacktrace" for example.
  
You can easily write your own request handler with the MIT licensed ESL framework, compile it as dynamic library (shared object on Linux or DLL on Windows). Again, for writing this library you only need the ESL framework. There is no other depency needed as well. Of course you can use your own request handler directly and run it without Jerry if you make your own easy server code with ESL. But if you want to use a real application server for it, jerry would be a good choise.
