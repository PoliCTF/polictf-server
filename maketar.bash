set -ex

mkdir -p ./build/polictf
cp ./build/register/registration ./build/polictf
cp ./build/scoreboard/scoreboard ./build/polictf
cp /usr/local/lib/libbooster.so.0 ./build/polictf
cp /usr/local/lib/libcppcms.so.1 ./build/polictf

echo "LD_LIBRARY_PATH=. ./scoreboard -c scoreboard.js" > ./build/polictf/run.bash

rm ./build/polictf-server.tar.gz || true
tar -czvf ./build/polictf-server.tar.gz -C ./build/ polictf