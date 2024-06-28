

# Concurrency
## Cafe sim
### How to run :
``` bash
gcc 1.c  
./a.out
```

### Implementation :
The program has ( n + b + 1 ) threads where `n` thread are for the `customers` and `b` threads are for `Baristas`. There are ( n + b ) semaphores also used for the locking specific sections of the above threads . Multiple locks are also used for locking in common sections of the `timerthread`, `timerthread` is used to print the information of arrival and exit of customers.

#### Code explaination
after taking the input all the threads are created. The customer thread are kept in wait till any barista chooses to serve the customer using `sem_wait(&sema[customernumber])`. The Barista thread first checks if any Barista with lower number if free then it hands overs the control the smaller thread (for implementing the ordering required in the question), if any  smaller thread is not found then it searches among all the customers whose orders are not still taken(using a hash array named `arrival`, whenever a order is taken its arrival time is set to be infinite so it will never be chosen again), after choosing the customer it stores the information that the chosen customer is serviced by this barista for further use. The barista thread then wakes up the customer thread using `sem_post(&sema[customernumber])`, After this Barista thread goes to wait using `sem_wait(&sema2[Baristanumber])`. In the customer thread  we first check the condition that is there sufficient time left for customer to take the order or not and make the conditions accordingly. After then the barista is again woken up to choose for another customer using `sem_post(&sema2[Baristanumber])` and the Barista starts searching for another customer. After all the customers are serviced or left due to tolerance, the program ends and prints the number of coffee wasted and avg. wait time .

All the semaphores used in the code are intialised to 0.

### Assumptions
If a order is completed a `t(s)` and the customer also has to leave at `t(s)` then it will not take the order and will leave without taking the order.


### Questions
1.Waiting Time: Calculate the average time a customer spends waiting for their coffee. Determine if this is more than (and how much) if the cafe had infinite baristas. Coffee prep time is not part of waiting time.

ANSWER : The avg time taken for each customer is calculated in the code(check for the arrival time and the time at which a barista chooses it, if no barista chooses it then whole tolerance time is taken as the waiting time). If there are infinite number( >=numberofcustomers ) of Baristas then the waiting time will be strictly `0` as all customers will be served just upon arrival.


2.Coffee Wastage: Determine how many coffees are wasted. Print the number of coffees wasted after the simulation ends.

ANSWER: The total number of coffees wasted in total are calculated in the code stored in the variable named `wasted coffee`. Whenever a Barista chooses a customer that can not be served completely due to tolerance time the variable is incremented.After the end of the program the total number of coffees wasted is printed .

### Conclusion

These metrics provide insights into the efficiency and customer satisfaction of the cafe. A lower average waiting time and minimal coffee wastage are generally indicative of a well-organized and customer-friendly service.

## Ice cream parlour

### How to run :
```bash
gcc 2.c  
./a.out
```

### Implementation
The program has (n + numorders + 2) threads where `n` is number of ice cream machines and `numorders` is the total number of orders given by the customers.There are 2 other thread named `printerthread` and `timerthread`, `timerthread` is used to check the availibilty of toppings when the customer arrives and if the toppings are not sufficient then return the customer at the same time.`printerthread` is used to print the arrival and exits of customers and also the arrival and exits of machines, it also prints the customer exits due to lack of ingredients or lack of machine availibilty.

### Code explaination
At the start of code all the n + numorders + 2 threads are made. All the order threads are kept in sleep till the arrival time of the customer they belong to, then they are kept in wait using `sem_wait(&sema[ordernumber])` in a semaphore till a machine picks the order.
The machine thread sleeps till the starting time of the machine and then it starts searching for a eligible order which it can complete in its left time. To make the ordering work if machine picks a order lets say with ordernumber `x` then I search for all the machines who are eligible to do this order having machine number less than the current machine number and give this order to that machine, if no machine exists like this then I check for then this machine thread wakes up the order using `sem_post(&sema[machinenumber])` and after this it goes to wait using `sem_wait(&sema2[machinenumber])`, In the order thread it goes to sleep for `timetoprepare` and prints the stuff accordingly, after this it wakes the machine back using `sem_post(&sema2[machinenumber])` after this it again starts for searches for another order till the time of exit.
The code checks for capacity in the printerthread at the time of arrival and then if capacity > k then the customer is rejected at the same time.
The code terminates when all the machines are closed and prints parlour closed.

### Questions

1.Minimizing Incomplete Orders: Describe your approach to redesign the simulation to minimize incomplete orders, given that incomplete orders can impact the parlor’s reputation. Note that the parlor’s reputation is unaffected if orders are instantly rejected due to topping shortages.


ANSWER: Since the parlour's reputation is unaffected if the orders are rejected instantly we ca use the following approach :-
We begin with checking the ingrediants availabilty of all orders of the customer with the smallest arrival time. If ingrediants are unavailable for even a single order we can reject that customer. If all the ingrediants are available then we will check if machine will be available for the customer's order to be prepared because we are bookeeping the machine status for the future as well. If machines won't be available we reject the customer else we accept the orders and start preparing them after updating the bookeeping we were doing.

2.Ingredient Replenishment: Assuming ingredients can be replenished by contacting the nearest supplier, outline how you would adjust the parlour order acceptance/rejection process based on ingredient availability.

ANSWER: Since ingrediants can be replenished we can say that we will have abundance of ingrediants at any point fo time. Hence our task is just to check if the machines are available for order prepartion.This we can do in the same way as mentioned in answer 1.

3.Unserviced Orders: Suggest ways by which we can avoid/minimise the number of unserviced orders or customers having to wait until the parlor closes.

 ANSWER: We can minimise the number of unserviced order by using a similar approach as mentioned in answer 1 where we are doing a book keeping of all the ingrediants and machines.

 


