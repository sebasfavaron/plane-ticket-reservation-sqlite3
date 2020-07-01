# System to reserve plane tickets using sqlite3.

## Usage:
Table creation:

create table flights (flightName string, seatNumber int, isTaken int, primary key (flightName,seatNumber));
