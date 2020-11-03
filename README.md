# Arduino-Windows-UBS-Communication
Felix Baessler, felix.baessler@gmail.com

The project deals with the communication over an USB connection between:<br/>
an Arduino server and a client running on a MS Windows system.<br/>
The intention is to demonstrate basic operation principles at the example 
of a simple interaction scenario.

### Features:

- active port recognition
- client initiated server reset (wakeup)
- EOD character triggered synchronization

### LICENSE

This example code is in the public domain.

### CONTEXT

This demo is related to the project [OOK Raw Data Receiver](https://github.com/ookraw/OOK-Raw-Data-Receiver), 
where a radio receiver records signal durations in a trace 
that is processed at the source.

Instead of integrating recording and processing on a same MCU (Arduino), 
an option consists in transmitting the recorded trace to a full-fledged 
computer for further processing.

In this approach, the recording part takes place on the **radio server** (Arduino), 
whereas the processing part is executed by a client on a MS Windows system.
