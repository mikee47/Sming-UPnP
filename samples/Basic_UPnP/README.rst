Basic UPnP
==========

Demonstrates use of UPnP library.

The sample devices here can be fully enumerated over the network.

Services
--------

UPnP devices may also provide services which can be enumerated and used to control it.

The Wemo device :cpp:class:`Wemo::Controllee` has two services for events and metadata.
At present only the events interface is implemented.

The service descriptions are stored in the ``wemo-service.xml`` file.
The UPnP stack handles decoding and parsing of incoming requests, which then calls
the :cpp:func:`UPnP::Service::handleAction` method which is overridden here by
:cpp:class:`Wemo::BasicEventService`.

