from piston.handler import BaseHandler
from mks.models import User, Playlists
import subprocess

class SpotHandler(BaseHandler):
    def read(self, request, expression):
        if expression == 'isEnabled':
            return "y"
        else:
            return "n"

    def update(self, request, expression):
        if expression == 'enable':
            i = request.PUT.get('ID')
            user = User.objects.filter(id=1)[0]
            user.enabled = True
            user.save()
            return user.enabled
        elif expression == 'next':
            return_code = subprocess.Popen("echo hello string", shell=True)
            return 'next'
        elif expression == 'disable':
            i = request.PUT.get('ID')
            user = User.objects.filter(id=1)[0]
            user.enabled = False
            user.save()
            return user.enabled
        else:
            return 'unrecognised call, ignored'