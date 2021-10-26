UPnP
====

`Universal Plug and Play <https://en.wikipedia.org/wiki/Universal_Plug_and_Play>`__.

Introduction
------------

A C++ library for managing UPnP devices and services using the Sming framework.

If you're not famililar with the mechanics of UPnP See :doc:`about` for some background information.


Schema
------

A separate :library:`UPnP-Schema` is used to manage UPnP device and service schema.
It generates C++ classes and sample code directly from these, which you can then use in your
application.

Generation of suitable schema can be done using the **scanner** tool.


Controlling devices
-------------------

The :sample:`Basic_ControlPoint` sample shows how this is done.

Registration
   To control UPnP-enabled devices on your local network they must first be located.

   In order for this to happen, the framework must be able to match the C++ class implementation
   against the schema. You must therefore register all devices and services that you wish to
   control by calling :cpp:func:`UPnP::ControlPoint::registerClasses`.
   
   This method takes a :cpp:class:`UPnP::ClassGroup` which typically defines all devices and services
   belonging to a specific domain. See :library:`UPnP-Schema` for further details about the available schema.

   Here's an example from the ControlPoint sample::

      #include <Network/UPnP/schemas-upnp-org/ClassGroup.h>

      // Let's make things a little easier for ourselves
      using namespace UPnP::schemas_upnp_org::device;
      using namespace UPnP::schemas_upnp_org::service;

      void initUPnP()
      {
         UPnP::schemas_upnp_org::registerClasses();
      }


Discovery

   This is done using a :cpp:func:`UPnP::ControlPoint::beginSearch` method, which takes
   two parameters: the first identifies what you are looking for, the second is a callback
   which gets invoked when a match has been found.
   You'll typically implement this callback using a lambda function.

   For example, let's find all ``MediaRenderer1`` devices::

      // Only one active search is permitted so be sure to cancel any existing ones first
      controlPoint.cancelSearch();
      controlPoint.beginSearch(Delegate<bool(MediaRenderer1&)>([](auto& device) {
         // We can now do stuff with the located device

         // Return true to keep the device, false to destroy it
         return false;
      });

   This method takes a template parameter which is the C++ class type defining the device you
   are searching for. The framework will fetch the description for each corresponding device
   and construct a :cpp:class:`DeviceControl` object with appropriate services and embedded devices.

Control
   Your search callback function gets a reference to a located device. These devices are created
   on the heap and owned by the :cpp:class:`UPnP::ControlPoint`. If you want to keep the device,
   you should take a reference to it and return ``true`` from the callback.

   To actually do anything useful typically requires use of a :cpp:class:`UPnP::ServiceControl` object.
   You'll usually get this by calling :cpp:class:`UPnP::DeviceControl::getService` or one of the
   generated helper methods. Note that this returns a pointer, which will be ``nullptr`` if the
   service isn't available::
   
      auto render = device.getRenderingControl();
      if(render != nullptr) {
         // ...
      }

   Once you have a Service object, you can control it using action methods::

         render->getVolume(0, RenderingControl1::Channel::fs_Master, [&device](auto response) {
            // Process response here
         });
   
   Action methods take a list of zero or more input parameters, with the final argument for the response.

.. note::

   The exact type of the response can be determined for you by the compiler.
   Here's the explicit call::

      render->getVolume(0, RenderingControl1::Channel::fs_Master, [&device](RenderingControl1::GetVolume::Response response response) {
         // ...
      });


   OK, so handling the action method response. You can get the result values using methods of ``response``,
   but you must first check that the device did not return a fault::

      Serial.println();
      Serial.println(_F("render->getVolume(0, Master):"));
      // Sample uses a `checkResponse` helper function
      if(auto fault = response.fault()) {
         fault.printTo(Serial);
      } else {
         Serial.print(device.friendlyName());
         Serial.print(_F(": Current Volume = "));
         Serial.println(response.getCurrentVolume());
      }
      Serial.println();


Implementing devices
--------------------

The :sample:`Basic_UPnP` sample contains a couple of examples of how to create your own hosted devices.
The ``TeaPot`` device is the simplest possible implementation, with no services.

The ``Wemo`` device is more elaborate and has two services.

Both of these are constructed using code generated from custom schema.
These are located in the project's ``schema`` directory which is picked up automatically
when the :library:`UPnP-Schema` library is built.

The framework generates a class template for each device and service.
For example, take a look in ``Wemo.h``::

   class BasicEventService : public service::basicevent1Template<BasicEventService>
   {
   public:
      // Need access to constructors
      using basicevent1Template::basicevent1Template;

      // Override methods if you need to customise any fields
      String getField(Field desc) const override
      {
         switch(desc) {
         case Field::serviceId:
            // You could also put this in the schema
            return F("urn:Belkin:serviceId:basicevent1");
         default:
            return basicevent1Template::getField(desc);
         }
      }

      // Access to our device implementation
      Controllee& controllee()
      {
         return reinterpret_cast<Controllee&>(device());
      }


      /* Here are the action methods */

      Error getBinaryState(GetBinaryState::Response response)
      {
         response.setBinaryState(controllee().getState());
         return Error::Success;
      }
   
      Error setBinaryState(bool state, SetBinaryState::Response response)
      {
         controllee().setState(state);
         return Error::Success;
      }
   };
   
This perhaps slightly strange construction uses
`CRTP <https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`__
to use static polymorphism and avoid virtual method tables.
This allows the compiler to generate more efficient code.


UPnP Tools
----------

Windows:

-  `Developer Tools for UPnP Technologies <https://www.meshcommander.com/upnptools>`__

Linux:

   Under Ubuntu Linux you can install `gupnp-tools`::

      sudo apt install gupnp-tools

   And then discover devices on the local network using the following command::

       gupnp-universal-cp

.. image:: upnp-browser.png

   You can also start a "software" smart bulb device and use it to test your control point application::

      gupnp-network-light


API Documentation
-----------------

.. doxygennamespace:: UPnP
   :members:
