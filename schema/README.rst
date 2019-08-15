Flatbuffers Schemas
-------------------

This directory contains the Flatbuffers schemas that make up the Clingo compute
server communications messages (between the server and worker and server and
client).

Message are sent over a stream (TCP socket or websocket) and are encoded in a
package consisting of a 4-byte (network endian) message length and the message
body being the Flatbuffers encoded object.


Terminology
-----------

Worker
^^^^^^

A worker is a process that wraps clingo with network communications.


Client - Driver
^^^^^^^^^^^^^^^

A client process intiates the solving of an ASP problem. The client talks to a
worker (either directly or through a server process).


Server
^^^^^^

The server connects workers to clients. It can spawn and manage multiple
workers. With a server running the client can disconnect from the worker without
killing the worker.


Client to Server
----------------




Worker to Server or Client
--------------------------

- The schema worker_write.fbs combines the different messages:

  - READY
  - APPLICATION
  - STOPPED

Workers are spawned as separate processes by the server (or client). Before
anything happens the worker must send an INIT_CONNECTION message followed by a
WORKER_READY message.
