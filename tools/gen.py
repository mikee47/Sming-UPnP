#!/usr/bin/env python3
#
# Script to generate device class units
#

from lxml import etree
import os
import argparse

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='UPnP Code Generator')
    parser.add_argument('-t', '--transform', metavar='filename', required=True, help='XSLT transform file')
    parser.add_argument('-b', '--base', metavar='filename', required=True, help='Base source directory')
    parser.add_argument('-i', '--input', metavar='filename', required=True, help='Relative path to device description .xml')
    parser.add_argument('-o', '--output', metavar='filename', required=True, help='Destination .h')
    args = parser.parse_args()

    src = os.path.join(args.base, args.input)
    src = src.replace("\\", "/")
    original_tree = etree.parse(src)
    xslt_tree = etree.parse(args.transform)
    xslt = etree.XSLT(xslt_tree)
    doc_root = "'{}'".format(os.path.relpath(args.base, os.path.dirname(args.transform)))
    doc_root = doc_root.replace("\\", "/")
    lom_tree = xslt(original_tree, DOC_ROOT=doc_root)

    outfile = open(args.output, 'w')
    outfile.write(str(lom_tree))
