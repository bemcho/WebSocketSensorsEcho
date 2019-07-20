using HTTP,JSON,Plots
gr()
gyro = []
accelero = []
magneto = []
jsons = Channel(1024)
@async HTTP.WebSockets.listen("192.168.100.8", UInt16(2001)) do ws
    while !eof(ws)
        data = readavailable(ws)
        write(ws, "Sec-WebSocket-Accept: DdLWT/1JcX+nQFHebYP+rqEx5xI=\r\n")
        put!(jsons, JSON.parse(String(data)))
        @async processRequest()
    end
end

function processRequest()
    display(Plots.scatter3d(50, 50, 50))
    println("---------------------------------------------------------")
    json = take!(jsons)
    println(json)

    # xlims!(-500,500)
    # ylims!(-500,500)
    # zlims!(-500,500)

    x, y, z = g = json["gyroscope"]
    push!(gyro, g)
    # push!(gyro[:y], y)
    # push!(gyro[:z], z)
    #plotOne(x, y, z, "Gyro")
    plotSensors(gyro, "Gyro") 

    x, y, z = g = json["accelerometer"]
    push!(accelero, g)
    # push!(accelero[:y], y)
    # push!(accelero[:z], z)
    #plotOne(x, y, z, "Accelero")
    plotSensors(accelero, "Accelero") 

    x, y, z = g = json["magnetometer"]
    push!(magneto, g)
    # push!(magneto[:y], y)
    # push!(magneto[:z], z)
    #plotOne(x, y, z, "Magneto")
    plotSensors(magneto, "Magneto") 
    

end

function plotOne(x, y, z, label)
    Plots.scatter3d!(x, y, z, label = label)
end

function plotSensors(sensorData, label)

    Plots.plot3d!(sensorData, label = label)
    
end
