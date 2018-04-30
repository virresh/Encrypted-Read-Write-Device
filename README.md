# Encrypted Read/Write Modules  

### Encryption scheme  
It uses a block encryption scheme. Input is divided into chunks of 
128 bytes.  
The first block is encrypted via a PRE SHARED KEY. The subsequent blocks
are encrypted via the block just before it.

Any block with less than 128 bits is padded with spaces and the 
decrypted output has those spaces.

### Usage
Clone this repository and then simply use the script provided.  
```
./run.sh
```
Only Ubuntu supported. Must to have latest version of gcc installed.

### Authors  
Viresh Gupta
