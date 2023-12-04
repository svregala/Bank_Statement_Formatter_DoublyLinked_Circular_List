# Bank_Statement_Formatter_DoublyLInked_Circular_List
 
**Synopsis**:
This is an implementation for a doubly-linked circular list (my402list.c, my402list.h). It will be used as the representation for a list of transactions; more specifically, it will process a file of unsorted transaction records and produce a sorted transaction history with computed balances. For example, given contents in test.tfile, it will produce the following:


+-----------------+--------------------------+----------------+----------------+
|       Date      | Description              |         Amount |        Balance |
+-----------------+--------------------------+----------------+----------------+
| Thu Aug 21 2008 | Initial deposit          |      1,723.00  |      1,723.00  |
| Wed Dec 31 2008 | Phone bill               | (       45.33) |      1,677.67  |
| Mon Jul 13 2009 | Stipend                  |     10,388.07  |     12,065.74  |
| Mon Aug  3 2009 | Rent                     | (    2,800.00) |      9,265.74  |
| Sun Jan 10 2010 | Car monthly payment      | (      654.32) |      8,611.42  |
+-----------------+--------------------------+----------------+----------------+


- To compile the program, use the command: make warmup1
- To run the program, use the command: ./warmup1 sort test.tfile
