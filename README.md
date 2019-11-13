# Operating Systems Assignment Five

## Build 
`make clean`

`make oss`

`make user`

## Run 
`oss [-h] [-t -l -m]`

    For:
        -h: Display help and exit
        -t: Amount of time to allow the program to run for
        -l: Log file path
        -m: Maximum time between requests

## Notes

This is still an incomplete project. Here's a list of known bugs, and 
missing features.

- The banker's algorithm is both corrupt, and slow. 
I made a mistake in attempt to simplify the code. Instead
of using a queue, I tried to use a request matrix; however,
this makes it impossible to seperate out multiple requests, and 
OSS has to iterate over the whole matrix to try and grant requests.
An alternative solution would be to flag which requests we've 
attempted to grant, and limit request per tick of the clock.
In reality, the appropriate solution is just to use a queue.

- There's either an error in the asynchronous part of the request 
structures, or the banker's algorithm itself. It's possible to
see the negative values for allocations, etc. 

- Tied to the last issue, if you use 18 processes, the program won't
see process turn over much, because the algorithm takes so long. 
Thusly, getting ticks is problematic. This could likely be solved 
in simple by playing with the time the clock is incremented.

- Something is causing the children to attempt to access the shared
semaphores after they have been removed, if an interrupt is 
generated. I think it's tied to the way I'm killing the processes.
Fortunately, the program still cleans up the children and the shared
memory. You will see a message
`resource: fail to get semlock: ... ` this is caused when the
children attempt to access the resource descriptors after they have
been released.

- The verbose option isn't implemented.

- No resources are shared. 
