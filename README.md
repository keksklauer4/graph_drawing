# graph_drawing
Our approaches to some algorithmic graph visualization project (experimental, obviously...)

## How to build / run
```
0. clone into this repository recursively!
i. e. git clone --recursive <url>

1. Install git cmake g++ make curl

2. Obtain Lemon by running (obtains the zip and unzips it)
bash prepare.sh

3. Build
mkdir build
cd build/
cmake ..
make

4. Run test
./gd_test

There is another executable gd that currently does nothing but printing hello world and a static library

For faster build times you should also install ninja-build and once you are in the build/ folder call
cmake .. -GNinja
ninja
```

## Libraries / Other software used (+ licences)
- todo
