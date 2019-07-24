Clingo Server
=============

The goal of this project is to build a Clingo compute server. Clients can then
connect to the server inorder to submit jobs and get the results. Clingo server
aims to provide a general mechanism for building ASP-backed applications.

Consequently, beyond providing the basic ability to submit jobs and get the
results (i.e. retrieve models) clingo server will also allow for a complex set
of interactions between the client and server.

With Python/Lua integration Clingo offers a wide variety of ways of interacting
with the solver; for example allowing for incremental solving. The objective of
this project is preserve this flexibility by providing a general
subscriber/publisher communications protocol that allows for arbitrary
interactions with the client beyond simply running jobs and getting the results.


There will be at least three components: the server, the worker, and the client
interfaces.

Server
------

The server provides a central connection point for clients to submit jobs and
retrieve the results.

Build requirements
^^^^^^^^^^^^^^^^^^

* Flatbuffers (comms data encoding)
* Boost ASIO (basic network async IO)
* Boost Beast (http, websockets?)
* OpenSSL?
* Other libraries to provides a REST API with OpenAPI/SwaggerUI support?
* Catch2 for unit testing

Worker
------

The worker does the actual computation. A worker is spawned whenever a job is
submitted. It communicates with the server. Could also look at communicating
directly with client but I think to go through the server might make things
simpler.

Build requirements
^^^^^^^^^^^^^^^^^^

* Flatbuffers
* Clingo
* Boost ASIO (basic network async IO)
* OpenSSL?
* Catch2 for unit testing

Client libraries
----------------

The client libraries are key to getting the rich interaction with the
server. Will also need to support the publisher/subscriber model.

* A C++ client library
* A Python client

Build requirements
^^^^^^^^^^^^^^^^^^

* Flatbuffers
* Boost ASIO (for C++ client)

  
