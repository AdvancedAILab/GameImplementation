import games

classes = [
    'TicTacToe',
    'Reversi',
    'AnimalShogi',
    'Go',
    'Geister',
]

for game in classes:
    s = getattr(games, game)()
    print(s)
    print(s.legal_actions())
    print([s.action2str(a) for a in s.legal_actions()])