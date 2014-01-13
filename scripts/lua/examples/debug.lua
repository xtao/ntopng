--
-- (C) 2013 - ntop.org
--

-- debug lua example

-- Set package.path information to be able to require lua module
dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

-- Here you can choose the type of your HTTP message {'text/html','application/json',...}. There are two main function that you can use:
-- function sendHTTPHeaderIfName(mime, ifname, maxage)
-- function sendHTTPHeader(mime)
-- For more information please read the scripts/lua/modules/lua_utils.lua file.
sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

print('<html><head><title>debug Lua example</title></head>')
print('<body>')
print('<h1>How to debug your lua scripts</h1>')
print('<br><h2>How to print debug information</h2>')
print('<p>There are two way to print some debug information:</p>')
print('<ul><li>io.write(\"I\'m debug string and you can read me as an output of the ntopng command  in the terminal.\n\")<li>print(\"I\'m debug string and you can read me as an HTML output of lua script in your browser.\n\")</ul>')
-- io.write("I'm debug string and you can read me as an output of the ntopng command  in the terminal.\n")
-- print("I'm debug string and you can read me as an HTML output of lua script in your browser.<br>")

print('<br><h2>How to read and print the _GET variables</h2>')
print('<p>Any lua scripts can be executed by passing one or more parameters. The simple way to get an input variable is "variable_name        = _GET["variable_name"]".<br><br>Try to this: <a href="?host=192.168.1.10&param=myparam&var=123456">/lua/examples/debug.lua?host=192.168.1.10&param=myparam&var=123456</a></p>')

-- Print _GET variable
print('<p>')
for key, value in pairs(_GET) do 
   print(key.."="..value.."<br>")
end
print('</p>')

print('</body></html>\n')




