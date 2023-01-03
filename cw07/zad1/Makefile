COOKS=4
DELIVERIES=4
arg=-DDELIVERIES="$(DELIVERIES)" -DCOOKS="$(COOKS)"

sysv_7: cook delivery
	gcc -Wall -o main sysv_7.c $(arg)

cook:
	gcc -Wall -o cook cook.c $(arg)

delivery:
	gcc -Wall -o delivery delivery.c $(arg)

clean:
	rm main cook delivery
