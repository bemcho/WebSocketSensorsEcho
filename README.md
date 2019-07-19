Sends sensors data as json to hard coded server url from Azure Iot DevKit device


example julia start web socket, the important is to return the: Sec-WebSocket-Accept header with used value  

```
using HTTP,JSON,Plots
gr()
gyro=Dict(:x=>[],:y=>[],:z=>[])
accelero=Dict(:x=>[],:y=>[],:z=>[])
magneto=Dict(:x=>[],:y=>[],:z=>[])
@async HTTP.WebSockets.listen("172.30.30.110", UInt16(2001)) do ws
    while !eof(ws)
        data = readavailable(ws)
        write(ws, "Sec-WebSocket-Accept: DdLWT/1JcX+nQFHebYP+rqEx5xI=\r\n")
        processRequest(JSON.parse(String(data)))
    end
end

function processRequest(json)
    println("---------------------------------------------------------")
    println(json)
    
    x,y,z=json["gyroscope"]
    push!(gyro[:x],x)
    push!(gyro[:y],y)
    push!(gyro[:z],z)

    display(Plots.plot3d(gyro[:x],gyro[:y],gyro[:z],label="Gyro",color=[:red]))

    x,y,z=json["accelerometer"]
    push!(accelero[:x],x)
    push!(accelero[:y],y)
    push!(accelero[:z],z)
    display(Plots.plot3d!(accelero[:x],accelero[:y],accelero[:z],label="Accelero",color=[:blue]))

    x,y,z=json["magnetometer"]
    push!(magneto[:x],x)
    push!(magneto[:y],y)
    push!(magneto[:z],z)
    display(Plots.plot3d!(magneto[:x],magneto[:y],magneto[:z],label="Magneto",color=[:green]))
end
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
