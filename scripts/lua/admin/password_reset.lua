--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

username = _GET["username"]
old_password = _GET["old_password"]
new_password = _GET["new_password"]
confirm_new_password = _GET["confirm_new_password"]

if(username == nil or old_password == nil or new_password == nil or confirm_new_password == nil) then
  print ("{ \"result\" : -1, \"message\" : \"Invalid parameters\" }")
  return
end

if(new_password ~= confirm_new_password) then
  print ("{ \"result\" : -1, \"message\" : \"New password does not match: typo?\" }")
  return
end

if(ntop.resetUserPassword(username, old_password, new_password)) then
  print ("{ \"result\" : 0, \"message\" : \"Password changed successfully\" }")
else
  print ("{ \"result\" : -1, \"message\" : \"Check username/password\" }")
end

