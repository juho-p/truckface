def src(s):
    return [env.Object(obj) for obj in s.split()]

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

game = env.Program(
    'game',
    src('main.cpp gfx/gfx.cpp physics/world.cpp util.cpp'),
    LIBS = ['GL', 'SDL2', 'BulletDynamics', 'BulletCollision', 'LinearMath']
)
env.Default(game)

tests = [
    env.Program(
        file.path[:-4],
        src('util.cpp physics/world.cpp ' + file.path),
        LIBS = ['BulletDynamics', 'BulletCollision', 'LinearMath'])
    for file in Glob('tests/test-*.cpp')]

build_tests = env.Alias('build-tests', tests)

test_action = ['valgrind --error-exitcode=255 %s' % t[0].abspath for t in tests]
test = Command(target='test', source=tests, action=test_action)
if GetOption('clean'):
    env.Default(build_tests)
