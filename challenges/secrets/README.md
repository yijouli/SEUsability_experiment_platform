# Secrets

## Challenge description 

This is a password manager. 
How do you decrypt the content of the `flag` file created by the `admin` user? 
Assume that you do not have direct access to the file system, but you can only interact with it through this command-line software. 


## Build & Run

```
mkdir -p cmake-build-release
cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
./secrets.out
```