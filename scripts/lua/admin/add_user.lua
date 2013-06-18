--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

username = _GET["username"]
full_name = _GET["full_name"]
password = _GET["password"]
confirm_password = _GET["confirm_password"]

if(username == nil or full_name == nil or password == nil or confirm_password == nil) then
  print ("{ \"result\" : -1, \"message\" : \"Invalid parameters\" }")
  return
end

if(password ~= confirm_password) then
  print ("{ \"result\" : -1, \"message\" : \"Passwords do not match: typo?\" }")
  return
end

if(ntop.addUser(username, full_name, password)) then
  print ("{ \"result\" : 0, \"message\" : \"User added successfully\" }")
else
  print ("{ \"result\" : -1, \"message\" : \"Error adding new user\" }")
end

