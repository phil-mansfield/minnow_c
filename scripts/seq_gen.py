from __future__ import print_function
import fileinput
import sys

"""
Author: Phil Mansfield (mansfield@uchicago.edu)

This script is designed to convert a base_seq.h/base_seq.c file containing
example statements around a type 'ExSeq' which wraps the element type 'Example'
into a file which contains both this example and a macro for creating more
declaration instances.

Usage examples:
    $ python seq_gen.py h < ../resources/seq_base.h > ../src/seq_base.h
    $ python seq_gen.py c < ../src/seq_base.c > ../src/seq_base.c

(Note that you can't pipe back into the source files because it will just
erase them. You also can't append because the macro lock won't work correctly.)

It is very hacky. You should read this and understand how it works before
modifying base_seq.h. There are valid C programs which will break this script.
"""

autogen_line = ("/* Autogenerated code below this point " +
                "(including this comment). */")

def main():

    if len(sys.argv) != 2 or sys.argv[1] not in ["h", "c"]:
        print("""Usage examples:
    $ python seq_gen.py h < ../resources/seq_base.h > ../src/seq_base.h
    $ python seq_gen.py c < ../resources/seq_base.c > ../src/seq_base/c""")
        exit(1)

    mode = sys.argv[1]

    saved_lines = []

    lines = sys.stdin.read().split("\n")
    for line in lines:
        print(line)

        if save_line(line): saved_lines.append(line)
        if line == autogen_line: break

    print()
    if mode == "h":
        print("#define GENERATE_SEQ_HEADER(type, seqType) \\")
    elif mode == "c":
        print("#define GENERATE_SEQ_BODY(type, seqType) \\")

    for i, line in enumerate(saved_lines):
        if i == len(saved_lines) - 1:
            print("   ", convert_to_macro(line))
        else:
            print("   ", convert_to_macro(line), "\\")

    print()
    if mode == "h":
        print("#endif /* MNW_BASE_SEQ_H_ */")
        
def save_line(line):
    s_line = line.strip()
    return (len(s_line) > 0 and s_line[0] not in ["/", "#", "."] and 
            s_line != "typedef double Example;")
    
def convert_to_macro(line):
    if '"' in line:
        ExSeq_replace = '"#seqType"_'
    else:
        ExSeq_replace = "seqType##_"

    line = line.replace("ExSeq_", ExSeq_replace)
    line = line.replace("ExSeq", "seqType")
    line = line.replace("Example", "type")
    
    return line

if __name__ == "__main__": main()
