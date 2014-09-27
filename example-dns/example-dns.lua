local dns = require("dns")
local inspect = require("inspect")
local sqlite3 = require("lsqlite3")

function udp53(srcaddr, srcport, data)
  print("Received udp53 from ", srcaddr, srcport)
  dns._resolver.time = 0
  local resp = dns._resolver:decode(data, true)
  -- print(inspect(resp))
end

function udp5353(srcaddr, srcport, data)
  print("Received udp5353 from ", srcaddr, srcport)
  dns._resolver.time = 0
  local resp = dns._resolver:decode(data, true)
  -- print(inspect(resp))
end



