# Cache-Memory-Hierarchy-Design
This project deals with building a C++ based cache hierarchy simulator. This project is a part of the course ECE 563 at NC State University. A generic cache class is built based on OOP, which is constructed into several hierarchy levels based on the input arguments while running the simulator. 

For all levels of hierarchy, Write Back Write Allocate (WBWA) policy is implemented, with a LRU replacement policy. The simulator is tested for various configuratiions of cache parameters such as size, associativty and block size, and the trends in performance were observed using the CACTI data.
In this project, L1 and L2 level caches were explored with the inclusion of Victim cache. Further, the code could be modified without much effort if further levels of hierarchy need to be added. 
