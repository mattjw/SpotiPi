import httplib, urllib 
headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
conn = httplib.HTTPConnection("localhost:8000")
conn.request("POST", "/api/", None, headers)
response = conn.getresponse()
print response.status, response.reason
print data = response.read()
conn.close()
