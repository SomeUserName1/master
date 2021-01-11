# Data Types
This directory shall provide all neccessary data types in a generic fashion and wrappers to restore type safety for the specialistations used.  
The following generic data structures and according methods shall be provided:  

- [x] Hashtable/dict/map  
- [x] list (array-based)  
- [x] Queue (as doubly linked list)  

Extenstions are to add sortedness to the list (using a flag to indicate) and to make the array-based list a mixture of linked lists and arrays to avoid large reallocations.

Specialistations are:  

- Hashtable:  
	- [x] ID, ID  
	- [x] ID, Ineger   
    - [x] ID, Node
    - [x] ID, Relationship
	- [x] Page, Frame  
- List:  
    - [x] ID
	- [x] Relationship  
	- [x] Node  
- Queue:  
	- [x] Node
