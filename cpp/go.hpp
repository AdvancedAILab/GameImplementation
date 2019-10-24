#pragma once

#include "util.hpp"
#include "boardgame.hpp"

using namespace std;

namespace Go
{
    const string X = "ABCDEFGHJKLMNOPQRSTUVWXYZ";
    const string Y = "123456789abcdefghjklmnopq";
    const string C = "XO.";
    const string CC = "BW";

    struct GNUGo : public Process
    {
        GNUGo(bool japanese): Process() {
            const char* path = "./modules/gnugo";
            char* const args[] = {
                const_cast<char*>(path),
                const_cast<char*>("--mode"),
                const_cast<char*>("gtp"),
                const_cast<char*>(japanese ? "--japanese" : "--chinese"),
                nullptr
            };
            start(args[0], args);
        }
        string communicate(const string& str) {
            printline(str.c_str());
            string line;
            while (!contains(line, "=")) getline(&line);
            return line;
        }
    };

    bool use_gnugo = true;
    GNUGo *gnugo = nullptr;

    void open_gnugo(bool japanese) {
        gnugo = new GNUGo(japanese);
    }

    union Entry
    {
        short s_[4];
        unsigned long long key_;
    };

    array<Entry, 65536 + 3> scores;

    inline void init()
    {
        memset(scores.data(), 0, sizeof(scores));
    }

    struct Ren {
        set<int> libs_;
        int size_;
        long long key_;

        void clear(long long skey = 0) {
            libs_.clear();
            size_ = skey ? 1 : 0;
            key_ = skey;
        }
        void merge(const Ren& ren) {
            for (int v : ren.libs_) libs_.insert(v);
            size_ += ren.size_;
            key_ ^= ren.key_;
        }
    };

    struct State
    {
        int LX_, LY_, B_;
        float komi_;
        bool superko_;
        bool japanese_;
        vector<array<long long, 3>> STONE_KEY_;

        vector<int> board_;
        int color_;
        int ko_;
        vector<Ren> ren_;
        vector<int> next_;
        vector<int> ren_id_;
        long long position_key_;
        set<long long> position_keys_;
        vector<int> record_;

        State()
        {
            LX_ = LY_ = 3;
            B_ = LX_ * LY_;
            komi_ = 7.0;
            superko_ = false;
            japanese_ = false;

            STONE_KEY_.resize(B_);
            mt19937_64 mt(0);
            for (int pos = 0; pos < B_; pos++) {
                STONE_KEY_[pos][0] = mt();
                STONE_KEY_[pos][1] = mt();
                STONE_KEY_[pos][2] = mt();
            }

            clear();
        }

        State(const State& s):
        LX_(s.LX_),
        LY_(s.LY_),
        B_(s.B_),
        komi_(s.komi_),
        superko_(s.superko_),
        japanese_(s.japanese_),
        STONE_KEY_(s.STONE_KEY_),
        board_(s.board_),
        color_(s.color_),
        ko_(s.ko_),
        ren_(s.ren_),
        next_(s.next_),
        ren_id_(s.ren_id_),
        position_key_(s.position_key_),
        position_keys_(s.position_keys_),
        record_(s.record_) {}

        array<int, 2> size() const
        {
            return {LX_, LY_};
        }

        void clear()
        {
            board_.resize(B_);
            ren_.resize(B_);
            next_.resize(B_);
            ren_id_.resize(B_);
            color_ = BLACK;
            ko_ = -1;
            for (int pos = 0; pos < B_; pos++) {
                board_[pos] = EMPTY;
                ren_[pos].clear();
                next_[pos] = pos;
                ren_id_[pos] = pos;
            }
            position_key_ = -1;
            position_keys_.clear();
            record_.clear();
        }

        string action2str(int action) const
        {
            if (action == B_) return "PASS";
            ostringstream oss;
            oss << X[action2x(action)] << Y[action2y(action)];
            return oss.str();
        }

        int str2action(const string& s) const
        {
            return xy2action(find(X, s[0]), find(Y, s[1]));
        }

        string path2str(const vector<int>& path) const {
            vector<string> ss;
            for (int action : path) ss.push_back(action2str(action));
            return join(ss, " ");
        }

