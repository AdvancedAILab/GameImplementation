#pragma once

#include <unistd.h>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <utility>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <map>

static bool contains(const std::string& s, const std::string& t)
{
    return s.find(t) != std::string::npos;
}

static int find(const std::string& s, char sep, std::string::size_type p = 0)
{
    std::string::size_type q = s.find(sep, p);
    if (q == std::string::npos) return -1;
    return q;
}

static std::vector<std::string> split(const std::string& s, char sep)
{
    std::vector<std::string> result;
    std::string::size_type p = 0, q;
    while ((q = s.find(sep, p)) != std::string::npos) {
        result.emplace_back(s, p, q - p);
        p = q + 1;
    }
    result.emplace_back(s, p);
    return result;
}

static std::string strip(const std::string& s, const char sep)
{
    std::string result;
    for (char c : s) {
        if (c != sep) result.push_back(c);
    }
    return result;
}

static std::string join(const std::vector<std::string>& v, const std::string& sep)
{
    if (v.size() == 0) return std::string();
    std::ostringstream os;
    os << v[0];
    for (std::size_t i = 1; i < v.size(); i++) os << sep << v[i];
    return os.str();
}

template <class T>
std::ostream& operator <<(std::ostream& ost, const std::vector<T>& v)
{
    ost << "{";
    if (v.size() > 0) {
        for (std::size_t i = 0; i < v.size() - 1; i++) ost << v[i] << ", ";
        ost << v.back();
    }
    ost << "}";
    return ost;
}

template <class T, std::size_t N>
std::ostream& operator <<(std::ostream& ost, const std::array<T, N>& v)
{
    ost << "{";
    if (v.size() > 0) {
        for (std::size_t i = 0; i < v.size() - 1; i++) ost << v[i] << ", ";
        ost << v.back();
    }
    ost << "}";
    return ost;
}

template <class T0, class T1>
std::ostream& operator <<(std::ostream& ost, const std::map<T0, T1>& v)
{
    ost << "{";
    if (v.size() > 0)
    {
        int cnt = 0;
        for (auto kv : v) {
            ost << kv.first << ":" << kv.second;
            if (cnt++ < (int)v.size()) ost << ", ";
        }
    }
    return ost;
}

template <class T0, class T1>
std::ostream& operator <<(std::ostream& ost, const std::pair<T0, T1>& v)
{
    ost << "(" << v.first << ", " << v.second << ")";
    return ost;
}

template <class T>
T sum_of(const std::vector<T>& v)
{
    return std::accumulate(v.begin(), v.end(), T(0));
}

template <class T, std::size_t N>
T sum_of(const std::array<T, N>& v)
{
    return std::accumulate(v.begin(), v.end(), T(0));
}

class Process
{
public:
    bool getline(std::string* const line) {
        line->clear();
        for (char c; (c = std::fgetc(this->stream_from_child_)) != EOF;) {
            if (c == '\n') return true;
            line->push_back(c);
        }
        return false;
    }

    int start(const char* const file, char *const argv[]) {
        const int kRead = 0, kWrite = 1;

        int pipe_from_child[2];
        int pipe_to_child[2];

        if (pipe(pipe_from_child) < 0) {
            std::perror("failed to create pipe_from_chlid.\n");
            return -1;
        }

        if (pipe(pipe_to_child) < 0) {
            std::perror("failed to create pipe_to_child.\n");
            close(pipe_from_child[kRead]);
            close(pipe_from_child[kWrite]);
            return -1;
        }

        pid_t process_id = fork();
        if (process_id < 0) {
            std::perror("fork() failed.\n");
            close(pipe_from_child[kRead]);
            close(pipe_from_child[kWrite]);
            close(pipe_to_child[kRead]);
            close(pipe_to_child[kWrite]);
            return -1;
        }

        if (process_id == 0) {
            close(pipe_to_child[kWrite]);
            close(pipe_from_child[kRead]);

            dup2(pipe_to_child[kRead], STDIN_FILENO);
            dup2(pipe_from_child[kWrite], STDOUT_FILENO);

            close(pipe_to_child[kRead]);
            close(pipe_from_child[kWrite]);

            if (execvp(file, argv) < 0) {
                std::perror("execvp() failed\n");
                close(pipe_to_child[kRead]);
                close(pipe_from_child[kWrite]);
                return -1;
            }
        }

        process_id_ = process_id;

        stream_to_child_ = fdopen(pipe_to_child[kWrite], "w");
        stream_from_child_ = fdopen(pipe_from_child[kRead], "r");

        std::setvbuf(stream_to_child_, NULL, _IONBF, 0);
        std::setvbuf(stream_from_child_, NULL, _IONBF, 0);

        return process_id;
    }

    void printline(const char* str) {
        std::fprintf(this->stream_to_child_, "%s\n", str);
    }

    pid_t process_id() const { return process_id_; }

private:
    pid_t process_id_;
    std::FILE *stream_to_child_, *stream_from_child_;
};
