# ComputerNetworkingProgram3
This assignment looks at behavior of the TCP protocol through a number of experiments. The Professor provided hw3 executable software is utilized as a basis for drawing a TCP state transition diagram as well as a corresponding timing chart. A custom version of the network traffic generator program “ttcp” is also used to determine how throughput is affected by setting a number of different parameters while in use, including message size, number of messages transferred, socket buffer size, and use of the Nagle’s algorithm. Throughout testing, the Linux commands tcpdump, netstat, and strace are used to observe how TCP segments are transmitted.