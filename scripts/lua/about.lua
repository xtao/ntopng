--
-- (C) 2013 - ntop.org
--

ntop.dumpFile("./httpdocs/inc/header.inc")

active_page = "about"
dofile "./scripts/lua/menu.lua"

print [[
    <hr />
    <h2>About</h2>

    <br />
    <h3>ntopng v.]]
info = ntop.getInfo()
print(info["version"])
print [[</h3>
    &copy; ntop.org 1998-]]
print(os.date("%Y"))
print [[
    <!-- TODO -->
    <br />
    <br />
    <br />
    <br />
    <br />
    <br />
    <br />
    <br />
    <br />
    <br />
]]

dofile "./scripts/lua/footer.inc.lua"
