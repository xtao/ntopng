--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

username = _GET["username"]

if(username == nil) then
  print ("{ \"result\" : -1, \"message\" : \"Invalid parameters\" }")
  return
end

if(ntop.deleteUser(username)) then
  print ("{ \"result\" : 0, \"message\" : \"User deleted successfully\" }")
else
  print ("{ \"result\" : -1, \"message\" : \"Error deleting user\" }")
end

