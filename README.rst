IOPylontech
===========

Pylontech storage battery control library.

This is a `Sming <https://github.com/SmingHub/Sming>` library to provide
basic access via the RS232 console port.

The primary purpose is to allow fetching of battery system status via the PWR and BAT commands.
These are structured as 'query' commands. To fetch summary status for all batteries:

.. code-block:: json

   {
      "device": "bms",
      "command": "query",
      "node": 0,
   }

To fetch data for a specific battery, set 'node' accordingly (e.g. 1).

.. note::

   The master battery connects to an inverter via CAN.
   The port labelled 'RS485' appears to also operate as CAN.

   This suggests that the battery operates in either CAN or RS485, not both.


.. image:: xxxxxxxx

.. doxygennamespace:: IO::RS485::Pylontech
   :members:
