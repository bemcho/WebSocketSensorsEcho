Sends sensors data as json to hard coded server url from Azure Iot DevKit device


julia start web socket 
using HTTP,JSON

```@async HTTP.WebSockets.listen("172.30.30.110", UInt16(2001)) do ws
    while !eof(ws)
        data = readavailable(ws)
        println("-----------------------------------------------------------------------")
        println(String(data))

        write(ws, "Sec-WebSocket-Accept: DdLWT/1JcX+nQFHebYP+rqEx5xI=")
    end
end
```
In WebSocketEcho.ino change webSocketServerUrl  = to your wifi net ip of server
