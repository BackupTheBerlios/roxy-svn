==============================================================================
                   Roxy - Asio based http proxy server
==============================================================================
Updated   04.06.2008
Support   andrew.selivanov at gmail dot com
Download  http://www.box.net/shared/eq4w6urggs
==============================================================================

Overview
========
Roxy is a multithreaded http proxy server based on http/server3 asio example 
by Christopher Kohlhoff.


Building
========
You need to have Boost 1.35 (the current release)
Unpack the archive (proxy.zip) into "%BOOST_HOME%/libs/asio/example"
Change directory to the "%BOOST_HOME%/libs/asio/example/proxy"
Run "bjam --toolset=gcc"


How to use it
=============
http_proxy <host> <port> <thread_num>
http_proxy localhost 8080 10

Thanks
======
Thanks to Chris for Asio! :)