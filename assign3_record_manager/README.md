# Fall 2021 - ADO Assignment - 3 : Record Manager
The goal of this assignment is to implement a simple record manager. The record manager handles tables with a fixed schema. Clients can insert records, delete records, update records, and scan through the records in a table.
A scan is associated with a search condition and only returns records that match the search condition. Each table should be stored in a separate page file and your record manager should access the pages of the file through the buffer manager implemented in the last assignment.

## Authors
- Chirag Dara - cdara1@hawk.iit.edu A20493550
- Malhar Hunge - mhunge@hawk.iit.edu A20492858
- Shishir Kondai- skondai@hawk.iit.edu A20492225

## RUNNING THE SCRIPT:

```
1. Run 'make clean'
2. Run 'make output1 test_expr'
3. Run ./output1
4. Run ./test_expr
```