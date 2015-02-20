# Truckface

Game including physics, vehicles, driving.

Physics implemented using Bullet-engine. Graphics with SDL2+OpenGL3. Scripting with LUA.

Currently very unfinished, only simple physics+graphics sandbox Lua-REPL is implemented.

When game is started, simple graphics window is opened and console displays the REPL. Following LUA-functions are exported:

    writeline(string)

write a line to stdout

    readline()

read a line from stdin

    setcam(x1,y1,z1, x2,y2,z2)

6 numbers, first 3 tell camera position, last 3 tell camera direction (point in coordinate system where to look at

    add_cube(x,y,z, size)

Add cube to given coordinates. Size is optional. Returns cube ID.

    add_car(x,y,z)

Add vehicle to given coordinate. Returns vehicle ID.

    carengine(vehicle_id, boolean)

Set car engine on/of

    carsteer(vehicle_id, steering)

Set car steering
