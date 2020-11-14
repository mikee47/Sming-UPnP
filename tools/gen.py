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
    parser.add_argument('-i', '--input', metavar='filename', required=True, help='Source .xml')
    parser.add_argument('-o', '--output', metavar='filename', required=True, help='Destination .h')
    args = parser.parse_args()

    original_tree = etree.parse(args.input)
    xslt_tree = etree.parse(args.transform)
    xslt = etree.XSLT(xslt_tree)
    doc_root = "'{}/config'".format(os.path.relpath(".", os.path.dirname(args.transform)))
    doc_root = doc_root.replace("\\", "/")
    lom_tree = xslt(original_tree, DOC_ROOT=doc_root)

    outfile = open(args.output, 'w')
    outfile.write(str(lom_tree))
