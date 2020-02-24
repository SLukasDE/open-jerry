# jerry
Jerry is a free reference HTTP application server for C++ comparable to Tomcat for JAVA. Is is based on the ESL framework. It also needs TinyXML2 to load the file server.xml.

ESL bindings are not needed to build this code. You just configure the necessary ESL bindings in the file server.xml with \"<library file=\"...\" module=\"esl\">\" and that's it. So, you can change your binding, for example the used HTTP server implementation just by modifing the file server.xml. Bindings are needed for "esl-http-server" and "esl-system". So you can use mhd4esl and zsystem4esl.
Other usefull, but not necessary bindings are "esl-logging" and "esl-stacktrace". You can use "logbook4esl" and "boostst4esl" for this.
  
You can easily write your own request handler with the MIT licensed ESL framework, compile it as dynamic library (shared object on Linux or DLL on Windows). For writing this library you only need the ESL framework. There is no other depency needed. Of course you can use your own request handlery directly without Jerry if you make your own easy server code with ESL. But if you want to use a real application server for it, jerry would be a good choise.
