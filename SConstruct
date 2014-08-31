def src(s):
    return [env.Object(obj) for obj in s.split()]

AddOption(
    '--debug-build',
    action='store_true',
    help='debug build',
    default=False)

common_flags = '-std=c++11 -Wall -Wextra -isystem libraries/bullet -isystem libraries/lua '
if GetOption('debug_build'):
    flags = common_flags + '-g -O0'
else:
    flags = common_flags + '-O2'

env = Environment(
    CXXFLAGS = flags,
    LIBPATH = ['libraries/'],
)

game = env.Program(
    'game',
    src('main.cpp gfx/gfx.cpp physics/world.cpp util/util.cpp'),
    LIBS = ['GL', 'SDL2', 'BulletDynamics', 'BulletCollision', 'LinearMath']
)
env.Default(game)


tests = [
    env.Program(
        file.path[:-4],
        src('util/util.cpp physics/world.cpp ' + file.path),
        LIBS = ['BulletDynamics', 'BulletCollision', 'LinearMath'])
    for file in Glob('tests/test-*.cpp')]

build_tests = env.Alias('build-tests', tests)
test_action = ['valgrind --error-exitcode=255 %s' % t[0].abspath for t in tests]
test = Command(target='test', source=tests, action=test_action)
if GetOption('clean'):
    env.Default(build_tests)

def bullet_builds():
    import os
    libs = []
    for bulletlib in ['BulletDynamics', 'BulletCollision', 'LinearMath']:
        sources = []
        for root, dirnames, filenames in os.walk('libraries/bullet/' + bulletlib):
            print root
            for filename in filenames:
                print filename
                if filename[-4:] == '.cpp':
                    sources.append(os.path.join(root, filename))
        libs.append(env.StaticLibrary('libraries/'+bulletlib, sources))
    return libs
def lua_builds():
    filenames = '''lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c
lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c
ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lbitlib.c
lcorolib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c ltablib.c
loadlib.c linit.c'''.split()
    sources = ['libraries/lua/' + f for f in filenames]
    return [env.StaticLibrary('libraries/lua.a', sources)]

if 'libraries' in COMMAND_LINE_TARGETS:
    libs = bullet_builds()
    libs += lua_builds()
    env.Alias('libraries', libs)
