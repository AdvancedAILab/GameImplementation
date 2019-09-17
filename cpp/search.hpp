#pragma once

// search algorithm

template <class state_t>
float minimaxSearchImpl(state_t& state)
{
    if (state.terminal()) return state.reward();
    float best = -10000;
    for (int action : state.legal_actions()) {
        state.play(action);
        best = std::max(best, -minimaxSearchImpl(state));
        state.undo();
    }
    return best;
}

template <class state_t>
std::pair<std::vector<int>, float> minimaxSearch(state_t& state)
{
    float best = -10000;
    std::vector<int> best_actions;
    if (state.terminal()) return std::make_pair(best_actions, state.reward());
    for (int action : state.legal_actions()) {
        state.play(action);
        float reward = -minimaxSearchImpl(state);
        if (reward >= best) {
            if (reward > best) {
                best = reward;
                best_actions.clear();
            }
            best_actions.push_back(action);
        }
        state.undo();
    }
    return std::make_pair(best_actions, best);
}

template <class state_t>
float alphaBetaSearchImpl(state_t& state, float alpha, float beta)
{
    if (state.terminal()) return state.reward();
    for (int action : state.legal_actions()) {
        state.play(action);
        alpha = std::max(alpha, -alphaBetaSearchImpl(state, -beta, -alpha));
        state.undo();
        if (alpha >= beta) return alpha;
    }
    return alpha;
}

template <class state_t>
std::pair<std::vector<int>, float> alphaBetaSearch(state_t& state)
{
    float best = -10000;
    std::vector<int> best_actions;
    if (state.terminal()) return std::make_pair(best_actions, state.reward());
    for (int action : state.legal_actions()) {
        state.play(action);
        float reward = -alphaBetaSearchImpl(state, -10000, -best + 1e-4);
        if (reward >= best) {
            if (reward > best) {
                best = reward;
                best_actions.clear();
            }
            best_actions.push_back(action);
        }
        state.undo();
    }
    return std::make_pair(best_actions, best);
}