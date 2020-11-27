Basic UPnP
==========

Demonstrates use of UPnP library.

The sample devices here can be fully enumerated over the network.

Services
--------

UPnP devices may also provide services which can be enumerated and used to control it.

The Wemo device :cpp:class:`Wemo::Controllee` has two services for events and metadata.

The device and service descriptions are stored in the ``schema`` directory.
This is used by UPnP to generate class code, so all we need to do is implement the action methods.
