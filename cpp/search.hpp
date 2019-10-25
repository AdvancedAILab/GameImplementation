#pragma once

// search algorithm

template <class state_t>
float minimax_search_impl(state_t& state)
{
    if (state.terminal()) return state.reward();
    float best = -10000;
    for (int action : state.legal_actions()) {
        state.play(action);
        best = std::max(best, -minimax_search_impl(state));
        state.undo();
    }
    return best;
}

template <class state_t>
std::pair<std::vector<int>, float> minimax_search(state_t& state)
{
    float best = -10000;
    std::vector<int> best_actions;
    if (state.terminal()) return std::make_pair(best_actions, state.reward());
    for (int action : state.legal_actions()) {
        state.play(action);
        float reward = -minimax_search_impl(state);
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
float alpha_beta_search_impl(state_t& state, float alpha, float beta)
{
    if (state.terminal()) return state.reward();
    for (int action : state.legal_actions()) {
        state.play(action);
        alpha = std::max(alpha, -alpha_beta_search_impl(state, -beta, -alpha));
        state.undo();
        if (alpha >= beta) return alpha;
    }
    return alpha;
}

template <class state_t>
std::pair<std::vector<int>, float> alpha_beta_search(state_t& state)
{
    float best = -10000;
    std::vector<int> best_actions;
    if (state.terminal()) return std::make_pair(best_actions, state.reward());
    for (int action : state.legal_actions()) {
        state.play(action);
        float reward = -alpha_beta_search_impl(state, -10000, -best + 1e-4);
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