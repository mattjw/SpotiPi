import httplib, urllib
params = urllib.urlencode({'enable': 'enable'})
headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
conn = httplib.HTTPConnection("localhost:8000")
conn.request("POST", "/api/", params, headers)
response = conn.getresponse()
print response.status, response.reason
data = response.read()
conn.close()
