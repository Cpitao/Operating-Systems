COOKS=4
DELIVERIES=4
arg=-DDELIVERIES="$(DELIVERIES)" -DCOOKS="$(COOKS)"

posix: cook delivery
	gcc -o main posix.c -lrt -lpthread $(arg)

cook:
	gcc -Wall -o cook cook.c -lrt -lpthread $(arg)

delivery:
	gcc -Wall -o delivery delivery.c -lrt -lpthread $(arg)

clean:
	rm cook delivery main
