AddOption(
    '--debug-build',
    action='store_true',
    help='debug build',
    default=False)

common_flags = '-std=c++11 -Wall -Wextra '
if GetOption('debug_build'):
    flags = common_flags + '-g -O0'
else:
    flags = common_flags + '-O2'

env = Environment(CXXFLAGS = flags)

env.Program(
    target = 'game',
    source = ['main.cpp', 'gfx/gfx.cpp', 'physics/world.cpp', 'util.cpp'],
    LIBS = ['GL', 'SDL2', 'BulletDynamics', 'BulletCollision', 'LinearMath']
)
