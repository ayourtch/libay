local dns = require("dns")
local sqlite3 = require("lsqlite3")


function request(idx)
  local reply = "Hello from lua, index: " .. idx .. "!"
  local db = sqlite3.open('test.db')

  for row in db:nrows("SELECT * FROM foo;") do
    reply = reply .. "\n" .. row.name
  end

  local headers = "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-length: " .. #reply .. "\r\nContent-type: text/plain\r\n\r\n"
  dsend(idx, headers .. reply)
end
