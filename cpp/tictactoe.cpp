#include "util.hpp"
#include "boardgame.hpp"

using namespace std;

const string X = "ABC";
const string Y = "123";
const string C = ".OX";

struct State
{
    const int L_ = 3;
    vector<int> board_;
    int color_;
    int win_color_;
    vector<int> record_;

    State()
    {
        board_.resize(L_ * L_);
        clear();
    }

    State(const State& s):
    color_(s.color_),
    win_color_(s.win_color_),
    record_(s.record_) {}

    array<int, 2> size() const
    {
        return {L_, L_};
    }

    void clear()
    {
        fill(board_.begin(), board_.end(), EMPTY);
        color_ = BLACK;
        win_color_ = EMPTY;
        record_.clear();
    }

    string action2str(int action) const
    {
        ostringstream oss;
        oss << X[action2x(action)] << Y[action2y(action)];
        return oss.str();
    }

    int str2action(const string& s) const
    {
        return xy2action(find(X, s[0]), find(Y, s[1]));
    }

    string record_string() const
    {
        vector<string> ss;
        for (int action : record_) {
            ss.push_back(action2str(action));
        }
        return join(ss, " ");
    }

    string to_string() const
    {
        ostringstream oss;
        oss << "  ";
        for (int y = 0; y < L_; y++) oss << Y[y];
        oss << endl;
        for (int x = 0; x < L_; x++) {
            oss << X[x] << " ";
            for (int y = 0; y < L_; y++) {
                oss << C[board_[xy2action(x, y)]];
            }
            oss << endl;
        }
        oss << "record = " << record_string() << endl;
        return oss.str();
    }

    void play(const int action)
    {
        // 行動で状態を進める関数
        // action は board 上の位置 (0 ~ 8) または行動系列の文字列
        board_[action] = color_;
        int ax = action2x(action), ay = action2y(action);

        // 3つ揃ったか調べる
        int xsum = 0, ysum = 0;
        for (int y = 0; y < L_; y++) xsum += board_[xy2action(ax, y)];
        for (int x = 0; x < L_; x++) ysum += board_[xy2action(x, ay)];
        if (xsum == color_ * L_ || ysum == color_ * L_) win_color_ = color_; 
        
        if (ax == ay) {
            int diagsum = 0;
            for (int x = 0; x < L_; x++) diagsum += board_[xy2action(x, x)];
            if (diagsum == color_ * L_) win_color_ = color_;
        }
        if (ax == 2 - ay) {
            int diagsum = 0;
            for (int x = 0; x < L_; x++) diagsum += board_[xy2action(x, L_ - 1 - x)];
            if (diagsum == color_ * L_) win_color_ = color_;
        }

        color_ = opponent(color_);
        record_.push_back(action);
    }

    void plays(const string& s)
    {
        vector<string> ss = split(s, ' ');
        for (const string& s : ss) play(str2action(s));
    }

    bool terminal() const
    {
        // 終端状態かどうか返す
        return win_color_ != 0 || int(record_.size()) == L_ * L_;
    }

    double reward(bool subjective = true) const
    {
        // 終端状態での勝敗による報酬を返す
        if (win_color_ == 0) return 0;
        int r = win_color_ == 1 ? 1 : -1;
        return (subjective && color_ != 1) ? -r : r;
    }

    bool legal(int action) const
    {
        return action >= 0 && action < L_ * L_ && board_[action] == 0;
    }

    vector<int> legal_actions() const
    {
        // 可能な行動リストを返す
        vector<int> actions;
        for (int i = 0; i < L_ * L_; i++) {
            if (legal(i)) actions.push_back(i);
        }
        return actions;
    }

    int action_length() const
    {
        return L_ * L_;
    }

    vector<float> feature() const
    {
        vector<float> f(2 * L_ * L_, 0.0f);
        for (int x = 0; x < L_; x++) {
            for (int y = 0; y < L_; y++)  {
                int a = xy2action(x, y);
                if      (board_[a] == color_)           f[a] = 1;
                else if (board_[a] == opponent(color_)) f[L_ * L_ + a] = 1;
            }
        }
        return f;
    }

    int action2x(int action) const
    {
        return action / L_;
    }

    int action2y(int action) const
    {
        return action % L_;
    }

    int xy2action(int x, int y) const
    {
        return x * L_ + y;
    }
};

#ifdef PY

using PyState = PythonState<State>;

namespace py = pybind11;
PYBIND11_MODULE(tictactoe, m)
{
    m.doc() = "implementation of game";

    py::class_<PyState>(m, "State")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState::action2str, "action index to string")
    .def("str2action",    &PyState::str2action, "string to action index")
    .def("__str__",       &PyState::to_string, "string output")
    .def("copy",          &PyState::copy, "deep copy")
    .def("clear",         &PyState::clear, "initialize state")
    .def("legal_actions", &PyState::legal_actions, "legal actions")
    .def("action_length", &PyState::action_length, "the number of legal action labels")
    .def("play",          &PyState::play, "state transition")
    .def("plays",         &PyState::plays, "sequential state transition")
    .def("terminal",      &PyState::terminal, "whether terminal state or not")
    .def("reward",        &PyState::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState::feature, "input feature");
};

#else

// build executable

int main()
{
    for (int i = 0; i < 100; i++) {
        State state;
        while (!state.terminal()) {
            auto actions = state.legal_actions();
            state.play(actions[rand() % actions.size()]);
        }
        cerr << state << endl;
    }
}

#endif