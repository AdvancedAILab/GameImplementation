#include "util.hpp"
#include "boardgame.hpp"

using namespace std;

const string X = "ABCDEFGH";
const string Y = "12345678";
const string C = "XO.";

namespace Reversi
{
    struct State
    {
        int L_ = 6;
        vector<int> board_;
        int color_;
        array<int, 2> score_;
        vector<int> record_;

        State()
        {
            board_.resize(L_ * L_);
            clear();
        }

        State(const State& s):
        L_(s.L_),
        board_(s.board_),
        color_(s.color_),
        score_(s.score_),
        record_(s.record_) {}

        array<int, 2> size() const
        {
            return {L_, L_};
        }

        void clear()
        {
            fill(board_.begin(), board_.end(), EMPTY);
            int mid = (L_ - 1) / 2;
            board_[xy2action(mid, mid)] = WHITE;
            board_[xy2action(mid, mid + 1)] = BLACK;
            board_[xy2action(mid + 1, mid)] = BLACK;
            board_[xy2action(mid + 1, mid + 1)] = WHITE;
            color_ = BLACK;
            score_.fill(2);
            record_.clear();
        }

        string action2str(int action) const
        {
            ostringstream oss;
            oss << X[action / L_] << Y[action % L_];
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
            oss << score_[0] << " - " << score_[1] << endl; 
            oss << "record = " << record_string() << endl;
            return oss.str();
        }

        void play(const int& action)
        {
            // 行動で状態を進める関数
            // action は board 上の位置
            board_[action] = color_;
            int flipped = flip_stones(action);
            score_[color_] += flipped;
            score_[color_] -= flipped;
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
            if (score_[0] == 0 || score_[1] == 0
                || score_[0] + score_[1] == L_ * L_) return true;
            // if there is no putting actions for both players, it's terninal Reversi
            for (int i = 0; i < L_ * L_; i++) {
                if (flipped_positions(i, color_).size()           > 0) return false;
                if (flipped_positions(i, opponent(color_)).size() > 0) return false;
            }
            return true;
        }

        float reward(bool subjective = true) const
        {
            // 終端状態での勝敗による報酬を返す
            int sc = score(subjective);
            if (sc > 0) return 1;
            else if (sc < 0) return -1;
            return 0;
        }

        bool legal(int action) const
        {
            if (action < 0 || action >= L_ * L_) return false;
            if (board_[action] != EMPTY) return false;
            if (flipped_positions(action).size() == 0) return false;
            return true;
        }

        vector<int> legal_actions() const
        {
            // 可能な行動リストを返す
            vector<int> actions;
            for (int i = 0; i < L_ * L_; i++) {
                if (legal(i)) actions.push_back(i);
            }
            if (actions.size() == 0) {
                actions.push_back(L_ * L_); // pass
            }
            return actions;
        }

        int action_length() const
        {
            return L_ * L_ + 1;
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

        int score(bool subjective = true) const
        {
            int diff = score_[0] - score_[1];
            return (subjective && color_ != 1) ? -diff : diff;
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

        vector<int> flipped_positions(int action, int color = EMPTY) const
        {
            vector<int> flipped;
            if (color == EMPTY) color = color_;
            if (!onboard(action, L_)) return flipped;
            if (board_[action] != EMPTY) return flipped;
            for (int d = 0; d < 4; d++) {
                int x = action2x(action);
                int y = action2y(action);
                vector<int> sandwitched;
                while (1) {
                    x += D2[d][0];
                    y += D2[d][1];
                    int pos = xy2action(x, y);
                    if (!onboard(x, y, L_)
                        || (board_[pos] != color && board_[pos] != opponent(color))) {
                        sandwitched.clear(); // no stones will be sandwitched
                        break;
                    }
                    if (board_[pos] == color) break;
                    sandwitched.push_back(pos);
                }
                flipped.insert(flipped.end(), sandwitched.begin(), sandwitched.end());
            }
            return flipped;
        }

        int flip_stones(int action)
        {
            auto positions = flipped_positions(action);
            for (int pos : positions) {
                board_[pos] = color_;
            }
            return positions.size();
        }
    };

#ifdef PY
    using PyState = PythonState<State>;
#endif

}

using namespace Reversi;

#ifdef PY

namespace py = pybind11;
PYBIND11_MODULE(reversi, m)
{
    m.doc() = "implementation of game";

    py::class_<PyState>(m, "Reversi")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState::action2str, "action index to string")
    .def("str2action",    &PyState::str2action, "string to action index")
    .def("__str__",       &PyState::to_string, "string output")
    .def("copy",          &PyState::copy, "deep copy")
    .def("clear",         &PyState::clear, "ifnitialize state")
    .def("legal_actions", &PyState::legal_actions, "legal actions")
    .def("action_length", &PyState::action_length, "the number of legal action labels")
    .def("play",          &PyState::play, "state transition")
    .def("plays",         &PyState::plays, "sequential state transition")
    .def("terminal",      &PyState::terminal, "whether terminal Reversi or not")
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
        cerr << state.to_string() << endl;
    }
}

#endif