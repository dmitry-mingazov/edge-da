# :page_facing_up: Edge#DA
Project for the Data Analytics course at the University of Camerino.

## Description
The aim of the project was to understand how a framework like [ZeroMQ](https://zeromq.org) could be used to make a pub-sub communication system that could improve the data analytics pipeline by reducing the latency between sending and receiving data by moving all the computation to the **Edge Computing** level. It was decided to use a framework like [ZeroMQ](https://zeromq.org) because of its lightness and simplicity as the devices on which everything would run would be equipped with limited resources for computation.
All the code implementation has been written in **C** and the final architecture of the system is the following:
<br><br>
![](presentation/diagram.png)
- `zpub`'s responsibility is to generate random data in an IOT-like format and send it to the Edge Computing layer composed of 1 or more `zpi`s.
- `zpi`'s responsibility is to receive data from `zpub`, filter all the necessary information and send it to `zsub`. If more than one `zpi` is running at
the same time then the data sent by `zpub` will be splitted among all the `zpi` available using a **Round Robin** scheduling, in order to improve the
system performances.
- `zsub`'s responsibility is to receive data from ZPI and to save it inside an [SQlite](https://sqlite.) database.

## Prerequisites
- Debian or Ubuntu based Linux distribution*
- make
- gcc

\* tested on Ubuntu 21.04
## Installation
In order to install all the required libraries one should run the command below:

```
sudo apt install libzmq3-dev sqlite3 libsqlite3-dev uuid-dev
```
That includes [ZeroMQ](https://zeromq.org), [SQlite](https://sqlite.) and a library to operate with UUIDs.

In order to compile the solution, make should be used, running the command below in the project directory:
```
make all
```
At this point three executables (`zpub`, `zpi`, `zsub`) should have been created inside the new `out` directory.
## Usage

To run the proposed demo, at least three sessions must be used. This can be achieved opening different terminals or using a terminal multiplexer (e.g. [screen](https://www.gnu.org/software/screen/)).

One terminal will run `zpub`:
```
out/zpub [OPTIONS]
```
Following options are available:
```
-q <QUANTITY>        the program will send QUANTITY messages

-f <FREQUENCY>       the program will send message every FREQUENCY ms
```
One or more terminal will run `zpi`.
```
out/zpi
```
One terminal will run `zsub`.
```
out/zsub
```
To check if everything went well just access the database called `test.db` generated thanks to `zsub` using `sqlite3`
```
sqlite3
```
```
.open test.db
```
and run the following query:
```
select count(*)/3 from data;
```
If the number that comes out as a result is the same of the sent messages it means it all went right and no messages were lost during the transfer. <br>
In order to get the right result the number must be divided by 3 because every single message contains **3 different measures** that will be saved individually into the database.
## Authors
- **Luca Cervioni** (115919) \
luca.cervioni@studenti.unicam.it
- **Dmitry Mingazov** (115857) \
dmitry.mingazov@studenti.unicam.it
- **Tommaso Carletti** (115853) \
tommaso.carletti@studenti.unicam.it
