# Network-Programming
Base64 encode/decode service

Usage:
1. Compile client & server:  

	``  
    gcc -o client client.c  

    gcc -o server server.c  

    ``
2. Run server:  

	``  
    ./server PORT_NUMBER  

    (PORT_NUMBER is optional)  

    ``
3. Run client:  

	``  
    ./client 127.0.0.1 PORT_NUMBER  

    (PORT_NUMBER is optional, if use PORT_NUMBER, it's must be same as PORT_NUMBER of server)  
    
    ``
4. Do what ever client's terminal ask
5. Wait for the result
6. You done!
