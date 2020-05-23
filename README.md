# WARNING: THIS PROJECT WAS POORLY IMPLEMENTATED AND NOT SECURE. PLEASE DON'T USE IT
# Filesec

Filesec is a program that will encrypt/decrypt files using the OPENSSL library.

## Makefile

Use the make file to perform different actions

```bash
make
make clean
make tests
```

## Sources

    ├── build
         ├── *.o
    ├── include
        ├── crypt.h
        ├── debug.h
        ├── filesec.h
        ├── getpass.h
        ├── global.h
        ├── io.h
        ├── version.h
    ├── src
        ├── crypt.c
        ├── filesec.c
        ├── getpass.c
        ├── io.c
        ├── main.c
    Makefile

## Usage

```C
filesec [-devh] [-D DBGVAL] [-p PASSFILE] infile outfile

-e                  encrypt the infile to outfile
-d                  decrypt the infile to outfile
-v                  show version
-h                  show usage
-D  DBGVAL          give a value to control the verbosity of the program. 
                    Debug value 0x00 (0d): no debug info printed

                    - Debug value 0x01 (1d): print a message on immediate entry and right before
                    exit to every filesec function.

                    - Debug value 0x02 (2d): print right before and right after calling any
                    library call.

                    - Debug value 0x04 (4d): print right before and right after calling any
                    system call.

                    - Debug value 0x10 (16d): print arguments to function calls

                    - Debug value 0x20 (32d): print return values from functions calls

-p PASSFILE         give a file to input the password. This will only read the first line.
```
if infile is '-', data will be read from STDIN
if outfile is '-', data will be written to STDOUT

## Basic information
filesec uses SHA256 has the hashing algorithm for the key. filesec also uses ARIA encryption with CTR mode.

filesec will ask for a password, hash it, and then proceed to encrypt/decrypt the data.

This programs approachs file encryption by reading in from the infile, writing it to a tmpfile, and finally transfering it to the outfile.
Within the crypting functions, filesec will read a bucket of data(PAGESIZE), encrypt/decrypt it, and write that bucket of data. This ensures that the memory usage of the program does not overflood the system.  

filesec will handle all failures and clean up all file descriptors and allocated memory.
## Resources

Functions and code in this program were inspired by these references.

/*https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption*/
/*https://www.gnu.org/software/libc/manual/html_node/getpass.html*/
/*https://wiki.openssl.org/index.php/EVP_Message_Digests*/

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[MIT](https://choosealicense.com/licenses/mit/)
