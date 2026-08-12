.. _clixon_sample:

Clixon Basic Sample
###################

Overview
********

This sample demonstrates the basic usage of the Clixon library with Zephyr RTOS.
Clixon is a YANG-based configuration manager with interactive CLI, NETCONF and 
RESTCONF interfaces, an embedded database and transaction mechanism.

Requirements
************

* A board with sufficient RAM (minimum 8KB heap recommended)
* Console support for output

Building and Running
********************

This application can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: modules/lib/clixon/zephyr/sample
   :host-os: unix
   :board: qemu_x86
   :goals: run
   :compact:

To build for another board:

.. code-block:: console

   west build -b <board> modules/lib/clixon/zephyr/sample

Sample Output
*************

.. code-block:: console

   Clixon Sample Application
   =========================

   Clixon version: 0.1.0
   This is a sample application demonstrating Clixon library integration
   with Zephyr RTOS.

   Initializing Clixon...
   Clixon initialization complete

   Demonstrating Clixon features:
   - YANG-based configuration management
   - NETCONF/RESTCONF interfaces
   - Embedded database
   - Transaction mechanism

   Clixon sample running...

Features Demonstrated
*********************

* Clixon library initialization
* Basic configuration management concepts
* Integration with Zephyr RTOS

Further Information
*******************

For more information about Clixon, see:

* Clixon Documentation: https://clixon-docs.readthedocs.io
* Clixon Project: https://www.clicon.org
* Clixon GitHub: https://github.com/clicon/clixon
