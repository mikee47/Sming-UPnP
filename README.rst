UPnP
====

`Universal Plug and Play <https://en.wikipedia.org/wiki/Universal_Plug_and_Play>`__.

Introduction
------------

A C++ library for managing UPnP devices and services using the Sming framework.

If you're not famililar with the mechanics of UPnP See :doc:`about` for some background information.


Control Points
--------------

To control existing UPnP devices the framework can generate C++ classes directly from device schema.
This is a two-stage process, supported by the **scanner** tool.

Creation of C++ classes is done using XSL transforms from modified versions of the descriptions.




UPnP Tools
----------

Windows:

-  `Developer Tools for UPnP Technologies <https://www.meshcommander.com/upnptools>`__

Linux:

   Under Ubuntu Linux you can install `gupnp-tools`::
   
      sudo apt install gupnp-tools

   And then discover devices on the local network using the following command::
      
      gssdp-discover

   Filter like this::
   
      gssdp-discover --target=upnp:rootdevice
