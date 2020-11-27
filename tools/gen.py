#!/usr/bin/env python3
#
# Script to generate device class units
#

from lxml import etree
import os
import argparse

def quote(s):
    if s[0].isdigit():
        s = '_' + s;
    return '"' + s + '"'

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='UPnP Code Generator')
    parser.add_argument('-t', '--transform', metavar='filename', required=True, help='XSLT transform file')
    parser.add_argument('-i', '--input', metavar='filename', required=True, help='Device description .xml')
    parser.add_argument('-o', '--output', metavar='filename', required=True, help='Output filename')
    args = parser.parse_args()

    srcfile = args.input.replace("\\", "/")
    dstfile = args.output.replace("\\", "/")
    original_tree = etree.parse(srcfile)
    xslt_tree = etree.parse(args.transform)
    xslt = etree.XSLT(xslt_tree)

    os.makedirs(os.path.dirname(dstfile), exist_ok=True)

    lom_tree = xslt(original_tree)
    with open(dstfile, 'w') as f:
        f.write(str(lom_tree))
