using HTTP,JSON,Plots
gr()
gyro = []
accelero = []
magneto = []
temp = []
pressure = []
humidity = []
jsons = Channel(1024)

@async HTTP.WebSockets.listen("192.168.100.8", UInt16(2001)) do ws
    while !eof(ws)
        data = readavailable(ws)
        write(ws, "Sec-WebSocket-Accept: DdLWT/1JcX+nQFHebYP+rqEx5xI=\r\n")
        put!(jsons, JSON.parse(String(data)))
        #@async processRequest()
    end
end

function processRequest()
    while true
        println("---------------------------------------------------------")
        json = take!(jsons)
        println(json)

        x, y, z = g = json["gyroscope"]
        push!(gyro, g)
        plt1 =  plotAxes(x, y, z, "Gyro")
        x, y, z = g = json["accelerometer"]
        push!(accelero, g)
        #x, y, z = [[xyz[1] for xyz in accelero],[xyz[2] for xyz in accelero],[xyz[3] for xyz in accelero]]
        plt2 = plotAxes(x, y, z, "Accelero") 

        x, y, z = g = json["magnetometer"]
        push!(magneto, g)
        #x, y, z = [[xyz[1] for xyz in magneto],[xyz[2] for xyz in magneto],[xyz[3] for xyz in magneto]]
        plt3 = plotAxes(x, y, z, "Magneto") 


        t = json["environmentTemp"]
        push!(temp, t)
        plt4 = Plots.plot(1:length(temp), temp, xlabel = "Time", ylabel = "Temperature", layout = 1, legend = false, color = [:red]) 

        h = json["humidity"]
        push!(humidity, h)
        plt6 = Plots.plot(1:length(humidity), humidity, xlabel = "Time", ylabel = "Humidity", layout = 1, legend = false, color = [:blue]) 

        p = json["environmentPressure"]
        push!(pressure, p)
        plt5 = Plots.plot(1:length(pressure), pressure, xlabel = "Time", ylabel = "Pressure", layout = 1, ylims = (0.0, 1000, 0.0), legend = false, color = [:black]) 
        

        display(plot(plt1, plt2, plt3, plt4, plt5, plt6))
    end
end
function plotAxes(x, y, z, title, max = 500.0)
    mX = x / 2;
    mY = y / 2;
    mZ = z / 2;
    #x
    plt1 = plot3d([mX,max], [mY,mY + y], [mZ,mZ + z], marker = 1, line = (:arrow, 4, :red), legend = false, title = title)
        #y
    plot3d!([mX,mX + x], [mY,max], [mZ,mZ + z], marker = 1, line = (:arrow,  4, :blue),   legend = false)
        #z
    plot3d!([mX,mX + x], [mY,mY + y], [mZ,max], marker = 1, line = (:arrow, 4, :green),  legend = false)
end
function plotSensors(x, y, z, label, layout;color = [:black])

    return  Plots.scatter3d(x, y, z, legend = false, title = label, layout = layout, color = color, size = size(350, 350))   
end
processRequest()