        vector<int> str2path(const string& s) const {
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
            for (int y = LY_ - 1; y >= 0; y--) {
                oss << Y[y] << " ";
                for (int x = 0; x < LX_; x++) {
                    oss << C[board_[xy2action(x, y)]];
                }
                oss << endl;
            }
            oss << "  ";
            for (int x = 0; x < LX_; x++) oss << X[x];
            oss << endl;
            oss << "record = " << record_string() << endl;
            return oss.str();
        }

        void chance(int seed=-1) {}

        void play(int action)
        {
            assert(legal(action));
            ko_ = -1;
            if (action != B_) { // non pass
                assert(onboard(action, LX_, LY_));
                board_[action] = color_;
                ren_id_[action] = action;

                long long stone_key = STONE_KEY_[action][color_];
                ren_[ren_id_[action]].clear(stone_key);
                position_key_ ^= stone_key;

                int x = action2x(action), y = action2y(action);
                for (int d = 0; d < 4; d++) {
                    int nx = x + D2[d][0], ny = y + D2[d][1];
                    if (onboard_xy(nx, ny, LX_, LY_)) {
                        int npos = xy2action(nx, ny);
                        if (board_[npos] == EMPTY) {
                            ren_[ren_id_[action]].libs_.insert(npos);
                        } else {
                            ren_[ren_id_[npos]].libs_.erase(action);
                        }
                    }
                }
                for (int d = 0; d < 4; d++) {
                    int nx = x + D2[d][0], ny = y + D2[d][1];
                    if (onboard_xy(nx, ny, LX_, LY_)) {
                        int npos = xy2action(nx, ny);
                        if (board_[npos] == color_
                            && ren_id_[action] != ren_id_[npos]) {
                            merge_ren(action, npos);
                        }
                    }
                }
                int remove_cnt = 0;
                for (int d = 0; d < 4; d++) {
                    int nx = x + D2[d][0], ny = y + D2[d][1];
                    if (onboard_xy(nx, ny, LX_, LY_)) {
                        int npos = xy2action(nx, ny);
                        if (board_[npos] == opponent(color_)
                            && ren_[ren_id_[npos]].libs_.empty()) {
                            remove_cnt += remove_ren(npos);
                        }
                    }
                }

                ko_ = -1;
                auto& ren = ren_[ren_id_[action]];
                if (remove_cnt == 1
                    && ren.size_ == 1
                    && ren.libs_.size() == 1) {
                    ko_ = *ren.libs_.begin();
                }
            }
            color_ = opponent(color_);
            record_.push_back(action);
        }

        int remove_ren(int pos) {
            int ren_id = ren_id_[pos];
            int remove_cnt = ren_[ren_id].size_;
            position_key_ ^= ren_[ren_id].key_;
            ren_[ren_id].size_ = 0;
            int tpos = pos;
            while (1) {
                board_[tpos] = EMPTY;
                ren_id_[tpos] = tpos;
                int x = action2x(tpos), y = action2y(tpos);
                for (int d = 0; d < 4; d++) {
                    int nx = x + D2[d][0], ny = y + D2[d][1];
                    if (onboard_xy(nx, ny, LX_, LY_)) {
                        int npos = xy2action(nx, ny);
                        if (board_[npos] != EMPTY) {
                            ren_[ren_id_[npos]].libs_.insert(tpos);
                        }
                    }
                }
                int next_pos = next_[tpos];
                next_[tpos] = tpos;
                tpos = next_pos;
                if (tpos == pos) break;
            }
            return remove_cnt;
        }

        void merge_ren(int pos0, int pos1) {
            int ren_id0 = ren_id_[pos0];
            int ren_id1 = ren_id_[pos1];
            ren_[ren_id0].merge(ren_[ren_id1]);
            int tpos = ren_id1;
            while (1) {
                ren_id_[tpos] = ren_id0;
                tpos = next_[tpos];
                if (tpos == ren_id1) break;
            }
            ren_[ren_id1].size_ = -1;
            swap(next_[pos0], next_[pos1]);
        }

        void plays(const string& s)
        {
            if (s.size() == 0) return;
            vector<string> ss = split(s, ' ');
            for (const string& s : ss) play(str2action(s));
        }

        bool terminal() const
        {
            // ignore 3-ko infinite games
            if (record_.size() >= B_ * 3) return true;
            // consecutive passes
            return record_.size() >= 2
                && record_.back() == B_
                && record_[record_.size() - 2] == B_;
        }

