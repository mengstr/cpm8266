#!/bin/bash

echo -e -n '\xE5\xE5\xE5\xE5' > f.4
cat f.4 f.4 f.4 f.4 > f.16
cat f.16 f.16 f.16 f.16 > f.64
cat f.64 f.64 f.64 f.64 > f.256
cat f.256  f.256  f.256 f.256 > f.1k
cat f.1k f.1k f.1k f.1k f.1k f.1k  f.1k f.1k > f.8k
cat f.8k f.8k  f.8k f.8k  f.8k f.8k f.8k f.8k > f.64k
cat f.64k f.64k f.64k f.8k f.8k f.8k f.8k f.8k f.8k f.8k f.1k f.1k f.256 > FORMATTED.DSK
rm f.*
 
