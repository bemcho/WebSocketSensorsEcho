using HTTP,JSON,Plots
gr()
gyro = []
accelero = []
acceleroGApplied = []
magneto = []
temp = []
pressure = []
humidity = []
currentPosition = [0.0,0.0,0.0]
jsons = Channel(1024)

@async HTTP.WebSockets.listen("192.168.100.8", UInt16(2001)) do ws
    while !eof(ws)
        data = readavailable(ws)
        write(ws, "Sec-WebSocket-Accept: DdLWT/1JcX+nQFHebYP+rqEx5xI=\r\n")
        put!(jsons, data)
        @async processRequest()
    end
end

function processRequest()
    #while true
    println("---------------------------------------------------------")
    data = String(take!(jsons))
    println(data)

    json = JSON.parse(data)

    x, y, z = g = json["gyroscope"]
        #push!(gyro, g)
    plt1 =  plotAxes(x, y, z, "Gyro", 6000)


    xa, ya, za = g = json["accelerometer"]
    global currentPosition+=[xa+x,ya+y,za+z]
    push!(acceleroGApplied, currentPosition)
       # push!(accelero, g)
        #x, y, z = [[xyz[1] for xyz in accelero],[xyz[2] for xyz in accelero],[xyz[3] for xyz in accelero]]
    plt2 = plotAxes(xa, ya, za, "Accelero", 1500) 

    x, y, z = g = json["magnetometer"]
        #push!(magneto, g)
        #x, y, z = [[xyz[1] for xyz in magneto],[xyz[2] for xyz in magneto],[xyz[3] for xyz in magneto]]
    plt3 = plotAxes(x, y, z, "Magneto", 1000) 

    plt7 = plotAccelero("AcceleroGyro", 1500);

    t = json["environmentTemp"]
    push!(temp, t)
    plt4 = Plots.plot(1:length(temp), temp, xlabel = "Time", ylabel = "Temperature", ylims = (-20.0, 120.0), label = "T: $t °C", layout = 1, color = [:red], size = (600, 150)) 

    h = json["humidity"]
    push!(humidity, h)
    plt5 = Plots.plot(1:length(humidity), humidity, ylabel = "Humidity", color = [:blue], ylims = (0.0, 100.0), label = "H: $h %", size = (600, 150)) 

    p = json["environmentPressure"]
    push!(pressure, p)
    plt6 = Plots.plot(1:length(pressure), pressure, ylabel = "Pressure", color = [:black], ylims = (260.0, 1260.0), label = ["P: $p hPa"], size = (600, 150))
    plt8 = plot(plt6, plt5, plt4, layout = (3, 1))    

    display(plot(plt1, plt2, plt3, plt7, plt8, size = (1000, 1000)))
    
end
function normalize(x)
    return sqrt(abs(x)) * sign(x)
    
end
function plotAccelero(title, max = 500, origin = 0.0)

    plot3d([xyz[1] + origin for xyz in acceleroGApplied], [xyz[2] + origin for xyz in acceleroGApplied], [xyz[3] + origin for xyz in acceleroGApplied], title = title, legend = false, line = (:path3d, :solid, :arrow, 2, :blue),xlims=(-500,500),ylims=(-500,500),zlims=(-500,500))
end

function plotAxes(x, y, z, title, max = 500.0, origin = 0.0)
    mX = x / 2;
    mY = y / 2;
    mZ = z / 2;
    #x
    plt1 = plot3d([-(max + x),max + x], [-(origin + y),origin + y], [-(origin + z),origin + z], xlims = (-max, max), ylims = (-max, max), zlims = (-max, max), marker = 1, line = (:line, :solid, :arrow, 2, :red), legend = false, title = title)
        #z
    plot3d!([-(origin + x),origin + x], [-(max + y),max + y], [-(origin + z),origin + z], marker = 1, line = (:line, :solid, :arrow,  2, :blue),   legend = false)
        #y
    plot3d!([-(origin + x),origin + x], [-(origin + y),origin + y], [-(max + z),max + z], marker = 1, line = (:line, :solid, :arrow, 2, :green),  legend = false)
         #R\_b
   # plot3d!([-max, if x < -max || x > max max else x end], [-max,if y < -max  || y > max max else  y end], [-max,if z < -max  || z > max max else  z end], marker = 1, line = (:arrow, 2, :black),  legend = false)
end
function plotSensors(x, y, z, label, layout;color = [:black])

    return  Plots.scatter3d(x, y, z, legend = false, title = label, layout = layout, color = color, size = size(350, 350))   
end
processRequest()