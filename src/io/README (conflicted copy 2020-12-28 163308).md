# Storage
The storage module shall provide IO methods, the concepts of a file, a page and a disk manager to manage and keep references to the afore mentioned.  
Further it shall provide a basic one page buffer manager (with a full implementation that is crippled to meet our needs), i.e. implement the notions of a frame and the cache/buffer manager itself.  
Finally the layout of the records and the structs to keep them in memory shall be described. Read and write operations shall be specified by these structs and contained as function pointers. The records must follow the index free adjacency list schema used in Neo4J.  

- IO
	+ [ ] Page
	+ [ ] File
	+ [ ] DiskManager
- Cache
	+ [ ] Frame
	+ [ ] CacheManager

- Records
	+ [ ] Node
	+ [ ] Relationship
	+ [ ] A dynamic storage scheme for variable length fields
	+ [ ] Property (Header, blocks, keys)
	+ [ ] node type
	+ [ ] relationship type
ŝ