game=("tictactoe" "reversi")
for g in ${game[@]}
do
   g++ -O3 -march=native -shared -fPIC -std=c++11 -DNDEBUG -I./modules/pybind11/include/ -DPY $(python3-config --cflags --ldflags) cpp/${g}.cpp -o ${g}.so
done