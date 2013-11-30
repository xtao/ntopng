--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "about"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
info = ntop.getInfo()

print [[
    <hr />
    <h2>About ntopng</h2>]]

print("<table class=\"table table-bordered table-striped\">\n")
print("<tr><th>Copyright</th><td>"..info["copyright"].."</td></tr>\n")
print("<tr><th>License</th><td><A HREF=http://www.gnu.org/licenses/gpl.html>"..info["license"].."</A>")

print("<tr><th>Version</th><td>"..info["version"].."</td></tr>\n")
print("<tr><th>Platform</th><td>"..info["platform"].."</td></tr>\n")
print("<tr><th><i class='fa fa-user fa-lg'></i> Currently Logged User</th><td>".._SESSION["user"].."</td></tr>\n")
print("<tr><th><i class='fa fa-time fa-lg'></i> Uptime</th><td>"..secondsToTime(info["uptime"]).."</td></tr>\n")
print("<tr><th colspan=2 align=center>&nbsp;</th></tr>\n")
print("<tr><th><a href=http://www.ntop.org/products/ndpi/>nDPI</A></th><td>".. info["version.ndpi"] .."</td></tr>\n")
print("<tr><th><a href=http://twitter.github.io/><i class=\'fa fa-twitter fa-lg'></i> Twitter Bootstrap</A></th><td>2.3.1</td></tr>\n")
print("<tr><th><a href=http://www.rrdtool.org/>RRDtool</A></th><td>"..info["version.rrd"].."</td></tr>\n")
print("<tr><th><a href=http://www.redis.io>Redis</A> Server</th><td>"..info["version.redis"].."</td></tr>\n")
print("<tr><th><a href=https://github.com/valenok/mongoose>Mongoose web server</A></th><td>"..info["version.httpd"].."</td></tr>\n")
print("<tr><th><a href=http://www.luajit.org>LuaJIT</A></th><td>"..info["version.luajit"].."</td></tr>\n")
print("<tr><th><a href=http://www.zeromq.org>ØMQ</A></th><td>"..info["version.zmq"].."</td></tr>\n")
if(info["version.geoip"] ~= nil) then
print("<tr><th><a href=http://www.maxmind.com>GeoIP</A></th><td>"..info["version.geoip"])
end

print [[ <p><small>
         <p><span class="alert alert-info">This product includes GeoLite data created by MaxMind, available from
	 <a href="http://www.maxmind.com">http://www.maxmind.com</a>. </span></small></p>
]]

print("</td></tr>\n")
print("<tr><th><a href=http://www.d3js.org>Data-Driven Documents (d3js)</A></th><td>2.9.1 / 3.0</td></tr>\n")
print("<tr><th><a href=https://github.com/lemire/EWAHBoolArray>Compressed Bitmap (EWAHBoolArray)</A></th><td>0.4.0</td></tr>\n")



print("</table>\n")


dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
