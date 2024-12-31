# Re-create database file
cd db_file
rm data.dbf
touch data.dbf

cd ..

# Compile
mkdir build && cd build
cmake .. && make clean && make -j
cd ..

# test LRU algorithm
./build/buffersim LRU ./db_file/data.dbf ./data/data-5w-50w-zipf.txt 10000

# test CLOCK algorithm
./build/buffersim CLOCK ./db_file/data.dbf ./data/data-5w-50w-zipf.txt 10000