        float reward(bool subjective = true) const
        {
            // 終端状態での勝敗による報酬を返す
            float sc = score(subjective);
            if (sc > 0) return 1;
            else if (sc < 0) return -1;
            return 0;
        }

        bool legal(int action) const
        {
            if (action == B_) return true; // pass
            if (!onboard(action, LX_, LY_)) return false;
            if (board_[action] != EMPTY) return false;
            if (action == ko_) return false;

            int x = action2x(action), y = action2y(action);
            for (int d = 0; d < 4; d++) {
                int nx = x + D2[d][0], ny = y + D2[d][1];
                if (onboard_xy(nx, ny, LX_, LY_)) {
                    int npos = xy2action(nx, ny);
                    int stone = board_[npos];
                    if (stone == EMPTY) return true;

                    int lib_cnt = ren_[ren_id_[npos]].libs_.size();
                    if (lib_cnt >= 2 && stone == color_) return true;
                    if (lib_cnt == 0 && stone == opponent(color_)) return true;
                }
            }

            return false;
        }

        vector<int> legal_actions() const
        {
            // 可能な行動リストを返す
            vector<int> actions;
            for (int pos = 0; pos < B_; pos++) {
                if (legal(pos)) {
                    // check superko illegality only in action generation
                    if (superko_) {
                        long long next_key = next_position_key(pos);
                        if (position_keys_.count(next_key) > 0) continue;
                    }
                    actions.push_back(pos);
                }
            }
            actions.push_back(B_); // pass
            return actions;
        }

        int action_length() const
        {
            return B_ + 1;
        }

        vector<float> feature() const
        {
            vector<float> f(3 * LX_ * LY_, 0.0f);
            for (int pos = 0; pos < B_; pos++) {
                if (board_[pos] == color_)           f[pos] = 1;
                if (board_[pos] == opponent(color_)) f[pos + B_] = 1;
                if (color_ == BLACK) f[pos + B_ * 2] = 1;
            }
            return f;
        }

        int score(bool subjective = true) const
        {
            float sc = 0;

            unsigned long long state_key = position_key_;
            if (ko_ != -1) state_key ^= STONE_KEY_[ko_][2];
            state_key ^= color_;
            int index = state_key % scores.size();
            Entry e = scores[index];

            if (e.key_ >> 16 == state_key >> 16) {
                sc = e.s_[0] / 2.0f;
            } else {
                if (!gnugo) open_gnugo(japanese_);

                gnugo->communicate("name");
                gnugo->communicate("boardsize " + std::to_string(LX_));
                gnugo->communicate("komi " + std::to_string(komi_));
                gnugo->communicate("clear_board");
                int color = BLACK;
                for (int action : record_) {
                    ostringstream oss;
                    oss << "play " << CC[color] << " " << action2str(action);
                    gnugo->communicate(oss.str());
                    color = opponent(color);
                }
                string score_str = gnugo->communicate("final_score");
                float abs_sc = stof(split(split(strip(score_str, '\n'), ' ')[1], '+').back());
                sc = contains(score_str, "W+") ? -abs_sc : abs_sc;

                e.key_ = (state_key >> 16) << 16;
                e.s_[0] = sc * 2;
                scores[index] = e;
            }

            if (subjective && color_ == WHITE) sc = -sc;
            return sc;
        }

        int action2x(int action) const
        {
            return action % LX_;
        }

        int action2y(int action) const
        {
            return LY_ - 1 - action / LX_;
        }

        int xy2action(int x, int y) const
        {
            return (LY_ - 1 - y) * LX_ + x;
        }

        long long next_position_key(int action) const {
            assert(onboard(action, LX_, LY_));
            long long next_key = position_key_;
            next_key ^= STONE_KEY_[color_][action];
            int x = action2x(action), y = action2y(action);
            for (int d = 0; d < 4; d++) {
                int nx = x + D2[d][0], ny = y + D2[d][1];
                if (onboard_xy(nx, ny, LX_, LY_)) {
                    int npos = xy2action(nx, ny);
                    int ren_id = ren_id_[npos];
                    if (board_[npos] == opponent(color_)
                        && ren_[ren_id].libs_.size() == 1) next_key ^= ren_[ren_id].key_;
                }
            }
            return next_key;
        }
    };
}
