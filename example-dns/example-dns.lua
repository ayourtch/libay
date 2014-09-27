local dns = require("dns")
local inspect = require("inspect")
local sqlite3 = require("lsqlite3")
local dnsdb = sqlite3.open('test.db')  

dnsdb:exec("create table foo (name string(255));")

--        myDB:close()  

function udp53(srcaddr, srcport, data)
  print("Received udp53 from ", srcaddr, srcport)
  dns._resolver.time = 0
  local resp = dns._resolver:decode(data, true)
  -- print(inspect(resp))
end

function udp5353(srcaddr, srcport, data)
  local stmt = dnsdb:prepare("insert into foo values (?1);")

  print("Received udp5353 from ", srcaddr, srcport)
  dns._resolver.time = 0
  local resp = dns._resolver:decode(data, true)
  dnsdb:exec("begin transaction;")
  for i,v in ipairs(resp.answer) do
    print(i, inspect(v.name))
    stmt:bind(1, v.name)
    print(inspect(stmt:step()))
    stmt:reset()
  end
  dnsdb:exec("commit;")
  -- dnsdb:close()
end




