#Sends sensors data as json to hard coded server url from Azure Iot DevKit device

## in julia
> ]
> add HTTP,JSON,Plots,ORCA,PlotlyJS
> backspace to return

example julia start web socket, the important is to return the: Sec-WebSocket-Accept header with used value  

```
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
        println(g)
        push!(gyro, g)
        x, y, z = [[xyz[1] for xyz in gyro],[xyz[2] for xyz in gyro],[xyz[3] for xyz in gyro]]
        plt1 = plotSensors(x, y, z, "Gyro", 1, color = [:red]) 

        x, y, z = g = json["accelerometer"]
        push!(accelero, g)
        x, y, z = [[xyz[1] for xyz in accelero],[xyz[2] for xyz in accelero],[xyz[3] for xyz in accelero]]
        plt2 = plotSensors(x, y, z, "Accelero", 1, color = [:brown]) 

        x, y, z = g = json["magnetometer"]
        push!(magneto, g)
        x, y, z = [[xyz[1] for xyz in magneto],[xyz[2] for xyz in magneto],[xyz[3] for xyz in magneto]]
        plt3 = plotSensors(x, y, z, "Magneto", 1, color = [:blue]) 


        t = json["environmentTemp"]
        push!(temp, t)

        h = json["humidity"]
        push!(humidity, h)

        p = json["environmentPressure"]
        push!(pressure,p/10)
        plt4 = Plots.plot(1:length(temp), temp, xlabel = "Time", ylabel = "Temperature", ylims=(-50.000,70.000),layout = 1, legend = false,color=[:red]) 
        plt4 = Plots.plot!(1:length(pressure), pressure, xlabel = "Time", ylabel = "Pressure", ylims=(-100.00,150.00),layout = 1, legend = false,color=[:black]) 
        plt4 = Plots.plot!(1:length(humidity), humidity, xlabel = "Time", ylabel = "Temperature,Humidity,Pressure", ylims=(0.00,100.00),layout = 1, legend = false,color=[:blue]) 

        display(plot(plt1, plt2, plt3, plt4))
    end
end
function plotSensors(x, y, z, label, layout;color = [:black])

    return  Plots.scatter3d(x, y, z, legend = false, title = label, layout = layout, color = color)   
end
processRequest()
```
In WebSocketEcho.ino change webSocketServerUrl  = to your wifi net ip of server
Response:
```
{
  "ipAddress": "172.30.30.121",
  "environmentTemp": 29.20,
  "environmentTempUnit": "C",
  "humidity": 39.00,
  "environmentPressure": 948.95,
  "pressureUnit": "hPa",
  "gyroscope": [
    350,
    490,
    70
  ],
  "accelerometer": [
    -3,
    -17,
    1030
  ],
  "magnetometer": [
    271,
    -18,
    -268
  ]
}
```
