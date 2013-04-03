# Import modules for CGI handling
import cgi, cgitb
import ntop
from time import localtime, strftime


# Parse URL
cgitb.enable();

form = cgi.FieldStorage();
hello = form.getvalue('hello', default="world")


ntop.sendString("<html><head><title>ntop</title></head><body>")
hello = strftime("%Y-%m-%d %H:%M:%S", localtime())

ntop.sendString("Hello World: '"+hello+"'")
ntop.sendString("</body></html>")
