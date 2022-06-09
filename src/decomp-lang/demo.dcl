.DATA
	counter 0
	pointer 0
	n 0

.MAIN
	# inputing initial values
	input n
	input pointer

	# filling decomp memory with trash values
	while [ sub n counter ] nz and ns do
		mov &pointer pointer

		add pointer 1
		mov pointer acm

		add counter 1
		mov counter acm
	end