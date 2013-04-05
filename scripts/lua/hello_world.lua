-- Hello world

print('<html><head><title>ntop</title></head><body>Hello ' .. os.date("%d.%m.%Y"))

-- Print _GET variable
for key, value in pairs(_GET) do 
   print(key.."="..value.."<p>\n")
end


print('</body></html>\n')



