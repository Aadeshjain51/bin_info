# bin_info
Binary executable file header information viewer written in C++
It uses Binary File Descriptor library (libbfd) for parsing the executable and Capstone (libcaptone) for disassembly.

## Installation
```bash
foo@bar:~$ git clone https://github.com/ashtrace/bin_info.git
foo@bar:~$ cd bin_info
foo@bar:~$ make
foo@bar:~$ ./bin_info -f <binary_file> -x # examine the header
foo@bar:~$ ./bin_info -f <binary_file> -l # perform linear disassembly
```
## Output
### Section Header
<p align="center">
  <kbd>
    <img src="./images/section_header.png" width="506px" height="218px" align="center">
  </kbd>
</p>



### Symbol Table
<p align="center">
  <kbd>
    <img src="./images/symbol_table.png" width="506px" height="218px">
  </kbd>
<p>

### Linear Disassembly
<p align="center">
  <kbd>
    <img src="./images/linear_disasm.png" width="506px" height="218px">
  </kbd>
<p>
