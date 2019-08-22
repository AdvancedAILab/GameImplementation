import games

classes = [
    'TicTacToe',
    'Reversi',
    'AnimalShogi',
    'Go',
]

for game in classes:
    s = getattr(games, game)()
    print(s)
    print(s.legal_actions())

