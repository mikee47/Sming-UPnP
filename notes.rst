Development Notes
-----------------

These may be of some interest.

Device state management

   It should be possible to add and remove devices dynamically.

   For example, an application may contain a MODBUS controller which communicates
   with various slaves to switch lights, garden pumps, etc. We've also decided
   to create a UPnP device object for each MODBUS slave.

   If we add a new slave to the network, then our application creates a new UPnP
   device object and passes it to e.g. 'UPnP::addDevice()', which does this:

   -  Add the device to the stack
   -  Queue a set of 'ssdp:alive' announcements
   -  Notify the application via callback that the device has been added

   If a slave is to be taken out of the network:

   -  Remove the device from the stack so it won't respond to any new requests
   -  Queue a set of 'ssdp:byebye' announcements
   -  When the final announcement has been sent, notify the application
      via callback that the device has been removed

   When the application gets a 'remove' notification it can safely destroy the
   object, if appropriate, as UPnP has finished using it.


Device tree

   Add ``parent`` device to embedded devices so we can track back up the tree.
   Still not clear on whether a device needs to know about its direct parent,
   or whether a reference to the root device is adequate.
   A service certainly needs to know about its parent.
   So we need both root and parent. Root can be obtained by traversing back up
   the tree.


Icons

   Add IconList support


Control Points

   This involves sending search requests and responding to advertisements.
   Requesting descriptions also parsing XML. Using a SAX parser (as listed above)
   would allow template classes to be populated and passed to an application callback
   for handling. It would be the application's responsibility to decide what information
   to keep and how to manage it.


UPnP version

   Decide how to handle versioning. We should support previous versions but selectively enforce
   version 1.0, 1.1 or 2.0 as required. The version must be consistent throughout a device stack
   so we should add this as a configurable value for root devices. This then gets propagated
   into embedded object descriptions as well.
   We can have multiple root devices at different versions.


URL handling

   Responses from VR900 are illuminating. Gateway is on port 1900, media server on 8200.
   There doesn't seem to be any technical reason why multiple root devices can share the same port,
   but using separate ports does mean we can use separate HttpServer instances.
   
   It would not make much sense for a URL to be somewhere completely different, but it doesn't
   appear to be prohibited by the V1.0 spec. V2, however, deprecates URLBase and mandates that URLs
   are all relative to the SSDP response location (where the description file is served from).


Demonstration of discovery

   Write a sample for Host which performs an ssdp:all discovery and writes all the description
   files into a directory tree structure.

      Issue multicast search, register callback to receive responses
      For each response, store in a table (Vector)
         If not already in table, queue an HTTP request for the description file
            On receipt, write out the description file into a directory:
               <ip>/<port>/<path>/
         Queue an HTTP request for the presentation page
            Write to same location

   Components required:

      Item table. Implement using Vector. Contains:
         IP Address
         Port number

      HTTP request queue. Only service one fetch at a time.

