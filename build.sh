git submodule init
git submodule update

# download GNUGo
pushd modules
if [[ ! -e ./gnugo-3.8.tar.gz ]]; then
  wget http://ftp.gnu.org/gnu/gnugo/gnugo-3.8.tar.gz
  tar -xzvf gnugo-3.8.tar.gz

  pushd gnugo-3.8
  ./configure --build=i686-pc-linux-gnu "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"
  make -j4
  popd

  cp gnugo-3.8/interface/gnugo .
fi

popd

make clean
make -j4
