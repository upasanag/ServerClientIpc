# ServerClientIpc
Server-Client IPC

**1. Source Code:**
- Files:
     - server.cpp
     - client.cpp
     - config.h
     - Makefile
     
**2. Makefile:**
   - Provided
 
**3. Build Instructions:**
   - make all: Compile all
   - make server: Compile server
   - make client: Compile client
   - make clean: cleans all
 
**4. Run instructions:**
   - Start Server: ./server
   - Start Client: ./client
   
**5. Data Structure Reasoning:**
   For storing the numbers and timestamp, std::map<int, time_t> is taken as primary store. Thresons are as follows:
   - Insertion and duplicate handling: std::map is built using a Red-Black Tree, which is a type of self-balancing binary search tree. The insertion and search is done in O(logn) time. 
   - print all numbers: Since the std::map is built using a Red-Black Tree, ensures that numbers are always stored in a sorted fashion. This makes the "Print all" requirement extremely efficient without needing a separate sort step.
   - Deletion handling: Removing the key from std::map takes O(log n) time.
