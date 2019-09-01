#include <set>
#include <random>

#include "util.hpp"
#include "boardgame.hpp"

using namespace std;

namespace AnimalShogi
{
    constexpr int LX = 4, LY = 3;
    constexpr int B = LX * LY;

    const string X = "1234";
    const string Y = "ABC*";
    const string C = "LGECFlgecf";

    enum {
        EMPTY = -1,
        LION, GIRRAFE, ELEPHANT, CHICK, FOWL 
    };

    const string ORIG = "gle c  C ELG";

    // direction 0~3 0~7

    // effect
    const int LONG[5][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
    };

    const int SHORT[5][2][8] = {
        {{1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1}},
        {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0}},
        {{1, 1, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 0, 1, 1}},
    };

    long long PIECE_KEY[10][B];
    long long HAND_KEY[2][4];

    void init() {
        mt19937_64 mt(0);
        for (int p = 0; p < 10; p++) {
            for (int pos = 0; pos < B; pos++) {
                PIECE_KEY[p][pos] = mt();
            }
        }
        for (int c = 0; c < 2; c++) {
            for (int type = 0; type < 4; type++) {
                HAND_KEY[c][type] = mt();
            }
        }
    }

    inline int action2from(int action)
    {
        return action % (B + 4);
    }

    inline int action2to(int action)
    {
        return action / (B + 4);
    }

    inline int fromto2action(int from, int to)
    {
        return to * (B + 4) + from;
    }

    inline int piece2color(int piece)
    {
        return piece / 5;
    }

    inline int piece2type(int piece)
    {
        return piece % 5;
    }

    inline int typecolor2piece(int type, int color)
    {
        return color * 5 + type;
    }

    inline int position2x(int pos)
    {
        return pos % LX;
    }

    inline int position2y(int pos)
    {
        return pos / LX;
    }

    inline int xy2position(int x, int y)
    {
        return y * LX + x;
    }

    struct State
    {
        array<int, B> board_;
        array<array<int, 4>, 2> hand_;
        int color_;
        long long key_;
        set<long long> keys_;
        vector<int> record_;

        State()
        {
            clear();
        }

        State(const State& s):
        board_(s.board_),
        hand_(s.hand_),
        color_(s.color_),
        key_(s.key_),
        keys_(s.keys_),
        record_(s.record_) {}

        array<int, 2> size() const
        {
            return {LX, LY};
        }

        void set_sfen(const string& sfen)
        {
            int index = 0;
            for (int x = 0; x < LX; x++) {
                for (int y = 0; y < LY; y++) {
                    char piece_char = sfen[index++];
                    if (piece_char != ' ') {
                        int piece = C.find(piece_char);
                        int pos = xy2position(x, y);
                        board_[pos] = piece;
                        key_ += PIECE_KEY[piece][pos];
                    }
                }
            }
        }

        void clear()
        {
            board_.fill(-1);
            for (auto& h : hand_) {
                h.fill(0);
            }
            key_ = 0;
            set_sfen(ORIG);
            color_ = BLACK;
            keys_.clear();
            record_.clear();
        }

        static string position2str(int pos)
        {
            ostringstream oss;
            oss << Y[position2y(pos)] << X[position2x(pos)];
            return oss.str();
        }

        static int str2position(const string& s)
        {
            int y = Y.find(s[0]);
            int x = X.find(s[1]);
            return xy2position(x, y);
        }

        static string action2str(int action)
        {
            int from = action2from(action);
            int to   = action2to(action);
            return position2str(from) + position2str(to);
        }

        static int str2action(const string& s)
        {
            int from = str2position(s.substr(0, 2));
            int to   = str2position(s.substr(2, 2));
            return fromto2action(from, to);
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
            for (int y = 0; y < LY; y++) oss << Y[y];
            oss << endl;
            for (int x = 0; x < LX; x++) {
                oss << X[x] << " ";
                for (int y = 0; y < LY; y++) {
                    int piece = board_[xy2position(x, y)];
                    oss << (piece >= 0 ? C[piece] : '.');
                }
                oss << endl;
            }
            oss << hand_ << endl;
            oss << "record = " << record_string();
            return oss.str();
        }

        void play(int action)
        {
            keys_.insert(key_ ^ color_);

            int from = action2from(action), to = action2to(action);
            int piece = -1;

            if (from >= B) {
                int drop_type = from - B;
                piece = typecolor2piece(drop_type, color_);

                hand_[color_][drop_type] -= 1;
                assert(hand_[color_][drop_type] >= 0);
            } else {
                piece = board_[from];
                if (piece2type(piece) == CHICK
                    && position2x(to) == (color_ == BLACK ? 0 : (LX - 1))) {
                    piece = typecolor2piece(FOWL, piece2color(piece));
                }

                board_[from] = -1;
                key_ -= PIECE_KEY[piece][from];
            }

            int piece_cap = board_[to];
            if (piece_cap >= 0) {
                if (piece2type(piece_cap) == FOWL) {
                    piece_cap = typecolor2piece(CHICK, piece2color(piece_cap));
                }
                int type = piece2type(piece_cap);
                hand_[color_][type] += 1;
                key_ += HAND_KEY[color_][type];
            }

            board_[to] = piece;
            key_ += PIECE_KEY[piece][to];

            color_ = opponent(color_);
            record_.push_back(action);
        }

        void plays(const string& s)
        {
            if (s.size() == 0) return;
            vector<string> ss = split(s, ' ');
            for (const string& s : ss) play(str2action(s));
        }

        bool terminal() const
        {
            if (keys_.count(key_ ^ color_) > 0) return true;
            return reward() != 0;
        }

        float reward(bool subjective = true) const
        {
            if (keys_.count(key_ ^ color_) > 0) return 0; // repetition
            float r = 0;
            // catch win
            if (hand_[BLACK][LION] > 0) r =  1;
            if (hand_[WHITE][LION] > 0) r = -1;
            // try win
            int target_piece = typecolor2piece(LION, color_);
            for (int y = 0; y < LY; y++) {
                if (color_ == BLACK && board_[xy2position(0,      y)] == target_piece) r =  1;
                if (color_ == WHITE && board_[xy2position(LX - 1, y)] == target_piece) r = -1;
            }
            if (subjective && color_ == WHITE) r = -r;
            return r;
        }

        bool legal(int action) const
        {
            int from = action2from(action);
            if (from >= B) {
                int drop_type = from - B;
                assert(drop_type < 4);
                if (hand_[color_][drop_type] == 0) return false;
            } else {
                assert(from >= 0);
                if (board_[from] < 0) return false;
            }
            int to = action2to(action);
            if (board_[to] >= 0 && piece2color(board_[to]) == color_) return false; 
            return true;
        }

        vector<int> legal_actions() const
        {
            vector<int> actions;

            // move actions
            for (int from = 0; from < B; from++) {
                int piece = board_[from];
                if (piece >= 0 && piece2color(piece) == color_) {
                    // find movable points
                    int type = piece2type(piece);
                    int x = position2x(from), y = position2y(from);
                    // short
                    for (int d = 0; d < 8; d++) {
                        if (SHORT[type][color_][d]) {
                            int x_to = x + D2[d][0], y_to = y + D2[d][1];
                            if (onboard_xy(x_to, y_to, LX, LY)) {
                                int to = xy2position(x_to, y_to);
                                int piece_cap = board_[to];
                                if (piece_cap < 0 || piece2color(piece_cap) != color_) {
                                    actions.push_back(fromto2action(from, to));
                                }
                            }
                        }
                    }
                    // long
                    for (int d = 0; d < 8; d++) {
                        if (LONG[type][d]) {
                            int x_to = x, y_to = y;
                            while (true) {
                                x_to += D2[d][0];
                                y_to += D2[d][1];
                                if (!onboard_xy(x_to, y_to, LX, LY)) break;
                                int to = xy2position(x_to, y_to);
                                int piece_cap = board_[to];
                                if (piece_cap >= 0 && piece2color(piece_cap) == color_) break;
                                actions.push_back(fromto2action(from, to));
                                if (piece_cap >= 0) break;
                            }
                        }
                    }
                }
            }

            // drop actions
            for (int type = 0; type < 4; type++) {
                if (hand_[color_][type] > 0) {
                    for (int to = 0; to < B; to++) {
                        if (board_[to] < 0) {
                            actions.push_back(fromto2action(B + type, to));
                        }
                    }
                }
            }

            return actions;
        }

        int action_length() const
        {
            return (B + 4) * B;
        }

        vector<float> feature() const
        {
            vector<float> f(27 * LX * LY, 0.0f);
            // board
            for (int pos = 0; pos < B; pos++) {
                int piece = board_[pos];
                if (piece >= 0) {
                    int index = piece2type(piece);
                    if (piece2color(piece) != color_) index += 5;
                    f[index * B + pos] = 1;
                }
            }
            // hand
            int colors[2] = {color_, opponent(color_)};
            for (int i = 0; i < 2; i++) {
                for (int type = 0; type < 4; type++) {
                    int piece_num = hand_[colors[i]][type];
                    if (piece_num > 0) {
                        int index = 10 + i * 8 + type * 2 + piece_num - 1;
                        for (int pos = 0; pos < B; pos++) {
                            f[index * B + pos] = 1;
                        }
                    }
                }
            }
            // turn
            if (color_ == WHITE) {
                for (int pos = 0; pos < B; pos++) {
                    f[26 * B + pos] = 1;
                }
            }
            return f;
        }
    };
}
