# Threaded-Resolver
PA6 for CSCI 3753 Operating Systems

This multi-threaded program resolves hostnames from input files using producer-consumer principles. Requester threads add hostnames to a shared buffer, while resolver threads perform DNS lookups and write results to an output file. It uses semaphores and mutexes for thread safety and provides error handling and performance stats.
