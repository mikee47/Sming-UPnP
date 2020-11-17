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
    parser.add_argument('-s', '--source', metavar='directory', required=True, help='Path to root source directory')
    parser.add_argument('-i', '--input', metavar='filename', required=True, help='Relative path to device description .xml')
    parser.add_argument('-o', '--output', metavar='filename', required=True, help='Output filename')
    args = parser.parse_args()

    srcfile = os.path.join(args.source, args.input)
    srcfile = srcfile.replace("\\", "/")
    original_tree = etree.parse(srcfile)
    xslt_tree = etree.parse(args.transform)
    xslt = etree.XSLT(xslt_tree)

    doc_root = "'{}'".format(os.path.relpath(args.source, os.path.dirname(args.transform)))
    doc_root = doc_root.replace("\\", "/")

    dstfile = os.path.relpath(os.path.splitext(args.output)[0], os.path.dirname(args.transform))
    
    print("doc_root = '{}'".format(doc_root))
    print("dstfle = '{}'".format(dstfile))

    os.makedirs(os.path.dirname(dstfile), exist_ok=True)

    lom_tree = xslt(original_tree, DOC_ROOT=doc_root, MODE='h')
    with open(dstfile + '.h', 'w') as f:
        f.write(str(lom_tree))

    lom_tree = xslt(original_tree, DOC_ROOT=doc_root, MODE='cpp')
    with open(dstfile + '.cpp', 'w') as f:
        f.write(str(lom_tree))
