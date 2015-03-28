function add_walls(y, a)
    for i = -a,a do
        add_cube(i, y, a)
        add_cube(i, y, -a)
        if i ~= -a and i ~= a then
            add_cube(a, y, i)
            add_cube(-a, y, i)
        end
    end
end

function setup_scene()
    setcam(15,15,15,0,0,0)

    add_walls(3, 12)
    add_walls(4, 12)
    add_walls(5, 12)

    for y = 1,3000 do
        add_cube(math.sin(y), y*0.2+5, math.sin(y+1), 0.1)
    end
end
