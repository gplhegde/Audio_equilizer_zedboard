* Use following compiler flags for compiling
	-Wall -O3 -c -fmessage-length=0 -ffast-math  -mfpu=neon
	Debug : none
* The noise learning time is 3s. This can be changed using macro "NOISE_LEARNING_TIME". The user is expected to remain silent during this interval.
This will help to avoid false noise estimation.
