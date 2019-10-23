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
        {0, 0, 0, 0, 0, 0, 0, 0}, // L
        {1, 1, 1, 1, 0, 0, 0, 0}, // G
        {0, 0, 0, 0, 1, 1, 1, 1}, // E
        {0, 0, 0, 0, 0, 0, 0, 0}, // C
        {0, 0, 0, 0, 0, 0, 0, 0}, // F
    };

    const int SHORT[5][2][8] = {
        {{1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1}}, // L
        {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}, // G
        {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}}, // E
        {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0}}, // C
        {{1, 1, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 0, 1, 1}}, // F
    };

    long long PIECE_KEY[10][B];
    long long HAND_KEY[2][4];

    inline void init() {
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

    struct State
    {
        array<int, B> board_;
        array<array<int, 4>, 2> hand_;
        int color_;
        long long key_;
        set<long long> keys_;
        vector<int> captured_;
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
        captured_(s.captured_),
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
            board_.fill(EMPTY);
            for (auto& h : hand_) {
                h.fill(0);
            }
            key_ = 0;
            set_sfen(ORIG);
            color_ = BLACK;
            keys_.clear();
            captured_.clear();
            record_.clear();
        }

        int action2from(int action) const
        {
            return action % (B + 4);
        }

        int action2to(int action) const
        {
            return action / (B + 4);
        }

        int fromto2action(int from, int to) const
        {
            return to * (B + 4) + from;
        }

        int piece2color(int piece) const
        {
            return piece / 5;
        }

        int piece2type(int piece) const
        {
            return piece % 5;
        }

        int typecolor2piece(int type, int color) const
        {
            return color * 5 + type;
        }

        int promote(int piece) const
        {
            if (piece2type(piece) == CHICK) {
                return typecolor2piece(FOWL, piece2color(piece));
            }
            return piece;
        }

        int unpromote(int piece) const {
            if (piece2type(piece) == FOWL) {
                return typecolor2piece(CHICK, piece2color(piece));
            }
            return piece;
        }

        int position2x(int pos) const
        {
            return pos % LX;
        }

        int position2y(int pos) const
        {
            return pos / LX;
        }

        int xy2position(int x, int y) const
        {
            return y * LX + x;
        }

        string position2str(int pos) const
        {
            ostringstream oss;
            oss << Y[position2y(pos)] << X[position2x(pos)];
            return oss.str();
        }

        int str2position(const string& s) const
        {
            int y = Y.find(s[0]);
            int x = X.find(s[1]);
            return xy2position(x, y);
        }

        string action2str(int action) const
        {
            int from = action2from(action);
            int to   = action2to(action);
            return position2str(from) + position2str(to);
        }

        int str2action(const string& s) const
        {
            int from = str2position(s.substr(0, 2));
            int to   = str2position(s.substr(2, 2));
            return fromto2action(from, to);
        }

        string path2str(const vector<int>& path) const
        {
            vector<string> ss;
            for (int action : path) ss.push_back(action2str(action));
            return join(ss, " ");
        }

        vector<int> str2path(const string& s) const
        {
            vector<int> path;
            if (s.size() == 0) return path;
            for (const string& as : split(s, ' ')) path.push_back(str2action(as));
            return path;
        }

        string record_string() const
        {
            return path2str(record_);
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
            assert(legal(action));
            keys_.insert(key_ ^ color_);

            int from = action2from(action), to = action2to(action);

            int piece_cap = board_[to];
            if (piece_cap >= 0) {
                board_[to] = EMPTY;
                key_ -= PIECE_KEY[piece_cap][to];
                int type = piece2type(unpromote(piece_cap));
                hand_[color_][type] += 1;
                key_ += HAND_KEY[color_][type];
            }
            captured_.push_back(piece_cap);

            int piece = -1;
            if (from >= B) {
                int type = from - B;
                piece = typecolor2piece(type, color_);

                assert(hand_[color_][type] > 0);
                hand_[color_][type] -= 1;
                key_ -= HAND_KEY[color_][type];
            } else {
                piece = board_[from];
                if (position2x(to) == (color_ == BLACK ? 0 : (LX - 1))) {
                    piece = promote(piece);
                }

                board_[from] = EMPTY;
                key_ -= PIECE_KEY[piece][from];
            }

            board_[to] = piece;
            key_ += PIECE_KEY[piece][to];

            color_ = opponent(color_);
            record_.push_back(action);
        }

        void undo()
        {
            assert(!record_.empty());
            int action = record_.back();
            record_.pop_back();
            color_ = opponent(color_);

            int from = action2from(action), to = action2to(action);
            int piece = board_[to];

            board_[to] = EMPTY;
            key_ -= PIECE_KEY[piece][to];

            if (from >= B) {
                int type = from - B;
                hand_[color_][type] += 1;
                key_ += HAND_KEY[color_][type];
            } else {
                piece = board_[from];
                if (position2x(from) != (color_ == BLACK ? 0 : (LX - 1))) {
                    piece = unpromote(piece);
                }

                board_[from] = piece;
                key_ += PIECE_KEY[piece][from];
            }

            assert(!captured_.empty());
            int piece_cap = captured_.back();
            captured_.pop_back();
            if (piece_cap >= 0) {
                int type = piece2type(unpromote(piece_cap));
                assert(hand_[color_][type] > 0);
                hand_[color_][type] -= 1;
                key_ -= HAND_KEY[color_][type];
                board_[to] = piece_cap;
                key_ += PIECE_KEY[piece_cap][to];
            }

            keys_.erase(key_ ^ color_);
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
