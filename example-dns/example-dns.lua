local dns = require("dns")
local inspect = require("inspect")
local sqlite3 = require("lsqlite3")
local dnsdb = sqlite3.open('test.db')  

dnsdb:exec("create table foo (name string(255));")
dnsdb:exec("create table cache (name string(255), value string(255), class integer, type integer, ttl integer, src string(255));")

--        myDB:close()  

function udp53(srcaddr, srcport, data)
  print("Received udp53 from ", srcaddr, srcport)
  dns._resolver.time = 0
  local resp = dns._resolver:decode(data, true)
  -- print(inspect(resp))
end

function udp5353(srcaddr, srcport, data)
  local stmt = dnsdb:prepare("insert into foo values (?1);")
  local stmt2 = dnsdb:prepare("insert into cache values (?1, ?2, ?3, ?4, ?5, ?6);")

  print("Received udp5353 from ", srcaddr, srcport)
  dns._resolver.time = 0
  local resp = dns._resolver:decode(data, true)
  dnsdb:exec("begin transaction;")
  for i,v in ipairs(resp.answer) do
    print(i, inspect(v))
    stmt:bind(1, v.name)
    stmt2:bind(1, v.name)
    stmt2:bind(2, v.val)
    stmt2:bind(3, dns.class[v.class])
    stmt2:bind(4, dns.type[v.type])
    stmt2:bind(5, v.ttl)
    stmt2:bind(6, srcaddr)
    print(inspect(stmt:step()))
    print(inspect(stmt2:step()))
    stmt:reset()
    stmt2:reset()
  end
  dnsdb:exec("commit;")
  -- dnsdb:close()
end




