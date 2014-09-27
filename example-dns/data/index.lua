local dns = require("dns")
local sqlite3 = require("lsqlite3")


function request(idx)
  local reply = "Hello from lua, index: " .. idx .. "!"
  local db = sqlite3.open_memory()

  db:exec[[
    CREATE TABLE test (id INTEGER PRIMARY KEY, content);

    INSERT INTO test VALUES (NULL, 'Hello World');
    INSERT INTO test VALUES (NULL, 'Hello Lua');
    INSERT INTO test VALUES (NULL, 'Hello Sqlite3')
  ]]

  for row in db:nrows("SELECT * FROM test") do
    reply = reply .. "\n" .. row.id .. "=" .. row.content
  end

  local headers = "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-length: " .. #reply .. "\r\nContent-type: text/plain\r\n\r\n"
  dsend(idx, headers .. reply)
end
