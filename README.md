# graph_drawing
Our approaches to some algorithmic graph visualization project (experimental, obviously...)

## How to build / run
```
0. clone into this repository recursively!
i. e. git clone --recursive <url>

1. Install git cmake g++ make curl
Also install python3, pip3 (i. e. the package python3-pip) and install
the library networkx using pip (i. e. pip3 install networkx)

2. Obtain Lemon, CGAL and Eigen by running (obtains the zip and unzips it)
bash prepare.sh
sudo apt-get install libcgal-dev
sudo apt install libeigen3-dev

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

## Libraries / Other software used (+ licenses)
#### [GoogleTest](https://github.com/google/googletest) (License: [BSD 3-Clause "New" or "Revised"](https://choosealicense.com/licenses/bsd-3-clause/))
#### [LEMON](https://lemon.cs.elte.hu/trac/lemon) (License: [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/))
#### [rapidjson](https://github.com/Tencent/rapidjson) (License: [MIT](https://choosealicense.com/licenses/mit/))
#### [CLI11](https://github.com/CLIUtils/CLI11) (License: [BSD 3-Clause "New" or "Revised"](https://choosealicense.com/licenses/bsd-3-clause/))
#### [CGAL](https://www.cgal.org/) (License: [GPL/LGPL3.0](https://www.cgal.org/license.html))
#### [Eigen](https://eigen.tuxfamily.org/) (License: [MPL2](https://www.mozilla.org/en-US/MPL/2.0/), disabled LGPL3+ code)
#### [KaHIP](https://github.com/KaHIP/KaHIP) (License: [MIT](https://choosealicense.com/licenses/mit/))
#### [AABBCC](https://github.com/lohedges/aabbcc) (we use [this fork](https://github.com/keksklauer4/aabbcc)) (License: [zlib](http://zlib.net/zlib_license.html))
#### [hclust](https://github.com/cdalitz/hclust-cpp) (we use [this fork](https://github.com/keksklauer4/hclust-cpp)) (License: [individual](https://github.com/cdalitz/hclust-cpp/blob/master/LICENSE))

//Todo
