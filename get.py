import urllib2
print urllib2.urlopen("http://127.0.0.1:8000/api/isEnabled").read()