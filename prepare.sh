
cd external/
curl http://lemon.cs.elte.hu/pub/sources/lemon-1.3.1.tar.gz --output lemon.tar.gz
tar -xvzf lemon.tar.gz
mv lemon-1.3.1/ lemon
cd ..


cd test
mkdir gdata
cd gdata
mkdir rome
mkdir temp
cd temp
curl http://www.graphdrawing.org/download/rome-graphml.tgz --output rome_graphml.tgz
tar -xvzf rome_graphml.tgz

cd ../../..
python3 prepare_graphs.py
